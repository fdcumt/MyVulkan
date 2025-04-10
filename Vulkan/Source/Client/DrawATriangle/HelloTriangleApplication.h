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

struct FSwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;

    bool IsValid() const
    {
        return !Formats.empty() && !PresentModes.empty();
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
    std::vector<char*> UsedValidationLayersForVulkanInstance =
    {
        "VK_LAYER_KHRONOS_validation"
        //"VK_LAYER_LUNARG_standard_validation"
    };

    // use for check physical device support and used in create logic device
    const std::vector<const char*> PhysicalDeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    

    VkDebugUtilsMessengerEXT DebugMessenger;
    
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;

    FQueueFamilyIndices QueueFamilyIndices;
    
    VkDevice LogicDevice;
    VkSurfaceKHR Surface; // Vulkan Instance extension

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;

    VkSwapchainKHR SwapChain;
    std::vector<VkImage> SwapChainImages;
    VkFormat SwapChainImageFormat;
    VkExtent2D SwapChainExtent;
    std::vector<VkImageView> SwapChainImageViews;

    VkRenderPass RenderPass;
    VkPipelineLayout PipelineLayout;

    VkPipeline GraphicsPipeline;
    
private:
    void InitWindow();
    void InitVulkan();
    void CreateSurface();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    
    // for swap chain
    FSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice InPhysicalDevice);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InAvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& InAvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& InCapabilities);
    
    
    void PickPhysicalDevice();
    bool IsPhysicalDeviceSuitable(VkPhysicalDevice InDevice);
    int CalDeviceScore(VkPhysicalDevice InDevice);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice InDevice);

    void MainLoop();
    void Cleanup();
    void CreateInstance();

    std::vector<const char*> GetRequiredExtensions();
    void PrintVulkanInstanceExtensionSupports();
    void PrintSelectedVulkanPhysicalDevice();
    void PrintVulkanPhysicalDevicesInThisComputer();
    void PrintVulkanPhysicalDeviceExtensionSupports();

    bool CheckValidationLayerSupport();

    void SetupDebugMessenger();

    FQueueFamilyIndices FindQueueFamily(VkPhysicalDevice InDevice);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    
};
