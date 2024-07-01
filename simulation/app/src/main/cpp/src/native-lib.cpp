#include <jni.h>
#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <chrono>
#include <netcdf>
#include <assert.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <thread>
#include <atomic>
#include <functional>
#include <regex>

#include "include/android_logging.h"
#include "include/netcdf_reader.h"
#include "include/mainview.h"
#include "include/particle.h"
#include "include/particles_handler.h"
#include "include/vector_field_handler.h"
#include "include/touch_handler.h"
#include "include/timer.h"
#include "include/ThreadPool.h"
#include "include/EGLContextManager.h"

struct appState {
    std::vector<int> fileDescriptors;

    Mainview* mainview;
    ParticlesHandler* particlesHandler;
    VectorFieldHandler* vectorFieldHandler;
    TouchHandler* touchHandler;
    Physics* physics;
    Timer<std::chrono::steady_clock>* timer;
    ThreadPool *readerThreadPool;
    EGLContextManager *eglContextManager;
    NetCDFReader *reader;

    float global_time_in_step;
    Mode mode;
    int currentFrame ;
    int numFrames;
    float aspectRatio;

};
void *userData = new appState();


inline void loadStep(int frame) {
    appState *state = static_cast<appState *>(userData);
    state->vectorFieldHandler->loadTimeStep(*(state->reader), (state->fileDescriptors)[frame], (state->fileDescriptors)[state->numFrames + frame], (state->fileDescriptors)[2*state->numFrames + frame]);
}

void loadInitStep() {
    appState *state = static_cast<appState *>(userData);
    if (state->numFrames == 0) {
        LOGE("native-lib", "No frames loaded");
        return;
    } else if (state->numFrames == 1) {
        loadStep(0);
    } else if (state->numFrames == 2) {
        loadStep(0);
        loadStep(1);
        state->currentFrame = 1;
    } else {
        loadStep(0);
        loadStep(1);
        loadStep(2);
        state->currentFrame = 2;
    }
}


void check_update() {
    appState *state = static_cast<appState *>(userData);

    state->global_time_in_step += (state->physics)->dt;
    if (state->global_time_in_step >= TIME_STEP) {
        state->global_time_in_step = 0.0f;

        (state->eglContextManager)->syncEGLContext(state->readerThreadPool);

        (state->vectorFieldHandler)->updateTimeStep();
        (state->mainview)->loadComputeBuffer();
        state->currentFrame = (state->currentFrame + 1) % state->numFrames;

        LOGI("native-lib", "Loading step %d", state->currentFrame);
        (state->readerThreadPool)->enqueue([appState = state]() {
            loadStep(appState->currentFrame);
            (appState->mainview)->preloadComputeBuffer((appState->vectorFieldHandler)->getFutureVertices(), (appState->eglContextManager)->globalFence);
        });
    }

    (state->timer)->measure();
}


void init(std::string packageName) {
    appState *state = static_cast<appState *>(userData);

    state->global_time_in_step = 0.0f;
    state->currentFrame = 0;
    state->numFrames = 0;
    state->aspectRatio = 1.0f;
    state->mode = Mode::computeShaders;

    state->touchHandler = new TouchHandler((state->mainview)->getTransforms());
    state->reader = new NetCDFReader(packageName);

    //////////////////////// Double gyre ////////////////////////
    state->vectorFieldHandler = new VectorFieldHandler(15, 5);
    state->physics = new Physics(*(state->vectorFieldHandler), Physics::Model::particles_advection, 0.1f);
    state->particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::two_lines ,*(state->physics), NUM_PARTICLES);
    /////////////////////////////////////////////////////////////

    //////////////////////// Perlin noise ////////////////////////
//    vectorFieldHandler = new VectorFieldHandler(1, 1);
//    physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection, 0.02f);
//    particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::uniform ,*physics, NUM_PARTICLES);
    /////////////////////////////////////////////////////////////

    // Initialization from file
