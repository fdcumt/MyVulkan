#pragma once
#include "GenericPlatform/GenericPlatform.h"
#include <cassert>

#define PLATFORM_BREAK() (__nop(), __debugbreak())
#define check(Expression) \
    if (!(Expression)) \
    {\
        PLATFORM_BREAK();\
    }


