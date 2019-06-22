#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <string>
#include <vector>

#include "net.h"
#include "cpu.h"
#include <sys/time.h>
#include <unistd.h>

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

static ncnn::Mat ncnn_param;
static ncnn::Mat ncnn_bin;
static ncnn::Net ncnn_net;

static struct timeval tv_time;
static long get_time()
{
    gettimeofday(&tv_time, nullptr);
    return (tv_time.tv_sec*1000000) + tv_time.tv_usec;
}

extern "C" {

// public native boolean Init(byte[] param, byte[] bin, byte[] words);
JNIEXPORT jboolean JNICALL
Java_com_example_zym_ncnn1_NcnnJni_Init(JNIEnv *env, jobject thiz, jbyteArray param, jbyteArray bin) {
    // init param
    {
        int len = env->GetArrayLength(param);
        ncnn_param.create(len, (size_t) 1u);
        env->GetByteArrayRegion(param, 0, len, (jbyte *) ncnn_param);
        int ret = ncnn_net.load_param((const unsigned char *) ncnn_param);
        __android_log_print(ANDROID_LOG_DEBUG, "NcnnJni", "load_param %d %d", ret, len);
        if (ret != len)
            return JNI_FALSE;
    }

    // init bin
    {
        int len = env->GetArrayLength(bin);
        ncnn_bin.create(len, (size_t) 1u);
        env->GetByteArrayRegion(bin, 0, len, (jbyte *) ncnn_bin);
        int ret = ncnn_net.load_model((const unsigned char *) ncnn_bin);
        __android_log_print(ANDROID_LOG_DEBUG, "NcnnJni", "load_model %d %d", ret, len);
        if (ret != len)
            return JNI_FALSE;
    }

    ncnn::Option opt;
    opt.lightmode = true;
    opt.num_threads = 4;
    opt.blob_allocator = &g_blob_pool_allocator;
    opt.workspace_allocator = &g_workspace_pool_allocator;

    //ncnn::set_default_option(opt);
    ncnn_net.opt = opt;
    // bind ncnn threads on specific cores,
    // which reducing context switches and may preventing being migrated from big cores to little cores.
    ncnn::set_cpu_powersave(2);

    ncnn::set_omp_dynamic(1);  // seems set_omp_num_threads failed

    return JNI_TRUE;
}

// public native String Detect(Bitmap bitmap);
JNIEXPORT jfloatArray JNICALL Java_com_example_zym_ncnn1_NcnnJni_Detect(JNIEnv* env, jobject thiz, jobject bitmap)
{
    // time
    long start_time;
    float read_time, preproc_time, inf_time, output_time, total_time;
    start_time = get_time();

    // ncnn from bitmap
    ncnn::Mat in;
    {
        AndroidBitmapInfo info;
        AndroidBitmap_getInfo(env, bitmap, &info);
        int width = info.width;
        int height = info.height;
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
            return NULL;
        void* indata;
        AndroidBitmap_lockPixels(env, bitmap, &indata);
        // 把像素转换成data，并指定通道顺序
        in = ncnn::Mat::from_pixels((const unsigned char*)indata, ncnn::Mat::PIXEL_RGBA2BGR, width, height);
        AndroidBitmap_unlockPixels(env, bitmap);
    }

    read_time = (get_time() - start_time) / 1000.0;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnRead", "Time: %f", read_time);

    // ncnn_net
    // 减去均值和乘上比例
    //const float mean_vals[3] = {104.f, 104.f, 104.f};
    const float mean_vals[3] = {103.94f, 116.78f, 123.68f};
    const float scale[3] = {0.008f, 0.008f, 0.008f};

    in.substract_mean_normalize(mean_vals, scale);

    preproc_time = (get_time() - start_time) / 1000.0 - read_time;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnPreproc", "Time: %f", preproc_time);

    ncnn::Extractor ex = ncnn_net.create_extractor();
    ex.input(0, in);

    ncnn::Mat out;

    ex.extract(219, out);

    inf_time = (get_time() - start_time) / 1000.0 - read_time - preproc_time;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnInf", "Time: %f", inf_time);

    // operate bitmap
    {
        const unsigned char R[19] = {128, 224, 70, 102, 190, 153, 250, 220, 107, 152, 0, 220, 255, 0, 0, 0, 0, 0, 119};
        const unsigned char G[19] = {64, 35, 70, 102, 153, 153, 170, 220, 142, 251, 130, 20, 0, 0, 0, 60, 80, 0, 11};
        const unsigned char B[19] = {128, 232, 70, 156, 153, 153, 30, 0, 35, 152, 180, 60, 0, 142, 70, 100, 100, 230, 32};
        AndroidBitmapInfo info;
        AndroidBitmap_getInfo(env, bitmap, &info);
        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
            return NULL;
        unsigned char *indata = NULL;
        AndroidBitmap_lockPixels(env, bitmap, (void**)&indata);
        // 把像素转换成cityscapes颜色
        int pixel = out.w * out.h;
        for (int i=0; i<pixel; i++) {
            int row = i / out.w;
            int col = i - row * out.w;
            int idx = row * info.width + col;
            indata[4*idx] = R[(int)out.row(row)[col]];
            indata[4*idx+1] = G[(int)out.row(row)[col]];
            indata[4*idx+2] = B[(int)out.row(row)[col]];
            indata[4*idx+3] = (unsigned char)255;
        }
        AndroidBitmap_unlockPixels(env, bitmap);
    }
    output_time = (get_time() - start_time) / 1000.0 - read_time - preproc_time - inf_time;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnColor", "Time: %f", output_time);
    total_time = (get_time() - start_time) / 1000.0;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnTime", "Total: %f", total_time);

    // output time info
    jfloat data[4];
    data[0] = read_time;
    data[1] = preproc_time;
    data[2] = inf_time;
    data[3] = output_time;
    __android_log_print(ANDROID_LOG_DEBUG, "NcnnData", "%f %f %f %f", data[0], data[1], data[2], data[3]);
    jfloatArray jOutputData = env->NewFloatArray(4);
    if (jOutputData == nullptr) return nullptr;
    env->SetFloatArrayRegion(jOutputData, 0, 4,
                             reinterpret_cast<const jfloat *>(data));
    return jOutputData;
}
}
