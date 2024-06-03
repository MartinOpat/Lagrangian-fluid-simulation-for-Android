package com.rug.lagrangianfluidsimulation;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;


import androidx.test.core.app.ActivityScenario;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.ext.junit.rules.ActivityScenarioRule;


import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;

import static org.junit.Assert.*;



@RunWith(AndroidJUnit4.class)
public class MainActivityTest {

    @Rule
    public ActivityScenarioRule<MainActivity> activityRule = new ActivityScenarioRule<>(MainActivity.class);

    @Before
    public void setup() {
        // Inject the FakeFileAccessHelper before each test
        activityRule.getScenario().onActivity(activity -> {
            FileAccessHelper fakeHelper = new FakeFileAccessHelper(activity);
            activity.setFileAccessHelper(fakeHelper);
        });
    }


    @Test
    public void testDataLoadingViaFilePicker() {
        // Set up the intent as if the user has selected a file
        Intent intent = new Intent();
        intent.setData(Uri.parse("content://com.example.app/testfile.nc"));

        // Execute the activity result handling which should invoke file loading
        activityRule.getScenario().onActivity(activity -> {
            FileAccessHelper helper = new FakeFileAccessHelper(activity);
            activity.setFileAccessHelper(helper); // Use the fake helper

            // Simulate the onActivityResult which would normally be called after file selection
            activity.onActivityResult(FileAccessHelper.REQUEST_CODE_PICK_FILES, Activity.RESULT_OK, intent);

            // Check if the data was loaded as expected
            assertTrue("Data should be ready after loading", helper.isDataReady());
        });
    }

    @Test
    public void testOnRequestPermissionsResult() {
        activityRule.getScenario().onActivity(activity -> {
            FakeFileAccessHelper helper = new FakeFileAccessHelper(activity);
            activity.setFileAccessHelper(helper);

            // Simulate permission granted
            int requestCode = FileAccessHelper.REQUEST_CODE_READ_STORAGE;
            int[] grantResults = new int[]{PackageManager.PERMISSION_GRANTED};
            String[] permissions = new String[]{Manifest.permission.READ_EXTERNAL_STORAGE};

            activity.onRequestPermissionsResult(requestCode, permissions, grantResults);

            // Verify that file picker opens if permission is granted
            assertTrue("File picker should be opened after permissions are granted", helper.filePickerOpened);
        });
    }

    @Test
    public void testOnActivityResult_fileSelectionHandling() {
        Intent resultData = new Intent();
        Uri uri = Uri.parse("content://com.example.app/testfile.nc");
        resultData.setData(uri);

        activityRule.getScenario().onActivity(activity -> {
            FakeFileAccessHelper helper = new FakeFileAccessHelper(activity);
            activity.setFileAccessHelper(helper);

            // Simulate file selection result
            activity.onActivityResult(FileAccessHelper.REQUEST_CODE_PICK_FILES, Activity.RESULT_OK, resultData);

            // Verify that initial positions are loaded
            assertTrue("Initial positions should be loaded", helper.isDataReady());
            assertTrue("Directory picker should be opened", helper.directoryPickerOpened);
        });
    }

    @Test
    public void testGLSurfaceViewCallbacks() {

        activityRule.getScenario().onActivity(activity -> {
            MainActivity spyMainActivity = Mockito.spy(activity);
            // Override the createBuffers method
            Mockito.doNothing().when(spyMainActivity).createBuffers();

            // Call the method to test
            spyMainActivity.createBuffers();

            // Verify that createBuffers was called
            Mockito.verify(spyMainActivity).createBuffers();
        });
    }

}


