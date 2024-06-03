package com.rug.lagrangianfluidsimulation;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import androidx.core.content.ContextCompat;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;

import static org.junit.Assert.assertTrue;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

@RunWith(AndroidJUnit4.class)
public class FileAccessHelperTest {

    @Mock
    private MainActivity mockMainActivity;
    @Mock
    private ContentResolver mockContentResolver;
    @Mock
    private Context mockContext;

    @Before
    public void setup() {
        // Initialize mocks manually
        mockMainActivity = Mockito.mock(MainActivity.class);
        // Assuming the context can be fetched from the instrumentation registry
        Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
        when(mockMainActivity.getApplicationContext()).thenReturn(context);
    }

    @Test
    public void testIsDataReady() {
        // Arrange
        FileAccessHelper helper = new FileAccessHelper(mockMainActivity);
        helper.setDataReady(true); // Set the data readiness status

        // Act
        boolean isDataReady = helper.isDataReady();

        // Assert
        assertTrue("Data readiness status should be true", isDataReady);
    }

    @Test
    public void testCheckAndRequestPermissions_PermissionGranted() {
        // Initialize mocks manually
        mockMainActivity = Mockito.mock(MainActivity.class);
        Context mockContext = Mockito.mock(Context.class);

        when(mockMainActivity.getApplicationContext()).thenReturn(mockContext);
        when(ContextCompat.checkSelfPermission(mockContext, android.Manifest.permission.READ_EXTERNAL_STORAGE))
                .thenReturn(PackageManager.PERMISSION_GRANTED);

        FileAccessHelper helper = new FileAccessHelper(mockMainActivity);

        helper.checkAndRequestPermissions();

        // Assert that the file picker activities are started
        verify(mockMainActivity, times(2)).startActivityForResult(any(Intent.class), anyInt());
    }




}
