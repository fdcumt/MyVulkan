#include "HelloTriangleApplication.h"
#include <iostream>
#include "Log/Log.h"



int main()
{
    InitLog();
    HelloTriangleApplication app;
    
    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        CriticalLog("Run Error:%s", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
