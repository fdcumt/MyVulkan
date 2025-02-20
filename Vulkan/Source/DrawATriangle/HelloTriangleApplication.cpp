#include "HelloTriangleApplication.h"

#include <vector>

void HelloTriangleApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
    system("pause");
}

void HelloTriangleApplication::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    SetupDebugMessenger();
}

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanup()
{
    DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    vkDestroyInstance(Instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void HelloTriangleApplication::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // extensions
    std::vector<const char*> RequiredExtensions =  GetRequiredExtensions();
    createInfo.enabledExtensionCount = RequiredExtensions.size();
    createInfo.ppEnabledExtensionNames = RequiredExtensions.data();

    // validation layer
    CheckValidationLayerSupport();
    createInfo.enabledLayerCount = UsedValidationLayers.size();
    createInfo.ppEnabledLayerNames = UsedValidationLayers.data();

    if (vkCreateInstance(&createInfo, nullptr, &Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }

    PrintExtensionSupport();
}

void HelloTriangleApplication::PrintExtensionSupport()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:\n";
    for (const auto& extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

bool HelloTriangleApplication::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    AvailableLayers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, AvailableLayers.data());

    bool bAllSupport = true;
    for (char* LayerName : UsedValidationLayers)
    {
        bool LayerFound = false;
        for (const VkLayerProperties& LayerProperties : AvailableLayers)
        {
            if (strcmp(LayerProperties.layerName, LayerName) == 0)
            {
                LayerFound = true;
                break;
            }
        }

        bAllSupport = bAllSupport && LayerFound;
        if (!LayerFound)
        {
            std::cout << "Do not support " << LayerName << '\n';
        }
        else
        {
            std::cout << "Support " << LayerName << '\n';
        }
    }

    return bAllSupport;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApplication::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = HelloTriangleApplication::DebugCallback;
    
    VkResult Result = VK_ERROR_EXTENSION_NOT_PRESENT;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        Result = func(Instance, &createInfo, nullptr, &DebugMessenger);
    }
    
    if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkBool32 HelloTriangleApplication::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData)
{
    if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    else
    {
        std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE; 
    }
    
}

std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}
