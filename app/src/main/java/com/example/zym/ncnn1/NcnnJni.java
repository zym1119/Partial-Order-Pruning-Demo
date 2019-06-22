package com.example.zym.ncnn1;

import android.graphics.Bitmap;

public class NcnnJni
{
    public native boolean Init(byte[] param, byte[] bin);

    public native float[] Detect(Bitmap bitmap);

    static {
        System.loadLibrary("ncnn_jni");
    }
}
