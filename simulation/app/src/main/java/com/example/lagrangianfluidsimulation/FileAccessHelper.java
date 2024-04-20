package com.example.lagrangianfluidsimulation;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;


public class FileAccessHelper {
    public static List<Uri> getFilesFromDownloads(Context context) {
        List<Uri> fileUris = new ArrayList<>();
        Uri collection = MediaStore.Downloads.getContentUri(MediaStore.VOLUME_EXTERNAL);

        String[] projection = new String[] {
                MediaStore.Downloads._ID,
                MediaStore.Downloads.DISPLAY_NAME
        };

        try (Cursor cursor = context.getContentResolver().query(
                collection,
                projection,
                null,
                null,
                null)) {

            assert cursor != null;
            int idColumn = cursor.getColumnIndexOrThrow(MediaStore.Downloads._ID);
            int nameColumn = cursor.getColumnIndexOrThrow(MediaStore.Downloads.DISPLAY_NAME);

            while (cursor.moveToNext()) {
                long id = cursor.getLong(idColumn);
                String name = cursor.getString(nameColumn);
                Uri contentUri = ContentUris.withAppendedId(MediaStore.Downloads.EXTERNAL_CONTENT_URI, id);
                Log.d("FileAccessHelper", "File: " + name + " URI: " + contentUri);
                fileUris.add(contentUri);
            }
        } catch (Exception e) {
            Log.e("FileAccessHelper", "Error accessing files", e);
        }
        return fileUris;
    }

}
