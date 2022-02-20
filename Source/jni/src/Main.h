#pragma once

#ifndef DYNO_H
#define DYNO_H

#include "Includes.h"
using namespace std;

//static ofstream file1;
static std::string Media_Folder;

static uintptr_t UE4 = 0;
static char* lib_name = "libUE4.so";
static std::string pkgName = "com.pubg.imobile";

static std::string gameVersion = "1.8.0";
static std::string gameShortName = "BGMI";
static std::string gameFullName = "Battlegrounds Mobile India";


#if defined(__LP64__)
//static uintptr_t GWorld_Offset = 0;
static uintptr_t GNames_Offset = 0;
static uintptr_t GUObjectArray_Offset =  0;
#else
//static uintptr_t GWorld_Offset = 0;
static uintptr_t GNames_Offset = 0;
static uintptr_t GUObjectArray_Offset = 0;
#endif

// Change this if dereferenced
static uintptr_t getTNameArrayAddr() {
     uintptr_t GNameAddr = *(uintptr_t*) (UE4 + GNames_Offset);
     return GNameAddr;
}

static uintptr_t getFUObjecrtArrayAddr() {
    uintptr_t GObjectAddr = UE4 + GUObjectArray_Offset;
    return GObjectAddr;
}

#endif
