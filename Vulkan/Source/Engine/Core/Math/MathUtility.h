#pragma once
#include <string>

#include "GenericPlatform/GenericPlatform.h"

class FMath
{
public:
    
    template<typename T>
    static T Min(const T& A, const T& B)
    {
        return (A <= B) ? A : B;
    }
    
    template<typename T>
    static T Max(const T& A, const T& B)
    {
        return (A <= B) ? B : A;
    }
    
    template<typename T>
    static T Clamp(const T& Value, const T& Min, const T& Max)
    {
        return (Value <= Min) ? Min : ((Value > Max) ? Max : Value);
    }

    static constexpr unsigned int(max)() noexcept {
        return 0xffffffff;
    }
};