//    particlesHandler = new ParticlesHandler(*physics, NUM_PARTICLES);

    state->timer = new Timer<std::chrono::steady_clock>();

    state->readerThreadPool = new ThreadPool(1);
    state->eglContextManager = new EGLContextManager();

    LOGI("native-lib", "init complete");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        appState *state = static_cast<appState *>(userData);

        check_update();
        (state->particlesHandler)->simulateParticles(*(state->mainview));
        (state->mainview)->setFrame();

        (state->vectorFieldHandler)->draw(*(state->mainview));
        (state->particlesHandler)->draw(*(state->mainview));
        (state->mainview)->drawUI();
    }

    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager, jstring path) {  // TODO: Rename
        appState *state = static_cast<appState *>(userData);

        state->mainview = new Mainview(AAssetManager_fromJava(env, assetManager));
        (state->mainview)->setupGraphics();

        std::string folderPath = env->GetStringUTFChars(path, nullptr);
        std::regex regexPattern("/data/user/0/([^/]+)/files");
        std::smatch match;
        std::regex_search(folderPath, match, regexPattern);
        std::string packageName = match[1].str();

        init(packageName);
        (state->mainview)->getTransforms().setAspectRatio(state->aspectRatio);
        (state->eglContextManager)->initContext();
        LOGI("native-lib", "Graphics setup complete");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
                JNIEnv* env, jobject /* this */, jintArray jfds) {
        appState *state = static_cast<appState *>(userData);

        LOGI("native-lib", "Loading file descriptors");
        jsize len = env->GetArrayLength(jfds);
        state->numFrames = len / 3;
        LOGI("native-lib", "Number of frames: %d", state->numFrames);

        jint* fds = env->GetIntArrayElements(jfds, nullptr);
        state->fileDescriptors.clear();
        state->fileDescriptors.reserve(len);
        for (int i = 0; i < len; i++) {
            state->fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
        LOGI("native-lib", "File descriptors loaded");
        loadInitStep();
        LOGI("native-lib", "Initial step loaded");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {  // TODO: Is there a reason for this to be exported ?
        appState *state = static_cast<appState *>(userData);

        (state->mainview)->createVectorFieldBuffer((state->vectorFieldHandler)->getOldVertices());
        (state->mainview)->createParticlesBuffer((state->particlesHandler)->getParticlesPositions());
        (state->mainview)->createComputeBuffer((state->vectorFieldHandler)->getOldVertices(), (state->vectorFieldHandler)->getNewVertices(), (state->vectorFieldHandler)->getFutureVertices());
        (state->mainview)->loadConstUniforms((state->physics)->dt, (state->vectorFieldHandler)->getWidth(), (state->vectorFieldHandler)->getHeight(), (state->vectorFieldHandler)->getDepth());
        LOGI("native-lib", "Buffers created");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_nativeSendTouchEvent(JNIEnv *env, jobject obj, jint pointerCount, jfloatArray xArray, jfloatArray yArray, jint action) {
        appState *state = static_cast<appState *>(userData);

        jfloat* xTemp = env->GetFloatArrayElements(xArray, nullptr);
        jfloat* yTemp = env->GetFloatArrayElements(yArray, nullptr);

        if (pointerCount == 1) {
            (state->touchHandler)->handleSingleTouch(xTemp[0], yTemp[0], action);
        } else if (pointerCount == 2) {
            float x[2] = {xTemp[0], xTemp[1]};
            float y[2] = {yTemp[0], yTemp[1]};
            (state->touchHandler)->handleDoubleTouch(x, y, action);
        }

        env->ReleaseFloatArrayElements(xArray, xTemp, 0);
        env->ReleaseFloatArrayElements(yArray, yTemp, 0);
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_onDestroyNative(JNIEnv *env, jobject thiz) {
        appState *state = static_cast<appState *>(userData);

        delete state->mainview;
        delete state->particlesHandler;
        delete state->vectorFieldHandler;
        delete state->physics;
        delete state->touchHandler;
        delete state->timer;
        delete state->readerThreadPool;
        delete state->eglContextManager;
        delete state->reader;

        delete state;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_loadDeviceInfo(JNIEnv *env, jobject thiz, jdouble jaspectRatio) {
        appState *state = static_cast<appState *>(userData);

        state->aspectRatio = (float) jaspectRatio;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadInitialPositions(JNIEnv *env, jobject thiz, jint fd) {
        appState *state = static_cast<appState *>(userData);

        if ((state->particlesHandler)->areParticlesInitialized()) {
            return;
        }
        LOGI("native-lib", "Loading initial positions");
        std::string tempFile = (state->reader)->writeTempFileFromFD(fd, "temp.nc");

        if (tempFile.empty()) {
            LOGE("native-lib", "Failed to create temporary file.");
            return;
        }

        (state->particlesHandler)->loadPositionsFromFile(tempFile);
        (state->mainview)->loadParticlesData((state->particlesHandler)->getParticlesPositions());
        LOGI("native-lib", "Particles initialized");
    }
} // extern "C"
