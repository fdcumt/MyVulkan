#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication
{
public:
    void run();

private:
    GLFWwindow* window = nullptr;
    VkInstance Instance;

    std::vector<VkLayerProperties> AvailableLayers;
    std::vector<char*> UsedValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkDebugUtilsMessengerEXT DebugMessenger;

private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();

    std::vector<const char*> GetRequiredExtensions();
    void PrintExtensionSupport();
    
    bool CheckValidationLayerSupport();

    void SetupDebugMessenger();
    
    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};

