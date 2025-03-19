#include "FileHelper/FileHelper.h"

#include <fstream>

#include "Log/Log.h"


std::vector<char> FFileHelper::ReadFile(std::string FileName)
{
    // 二进制文件的打开方式一定是binary, 否则读取错误.
    std::ifstream file(FileName, std::ios::binary);
    if (!file.is_open())
    {
        ErrorLog("File[%s] does not exist", FileName.data());
        return std::vector<char>();
    }
    
    std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
    //std::vector<char> FileData;
    //file.seekg(0, std::ios::end);
    //size_t fileSize = (size_t) file.tellg();
    //FileData.resize(fileSize);
    //file.seekg(0);
    //file.read(FileData.data(), fileSize);
    
    file.close();

    return buffer;
}
