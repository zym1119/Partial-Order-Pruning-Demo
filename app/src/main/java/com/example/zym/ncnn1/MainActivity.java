package com.example.zym.ncnn1;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.text.DecimalFormat;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;
import com.bumptech.glide.request.RequestOptions;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;


public class MainActivity extends Activity {
    private ImageView input_image;
    private ImageView pred_image;
    private TextView inf_speed;
    private Button test;
    private String model_name = "pl1a_new";
    private boolean load_result = false;
    private NcnnJni ncnn = new NcnnJni();
    private float avg_time = 0.0f;
    private int image_count;
    private float[] result;

    // bitmaps
    private Bitmap input_bmp;

    // threads
    private Thread mThread;
    private Handler mHandler = new Handler();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.w("Ncnn", "Start");
        try {
            initNcnn();
        } catch (IOException e) {
            Log.e("MainActivity", "initNcnn error");
        }

        init_view();

        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                synchronized (MainActivity.this) {
                    super.handleMessage(msg);
                    Log.d("NcnnIdx", "Update " + image_count);
                    input_image.setImageResource((int) msg.obj);
                    input_image.invalidate();
                    result = ncnn.Detect(input_bmp);
//                    avg_time += result[2];
                    avg_time += (result[0]+result[1]+result[2]+result[3]);
                    pred_image.setImageBitmap(Bitmap.createBitmap(input_bmp, 0, 0, 160, 96));
                    // update
                    pred_image.invalidate();
                    String res = " Pre/Post Processing: " + String.format("%.2f", result[0]+result[1]+result[3]) +
                            "ms\n Network Inference: " + String.format("%.2f", result[2]) +
                            "ms\n Avg. Inference Time: " + String.format("%.2f", avg_time / image_count) +
                            "ms\n Avg. FPS: " + String.format("%.2f", 1000*image_count/avg_time);
                    inf_speed.setText(res);
                    image_count ++;
                }
            }
        };

    }

    private void initNcnn() throws IOException {
        byte[] param = null;
        byte[] bin = null;

        {
            InputStream assetsInputStream = getAssets().open(model_name+".param.bin");
            int available = assetsInputStream.available();
            param = new byte[available];
            int byteCode = assetsInputStream.read(param);
            assetsInputStream.close();
        }
        {
            InputStream assetsInputStream = getAssets().open(model_name+".bin");
            int available = assetsInputStream.available();
            bin = new byte[available];
            int byteCode = assetsInputStream.read(bin);
            assetsInputStream.close();
        }

        load_result = ncnn.Init(param, bin);
    }

    // initialize view
    private void init_view() {
//        request_permissions();
        input_image = (ImageView) findViewById(R.id.input_image);
        inf_speed = (TextView) findViewById(R.id.inf_speed);
        pred_image = (ImageView) findViewById(R.id.pred_image);
        inf_speed.setMovementMethod(ScrollingMovementMethod.getInstance());
        test = (Button) findViewById(R.id.test);

        test.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!load_result) {
                    Toast.makeText(MainActivity.this, "Model not loaded!", Toast.LENGTH_LONG).show();
                    return;
                }
                mThread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            image_count = 1;
                            avg_time = 0.0f;
                            inference_loop();  // send message inside
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                });
                mThread.setPriority(4);
                mThread.setDaemon(true);
                mThread.start();
            }
        });
    }

    // inference
    private void inference_loop() {
        for (int image_idx=1; image_idx<661; image_idx++) {
            String cur_image = "demo" + image_idx;
            int id = MainActivity.this.getResources().getIdentifier(cur_image, "drawable", getPackageName());
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inScaled = false;
            opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
            input_bmp = BitmapFactory.decodeResource(MainActivity.this.getResources(), id, opts);
            // send message
            Log.d("NcnnIdx", "-----" + image_idx + "-----");
            mHandler.obtainMessage(0, id).sendToTarget();
        }
    }
}
