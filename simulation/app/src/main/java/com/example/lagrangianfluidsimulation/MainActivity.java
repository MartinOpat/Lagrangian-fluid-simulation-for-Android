package com.example.lagrangianfluidsimulation;

import androidx.appcompat.app.AppCompatActivity;


import android.app.Activity;
import android.opengl.GLES20;
import android.os.Bundle;
import android.opengl.GLSurfaceView;
import android.widget.TextView;

import com.example.lagrangianfluidsimulation.databinding.ActivityMainBinding;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("lagrangianfluidsimulation");
    }

    // Attributes
    private native void drawFrame();
    private native void setupGraphics();
    private GLSurfaceView glSurfaceView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(2);

        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                setupGraphics();
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



//public class MainActivity extends AppCompatActivity {
//
//    // Used to load the 'lagrangianfluidsimulation' library on application startup.
//    static {
//        System.loadLibrary("lagrangianfluidsimulation");
//    }
//
//    private ActivityMainBinding binding;
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//
//        binding = ActivityMainBinding.inflate(getLayoutInflater());
//        setContentView(binding.getRoot());
//
//        // Example of a call to a native method
//        TextView tv = binding.sampleText;
//        tv.setText(stringFromJNI());
//    }
//
//    /**
//     * A native method that is implemented by the 'lagrangianfluidsimulation' native library,
//     * which is packaged with this application.
//     */
//    public native String stringFromJNI();
//}