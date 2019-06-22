# Partial-Order-Pruning-Demo
This is a demo of deploying PL1A model found by Partial Order Pruning on MI8 cellphone.

## What is Partial Order Pruning?
Please refer to our paper accepted by CVPR2019

https://arxiv.org/abs/1903.03777

https://github.com/lixincn2015/Partial-Order-Pruning

## Must know before use
1. This is just a **DEMO** app to show the inference speed of our network. I am not a good JAVA or Android developper, i just learned them for one week, so my implementation may looks shit :( If you have any suggestions or improvements just pull request.
2. The inference on cellphone uses the **ncnn** framework from https://github.com/Tencent/ncnn.
3. I personlly implement the argmax layer in ncnn, if my pull request is closed, i will upload my own implementation.
4. The inference is placed in the main thread of app cause i found it became much slower when i place it on another thread( from 23ms -> 40+ms), this may cause the some UI problem like:
>I/ Skipped 802 frames!  The application may be doing too much work on its main thread.
Currently i don't know how to solve it.
5. Open your game mode or high performance mode or whatever mode that will improve your cellphone performance.
6. Images are in folder `res/drawable-v24`, c++ code in `jni`, JAVA code in `java/com/example/zym/ncnn1`

## How to use?

**First of all, download ncnn and learn how to use**

Second, go into the `jni` folder and run `ndk-build` in terminal, edit `Android.mk` and change line 6 into your own path to ncnn.

Third, debug or build this project in Android Studio.

## Show time

!(GIF)[https://github.com/zym1119/Partial-Order-Pruning-Demo/blob/master/gifhome_464x960_23s.gif]
