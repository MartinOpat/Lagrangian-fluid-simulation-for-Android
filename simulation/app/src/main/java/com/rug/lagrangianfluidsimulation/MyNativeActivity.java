package com.rug.lagrangianfluidsimulation;

import android.app.NativeActivity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.documentfile.provider.DocumentFile;

import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

public class MyNativeActivity extends NativeActivity {
    static {
        System.loadLibrary("lagrangianfluidsimulation");
        System.loadLibrary("netcdf");
        System.loadLibrary("netcdf_c++4");
    }

    protected FileAccessHelper fileAccessHelper = new FileAccessHelper(this);

    public native void loadDeviceInfo(double aspectRatio);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        fileAccessHelper.checkAndRequestPermissions();
        getAspectRatio();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d("MainActivity", "Request code: " + requestCode + ", Result code: " + resultCode);
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == FileAccessHelper.REQUEST_CODE_PICK_FILES && resultCode == RESULT_OK) {
            Log.d("MainActivity", "Files picked");

            Uri uriPos = data.getData();
            fileAccessHelper.loadInitialPositions(uriPos);
            fileAccessHelper.openDirectoryPicker();

        } else if (requestCode == FileAccessHelper.REQUEST_CODE_PICK_DIRECTORY && resultCode == RESULT_OK) {
            // Extract the uri of all files from directory
            Uri uri = data.getData();
            assert uri != null;

            DocumentFile directory = DocumentFile.fromTreeUri(this, uri);
            assert directory != null;

            Log.i("MainActivity", "Directory picked: "  + directory.getName());
            DocumentFile[] files = directory.listFiles();

            // Sort files by name
            Map<DocumentFile, String> nameCache = new HashMap<>();
            for (DocumentFile file : files) {
                nameCache.put(file, file.getName());
            }

            DocumentFile[] sortedFiles = Arrays.stream(files)
                    .parallel()
                    .sorted(Comparator.comparing(file -> {
                        String name = nameCache.get(file);
                        return (name != null) ? name : "";
                    }))
                    .toArray(DocumentFile[]::new);


            Log.i("MainActivity", "Files loaded: " + sortedFiles.length);
            Uri[] uris = new Uri[sortedFiles.length];
            for (int i = 0; i < sortedFiles.length; i++) {
                uris[i] = sortedFiles[i].getUri();
            }
            fileAccessHelper.loadNetCDFData(uris);
            Log.i("MainActivity", "Data loaded");

        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Log.d("Permissions", "Request code: " + requestCode);
        if (requestCode == FileAccessHelper.REQUEST_CODE_READ_STORAGE) {
            fileAccessHelper.openFilePicker();
        }
    }

    private void getAspectRatio() {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int widthPixels = displayMetrics.widthPixels;
        int heightPixels = displayMetrics.heightPixels;
        double aspectRatio = (double) widthPixels / heightPixels;
        Log.i("MainActivity", "Detected aspect ratio: " + aspectRatio);
        loadDeviceInfo(aspectRatio);
    }


}
