package com.rug.lagrangianfluidsimulation;


import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;


public class FileAccessHelper {

    MainActivity mainActivity;
    private volatile boolean dataReady = false;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();


    public static final int REQUEST_CODE_READ_STORAGE = 100;
    public static final int REQUEST_CODE_PICK_FILES = 101;
    public static final int REQUEST_CODE_PICK_DIRECTORY = 102;

    public native void loadFilesFDs(int[] fds);
    public native void loadInitialPositions(int fd);

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

    public void openDirectoryPicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        mainActivity.startActivityForResult(intent, REQUEST_CODE_PICK_DIRECTORY);
    }


    public void checkAndRequestPermissions() {
        Log.d("Permissions", "Checking permissions");
        if (ContextCompat.checkSelfPermission(mainActivity, android.Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.d("Permissions", "Permission not granted, requesting...");
            ActivityCompat.requestPermissions(mainActivity, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, REQUEST_CODE_READ_STORAGE);
        } else {
            Log.d("Permissions", "Permission already granted, opening picker");
            openFilePicker();
            openDirectoryPicker();
        }
    }
    public void loadInitialPositions(Uri uri) {
        executor.submit(() -> {
            int fd = getFileDescriptor(uri);
            if (fd != -1) {
                loadInitialPositions(fd);
            }
        });
    }

    public void loadNetCDFData(Uri[] uris) {
        executor.submit(() -> {

            ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
            List<Future<Integer>> futureList = new ArrayList<>();
            for (Uri uri : uris) {
                futureList.add(executor.submit(() -> getFileDescriptor(uri)));
            }
            executor.shutdown();
            try {
                boolean res = executor.awaitTermination(1, TimeUnit.MINUTES); // Wait for all tasks to finish
                if (!res) {
                    Log.e("MainActivity", "Executor service timed out");
                }
            } catch (InterruptedException e) {
                Log.e("MainActivity", "Executor service interrupted", e);
            }
            int[] fds = new int[uris.length];
            for (int i = 0; i < futureList.size(); i++) {
                try {
                    fds[i] = futureList.get(i).get();
                } catch (Exception e) {
                    Log.e("MainActivity", "Error getting file descriptor", e);
                }
            }
            mainActivity.runOnUiThread(() -> {
                loadFilesFDs(fds);
                mainActivity.onDataLoaded();
            });
        });
    }


    public int getFileDescriptor(Uri uri) {
        try (ParcelFileDescriptor pfd = mainActivity.getContentResolver().openFileDescriptor(uri, "r")) {
            if (pfd != null) {
                return pfd.detachFd(); // Detach the file descriptor to pass it to native code
            }
        } catch (IOException e) {
            Log.e("MainActivity", "File descriptor for file not found.", e);
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
