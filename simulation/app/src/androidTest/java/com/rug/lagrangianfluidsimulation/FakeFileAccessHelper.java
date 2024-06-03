package com.rug.lagrangianfluidsimulation;

import android.net.Uri;

public class FakeFileAccessHelper extends FileAccessHelper {
    public boolean permissionsChecked = false;
    public boolean filePickerOpened = false;
    public boolean directoryPickerOpened = false;

    public FakeFileAccessHelper(MainActivity mainActivity) {
        super(mainActivity);
    }

    @Override
    public void openFilePicker() {

        filePickerOpened = true;
    }

    @Override
    public void openDirectoryPicker() {
        directoryPickerOpened = true;
    }

    @Override
    public void checkAndRequestPermissions() {
        // Directly simulate having permissions and skip actual permission checking
        openFilePicker();
        openDirectoryPicker();
        permissionsChecked = true;
    }

    @Override
    public void loadInitialPositions(Uri uri) {
        // Simulate immediate load
        setDataReady(true);
    }

    @Override
    public void loadNetCDFData(Uri[] uris) {
        // Simulate data loading
        setDataReady(true);
        mainActivity.runOnUiThread(mainActivity::onDataLoaded);
    }

    @Override
    public int getFileDescriptor(Uri uri) {
        // Return a dummy file descriptor, typically valid ones are non-negative
        return 1; // Dummy non-negative file descriptor for testing
    }
}
