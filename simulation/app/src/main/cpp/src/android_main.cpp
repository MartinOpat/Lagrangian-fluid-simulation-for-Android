//
// Created by martin on 15-06-2024.
//

#include <android_native_app_glue.h>
#include <jni.h>

#include "include/android_logging.h"
#include "include/EGLContextManager.h"
#include "include/mainview.h"
#include "include/particles_handler.h"
#include "include/vector_field_handler.h"
#include "include/touch_handler.h"
#include "include/physics.h"
#include "include/ThreadPool.h"
#include "include/timer.h"


// From consts.h  TODO: Could be below?
float global_time_in_step = 0.0f;
Mode mode;

// Global application state.
struct app_state {
    EGLContextManager *eglContextManager;
    Mainview *mainview;
    ParticlesHandler *particlesHandler;
    VectorFieldHandler *vectorFieldHandler;
    TouchHandler *touchHandler;
    Physics *physics;
    ThreadPool *threadPool;
    Timer<std::chrono::steady_clock>* timer;

    float aspectRatio = 1.0f;

    int numFrames = 0;
    int currentFrame = 0;
    std::vector<int> fileDescriptors;

    std::atomic<bool> initialized{false};
    std::atomic<bool> dataLoaded{false};

} typedef app_state;
app_state global_app_state;

void initialize(struct android_app* app) {
    AAssetManager* mgr = app->activity->assetManager;

    mode = Mode::computeShaders;

    global_app_state.eglContextManager = new EGLContextManager();
    global_app_state.mainview = new Mainview(mgr);
    global_app_state.vectorFieldHandler = new VectorFieldHandler(15, 5);
    global_app_state.physics = new Physics(*global_app_state.vectorFieldHandler, Physics::Model::particles_advection);
    global_app_state.particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::uniform, *global_app_state.physics, NUM_PARTICLES);
    global_app_state.touchHandler = new TouchHandler(global_app_state.mainview->getTransforms());
    global_app_state.threadPool = new ThreadPool(1);
    global_app_state.timer = new Timer<std::chrono::steady_clock>();
}

void create_buffers() {
    (global_app_state.mainview)->createVectorFieldBuffer((global_app_state.vectorFieldHandler)->getOldVertices());
    (global_app_state.mainview)->createParticlesBuffer((global_app_state.particlesHandler)->getParticlesPositions());
    (global_app_state.mainview)->createComputeBuffer((global_app_state.vectorFieldHandler)->getOldVertices(), (global_app_state.vectorFieldHandler)->getNewVertices(), (global_app_state.vectorFieldHandler)->getFutureVertices());
    (global_app_state.mainview)->loadConstUniforms((global_app_state.physics)->dt, (global_app_state.vectorFieldHandler)->getWidth(), (global_app_state.vectorFieldHandler)->getHeight(), (global_app_state.vectorFieldHandler)->getDepth());
    LOGI("native-lib", "Buffers created");
}

void clean_up(app_state &state, struct android_app* app) {
    delete state.eglContextManager;
    delete state.mainview;
    delete state.particlesHandler;
    delete state.vectorFieldHandler;
    delete state.physics;
    delete state.touchHandler;
    delete state.threadPool;
}

void handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != nullptr && !global_app_state.initialized && global_app_state.dataLoaded) {
                // Initialize EGL, shaders, and other graphics resources here.
                LOGI("handle_cmd", "Initializing graphics");
                global_app_state.eglContextManager->initContext(app->window);
                global_app_state.mainview->setupGraphics();
                global_app_state.mainview->getTransforms().setAspectRatio(global_app_state.aspectRatio);
                create_buffers();
                global_app_state.initialized = true;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            if (app->window != NULL) {
                global_app_state.eglContextManager->destroyContext();
            }
            global_app_state.initialized = false;
            break;
        case APP_CMD_LOST_FOCUS:
            // Also stop animations or other heavy processing when the app is not visible.
            break;
        case APP_CMD_GAINED_FOCUS:
            // Restart any paused processes here.
            break;
    }
}

inline void loadStep(int frame) {
    (global_app_state.vectorFieldHandler)->loadTimeStep((global_app_state.fileDescriptors)[frame], (global_app_state.fileDescriptors)[global_app_state.numFrames + frame], (global_app_state.fileDescriptors)[2*global_app_state.numFrames + frame]);
}

