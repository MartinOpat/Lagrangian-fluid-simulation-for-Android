#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/log.h>
#define LOG_TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


extern "C" {

// Vertex shader source
const char* vertexShaderSource = "attribute vec4 vPosition; void main() { gl_Position = vPosition; }";

// Fragment shader source
const char* fragmentShaderSource =
        "precision mediump float;"
        "void main() {"
        "  gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);"
        "}";

GLuint shaderProgram;
GLuint vertexShader, fragmentShader;

void compileAndLinkShaders() {
    LOGE("Compiling and linking shaders");

    // Compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    LOGE("Here");
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    LOGE("Vertex shader compiled");

    // Compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    LOGE("Fragment shader compiled");

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    LOGE("Shaders linked");

    // Delete shaders
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOGE("Successfully compiled and linked shaders");
}


JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject /* this */) {
    compileAndLinkShaders();

    static const GLfloat vertices[] = {
            0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
    };

    GLuint VBO;  // TODO: Make this global so it can get properly cleaned up ?
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}


} // extern "C"
