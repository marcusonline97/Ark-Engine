#pragma once
#include "../../Common/ArkEnums.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace VulkanBackEnd {
    // Core
    bool Init();
    void BeginFrame();
    void EndFrame();
    void Cleanup();

    // Instance and Device
    VkInstance GetInstance();
    VkDevice GetDevice();
    VkPhysicalDevice GetPhysicalDevice();
    VkQueue GetGraphicsQueue();
    uint32_t GetGraphicsQueueFamily();

    // Swapchain
    VkSwapchainKHR GetSwapchain();
    std::vector<VkImage> GetSwapchainImages();
    std::vector<VkImageView> GetSwapchainImageViews();
    VkFormat GetSwapchainImageFormat();
    VkExtent2D GetSwapchainExtent();

    // Command Pool and Buffers
    VkCommandPool GetCommandPool();
    std::vector<VkCommandBuffer> GetCommandBuffers();

    // Synchronization
    std::vector<VkSemaphore> GetImageAvailableSemaphores();
    std::vector<VkSemaphore> GetRenderFinishedSemaphores();
    std::vector<VkFence> GetInFlightFences();

    // Render Pass and Framebuffers
    VkRenderPass GetRenderPass();
    std::vector<VkFramebuffer> GetSwapchainFramebuffers();

    // Current frame index
    uint32_t GetCurrentFrame();
}