void check_update() {
    global_time_in_step += (global_app_state.physics)->dt;

    if (global_time_in_step >= TIME_STEP) {
        global_time_in_step = 0.0f;

//        (global_app_state.eglContextManager)->syncEGLContext((global_app_state.threadPool));

        (global_app_state.vectorFieldHandler)->updateTimeStep();
        (global_app_state.mainview)->loadComputeBuffer();
        global_app_state.currentFrame = (global_app_state.currentFrame + 1) % global_app_state.numFrames;

//        LOGI("native-lib", "Loading step %d", global_app_state.currentFrame);
//        (global_app_state.threadPool)->enqueue([]() {
//            loadStep(global_app_state.currentFrame);
//            (global_app_state.mainview)->preloadComputeBuffer((global_app_state.vectorFieldHandler)->getFutureVertices(), (global_app_state.eglContextManager)->globalFence);
//        });
            loadStep(global_app_state.currentFrame);
            (global_app_state.mainview)->preloadComputeBuffer((global_app_state.vectorFieldHandler)->getFutureVertices(), (global_app_state.eglContextManager)->globalFence);
    }

    (global_app_state.timer)->measure();
}


void update_and_render() {
    // Ensure all prerequisites for drawing are met
    if (!global_app_state.initialized || global_app_state.mainview == nullptr) return;

    check_update();

    // Update physics and application logic
    global_app_state.particlesHandler->simulateParticles(*(global_app_state.mainview));

    // Prepare frame and draw the scene
    global_app_state.mainview->setFrame();
    global_app_state.vectorFieldHandler->draw(*(global_app_state.mainview));
    global_app_state.particlesHandler->draw(*(global_app_state.mainview));
    global_app_state.mainview->drawUI();

    // Egl swap if using EGLContextManager to manage the display
    global_app_state.eglContextManager->swapBuffers();
}

void android_main(struct android_app* app) {
    app_dummy();  // Ensure native app glue is not stripped (there is a better work around but good enough for now)

    initialize(app);

    LOGI("android_main", "Starting application");

    app->userData = &global_app_state;
    app->onAppCmd = handle_cmd;

    static bool isSetup = false;

    while (1) {
        int events;
        android_poll_source* source;

        // Poll all events.
        while (ALooper_pollAll(global_app_state.initialized ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
            if (app->destroyRequested != 0) {
                LOGI("android_main", "Destroy requested");
                clean_up(global_app_state, app);
                return;
            }
        }

        // Update and render if initialized
        if (global_app_state.initialized) {
            update_and_render();
        }
    }
}



///////////////////////////////////////////////////////////////////////// Java-Native interface

void loadInitStep() {
    if (global_app_state.numFrames == 0) {
        LOGE("native-lib", "No frames loaded");
        return;
    } else if (global_app_state.numFrames == 1) {
        loadStep(0);
    } else if (global_app_state.numFrames == 2) {
        loadStep(0);
        loadStep(1);
        global_app_state.currentFrame = 1;
    } else {
        loadStep(0);
        loadStep(1);
        loadStep(2);
        global_app_state.currentFrame = 2;
    }
}

extern "C" {
JNIEXPORT void JNICALL
Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadInitialPositions(JNIEnv *env, jobject thiz, jint fd) {
    if ((global_app_state.particlesHandler)->areParticlesInitialized()) {
        return;
    }
    LOGI("native-lib", "Loading initial positions");
    NetCDFReader reader;
    std::string tempFile = reader.writeTempFileFromFD(fd, "temp.nc");

    if (tempFile.empty()) {
        LOGE("native-lib", "Failed to create temporary file.");
        return;
    }

    (global_app_state.particlesHandler)->loadPositionsFromFile(tempFile);
    (global_app_state.mainview)->loadParticlesData((global_app_state.particlesHandler)->getParticlesPositions());
    LOGI("native-lib", "Particles initialized");
}

JNIEXPORT void JNICALL
Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
        JNIEnv* env, jobject /* this */, jintArray jfds) {
    LOGI("native-lib", "Loading file descriptors");
    jsize len = env->GetArrayLength(jfds);
    global_app_state.numFrames = len / 3;
    LOGI("native-lib", "Number of frames: %d", global_app_state.numFrames);

    jint* fds = env->GetIntArrayElements(jfds, nullptr);
    global_app_state.fileDescriptors.clear();
    global_app_state.fileDescriptors.reserve(len);
    for (int i = 0; i < len; i++) {
        global_app_state.fileDescriptors.push_back(fds[i]);
    }

    env->ReleaseIntArrayElements(jfds, fds, 0);
    LOGI("native-lib", "File descriptors loaded");
    loadInitStep();
    global_app_state.dataLoaded = true;
    LOGI("native-lib", "Initial step loaded");
}

JNIEXPORT void JNICALL
Java_com_rug_lagrangianfluidsimulation_MyNativeActivity_loadDeviceInfo(JNIEnv *env, jobject thiz, jdouble jaspectRatio) {
    global_app_state.aspectRatio = (float) jaspectRatio;
}

}

