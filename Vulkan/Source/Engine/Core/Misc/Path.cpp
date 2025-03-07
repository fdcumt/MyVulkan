

#include "Path.h"

std::string FPath::GetProjectDir()
{
    return std::string("../../../../");
}

std::string FPath::GetLogDir()
{
    return GetProjectDir()+"Log/";
}
