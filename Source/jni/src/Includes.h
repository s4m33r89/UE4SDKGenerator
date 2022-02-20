#ifndef INCLUDES_H
#define INCLUDES_H

#include <jni.h>
#include <string>
#include <cstdlib>
#include <math.h>
#include <cstdio>
#include <cstddef>
//#include <cmath>
#include <ctime>
#include <stack>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <android/log.h>
#include <pthread.h>
#include <dirent.h>
#include <list>
#include <libgen.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <codecvt>
#include <chrono>
#include <queue>
#include <memory.h>
#include <dlfcn.h>
#include <unordered_set>
#include <unordered_map>
#include <bitset>

enum daLogType {
    daDEBUG = 3,
    daERROR = 6,
    daINFO  = 4,
    daWARN  = 5
};

//Change this to another Log Tag if ya want. IN the batch script I provide you change the log tag then too
#define TAG "Dyno-SDKGen"

#define LOGD(...) ((void)__android_log_print(daDEBUG, TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(daERROR, TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(daINFO,  TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(daWARN,  TAG, __VA_ARGS__))


#endif
