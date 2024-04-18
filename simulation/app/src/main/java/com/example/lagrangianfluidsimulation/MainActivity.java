package com.example.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("lagrangianfluidsimulation");
//        System.loadLibrary("z.so.1");
//        System.loadLibrary("z1");
//        System.loadLibrary("zlib");
//        System.loadLibrary("z1lib");
//        System.loadLibrary("zlib1");
        System.loadLibrary("hdf5_hl");
        System.loadLibrary("hdf5");
        System.loadLibrary("netcdf");
    }

    // Attributes
    private native void drawFrame();
    private native void setupGraphics(AssetManager assetManager);
    private native void temp(AssetManager assetManager, String assetName);
    private GLSurfaceView glSurfaceView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(2);

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                AssetManager assetManager = getAssets();
                setupGraphics(assetManager);
                temp(assetManager, "test_data/doublegyreU.nc");
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                gl.glViewport(0, 0, width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                drawFrame();
            }
        });

        setContentView(glSurfaceView);
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
}
