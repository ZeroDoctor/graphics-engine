#pragma once

#include "build_order.hpp"
#include "queue_family.hpp"
#include "physical_device.hpp"
#include "device.hpp"

class VulkanSwapChain {
public:
    VulkanSwapChain(VulkanInstance*, VulkanPhysicalDevice*, VulkanDevice*, VulkanSurface*, uint32_t, uint32_t);
    ~VulkanSwapChain();

    VkSwapchainKHR GetSwapchain();
    VkFormat GetFormat();
    VkExtent2D GetExtent();
    std::vector<VkImage> GetImages();
    std::vector<VkImageView> GetImageViews();
    
private:
    VkSwapchainKHR m_swapchain=nullptr;
    VkFormat m_swapchain_image_format;
    VkExtent2D m_swapchain_extent;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_views;
    VulkanDevice* m_device;

    void createSwapChainImageViews(VkDevice);
    SwapChainSupportDetails createSwapChainSupport(VkPhysicalDevice, VkSurfaceKHR);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&, uint32_t*, uint32_t*);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR>&);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>&);
};
