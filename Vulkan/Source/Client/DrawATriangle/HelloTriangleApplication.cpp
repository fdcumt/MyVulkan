#include "HelloTriangleApplication.h"
#include <vector>
#include <iostream>
#include <set>
#include <limits>

#include "FileHelper/FileHelper.h"
#include "Log/Log.h"
#include "Math/MathUtility.h"
#include "Misc/Path.h"


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

HelloTriangleApplication::HelloTriangleApplication()
{

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
    glfwSetWindowUserPointer(Window, this);
    glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);
}

void HelloTriangleApplication::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFrameBuffers();
    CreateCommandPool();
    CreateCommandPoolForCopy();
    CreateVertexBuffer();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangleApplication::CreateSurface()
{
    //check(glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) == VK_SUCCESS);
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(Window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    VKFollowLog("vkCreateWin32SurfaceKHR or use glfwCreateWindowSurface for create surface");
    check(vkCreateWin32SurfaceKHR(Instance, &createInfo, nullptr, &Surface) == VK_SUCCESS);
}

void HelloTriangleApplication::CreateLogicalDevice()
{
    FQueueFamilyIndices indices = FindQueueFamily(PhysicalDevice);
    
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos{};
    std::set<uint32> UniqueQueueFamilies = {indices.GraphicsFamily.GetValue(), indices.PresentFamily.GetValue()};

    float queuePriority = 1.0f;
    for (uint32 UniqueQueueFamily : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo DeviceQueueCreateInfo{};
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
    
    createInfo.enabledExtensionCount = PhysicalDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = PhysicalDeviceExtensions.data();
    
    createInfo.enabledLayerCount = static_cast<uint32_t>(UsedValidationLayersForVulkanInstance.size());
    createInfo.ppEnabledLayerNames = UsedValidationLayersForVulkanInstance.data();

    VKFollowLog("vkCreateDevice Create Vulkan Logic Device by physical device");
    VkResult Result = vkCreateDevice(PhysicalDevice, &createInfo, nullptr, &LogicDevice);
    check(Result==VK_SUCCESS);

    VKFollowLog("vkGetDeviceQueue for GraphicsQueue");
    vkGetDeviceQueue(LogicDevice, indices.GraphicsFamily.ValueRef(), 0, &GraphicsQueue);

    VKFollowLog("vkGetDeviceQueue for PresentQueue");
    vkGetDeviceQueue(LogicDevice, indices.PresentFamily.ValueRef(), 0, &PresentQueue);
}

void HelloTriangleApplication::CreateSwapChain()
{
    FSwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

    uint32 ImageCount = SwapChainSupport.Capabilities.minImageCount+1;
    if (SwapChainSupport.Capabilities.maxImageCount>0 && ImageCount>SwapChainSupport.Capabilities.maxImageCount)
    {
        ImageCount = SwapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Surface;
    createInfo.minImageCount = ImageCount;
    SwapChainImageFormat = createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    SwapChainExtent = createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    FQueueFamilyIndices indices = FindQueueFamily(PhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.GraphicsFamily.GetValue(), indices.PresentFamily.GetValue()};

    if (indices.GraphicsFamily != indices.PresentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    
    createInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    VKFollowLog("vkCreateSwapchainKHR");
    check(vkCreateSwapchainKHR(LogicDevice, &createInfo, nullptr, &SwapChain) == VK_SUCCESS);

    // retrieve the swap chain image list
    VKFollowLog("RetrievingTheSwapChainImageList");
    uint32 RetrieveImageCount = 0;
    vkGetSwapchainImagesKHR(LogicDevice, SwapChain, &RetrieveImageCount, nullptr);
    SwapChainImages.resize(RetrieveImageCount);
    vkGetSwapchainImagesKHR(LogicDevice, SwapChain, &RetrieveImageCount, SwapChainImages.data());
    DebugLog("retrieve image num[%u]", RetrieveImageCount);
}

void HelloTriangleApplication::CreateImageViews()
{
    SwapChainImageViews.resize(SwapChainImages.size());

    for (size_t i = 0; i < SwapChainImageViews.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = SwapChainImages[i];
        
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = SwapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        VKFollowLog("vkCreateImageView for Image[%d]", i);
        check(vkCreateImageView(LogicDevice, &createInfo, nullptr, &SwapChainImageViews[i]) == VK_SUCCESS);
    }
}

void HelloTriangleApplication::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 用于指定在RenderPass开始之前, 图片的Layout
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 在RenderPass之后, 图片格式转变成哪种layout
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    // 当subpass开始时, 会自动将Attachment格式转换成该layout
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    // 未来可能还会支持compute类型的pipeline, 所以, 这里必须指定为图形类型.
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    check(vkCreateRenderPass(LogicDevice, &renderPassInfo, nullptr, &RenderPass) == VK_SUCCESS)
}

void HelloTriangleApplication::CreateGraphicsPipeline()
{
    VKFollowLog("CreateGraphicsPipeline begin");

    std::vector<char> VertexShaderData = FFileHelper::ReadFile(FPath::GetShaderSpvDir()+"VertexShader.spv");
    std::vector<char> FragmentShaderData = FFileHelper::ReadFile(FPath::GetShaderSpvDir()+"FragmentShader.spv");

    VKFollowLog("CreateShaderStage for Vertex shader(CreateShaderModule)");
    VkShaderModule VertexShaderModule = CreateShaderModule(VertexShaderData);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = VertexShaderModule;
    vertShaderStageInfo.pName = "main";

    VKFollowLog("CreateShaderStage for Fragment shader(CreateShaderModule)");
    VkShaderModule FragmentShaderModule = CreateShaderModule(FragmentShaderData);
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = FragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VKFollowLog("fill VkPipelineVertexInputStateCreateInfo");

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VKFollowLog("VkPipelineInputAssemblyStateCreateInfo for create input assembly");
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VKFollowLog("VkPipelineViewportStateCreateInfo for create viewport");
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VKFollowLog("VkPipelineRasterizationStateCreateInfo for create rasterizer");
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VKFollowLog("VkPipelineMultisampleStateCreateInfo for create multisample");
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VKFollowLog("VkPipelineColorBlendAttachmentState for create color blend attachment");
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VKFollowLog("fill VkPipelineColorBlendAttachmentState");
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VKFollowLog("fill VkPipelineDynamicStateCreateInfo");
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VKFollowLog("fill VkPipelineLayoutCreateInfo");
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VKFollowLog("vkCreatePipelineLayout");
    check(vkCreatePipelineLayout(LogicDevice, &pipelineLayoutInfo, nullptr, &PipelineLayout) == VK_SUCCESS);

    VKFollowLog("fill VkGraphicsPipelineCreateInfo");
    VkGraphicsPipelineCreateInfo GraphicPipelineCreateInfo{};
    GraphicPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    GraphicPipelineCreateInfo.stageCount = 2;
    GraphicPipelineCreateInfo.pStages = shaderStages;
    GraphicPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    GraphicPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    GraphicPipelineCreateInfo.pViewportState = &viewportState;
    GraphicPipelineCreateInfo.pRasterizationState = &rasterizer;
    GraphicPipelineCreateInfo.pMultisampleState = &multisampling;
    GraphicPipelineCreateInfo.pColorBlendState = &colorBlending;
    GraphicPipelineCreateInfo.pDynamicState = &dynamicState;
    GraphicPipelineCreateInfo.layout = PipelineLayout;
    GraphicPipelineCreateInfo.renderPass = RenderPass;
    GraphicPipelineCreateInfo.subpass = 0;
    GraphicPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    GraphicPipelineCreateInfo.basePipelineIndex = -1; // Optional
    
    check(vkCreateGraphicsPipelines(LogicDevice, VK_NULL_HANDLE, 1, &GraphicPipelineCreateInfo, nullptr, &GraphicsPipeline) == VK_SUCCESS);
    
    vkDestroyShaderModule(LogicDevice, VertexShaderModule, nullptr);
    vkDestroyShaderModule(LogicDevice, FragmentShaderModule, nullptr);
    VKFollowLog("CreateGraphicsPipeline end");
}

void HelloTriangleApplication::CreateFrameBuffers()
{
    SwapChainFramebuffers.resize(SwapChainImageViews.size());

    // 
    for (size_t i = 0; i < SwapChainImageViews.size(); i++)
    {
        VkImageView attachments[] =
        {
            SwapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = SwapChainExtent.width;
        framebufferInfo.height = SwapChainExtent.height;
        framebufferInfo.layers = 1;
        
        VKFollowLog("vkCreateFramebuffer for image %d", i);
        check(vkCreateFramebuffer(LogicDevice, &framebufferInfo, nullptr, &SwapChainFramebuffers[i]) == VK_SUCCESS);
    }
}

void HelloTriangleApplication::CreateSyncObjects()
{
    ImageAvailableSemaphores.resize(MaxFrameInFlight);
    RenderFinishedSemaphores.resize(MaxFrameInFlight);
    InFlightFences.resize(MaxFrameInFlight);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MaxFrameInFlight; i++)
    {
        check(vkCreateSemaphore(LogicDevice, &semaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) == VK_SUCCESS);
        check(vkCreateSemaphore(LogicDevice, &semaphoreInfo, nullptr, &RenderFinishedSemaphores[i]) == VK_SUCCESS);
        check(vkCreateFence(LogicDevice, &fenceInfo, nullptr, &InFlightFences[i]) == VK_SUCCESS);
    }
}

void HelloTriangleApplication::CleanupSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(Window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(Window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(LogicDevice);
    
    for (auto framebuffer : SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(LogicDevice, framebuffer, nullptr);
    }
    SwapChainFramebuffers.clear();

    VKFollowLog("vkDestroyImageView");
    for (auto imageView : SwapChainImageViews)
    {
        vkDestroyImageView(LogicDevice, imageView, nullptr);
    }
    SwapChainImageViews.clear();

    VKFollowLog("vkDestroySwapchainKHR");
    vkDestroySwapchainKHR(LogicDevice, SwapChain, nullptr);
}

void HelloTriangleApplication::RecreateSwapChain()
{
    vkDeviceWaitIdle(LogicDevice);
    
    CleanupSwapChain();
    
    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
}

void HelloTriangleApplication::DrawFrame()
{
    vkWaitForFences(LogicDevice, 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    // 这里将timeout设置为UINT64_MAX, 表明需要一直等, 直到成功获取Image为止.
    VkResult result = vkAcquireNextImageKHR(LogicDevice, SwapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result==VK_ERROR_OUT_OF_DATE_KHR)
    {
        bFramebufferResized = false;
        RecreateSwapChain();
        return ;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(LogicDevice, 1, &InFlightFences[CurrentFrame]);
    
    vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0);

    RecordCommandBuffer(CommandBuffers[CurrentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // 在GPU端, 等待ImageAvailableSemaphore signaled, 即已经从SwapChain中请求到一张img, 然后执行Commandbuffer.
    VkSemaphore waitSemaphores[] = {ImageAvailableSemaphores[CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];

    // 等待CommandBuffer执行完毕, 触发RenderFinishedSemaphore
    VkSemaphore signalSemaphores[] = {RenderFinishedSemaphores[CurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    check(vkQueueSubmit(GraphicsQueue, 1, &submitInfo, InFlightFences[CurrentFrame]) == VK_SUCCESS);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {SwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(PresentQueue, &presentInfo);
    if (result==VK_ERROR_OUT_OF_DATE_KHR)
    {
        bFramebufferResized = false;
        RecreateSwapChain();
        return ;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    CurrentFrame = (CurrentFrame + 1) % MaxFrameInFlight;
}

void HelloTriangleApplication::CreateCommandPool()
{
    FQueueFamilyIndices queueFamilyIndices = FindQueueFamily(PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.ValueRef();
    
    VKFollowLog("vkCreateCommandPool");
    check(vkCreateCommandPool(LogicDevice, &poolInfo, nullptr, &CommandPool) == VK_SUCCESS);
}

void HelloTriangleApplication::CreateCommandPoolForCopy()
{
    FQueueFamilyIndices queueFamilyIndices = FindQueueFamily(PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.ValueRef();
    
    VKFollowLog("vkCreateCommandPool for copy");
    check(vkCreateCommandPool(LogicDevice, &poolInfo, nullptr, &CommandPoolForCopy) == VK_SUCCESS);
}

void HelloTriangleApplication::CreateCommandBuffers()
{
    CommandBuffers.resize(MaxFrameInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32)CommandBuffers.size();

    check(vkAllocateCommandBuffers(LogicDevice, &allocInfo, CommandBuffers.data()) == VK_SUCCESS);
}

VkShaderModule HelloTriangleApplication::CreateShaderModule(const std::vector<char>& Code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = Code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());
    VkShaderModule shaderModule;
    check(vkCreateShaderModule(LogicDevice, &createInfo, nullptr, &shaderModule) == VK_SUCCESS);
    return shaderModule;
}

FSwapChainSupportDetails HelloTriangleApplication::QuerySwapChainSupport(VkPhysicalDevice InPhysicalDevice)
{
    FSwapChainSupportDetails Details;

    VKFollowLog("vkGetPhysicalDeviceSurfaceCapabilitiesKHR for get surface capability");
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InPhysicalDevice, Surface, &Details.Capabilities);

    // fill the format
    VKFollowLog("vkGetPhysicalDeviceSurfaceCapabilitiesKHR for get surface formats");
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, Surface, &formatCount, nullptr);
    check(formatCount != 0);
    Details.Formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, Surface, &formatCount, Details.Formats.data());
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, Surface, &presentModeCount, nullptr);
    check(presentModeCount != 0);
    Details.PresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, Surface, &presentModeCount, Details.PresentModes.data());
    
    return Details;
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InAvailableFormats)
{
    VKFollowLog("Choose swap surface format");
    for (const auto& AvailableFormat : InAvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return AvailableFormat;
        }
    }

    return InAvailableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& InAvailablePresentModes)
{
    for (const auto& PresentMode : InAvailablePresentModes)
    {
        if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
}

void HelloTriangleApplication::RecordCommandBuffer(VkCommandBuffer InCommandBuffer, uint32 InImageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    //VKFollowLog("vkBeginCommandBuffer");
    check(vkBeginCommandBuffer(InCommandBuffer, &beginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = RenderPass;
    renderPassInfo.framebuffer = SwapChainFramebuffers[InImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = SwapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //VKFollowLog("vkCmdBeginRenderPass");
    vkCmdBeginRenderPass(InCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        //VKFollowLog("vkCmdBindPipeline");
        vkCmdBindPipeline(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(SwapChainExtent.width);
        viewport.height = static_cast<float>(SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        //VKFollowLog("vkCmdSetViewport");
        vkCmdSetViewport(InCommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = SwapChainExtent;
        //VKFollowLog("vkCmdSetScissor");
        vkCmdSetScissor(InCommandBuffer, 0, 1, &scissor);

        //VKFollowLog("bind vertex buffer");
        VkBuffer vertexBuffers[] = {VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(InCommandBuffer, 0, 1, vertexBuffers, offsets);
        
        //VKFollowLog("vkCmdDraw");
        vkCmdDraw(InCommandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }
    //VKFollowLog("vkCmdEndRenderPass");
    vkCmdEndRenderPass(InCommandBuffer);

    //VKFollowLog("vkEndCommandBuffer");
    check(vkEndCommandBuffer(InCommandBuffer) == VK_SUCCESS);
}

void HelloTriangleApplication::CreateBuffer(VkDeviceSize InBufferSize, VkBufferUsageFlags InBufferUsage,
    VkMemoryPropertyFlags properties, VkBuffer& OutBuffer, VkDeviceMemory& OutBufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = InBufferSize;
    bufferInfo.usage = InBufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VKFollowLog("vkCreateBuffer")
    check(vkCreateBuffer(LogicDevice, &bufferInfo, nullptr, &OutBuffer) == VK_SUCCESS);
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(LogicDevice, OutBuffer, &memRequirements);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = memRequirements.size;
    AllocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
    check(vkAllocateMemory(LogicDevice, &AllocInfo, nullptr, &OutBufferMemory) == VK_SUCCESS);
    
    // 为buffer指定内存, 在Vulkan中的操作为绑定.
    vkBindBufferMemory(LogicDevice, OutBuffer, OutBufferMemory, 0);
}

void HelloTriangleApplication::CreateVertexBuffer()
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize BufferSize = sizeof(vertices[0]) * vertices.size();
    
    CreateBuffer( BufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    
    // 将Vertex数据拷贝到VertexBufferMemory中
    void* data;
    vkMapMemory(LogicDevice, stagingBufferMemory, 0, BufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) BufferSize);
    vkUnmapMemory(LogicDevice, stagingBufferMemory);

    CreateBuffer(BufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VertexBuffer, VertexBufferMemory);
    
    CopyBuffer(stagingBuffer, VertexBuffer, BufferSize);
    
    vkDestroyBuffer(LogicDevice, stagingBuffer, nullptr);
    vkFreeMemory(LogicDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    // 创建一个专门用于GPU内部拷贝数据的buffer(从GPU一块内存的数据, 拷贝到另一块内存上.)
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = CommandPoolForCopy;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(LogicDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 只使用一次该CmdBuffer

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    {
        // copy
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    }
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    // 等待GraphicQueue执行完
    vkQueueWaitIdle(GraphicsQueue);

    vkFreeCommandBuffers(LogicDevice, CommandPoolForCopy, 1, &commandBuffer);
    
}

uint32 HelloTriangleApplication::FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0;
    
}

VkExtent2D HelloTriangleApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& InCapabilities)
{
    if (InCapabilities.currentExtent.width != (std::numeric_limits<uint32>::max)())
    {
        return InCapabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(Window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = FMath::Clamp(actualExtent.width, InCapabilities.minImageExtent.width, InCapabilities.maxImageExtent.width);
        actualExtent.height = FMath::Clamp(actualExtent.height, InCapabilities.minImageExtent.height, InCapabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void HelloTriangleApplication::PickPhysicalDevice()
{
    VKFollowLog("vkEnumeratePhysicalDevices for check PhysicalDevice Valid");
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
    check(DeviceCount != 0);

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());
    for (VkPhysicalDevice Device : Devices)
    {
        if (IsPhysicalDeviceSuitable(Device))
        {
            PhysicalDevice = Device;
            break;
        }
    }

    check(PhysicalDevice != VK_NULL_HANDLE);
    PrintSelectedVulkanPhysicalDevice();
    PrintVulkanPhysicalDevicesInThisComputer();
    PrintVulkanPhysicalDeviceExtensionSupports();
}

bool HelloTriangleApplication::IsPhysicalDeviceSuitable(VkPhysicalDevice InDevice)
{
    bool CheckResult = FindQueueFamily(InDevice).IsComplete() &&
            CheckDeviceExtensionSupport(InDevice) &&
            QuerySwapChainSupport(InDevice).IsValid();
    
    return CheckResult;
}

int HelloTriangleApplication::CalDeviceScore(VkPhysicalDevice InDevice)
{
    int score = 0;
    VkPhysicalDeviceProperties DeviceProperties;
    VkPhysicalDeviceFeatures DeviceFeatures;
    vkGetPhysicalDeviceProperties(InDevice, &DeviceProperties);
    vkGetPhysicalDeviceFeatures(InDevice, &DeviceFeatures);
    VKFollowLog("vkGetPhysicalDeviceProperties and vkGetPhysicalDeviceFeatures for CalDeviceScore");
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

bool HelloTriangleApplication::CheckDeviceExtensionSupport(VkPhysicalDevice InDevice)
{
    VKFollowLog("CheckDeviceExtensionSupport by vkEnumerateDeviceExtensionProperties");
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &extensionCount, availableExtensions.data());
    
    std::set<std::string> requiredExtensions(PhysicalDeviceExtensions.begin(), PhysicalDeviceExtensions.end());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    check(requiredExtensions.empty());
    return requiredExtensions.empty();
}

void HelloTriangleApplication::MainLoop()
{
    while (!glfwWindowShouldClose(Window))
    {
        glfwPollEvents();
        DrawFrame();
    }

    // 等待Device空闲了, 才能释放各种资源, 例如Semaphores
    vkDeviceWaitIdle(LogicDevice);
}

void HelloTriangleApplication::Cleanup()
{
    CleanupSwapChain();

    { // pipeline and render pass
        VKFollowLog("vkDestroyPipeline");
        vkDestroyPipeline(LogicDevice, GraphicsPipeline, nullptr);
        VKFollowLog("vkDestroyPipelineLayout");
        vkDestroyPipelineLayout(LogicDevice, PipelineLayout, nullptr);
        VKFollowLog("vkDestroyRenderPass");
        vkDestroyRenderPass(LogicDevice, RenderPass, nullptr);
    }

    { // buffer and memory
        VKFollowLog("vkDestroyBuffer-VertexBuffer");
        vkDestroyBuffer(LogicDevice, VertexBuffer, nullptr);
        VKFollowLog("vkFreeMemory-VertexBufferMemory");
        vkFreeMemory(LogicDevice, VertexBufferMemory, nullptr);
    }

    // semaphone and fence
    for (size_t i = 0; i < MaxFrameInFlight; i++)
    {
        vkDestroySemaphore(LogicDevice, ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(LogicDevice, RenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(LogicDevice, InFlightFences[i], nullptr);
    }


    // CommandPool销毁时会自动销毁其内部的Commandbuffer, 所以不用再处理CommandBuffer.
    VKFollowLog("vkDestroyCommandPool");
    vkDestroyCommandPool(LogicDevice, CommandPool, nullptr);

    // CommandPool销毁时会自动销毁其内部的Commandbuffer, 所以不用再处理CommandBuffer.
    VKFollowLog("vkDestroyCommandPool For Copy");
    vkDestroyCommandPool(LogicDevice, CommandPoolForCopy, nullptr);

    VKFollowLog("vkDestroyDevice");
    vkDestroyDevice(LogicDevice, nullptr);
    
    DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    
    VKFollowLog("vkDestroySurfaceKHR");
    vkDestroySurfaceKHR(Instance, Surface, nullptr);

    VKFollowLog("vkDestroyInstance");
    vkDestroyInstance(Instance, nullptr);
    PhysicalDevice = VK_NULL_HANDLE;

    VKFollowLog("glfwDestroyWindow");
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
    createInfo.enabledLayerCount = static_cast<uint32>(UsedValidationLayersForVulkanInstance.size());
    createInfo.ppEnabledLayerNames = UsedValidationLayersForVulkanInstance.data();

    VKFollowLog("vkCreateInstance");
    check(vkCreateInstance(&createInfo, nullptr, &Instance) == VK_SUCCESS);
    PrintVulkanInstanceExtensionSupports();
}

void HelloTriangleApplication::PrintVulkanInstanceExtensionSupports()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    VKFollowLog("vkEnumerateInstanceExtensionProperties print vulkan instance extension support");
    DebugLog("available vulkan instance extensions:");
    for (const auto& extension : extensions)
    {
        DebugLog("    extension:%s", extension.extensionName);
    }
}

void HelloTriangleApplication::PrintSelectedVulkanPhysicalDevice()
{
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(PhysicalDevice, &deviceProperties);
    DebugLog("SelectedPhysicalDeviceProperties---deviceName[%s] deviceType[%lu] vendorID[%lu] deviceID[%lu] apiVersion[%lu]",
        deviceProperties.deviceName,
        deviceProperties.deviceType,
        deviceProperties.vendorID,
        deviceProperties.deviceID,
        deviceProperties.apiVersion);
}

void HelloTriangleApplication::PrintVulkanPhysicalDevicesInThisComputer()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

    for (VkPhysicalDevice Device : Devices)
    {
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties(Device, &deviceProperties);
        DebugLog("PhysicalDeviceProperties---deviceName[%s] deviceType[%lu] vendorID[%lu] deviceID[%lu] apiVersion[%lu]",
            deviceProperties.deviceName,
            deviceProperties.deviceType,
            deviceProperties.vendorID,
            deviceProperties.deviceID,
            deviceProperties.apiVersion);
    }
    
}

void HelloTriangleApplication::PrintVulkanPhysicalDeviceExtensionSupports()
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &extensionCount, extensions.data());
    VKFollowLog("vkEnumerateDeviceExtensionProperties print vulkan device extension support");
    DebugLog("available vulkan physical device extensions:");
    for (const auto& extension : extensions)
    {
        DebugLog("    extension:%s", extension.extensionName);
    }
}

bool HelloTriangleApplication::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    AvailableLayers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, AvailableLayers.data());
    VKFollowLog("vkEnumerateInstanceLayerProperties for check Validation layer support");
    
    bool bAllSupport = true;
    for (char* LayerName : UsedValidationLayersForVulkanInstance)
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
    VKFollowLog("vkGetInstanceProcAddr for CreateDebugUtilsMessengerEXT");

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


    VKFollowLog("vkGetInstanceProcAddr for SetupDebugMessenger");

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
            VKFollowLog("vkGetPhysicalDeviceQueueFamilyProperties for find queue family which support graphic");
            Indices.GraphicsFamily = i;
        }

        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, i, Surface, &PresentSupport);
        VKFollowLog("vkGetPhysicalDeviceSurfaceSupportKHR for find queue family which support surface");

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

void HelloTriangleApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->bFramebufferResized = true;
}

std::vector<const char*> HelloTriangleApplication::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}
