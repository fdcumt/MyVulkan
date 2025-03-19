

#include "Path.h"

std::string FPath::GetProjectDir()
{
    return std::string("../../../../");
}

std::string FPath::GetContentDir()
{
    return GetProjectDir() + "Content/";
}

std::string FPath::GetShaderSpvDir()
{
    return GetContentDir() + std::string("Shader/Spv/");
}

std::string FPath::GetLogDir()
{
    return GetProjectDir()+"Log/";
}
