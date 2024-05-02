package com.example.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.opengl.GLES20;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;
//
//
//public class MainActivity extends Activity {
//    static {
//        System.loadLibrary("lagrangianfluidsimulation");
//        System.loadLibrary("netcdf");
//        System.loadLibrary("netcdf_c++4");
//    }
//
//
//    // Attributes
//    private GLSurfaceView glSurfaceView;
//    private final FileAccessHelper fileAccessHelper = new FileAccessHelper(this);
//
//    private native void drawFrame();
//    private native void setupGraphics(AssetManager assetManager);
//    public native void createBuffers();
//
//    private static final int REQUEST_CODE_READ_STORAGE = 100;
//    private static final int REQUEST_CODE_PICK_FILES = 101;
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setupGLSurfaceView();
//        setContentView(glSurfaceView);
//        fileAccessHelper.checkAndRequestPermissions();
//    }
//
//    private void setupGLSurfaceView() {
//        glSurfaceView = new GLSurfaceView(this);
//        glSurfaceView.setEGLContextClientVersion(2);
//        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
//            @Override
//            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
//                AssetManager assetManager = getAssets();
//                setupGraphics(assetManager);
//            }
//
//            @Override
//            public void onSurfaceChanged(GL10 gl, int width, int height) {
//                gl.glViewport(0, 0, width, height);
//            }
//
//            @Override
//            public void onDrawFrame(GL10 gl) {
//                if (!fileAccessHelper.isDataReady()) {
//                    gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
//                    return;
//                }
//                drawFrame();
//            }
//        });
//    }
//
//    @Override
//    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
//        super.onActivityResult(requestCode, resultCode, data);
//        if (requestCode == REQUEST_CODE_PICK_FILES && resultCode == RESULT_OK) {
//            Uri uriU = data.getClipData().getItemAt(0).getUri();
//            Uri uriV = data.getClipData().getItemAt(1).getUri();
//            try {
//                Uri uriW = data.getClipData().getItemAt(2).getUri();
//                Log.i("MainActivity", "3D mode");
//                fileAccessHelper.loadNetCDFData(uriU, uriV, uriW);
//            } catch (IndexOutOfBoundsException e) {
//                Log.i("MainActivity", "2D mode");
//                fileAccessHelper.loadNetCDFData(uriU, uriV);
//            }
//        }
//    }
//
//
//    public void onDataLoaded() {
//        runOnUiThread(() -> fileAccessHelper.setDataReady(true));
//        glSurfaceView.queueEvent(this::createBuffers);
//    }
//
//
//    @Override
//    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
//        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
//        Log.d("Permissions", "Request code: " + requestCode);
//        if (requestCode == REQUEST_CODE_READ_STORAGE) {
//                fileAccessHelper.openFilePicker();
//        }
//    }
//
//
//    @Override
//    protected void onResume() {
//        super.onResume();
//        if (glSurfaceView != null) {
//            glSurfaceView.onResume();
//        }
//    }
//
//    @Override
//    protected void onPause() {
//        super.onPause();
//        if (glSurfaceView != null) {
//            glSurfaceView.onPause();
//        }
//    }
//
//    @Override
//    protected void onDestroy() {
//        super.onDestroy();
//        fileAccessHelper.shutdown();
//    }
//}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//public class MainActivity extends AppCompatActivity {
//    static {
//        System.loadLibrary("lagrangianfluidsimulation");
//    }
//
//    public native void displayCube();
//    private GLSurfaceView glSurfaceView;
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
//
//        glSurfaceView = (GLSurfaceView) findViewById(R.id.glSurface);
//        glSurfaceView.setEGLContextClientVersion(2); // Use OpenGL ES 2.0
//
//        // Configure the EGL settings
//        glSurfaceView.setEGLConfigChooser(new GLSurfaceView.EGLConfigChooser() {
//            @Override
//            public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
//                int[] attribs = {
//                        EGL10.EGL_RENDERABLE_TYPE, 4,  // This is important for OpenGL ES 2.0
//                        EGL10.EGL_RED_SIZE, 8,
//                        EGL10.EGL_GREEN_SIZE, 8,
//                        EGL10.EGL_BLUE_SIZE, 8,
//                        EGL10.EGL_ALPHA_SIZE, 8,
//                        EGL10.EGL_DEPTH_SIZE, 16,  // Specify the depth size for 3D rendering
//                        EGL10.EGL_STENCIL_SIZE, 8,
//                        EGL10.EGL_NONE
//                };
//                EGLConfig[] configs = new EGLConfig[1];
//                int[] numConfig = new int[1];
//                egl.eglChooseConfig(display, attribs, configs, 1, numConfig);
//                return configs[0];
//            }
//        });
//
//        glSurfaceView.setRenderer(new MyGLRenderer()); // Your renderer that calls native methods
//        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
//    }
//
//
//    class MyGLRenderer implements GLSurfaceView.Renderer {
//        public void onSurfaceCreated(GL10 unused, EGLConfig config) {
//            // Set a clear color to see if the onDrawFrame is called
//            GLES20.glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
//        }
//
//        public void onDrawFrame(GL10 unused) {
//            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
//            // Call your native function to render the cube
//            displayCube();
//        }
//
//        public void onSurfaceChanged(GL10 unused, int width, int height) {
//            // Handle changes here
//        }
//    }
//
//    @Override
//    protected void onPause() {
//        super.onPause();
//        if (glSurfaceView != null) {
//            glSurfaceView.onPause();
//        }
//    }
//
//    @Override
//    protected void onResume() {
//        super.onResume();
//        if (glSurfaceView != null) {
//            glSurfaceView.onResume();
//        }
//    }
//
//}


import android.app.Activity;
import android.os.Bundle;
import android.opengl.GLSurfaceView;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("lagrangianfluidsimulation");
    }

    private GLSurfaceView glSurfaceView;
    private native void onSurfaceCreatedNative();
    private native void onDrawFrameNative();
    private native void onSurfaceChangedNative(int width, int height);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.glSurface);
        glSurfaceView.setEGLContextClientVersion(3); // Use OpenGL ES 3.0 if available
//        glSurfaceView.setRenderer(new NativeRenderer()); // Set the renderer

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                onSurfaceCreatedNative();
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                onDrawFrameNative();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                onSurfaceChangedNative(width, height);
            }
        });
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    @Override
    protected void onPause() {
        super.onPause();
        glSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glSurfaceView.onResume();
    }
}
