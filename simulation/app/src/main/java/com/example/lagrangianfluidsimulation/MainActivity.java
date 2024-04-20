package com.example.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.util.List;

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
//        System.loadLibrary("hdf5_hl");
//        System.loadLibrary("hdf5");
        System.loadLibrary("netcdf");
        System.loadLibrary("netcdf_c++4");
    }

    // Attributes
    private native void drawFrame();
    private native void setupGraphics(AssetManager assetManager);
    public native void initializeNetCDFVisualization(String fileUPath, String fileVPath);
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

                List<Uri> downloadFileUris = FileAccessHelper.getFilesFromDownloads(MainActivity.this);

                // Assuming the filenames are known
                String fileUPath = "", fileVPath = "";
                for (Uri uri : downloadFileUris) {
                    String path = uri.getPath();
                    assert path != null;
                    if (path.contains("doublegyreU.nc")) {
                        fileUPath = path;
                    } else if (path.contains("doublegyreV.nc")) {
                        fileVPath = path;
                    }
                }

                if (!fileUPath.isEmpty() && !fileVPath.isEmpty()) {
                    initializeNetCDFVisualization(fileUPath, fileVPath);
                } else {
                    Log.e("MainActivity", "Required NetCDF files are not found in Downloads.");
                }
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
