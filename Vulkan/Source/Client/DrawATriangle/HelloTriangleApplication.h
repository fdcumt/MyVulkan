#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vector>
#include "Misc/Optional.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct QueueFamilyIndices
{
    TOptional<uint32_t> GraphicsFamily;

    bool IsComplete() const
    {
        return GraphicsFamily.IsSet();
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
    };

    VkDebugUtilsMessengerEXT DebugMessenger;

    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

private:
    void initWindow();

    void initVulkan();
    void PickPhysicalDevice();
    bool IsDeviceSuitable(VkPhysicalDevice InDevice);
    int CalDeviceScore(VkPhysicalDevice InDevice);

    void mainLoop();
    void cleanup();
    void CreateInstance();

    std::vector<const char*> GetRequiredExtensions();
    void PrintExtensionSupport();

    bool CheckValidationLayerSupport();

    void SetupDebugMessenger();

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    
};
