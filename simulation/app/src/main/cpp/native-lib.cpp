#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <chrono>

#include "png_loader.h"

#define LOG_TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


std::string loadShaderFile(AAssetManager* assetManager, const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) return "";

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, ' ');
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    return buffer;
}

std::string vertexShaderSource;
std::string fragmentShaderSource;

GLuint shaderProgram;
GLuint vertexShader, fragmentShader;
GLuint textureID;

GLuint triangleVBO;
GLuint particleVBO;

auto startTime = std::chrono::steady_clock::now();

void createTexture(ImageData& texData) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void compileAndLinkShaders() {
    LOGI("Compiling and linking shaders");

    // Compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(vertexShader);

    LOGI("Vertex shader compiled");

    // Compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);

    LOGI("Fragment shader compiled");

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    LOGI("Shaders linked");

    // Delete shaders
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOGI("Successfully compiled and linked shaders");
}

struct Vec3 {
    float x, y, z;
};

Vec3 particlePosition = {0.0f, -0.25f, 0.0f}; // Initial position
Vec3 velocity = {0.1f, 0.0f, 0.0f}; // Speed and direction

void updateParticlePosition(float deltaTime) {  // TODO: This should probably be done in the GPU (not CPU) cause SIMD, look into compute shaders
    // Update position based on velocity
    particlePosition.x += velocity.x * deltaTime;
    particlePosition.y += velocity.y * deltaTime;

    // Wrap the position around the screen
    if (particlePosition.x > 1.0f) particlePosition.x = -1.0f;
    else if (particlePosition.x < -1.0f) particlePosition.x = 1.0f;

    if (particlePosition.y > 1.0f) particlePosition.y = -1.0f;
    else if (particlePosition.y < -1.0f) particlePosition.y = 1.0f;
}

void setParticlePosition() {
    // Update deltaTime based on your application's timing logic
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - startTime).count();
    startTime = currentTime;

    updateParticlePosition(deltaTime);

    // Set uniform for updated position
    GLint posLocation = glGetUniformLocation(shaderProgram, "uPosition");
    glUniform3f(posLocation, particlePosition.x, particlePosition.y, particlePosition.z);
}



extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND); // Enable blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(shaderProgram);

        GLint isPointLocation = glGetUniformLocation(shaderProgram, "uIsPoint");
        glUniform1i(isPointLocation, 0);

        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUniform1i(isPointLocation, 1);
//        GLint timeLocation = glGetUniformLocation(shaderProgram, "uTime");
//        glUniform1f(timeLocation, elapsedTime);
//        GLint velocityLocation = glGetUniformLocation(shaderProgram, "uVelocity");
//        glUniform3f(velocityLocation, 0.1f, 0.0f, 0.0f);
        setParticlePosition();

        glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture for the particle
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

        vertexShaderSource = loadShaderFile(mgr, "vertex_shader.glsl");
        fragmentShaderSource = loadShaderFile(mgr, "fragment_shader.glsl");

        ImageData texData = loadSimpleTGA(mgr, "textures/pt_tex_debug.tga");
        LOGI("Loaded texture with width %d and height %d", texData.width, texData.height);
        LOGI("The pixel color is %d %d %d %d", texData.data[4*5050], texData.data[4*5050+1], texData.data[4*5050+2], texData.data[4*5050+3]);

        compileAndLinkShaders();

        createTexture(texData);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Triangle vertices
        static const GLfloat vertices[] = {
                0.0f,  0.0f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f
        };
        glGenBuffers(1, &triangleVBO);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


        static const GLfloat particleVertex[] = {0.0f, -0.25f, 0.0f};
        glGenBuffers(1, &particleVBO);
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertex), particleVertex, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
    }
} // extern "C"
