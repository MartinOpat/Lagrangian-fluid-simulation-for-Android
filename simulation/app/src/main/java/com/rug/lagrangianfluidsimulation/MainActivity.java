package com.rug.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;

import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class MainActivity extends Activity {
    static {
        System.loadLibrary("lagrangianfluidsimulation");
        System.loadLibrary("netcdf");
        System.loadLibrary("netcdf_c++4");
    }


    // Attributes
    private GLSurfaceView glSurfaceView;
    private final FileAccessHelper fileAccessHelper = new FileAccessHelper(this);
    private Uri[] uris;

    private native void drawFrame();
    private native void setupGraphics(AssetManager assetManager);
    public native void createBuffers();
    public native void nativeSendTouchEvent(int pointerCount, float[] x, float[] y, int action);
    public native void onDestroyNative();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupGLSurfaceView();
        setContentView(glSurfaceView);
        fileAccessHelper.checkAndRequestPermissions();
    }

    private void setupGLSurfaceView() {
        glSurfaceView = new MyGLSurfaceView(this);
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
        if (requestCode == FileAccessHelper.REQUEST_CODE_PICK_FILES && resultCode == RESULT_OK) {
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
        } else if (requestCode == FileAccessHelper.REQUEST_CODE_PICK_DIRECTORY && resultCode == RESULT_OK) {
            // Extract the uri of all files from directory
            Uri uri = data.getData();

            DocumentFile directory = DocumentFile.fromTreeUri(this, uri);
            Log.i("MainActivity", "Directory picked: "  + directory.getName());
            DocumentFile[] files = directory.listFiles();

            // Sort files by name
            Map<DocumentFile, String> nameCache = new HashMap<>();
            for (DocumentFile file : files) {
                nameCache.put(file, file.getName());
            }

            DocumentFile[] sortedFiles = Arrays.stream(files)
                    .parallel()
                    .sorted(Comparator.comparing(nameCache::get))
                    .toArray(DocumentFile[]::new);


            Log.i("MainActivity", "Files loaded: " + sortedFiles.length);
            uris = new Uri[sortedFiles.length];
            for (int i = 0; i < sortedFiles.length; i++) {
                uris[i] = sortedFiles[i].getUri();
            }
            fileAccessHelper.loadNetCDFData(uris);
            Log.i("MainActivity", "Data loaded");

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
        if (requestCode == FileAccessHelper.REQUEST_CODE_READ_STORAGE) {
                fileAccessHelper.openDirectoryPicker();
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
        onDestroyNative();
        fileAccessHelper.shutdown();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();
        float[] x = new float[pointerCount];
        float[] y = new float[pointerCount];

        for (int i = 0; i < pointerCount; i++) {
            x[i] = event.getX(i);
            y[i] = event.getY(i);
        }

        // Pass the touch event details along with pointer count and coordinates to native code
        nativeSendTouchEvent(pointerCount, x, y, action);

        return true;
    }

}
