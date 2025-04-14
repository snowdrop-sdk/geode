#pragma once

#include "CCPlatformConfig.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    #include "third_party/win32/libtiff/tiffio.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    // nothing?
    #include <tiffio.h>
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID && __arm__)
    #include "third_party/android/prebuilt/libtiff/include/tiffio.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID && __aarch64__)
    #include "third_party/android64/prebuilt/libtiff/include/tiffio.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    // nothing?
    #include <tiffio.h>
#endif

