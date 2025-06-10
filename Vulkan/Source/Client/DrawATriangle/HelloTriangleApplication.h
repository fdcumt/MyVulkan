#pragma once


#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include "Misc/Optional.h"
#include "glm/glm.hpp"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static const std::array<VkVertexInputAttributeDescription, 2>& getAttributeDescriptions() {
        static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
};

// const std::vector<Vertex> vertices = {
//     //{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//     //{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//     //{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//     {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
//  {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//  {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
// };

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> Indices = {
    0, 1, 2, 2, 3, 0
};

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
    HelloTriangleApplication();

public:
    void run();

private:
    GLFWwindow* Window = nullptr;
    VkInstance Instance;

    static constexpr int32 MaxFrameInFlight = 2;

    uint32_t CurrentFrame = 0;
    bool bFramebufferResized = false;

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

    std::vector<VkFramebuffer> SwapChainFramebuffers;
    
    VkRenderPass RenderPass;
    VkPipelineLayout PipelineLayout;

    VkPipeline GraphicsPipeline;

    VkBuffer VertexBuffer;
    VkDeviceMemory VertexBufferMemory;

    VkBuffer IndexBuffer;
    VkDeviceMemory IndexBufferMemory;
    
    std::vector<VkCommandBuffer> CommandBuffers;
    VkCommandPool CommandPool;
    VkCommandPool CommandPoolForCopy;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkFence> InFlightFences;
    
private:
    void InitWindow();
    void InitVulkan();
    void CreateSurface();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFrameBuffers();
    void CreateSyncObjects();

    void CleanupSwapChain();
    void RecreateSwapChain();
    
    void DrawFrame();
    
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    
    // for swap chain
    FSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice InPhysicalDevice);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InAvailableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& InAvailablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& InCapabilities);
    
    // for command buffer
    void CreateCommandPool();
    void CreateCommandPoolForCopy();
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32 InImageIndex);

    void CreateBuffer(VkDeviceSize InBufferSize, VkBufferUsageFlags InBufferUsage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CreateVertexBuffer();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void CreateIndexBuffer();
    
    uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) ;
    
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

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    
};


