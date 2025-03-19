#pragma once
#include <string>

class FPath
{
public:
    static std::string GetProjectDir();
    static std::string GetContentDir();
    static std::string GetShaderSpvDir();
    static std::string GetLogDir();
};


