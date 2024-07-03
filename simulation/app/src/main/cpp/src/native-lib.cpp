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

    int currentFrame ;
    int numFrames;
    float aspectRatio;

};
appState *globalAppState = new appState();

// System-wide variables from consts.h
float global_time_in_step = 0.0f;
Mode mode;


inline void loadStep(int frame) {
    globalAppState->vectorFieldHandler->loadTimeStep(*(globalAppState->reader), (globalAppState->fileDescriptors)[frame], (globalAppState->fileDescriptors)[globalAppState->numFrames + frame], (globalAppState->fileDescriptors)[2 * globalAppState->numFrames + frame]);
}

void loadInitStep() {
    if (globalAppState->numFrames == 0) {
        LOGE("native-lib", "No frames loaded");
        return;
    } else if (globalAppState->numFrames == 1) {
        loadStep(0);
    } else if (globalAppState->numFrames == 2) {
        loadStep(0);
        loadStep(1);
        globalAppState->currentFrame = 1;
    } else {
        loadStep(0);
        loadStep(1);
        loadStep(2);
        globalAppState->currentFrame = 2;
    }
}


void check_update() {
    global_time_in_step += (globalAppState->physics)->dt;
    if (global_time_in_step >= TIME_STEP) {
        global_time_in_step = 0.0f;

        (globalAppState->eglContextManager)->syncEGLContext(globalAppState->readerThreadPool);

        (globalAppState->vectorFieldHandler)->updateTimeStep();
        (globalAppState->mainview)->loadComputeBuffer();
        globalAppState->currentFrame = (globalAppState->currentFrame + 1) % globalAppState->numFrames;

        LOGI("native-lib", "Loading step %d", globalAppState->currentFrame);
        (globalAppState->readerThreadPool)->enqueue([appState = globalAppState]() {
            loadStep(appState->currentFrame);
            (appState->mainview)->preloadComputeBuffer((appState->vectorFieldHandler)->getFutureVertices(), (appState->eglContextManager)->globalFence);
        });
    }

    (globalAppState->timer)->measure();
}


void init(std::string packageName) {
    mode = Mode::computeShaders;

    globalAppState->currentFrame = 0;

    globalAppState->touchHandler = new TouchHandler((globalAppState->mainview)->getTransforms());
    globalAppState->reader = new NetCDFReader(packageName);

    //////////////////////// Double gyre ////////////////////////
    globalAppState->vectorFieldHandler = new VectorFieldHandler(7, 7, 1);
    globalAppState->physics = new Physics(*(globalAppState->vectorFieldHandler), Physics::Model::particles_advection, 0.1f);
    globalAppState->particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line , *(globalAppState->physics), NUM_PARTICLES);  // Keep this commented if using loading from file
    /////////////////////////////////////////////////////////////

    //////////////////////// Perlin noise ////////////////////////
//    globalAppState->vectorFieldHandler = new VectorFieldHandler();
//    globalAppState->physics = new Physics(*globalAppState->vectorFieldHandler, Physics::Model::particles_advection, 0.02f);
//    globalAppState->particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::uniform ,*(globalAppState->physics), NUM_PARTICLES);  // Keep this commented if using loading from file
    /////////////////////////////////////////////////////////////

    // Initialization from file
//    globalAppState->particlesHandler = new ParticlesHandler(*globalAppState->physics, NUM_PARTICLES);

    globalAppState->timer = new Timer<std::chrono::steady_clock>();

    globalAppState->readerThreadPool = new ThreadPool(1);
    globalAppState->eglContextManager = new EGLContextManager();

    LOGI("native-lib", "init complete");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
//        appState *state = static_cast<appState *>(userData);
        check_update();
//        (globalAppState->particlesHandler)->simulateParticles(*(globalAppState->mainview));
        (globalAppState->mainview)->setFrame();

        (globalAppState->vectorFieldHandler)->draw(*(globalAppState->mainview));
