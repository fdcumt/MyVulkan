#include "HelloTriangleApplication.h"
#include <vector>
#include <iostream>
#include <set>

#include "Log.h"

VkBool32 DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                       void* pUserData)
{
    if (messageSeverity >= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        ErrorLog("validation layer:%s", pCallbackData->pMessage);
        return VK_FALSE;
    }
    else
    {
        DebugLog("validation layer:%s", pCallbackData->pMessage);
        return VK_SUCCESS;
    }
}

void HelloTriangleApplication::run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
    system("pause");
}

void HelloTriangleApplication::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicDevice();
}

void HelloTriangleApplication::CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hwnd = glfwGetWin32Window(Window);
    CreateInfo.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(Instance, &CreateInfo, nullptr, &Surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void HelloTriangleApplication::CreateLogicDevice()
{
    FQueueFamilyIndices indices = FindQueueFamily(PhysicalDevice);

    
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos{};
    std::set<uint32> UniqueQueueFamilies = {indices.GraphicsFamily.GetValue(), indices.PresentFamily.GetValue()};

    float queuePriority = 1.0f;
    for (uint32 UniqueQueueFamily : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo DeviceQueueCreateInfo;
        DeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        DeviceQueueCreateInfo.queueFamilyIndex = UniqueQueueFamily;
        DeviceQueueCreateInfo.queueCount = 1;
        DeviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        QueueCreateInfos.push_back(DeviceQueueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = QueueCreateInfos.data();
    createInfo.queueCreateInfoCount = (uint32)QueueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = static_cast<uint32_t>(UsedValidationLayers.size());
    createInfo.ppEnabledLayerNames = UsedValidationLayers.data();

    VkResult Result = vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &LogicDevice);
    if ( Result!= VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(LogicDevice, indices.GraphicsFamily.ValueRef(), 0, &GraphicsQueue);
    vkGetDeviceQueue(LogicDevice, indices.PresentFamily.ValueRef(), 0, &PresentQueue);
}

void HelloTriangleApplication::PickPhysicalDevice()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
    if (DeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());
    for (VkPhysicalDevice Device : Devices)
    {
        if (IsDeviceSuitable(Device))
        {
            PhysicalDevice = Device;
            break;
        }
    }

    if (PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool HelloTriangleApplication::IsDeviceSuitable(VkPhysicalDevice InDevice)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(InDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(InDevice, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader;
}

int HelloTriangleApplication::CalDeviceScore(VkPhysicalDevice InDevice)
{
    int score = 0;
    VkPhysicalDeviceProperties DeviceProperties;
    VkPhysicalDeviceFeatures DeviceFeatures;
    vkGetPhysicalDeviceProperties(InDevice, &DeviceProperties);
    vkGetPhysicalDeviceFeatures(InDevice, &DeviceFeatures);

    // Discrete GPUs have a significant performance advantage
    if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += DeviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!DeviceFeatures.geometryShader)
    {
        return 0;
    }

    return score;
}

void HelloTriangleApplication::MainLoop()
{
    while (!glfwWindowShouldClose(Window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::Cleanup()
{
    DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
    PhysicalDevice = VK_NULL_HANDLE;
    glfwDestroyWindow(Window);

    glfwTerminate();
}

void HelloTriangleApplication::CreateInstance()
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
    std::vector<const char*> RequiredExtensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32>(RequiredExtensions.size());
    createInfo.ppEnabledExtensionNames = RequiredExtensions.data();

    // validation layer
    CheckValidationLayerSupport();
    createInfo.enabledLayerCount = static_cast<uint32>(UsedValidationLayers.size());
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

    DebugLog("available extensions:");
    for (const auto& extension : extensions)
    {
        DebugLog("extension:%s", extension.extensionName);
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
            WarningLog("Do not support %s", LayerName);
        }
        else
        {
            DebugLog("Support LayerName:%s", LayerName);
        }
    }

    return bAllSupport;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApplication::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    VkResult Result = VK_ERROR_EXTENSION_NOT_PRESENT;
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        Result = func(Instance, &createInfo, nullptr, &DebugMessenger);
    }

    if (Result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

FQueueFamilyIndices HelloTriangleApplication::FindQueueFamily(VkPhysicalDevice InDevice)
{
    FQueueFamilyIndices Indices;
    uint32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, QueueFamilyProperties.data());
    
    for (uint32 i=0; i<QueueFamilyCount; ++i)
    {
        if (QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Indices.GraphicsFamily = i;
        }

        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, i, Surface, &PresentSupport);

        if (PresentSupport)
        {
            Indices.PresentFamily = i;
        }

        if (Indices.IsComplete())
        {
            break;
        }
    }

    

    return Indices;
}

void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                             VkDebugUtilsMessengerEXT debugMessenger,
                                                             const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
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
