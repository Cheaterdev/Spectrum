package com.spectrum;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends Activity
{
    // Loading the library runs JNI_OnLoad (android_entry.cpp): it creates the Application
    // singleton and starts logging. Watch it with: adb logcat -s Spectrum
    static { System.loadLibrary("spectrum"); }

    private static native void nativeRun();
    private static native void nativeProbeGpu();

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i("Spectrum", "MainActivity.onCreate");

        nativeRun();
        nativeProbeGpu();

        TextView text = new TextView(this);
        text.setText("Spectrum\n\nNative library loaded.\nadb logcat -s Spectrum");
        text.setTextSize(20);
        text.setPadding(48, 96, 48, 48);
        setContentView(text);
    }
}
