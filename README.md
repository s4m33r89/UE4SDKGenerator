## UE4-SDK-GENERATOR for Android
UnrealEngine-4 Sdk Generator for Android Devices.
* Generated SDKs [HERE](https://github.com/D-R-99/UE4SDKGenerator/tree/master/SDKs/)
* This Tool is just Converted for Android from @KN4CK3R 's UE4SDKGen.
* Currently Tested on 64Bit Version of BGMI, But can support other games that have EngineVersion from 4.18 to 4.22.

## How to Use:

* Add Apk's pkgname and other details in Main.h.
* Add GName and GObject Offsets in Main.h.
* Change Files in Target Dir according to Game's structure and replace that files in Source Dir.
* Compile all files in the Source Directory (x64 Release).
   * Comiple With AIDE(64bit NDK Support)  -OR-
   * Compile With AndroidStudio (click on ndk.cmd file and RUN, if u have cmd execute plugin Installed.)
* Add Lib in Apk. And add this Smali code to Load Lib

    ```
    const-string v0, "native-lib"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
    ```

* Make sure Apk have Write Permission in media folder.
* SDK will be Generated in /sdcard/Android/media/PKG-NAME folder.

## Notes

* It will Start generating sdk after 5 second delay, at that time maybe not all objects loaded, so less files will be generated, Increase Delay in Main.cpp to Generate SDK in Lobby.
* Generator.log This file contains the log messages the generator outputs. NamesDump.txt This file is generated if ShouldDumpArrays() is true and it contains all names available in the names array. ObjectsDump.txt This file is generated if ShouldDumpArrays() is true and it contains all objects names available in the objects array. SDK.hpp This file contains all includes you need for the SDK.
* Most of the time you don't need all the cpp files. Add Needed Headers in SDK.hpp.

## ToDo

* Adding Pattern scan for ProcessEvent.
* Adding Missing Field like "ComponentToWorld".
* Fix FunctionFlags not dumped in xxx_functions.cpp.

if you have a suggestion on how to improve this sdk, please open an issue.
