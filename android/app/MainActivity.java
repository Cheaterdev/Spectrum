package com.spectrum;

import android.app.Activity;
import android.os.Bundle;
import android.system.Os;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;

public class MainActivity extends Activity implements SurfaceHolder.Callback
{
    // Loading the library runs JNI_OnLoad (android_entry.cpp): it creates the Application
    // singleton and starts logging. Watch it with: adb logcat -s Spectrum
    static { System.loadLibrary("spectrum"); }

    private static native void nativeRun();
    private static native void nativeEnterWorkDir();   // chdir to $SPECTRUM_WORKDIR
    private static native void nativeProbeGpu();
    private static native void nativeSurface(Surface surface, int width, int height);

    private SurfaceView view;
    private final Object lock = new Object();
    private Surface surface;
    private int surfaceWidth, surfaceHeight;
    private boolean surfaceReady;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i("Spectrum", "MainActivity.onCreate");
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        view = new SurfaceView(this);
        view.getHolder().addCallback(this);
        setContentView(view);

        // Engine start-up (device, queues, PSO attempts) takes seconds; keep it off the UI thread
        // or Android reports the activity as not responding.
        new Thread(() -> {
            prepareWorkDir();
            nativeRun();
            nativeProbeGpu();

            synchronized (lock)
            {
                while (!surfaceReady)
                    try { lock.wait(); } catch (InterruptedException e) { return; }
                nativeSurface(surface, surfaceWidth, surfaceHeight);
            }
        }, "spectrum-main").start();
    }

    // The engine reads/writes cache/vulkan/<hash>.bin relative to the working directory. Make that
    // the app's files dir and fill it with the precompiled shaders packed in assets/shader_cache
    // (re-extracted whenever the APK is reinstalled, since a blob's name is a hash of its inputs,
    // not of its content).
    private void prepareWorkDir()
    {
        try
        {
            File root = getFilesDir();
            Os.setenv("SPECTRUM_WORKDIR", root.getAbsolutePath(), true);
            // UI brightness in the engine's scRGB convention (DisplayOutput.ui_scale; the desktop uses
            // sdr_white_nits / 80). adb screencap re-renders the FP16 layer and does not match the
            // panel, so judge it on the display. Tune without a rebuild:
            //   adb shell am start -n com.spectrum/.MainActivity --ef ui_scale 0.7
            Os.setenv("SPECTRUM_UI_SCALE", String.valueOf(getIntent().getFloatExtra("ui_scale", 1.0f)), true);
            // Adreno's shader compiler never finishes the ray-query DDGI debug shader (runaway CPU/memory).
            Os.setenv("SPECTRUM_DISABLE_RAY_QUERY", "1", true);
            nativeEnterWorkDir();

            File stamp = new File(root, "cache/.apk_stamp");
            File shaders = new File(root, "cache/vulkan");
            String current = String.valueOf(getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime);
            if (stamp.exists() && current.equals(new String(Files.readAllBytes(stamp.toPath()))) && shaders.isDirectory())
                return;

            File[] old = shaders.listFiles();
            if (old != null)
                for (File f : old)
                    f.delete();
            shaders.mkdirs();

            String[] names = getAssets().list("shader_cache");
            byte[] buffer = new byte[64 * 1024];
            for (String name : names)
            {
                try (InputStream in = getAssets().open("shader_cache/" + name);
                     OutputStream out = new FileOutputStream(new File(shaders, name)))
                {
                    for (int n; (n = in.read(buffer)) > 0; )
                        out.write(buffer, 0, n);
                }
            }
            Files.write(stamp.toPath(), current.getBytes());
            Log.i("Spectrum", "extracted " + names.length + " shader blobs to " + shaders);
        }
        catch (Exception e)
        {
            Log.e("Spectrum", "prepareWorkDir failed", e);
        }
    }

    @Override public void surfaceCreated(SurfaceHolder holder) {}

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
    {
        Log.i("Spectrum", "surfaceChanged " + width + "x" + height);
        synchronized (lock)
        {
            surface = holder.getSurface();
            surfaceWidth = width;
            surfaceHeight = height;
            surfaceReady = true;
            lock.notifyAll();
        }
    }

    @Override public void surfaceDestroyed(SurfaceHolder holder) {}
}