//        (globalAppState->particlesHandler)->draw(*(globalAppState->mainview));
        (globalAppState->mainview)->drawUI();
    }

    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager, jstring path) {  // TODO: Rename
        globalAppState->mainview = new Mainview(AAssetManager_fromJava(env, assetManager));
        (globalAppState->mainview)->setupGraphics();

        std::string folderPath = env->GetStringUTFChars(path, nullptr);
        std::regex regexPattern("/data/user/0/([^/]+)/files");
        std::smatch match;
        std::regex_search(folderPath, match, regexPattern);
        std::string packageName = match[1].str();

        init(packageName);
        (globalAppState->mainview)->getTransforms().setAspectRatio(globalAppState->aspectRatio);
        (globalAppState->eglContextManager)->initContext();
        LOGI("native-lib", "Graphics setup complete");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
                JNIEnv* env, jobject /* this */, jintArray jfds) {
        LOGI("native-lib", "Loading file descriptors");
        jsize len = env->GetArrayLength(jfds);
        globalAppState->numFrames = len / 3;
        LOGI("native-lib", "Number of frames: %d", globalAppState->numFrames);

        jint* fds = env->GetIntArrayElements(jfds, nullptr);
        globalAppState->fileDescriptors.clear();
        globalAppState->fileDescriptors.reserve(len);
        for (int i = 0; i < len; i++) {
            globalAppState->fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
        LOGI("native-lib", "File descriptors loaded");
        loadInitStep();
        LOGI("native-lib", "Initial step loaded");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {  // TODO: Is there a reason for this to be exported ?
        (globalAppState->mainview)->createVectorFieldBuffer((globalAppState->vectorFieldHandler)->getOldVertices());
//        (globalAppState->mainview)->createParticlesBuffer((globalAppState->particlesHandler)->getParticlesPositions());
        (globalAppState->mainview)->createComputeBuffer((globalAppState->vectorFieldHandler)->getOldVertices(), (globalAppState->vectorFieldHandler)->getNewVertices(), (globalAppState->vectorFieldHandler)->getFutureVertices());
        (globalAppState->mainview)->loadConstUniforms((globalAppState->physics)->dt, (globalAppState->vectorFieldHandler)->getWidth(), (globalAppState->vectorFieldHandler)->getHeight(), (globalAppState->vectorFieldHandler)->getDepth());
        LOGI("native-lib", "Buffers created");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_nativeSendTouchEvent(JNIEnv *env, jobject obj, jint pointerCount, jfloatArray xArray, jfloatArray yArray, jint action) {
        jfloat* xTemp = env->GetFloatArrayElements(xArray, nullptr);
        jfloat* yTemp = env->GetFloatArrayElements(yArray, nullptr);

        if (pointerCount == 1) {
            (globalAppState->touchHandler)->handleSingleTouch(xTemp[0], yTemp[0], action);
        } else if (pointerCount == 2) {
            float x[2] = {xTemp[0], xTemp[1]};
            float y[2] = {yTemp[0], yTemp[1]};
            (globalAppState->touchHandler)->handleDoubleTouch(x, y, action);
        }

        env->ReleaseFloatArrayElements(xArray, xTemp, 0);
        env->ReleaseFloatArrayElements(yArray, yTemp, 0);
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_onDestroyNative(JNIEnv *env, jobject thiz) {
        delete globalAppState->mainview;
        delete globalAppState->particlesHandler;
        delete globalAppState->vectorFieldHandler;
        delete globalAppState->physics;
        delete globalAppState->touchHandler;
        delete globalAppState->timer;
        delete globalAppState->readerThreadPool;
        delete globalAppState->eglContextManager;
        delete globalAppState->reader;

        delete globalAppState;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_loadDeviceInfo(JNIEnv *env, jobject thiz, jdouble jaspectRatio) {
        globalAppState->aspectRatio = (float) jaspectRatio;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadInitialPositions(JNIEnv *env, jobject thiz, jint fd) {
        if ((globalAppState->particlesHandler)->areParticlesInitialized()) {
            return;
        }
        LOGI("native-lib", "Loading initial positions");
        std::string tempFile = (globalAppState->reader)->writeTempFileFromFD(fd, "temp.nc");

        if (tempFile.empty()) {
            LOGE("native-lib", "Failed to create temporary file.");
            return;
        }

        (globalAppState->particlesHandler)->loadPositionsFromFile(tempFile);
//        (globalAppState->mainview)->loadParticlesData((globalAppState->particlesHandler)->getParticlesPositions());
        LOGI("native-lib", "Particles initialized");
    }
} // extern "C"
