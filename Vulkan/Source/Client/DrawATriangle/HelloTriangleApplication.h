#pragma once


#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include "Misc/Optional.h"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct FQueueFamilyIndices
{
    FQueueFamilyIndices() = default;
    
    FQueueFamilyIndices(const FQueueFamilyIndices& Other)
    {
        GraphicsFamily = Other.GraphicsFamily;
        PresentFamily = Other.PresentFamily;
    }
    
    TOptional<uint32> GraphicsFamily;
    TOptional<uint32> PresentFamily;

    bool IsComplete() const
    {
        return GraphicsFamily.IsSet() && PresentFamily.IsSet();
    }
};

class HelloTriangleApplication
{
public:
    void run();

private:
    GLFWwindow* Window = nullptr;
    VkInstance Instance;

    std::vector<VkLayerProperties> AvailableLayers;
    std::vector<char*> UsedValidationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
        //"VK_LAYER_LUNARG_standard_validation"
    };
    
    const std::vector<const char*> DeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    

    VkDebugUtilsMessengerEXT DebugMessenger;
    
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

    FQueueFamilyIndices QueueFamilyIndices;
    
    VkDevice LogicDevice;
    VkSurfaceKHR Surface;

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;

private:
    void InitWindow();
    void InitVulkan();
    void CreateSurface();
    void CreateLogicalDevice();

    
    void PickPhysicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice InDevice);
    int CalDeviceScore(VkPhysicalDevice InDevice);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice InDevice);

    void MainLoop();
    void Cleanup();
    void CreateInstance();

    std::vector<const char*> GetRequiredExtensions();
    void PrintExtensionSupport();

    bool CheckValidationLayerSupport();

    void SetupDebugMessenger();

    FQueueFamilyIndices FindQueueFamily(VkPhysicalDevice InDevice);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    
};
