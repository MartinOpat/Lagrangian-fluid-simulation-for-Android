package com.example.lagrangianfluidsimulation;


import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.documentfile.provider.DocumentFile;

import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.Manifest;
import android.opengl.GLSurfaceView;


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
    private ExecutorService executor = Executors.newSingleThreadExecutor();
    private volatile boolean dataReady = false;
    private GLSurfaceView glSurfaceView;

    private native void drawFrame();
    private native void setupGraphics(AssetManager assetManager);
    private native void createBuffers();
    public native void initializeNetCDFVisualization(int fdU, int fdV);

    private static final int REQUEST_CODE_READ_STORAGE = 100;
    private static final int REQUEST_CODE_PICK_FILES = 101;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupGLSurfaceView();
        setContentView(glSurfaceView);
        checkAndRequestPermissions();
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
                if (!isDataReady()) {
                    gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
                    return;
                }
                drawFrame();
            }
        });
    }

//    private void checkAndRequestPermissions() {
//        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
//                != PackageManager.PERMISSION_GRANTED) {
//            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, REQUEST_CODE_READ_STORAGE);
//        } else {
//            openFilePicker();
//        }
//    }

    private void checkAndRequestPermissions() {
        Log.d("Permissions", "Checking permissions");
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.d("Permissions", "Permission not granted, requesting...");
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, REQUEST_CODE_READ_STORAGE);
        } else {
            Log.d("Permissions", "Permission already granted, opening picker");
            openFilePicker();
        }
    }


    private void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        String[] mimeTypes = {"application/netcdf", "application/x-netcdf"};
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes);
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        startActivityForResult(intent, REQUEST_CODE_PICK_FILES);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_FILES && resultCode == RESULT_OK) {
            Uri uriU = data.getClipData().getItemAt(0).getUri();
            Uri uriV = data.getClipData().getItemAt(1).getUri();
            loadNetCDFData(uriU, uriV);
        }
    }

    private void loadNetCDFData(Uri uriU, Uri uriV) {
        executor.submit(() -> {
            int fdU = getFileDescriptor(uriU);
            int fdV = getFileDescriptor(uriV);
            if (fdU != -1 && fdV != -1) {
                initializeNetCDFVisualization(fdU, fdV);
            }
            runOnUiThread(this::onDataLoaded);
        });
    }


    public int getFileDescriptor(Uri uri) {
        try {
            ParcelFileDescriptor pfd = getContentResolver().openFileDescriptor(uri, "r");
            if (pfd != null) {
                return pfd.detachFd(); // Detach the file descriptor to pass it to native code
            }
        } catch (FileNotFoundException e) {
            Log.e("MainActivity", "File not found.", e);
        }
        return -1; // Return an invalid file descriptor in case of error
    }

    public void onDataLoaded() {
        runOnUiThread(() -> setDataReady(true));
        glSurfaceView.queueEvent(this::createBuffers);
    }


    public void setDataReady(boolean dataReady) {
        this.dataReady = dataReady;
    }

    public boolean isDataReady() {
        return dataReady;
    }



//    @Override
//    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
//        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
//        if (requestCode == REQUEST_CODE_READ_STORAGE) {
//            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
//                openFilePicker();
//            } else {
//                Toast.makeText(this, "Permission denied to read your External storage", Toast.LENGTH_LONG).show();
//            }
//        }
//    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Log.d("Permissions", "Request code: " + requestCode);
        if (requestCode == REQUEST_CODE_READ_STORAGE) {
                openFilePicker();
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
        executor.shutdown();
    }
}
