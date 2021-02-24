#include "swapchain.hpp"

VulkanSwapChain::VulkanSwapChain(
        VulkanInstance* instance, VulkanPhysicalDevice* physical_device, 
        VulkanDevice* device, VulkanSurface* surface,
        uint32_t width, uint32_t height
    )
{
    m_device = device;
    SwapChainSupportDetails details = createSwapChainSupport(physical_device->GetDevice(), surface->GetSurface());
    
    VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR present_mode = chooseSwapPresentMode(details.present_modes);  
    VkExtent2D extent = chooseSwapExtent(details.capabilities, &width, &height);

    uint32_t image_count = details.capabilities.minImageCount + 1;
    if(details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR info = init::swapchain_info(surface->GetSurface(), surface_format, extent, image_count);
    if(details.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        printfi("Found capable transfer source bit\n");
        info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }


    QueueFamilyIndices indices = physical_device->GetQueueFamily();
    uint32_t queue_indices[] = {indices.graphics_index, indices.present_index};

    if(indices.graphics_index != indices.present_index) {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queue_indices;
    } else {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    info.preTransform = details.capabilities.currentTransform;
    info.presentMode = present_mode;

    ErrorCheck(vkCreateSwapchainKHR(
            device->GetDevice(), &info,
            nullptr, &m_swapchain
        ),
        "Create Swapchain"
    );

    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapchain, &image_count, nullptr);
    m_swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device->GetDevice(), m_swapchain, &image_count, m_swapchain_images.data());

    m_swapchain_image_format = surface_format.format;
    m_swapchain_extent = extent;

    createSwapChainImageViews(device->GetDevice());
}

VulkanSwapChain::~VulkanSwapChain() 
{
    for(auto image_view : m_swapchain_views) {
        vkDestroyImageView(m_device->GetDevice(), image_view, nullptr);
    }

    if(m_swapchain != nullptr) {
        vkDestroySwapchainKHR(m_device->GetDevice(), m_swapchain, nullptr);
    }
}

VkSwapchainKHR VulkanSwapChain::GetSwapchain() { return m_swapchain; }
VkFormat VulkanSwapChain::GetFormat() { return m_swapchain_image_format; }
VkExtent2D VulkanSwapChain::GetExtent() { return m_swapchain_extent; }
std::vector<VkImage> VulkanSwapChain::GetImages() { return m_swapchain_images; }
std::vector<VkImageView> VulkanSwapChain::GetImageViews() { return m_swapchain_views; }

void VulkanSwapChain::createSwapChainImageViews(VkDevice device)
{
    m_swapchain_views.resize(m_swapchain_images.size());
    for(size_t i = 0; i < m_swapchain_images.size(); i++)
    {
        VkImageViewCreateInfo info = init::image_view_info(m_swapchain_images[i], m_swapchain_image_format, true);

        ErrorCheck(
            vkCreateImageView(device, &info, nullptr, &m_swapchain_views[i]),
            "Create Swapchain Image Views"
        );
    }
}

SwapChainSupportDetails VulkanSwapChain::createSwapChainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface) 
{
    SwapChainSupportDetails details = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if(format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, 
            &format_count, details.formats.data()
        );
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if(present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, surface, 
            &present_mode_count, details.present_modes.data()
        );   
    }

    return details;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t* pwidth, uint32_t* pheight) 
{
    if(capabilities.currentExtent.width != UINT32_MAX) {
        *pwidth = capabilities.currentExtent.width;
        *pheight = capabilities.currentExtent.height;
        return capabilities.currentExtent;
    }

    uint32_t width = *pwidth;
    uint32_t height = *pheight;
    VkExtent2D actualExtent = {width, height};

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    
    return actualExtent;
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(std::vector<VkPresentModeKHR>& present_modes) 
{
    for(const auto& mode : present_modes)
    {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& surface_formats) 
{
    for(const auto& format : surface_formats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            printfi("Found desired surface format for image views\n");
            return format;
        }
    }
    return surface_formats[0];
}
