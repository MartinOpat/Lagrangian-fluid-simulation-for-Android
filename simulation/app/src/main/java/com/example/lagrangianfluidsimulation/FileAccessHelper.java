package com.example.lagrangianfluidsimulation;


import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.FileNotFoundException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class FileAccessHelper {

    MainActivity mainActivity;
    private volatile boolean dataReady = false;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();


    private static final int REQUEST_CODE_READ_STORAGE = 100;
    private static final int REQUEST_CODE_PICK_FILES = 101;

    public native void initializeNetCDFVisualization(int fdU, int fdV);

    // Constructor
    public FileAccessHelper(MainActivity mainActivity) {
        this.mainActivity = mainActivity;
    }

    public void openFilePicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        String[] mimeTypes = {"application/netcdf", "application/x-netcdf"};
        intent.putExtra(Intent.EXTRA_MIME_TYPES, mimeTypes);
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        mainActivity.startActivityForResult(intent, REQUEST_CODE_PICK_FILES);
    }

    public void checkAndRequestPermissions() {
        Log.d("Permissions", "Checking permissions");
        if (ContextCompat.checkSelfPermission(mainActivity, android.Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.d("Permissions", "Permission not granted, requesting...");
            ActivityCompat.requestPermissions(mainActivity, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, REQUEST_CODE_READ_STORAGE);
        } else {
            Log.d("Permissions", "Permission already granted, opening picker");
            openFilePicker();
        }
    }

    public void loadNetCDFData(Uri uriU, Uri uriV) {
        executor.submit(() -> {
            int fdU = getFileDescriptor(uriU);
            int fdV = getFileDescriptor(uriV);
            if (fdU != -1 && fdV != -1) {
                initializeNetCDFVisualization(fdU, fdV);
            }
            mainActivity.runOnUiThread(mainActivity::onDataLoaded);
        });
    }


    public int getFileDescriptor(Uri uri) {
        try {
            ParcelFileDescriptor pfd = mainActivity.getContentResolver().openFileDescriptor(uri, "r");
            if (pfd != null) {
                return pfd.detachFd(); // Detach the file descriptor to pass it to native code
            }
        } catch (FileNotFoundException e) {
            Log.e("MainActivity", "File not found.", e);
        }
        return -1; // Return an invalid file descriptor in case of error
    }


    public void setDataReady(boolean dataReady) {
        this.dataReady = dataReady;
    }

    public boolean isDataReady() {
        return dataReady;
    }

    public void shutdown() {
        executor.shutdown();
    }

}
