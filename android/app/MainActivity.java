package com.spectrum;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

public class MainActivity extends Activity implements SurfaceHolder.Callback
{
    // Loading the library runs JNI_OnLoad (android_entry.cpp): it creates the Application
    // singleton and starts logging. Watch it with: adb logcat -s Spectrum
    static { System.loadLibrary("spectrum"); }

    private static native void nativeRun();
    private static native void nativeProbeGpu();
    private static native void nativeSurface(Surface surface, int width, int height);

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

        SurfaceView view = new SurfaceView(this);
        view.getHolder().addCallback(this);
        setContentView(view);

        // Engine start-up (device, queues, PSO attempts) takes seconds; keep it off the UI thread
        // or Android reports the activity as not responding.
        new Thread(() -> {
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
