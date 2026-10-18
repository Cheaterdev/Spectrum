package com.spectrum;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
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
    private volatile boolean testsRunning;

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
                testsRunning = true;
                new Thread(this::paintTestProgress, "spectrum-test-progress").start();
                nativeRunTests();
                testsRunning = false;
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

    // Test mode draws no UI: the surface shows a progress bar instead, from the file the native test
    // runner rewrites before every test ("done total current-test", then one failed test per line). The CPU canvas is fine
    // here because nothing else (no swapchain) connects to this surface in test mode.
    private void paintTestProgress()
    {
        final File file = new File(getFilesDir(), "test_progress.txt");
        file.delete();   // the previous run's last state
        final String appName = getApplicationInfo().loadLabel(getPackageManager()).toString();
        final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        final long start = System.currentTimeMillis();
        while (testsRunning)
        {
            int done = 0, total = 0;
            String current = "starting up";
            String[] failedTests = new String[0];
            try
            {
                String[] lines = new String(Files.readAllBytes(file.toPath())).split("\n");
                String[] head = lines[0].split(" ", 3);
                done = Integer.parseInt(head[0]); total = Integer.parseInt(head[1]);
                current = head[2];
                failedTests = java.util.Arrays.copyOfRange(lines, 1, lines.length);
            }
            catch (Exception e) { /* not written yet, or caught mid-write: show the last state next round */ }

            boolean ready;
            synchronized (lock) { ready = surfaceReady; }
            Canvas canvas = ready ? view.getHolder().lockCanvas() : null;
            if (canvas != null)
            {
                try
                {
                    final float w = canvas.getWidth(), h = canvas.getHeight(), margin = w * 0.08f, bar = h * 0.025f;
                    final float y = h * 0.22f;
                    final int failed = failedTests.length;
                    canvas.drawColor(Color.rgb(16, 18, 26));

                    paint.setColor(Color.WHITE);
                    paint.setTextSize(w * 0.08f);
                    paint.setFakeBoldText(true);
                    canvas.drawText(appName + " tests", margin, h * 0.12f, paint);
                    paint.setFakeBoldText(false);

                    paint.setColor(Color.rgb(48, 52, 66));
                    canvas.drawRoundRect(margin, y, w - margin, y + bar, bar / 2, bar / 2, paint);
                    paint.setColor(failed > 0 ? Color.rgb(224, 82, 82) : Color.rgb(60, 168, 255));
                    final float fraction = total > 0 ? Math.min(1f, (float) done / total) : 0f;
                    canvas.drawRoundRect(margin, y, margin + (w - 2 * margin) * fraction, y + bar, bar / 2, bar / 2, paint);

                    paint.setColor(Color.WHITE);
                    paint.setTextSize(w * 0.05f);
                    canvas.drawText(done + " / " + total + (failed > 0 ? "   failed " + failed : ""), margin, y - bar * 0.6f, paint);
                    paint.setColor(Color.rgb(170, 176, 196));
                    paint.setTextSize(w * 0.034f);
                    canvas.drawText(current.equals("-") ? "finishing" : current, margin, y + bar * 2.6f, paint);
                    canvas.drawText((System.currentTimeMillis() - start) / 1000 + " s", margin, y + bar * 4.0f, paint);

                    // Failed tests, newest last; what does not fit is summarized.
                    paint.setTextSize(w * 0.032f);
                    final float line = paint.getTextSize() * 1.35f;
                    float ty = y + bar * 6.5f;
                    final int room = Math.max(1, (int) ((h - ty - line) / line));
                    final int shown = failed > room ? room - 1 : failed;
                    paint.setColor(Color.rgb(224, 82, 82));
                    for (int i = 0; i < shown; i++, ty += line)
                        canvas.drawText(failedTests[i], margin, ty, paint);
                    if (shown < failed)
                        canvas.drawText("+ " + (failed - shown) + " more", margin, ty, paint);
                }
                finally { view.getHolder().unlockCanvasAndPost(canvas); }
            }
            try { Thread.sleep(200); } catch (InterruptedException e) { return; }
        }
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
            // Engine assets (.asset files saved by spectrum_tool assets): the AssetManager scans <files>/assets.
            File assetRoot = new File(root, "assets");
            deleteTree(assetRoot);
            int assetCount = extractTree("engine_assets", assetRoot, buffer);

            int fontCount = extractTree("fonts", new File(root, "fonts"), buffer);

            Files.write(stamp.toPath(), current.getBytes());
            Log.i("Spectrum", "extracted " + names.length + " shader blobs to " + shaders + ", " + assetCount + " engine assets, " + fontCount + " fonts");
        }
        catch (Exception e)
        {
            Log.e("Spectrum", "prepareWorkDir failed", e);
        }
    }

    private static void deleteTree(File f)
    {
        File[] children = f.listFiles();
        if (children != null)
            for (File c : children)
                deleteTree(c);
        f.delete();
    }

    // Copies the APK asset folder `path` into `dst`, keeping its sub-folders. Returns the number of files.
    private int extractTree(String path, File dst, byte[] buffer) throws java.io.IOException
    {
        String[] entries = getAssets().list(path);
        if (entries == null || entries.length == 0)
            return 0;

        dst.mkdirs();
        int count = 0;
        for (String entry : entries)
        {
            String child = path + "/" + entry;
            String[] below = getAssets().list(child);
            if (below != null && below.length > 0)
                count += extractTree(child, new File(dst, entry), buffer);
            else
            {
                try (InputStream in = getAssets().open(child);
                     OutputStream out = new FileOutputStream(new File(dst, entry)))
                {
                    for (int n; (n = in.read(buffer)) > 0; )
                        out.write(buffer, 0, n);
                }
                ++count;
            }
        }
        return count;
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
