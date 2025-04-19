#pragma once

#include "core/Prelude.hpp"

#if defined(GEODE_EXPOSE_SECRET_INTERNALS_IN_HEADERS_DO_NOT_DEFINE_PLEASE)
    #if !defined(__clang__)
        #error Geode Loader only compiles with Clang.
    #endif 
#endif

#include "Loader.hpp"
#include "Utils.hpp"
#include "modify/Modify.hpp"