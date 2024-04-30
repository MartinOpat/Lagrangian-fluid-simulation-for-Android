package com.example.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;


public class MainActivity extends Activity {
    static {
        System.loadLibrary("lagrangianfluidsimulation");
        System.loadLibrary("netcdf");
        System.loadLibrary("netcdf_c++4");
        System.loadLibrary("hdf5");
        System.loadLibrary("hdf5_cpp");
    }


    // Attributes
    private GLSurfaceView glSurfaceView;
    private final FileAccessHelper fileAccessHelper = new FileAccessHelper(this);

    private native void drawFrame();
    private native void setupGraphics(AssetManager assetManager);
    public native void createBuffers();

    private static final int REQUEST_CODE_READ_STORAGE = 100;
    private static final int REQUEST_CODE_PICK_FILES = 101;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupGLSurfaceView();
        setContentView(glSurfaceView);
        fileAccessHelper.checkAndRequestPermissions();
    }

    private void setupGLSurfaceView() {
        glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                AssetManager assetManager = getAssets();
                setupGraphics(assetManager);
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                gl.glViewport(0, 0, width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (!fileAccessHelper.isDataReady()) {
                    gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
                    return;
                }
                drawFrame();
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_FILES && resultCode == RESULT_OK) {
            Uri uriU = data.getClipData().getItemAt(0).getUri();
            Uri uriV = data.getClipData().getItemAt(1).getUri();
            try {
                Uri uriW = data.getClipData().getItemAt(2).getUri();
                Log.i("MainActivity", "3D mode");
                fileAccessHelper.loadNetCDFData(uriU, uriV, uriW);
            } catch (IndexOutOfBoundsException e) {
                Log.i("MainActivity", "2D mode");
                fileAccessHelper.loadNetCDFData(uriU, uriV);
            }
        }
    }


    public void onDataLoaded() {
        runOnUiThread(() -> fileAccessHelper.setDataReady(true));
        glSurfaceView.queueEvent(this::createBuffers);
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Log.d("Permissions", "Request code: " + requestCode);
        if (requestCode == REQUEST_CODE_READ_STORAGE) {
                fileAccessHelper.openFilePicker();
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        if (glSurfaceView != null) {
            glSurfaceView.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (glSurfaceView != null) {
            glSurfaceView.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        fileAccessHelper.shutdown();
    }
}
