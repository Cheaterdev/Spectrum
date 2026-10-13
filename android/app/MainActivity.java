package com.spectrum;

import android.app.Activity;
import android.os.Bundle;
import android.system.Os;
import android.util.Log;
import android.view.MotionEvent;
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
    // Renders into `surface` until nativeSetGeneration() is called with a number other than `generation`
    // (the surface was resized, rotated or destroyed), then releases the swapchain and returns.
    private static native void nativeRenderSurface(Surface surface, int generation);
    private static native void nativeSetGeneration(int generation);
    private static native void nativeRunTests();   // the CPU tests linked into the library; filter in $SPECTRUM_TEST_FILTER

    // Touch: one nativeTouchPoint per finger the event concerns, then nativeTouchFlush(flags) with
    // MOVE (1), DOWN (2) or UP (4) - the shape of GUI::touch_event, so native builds one message.
    private static native void nativeTouchPoint(int id, float x, float y);
    private static native void nativeTouchFlush(int flags);

    private SurfaceView view;
    private final Object lock = new Object();
    private Surface surface;
    private boolean surfaceReady;
    private boolean renderActive;   // the render thread is inside nativeRenderSurface
    private boolean running = true;
    private int generation;         // bumped on every surface change; the native loop ends when it moves

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

            // Test mode: run the tests instead of the UI, then close.   adb shell am start -n
            // com.spectrum/.MainActivity --es tests Core.Serialization      ("all" runs everything)
            String tests = getIntent().getStringExtra("tests");
            if (tests != null)
            {
                if (tests.equals("all"))
                    tests = "";
                try { Os.setenv("SPECTRUM_TEST_FILTER", tests, true); } catch (Exception e) { Log.e("Spectrum", "setenv failed", e); }
                nativeRunTests();
                runOnUiThread(this::finish);
                return;
            }

            nativeRun();
            nativeProbeGpu();

            // One pass per surface: rotation or resize destroys the old swapchain and builds a new one.
            while (true)
            {
                Surface current;
                int currentGeneration;
                synchronized (lock)
                {
                    while (running && !surfaceReady)
                        try { lock.wait(); } catch (InterruptedException e) { return; }
                    if (!running)
                        return;
                    current = surface;
                    currentGeneration = generation;
                    renderActive = true;
                }

                try { nativeRenderSurface(current, currentGeneration); }
                finally
                {
                    synchronized (lock)
                    {
                        renderActive = false;
                        lock.notifyAll();
                    }
                }
            }
        }, "spectrum-main").start();
    }

    // Fingers go to native code untouched by any mouse emulation. The action says which finger changed:
    // DOWN / UP for the one that went down or up, MOVE for all of them (Android batches moves).
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        final int index = event.getActionIndex();
        switch (event.getActionMasked())
        {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                nativeTouchPoint(event.getPointerId(index), event.getX(index), event.getY(index));
                nativeTouchFlush(2);
                break;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                nativeTouchPoint(event.getPointerId(index), event.getX(index), event.getY(index));
                nativeTouchFlush(4);
                break;

            case MotionEvent.ACTION_MOVE:
                for (int i = 0; i < event.getPointerCount(); i++)
                    nativeTouchPoint(event.getPointerId(i), event.getX(i), event.getY(i));
                nativeTouchFlush(1);
                break;

            case MotionEvent.ACTION_CANCEL:   // the system took the gesture: lift every finger where it is
                for (int i = 0; i < event.getPointerCount(); i++)
                    nativeTouchPoint(event.getPointerId(i), event.getX(i), event.getY(i));
                nativeTouchFlush(4);
                break;

            default:
                return super.onTouchEvent(event);
        }
        return true;
    }

    @Override
    protected void onDestroy()
    {
        synchronized (lock)
        {
            running = false;
            generation++;
            nativeSetGeneration(generation);
            lock.notifyAll();
        }
        super.onDestroy();
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
            Os.setenv("TMPDIR", getCacheDir().getAbsolutePath(), true);   // std::filesystem::temp_directory_path()
            // UI brightness in the engine's scRGB convention (DisplayOutput.ui_scale; the desktop uses
            // sdr_white_nits / 80). adb screencap re-renders the FP16 layer and does not match the
            // panel, so judge it on the display. Tune without a rebuild:
            //   adb shell am start -n com.spectrum/.MainActivity --ef ui_scale 0.7
            Os.setenv("SPECTRUM_UI_SCALE", String.valueOf(getIntent().getFloatExtra("ui_scale", 1.0f)), true);

            // UI size: 1 layout unit = 1 dp, so DisplayMetrics.density (2.6 on this phone). The old NDK GUI
            // derived its scale from DPI instead (max(1, DPI / 260)); the density factor is the same idea
            // in the platform's own terms. Override with:  --ef density 2
            Os.setenv("SPECTRUM_DENSITY", String.valueOf(getIntent().getFloatExtra("density", getResources().getDisplayMetrics().density)), true);
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
            surfaceReady = true;
            // A new generation ends the running native loop; the render thread then starts a fresh
            // one for this surface (new swapchain at the new size).
            generation++;
            nativeSetGeneration(generation);
            lock.notifyAll();
        }
    }

    // The surface must not be touched once this returns, so wait for the native loop to let go of it.
    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Log.i("Spectrum", "surfaceDestroyed");
        synchronized (lock)
        {
            surfaceReady = false;
            generation++;
            nativeSetGeneration(generation);
            while (renderActive)
                try { lock.wait(); } catch (InterruptedException e) { break; }
        }
    }
}
