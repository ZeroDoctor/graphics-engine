#pragma once

#include "build_order.hpp"
#include "device.hpp"

class VulkanImageView {

public:
    VulkanImageView(VulkanDevice*);
    ~VulkanImageView();
    std::vector<VkImage> GetImages();
    std::vector<VkDeviceMemory> GetImageMemories();
    std::vector<VkImageView> GetImageViews();
    std::vector<VkSampler> GetSamplers();
    void CreateImageView(VkImageAspectFlags*);
    void LoadImage(uint32_t, uint32_t, uint8_t*);
    void LoadImageFromFile(std::string, VkFormat);
    void GenerateImage(
        uint32_t, uint32_t, 
        VkFormat, VkImageUsageFlags,
        VkImageTiling tiling=VK_IMAGE_TILING_OPTIMAL,
        VkMemoryPropertyFlags properties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    void GenerateTextureImage(
        uint32_t, uint32_t, 
        VkFormat, VkImageUsageFlags,
        VkImageTiling tiling=VK_IMAGE_TILING_OPTIMAL,
        VkMemoryPropertyFlags properties=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    void TransitionImageLayout(
        VkCommandBuffer,VkImage, 
        VkImageLayout, VkImageLayout,
        VkPipelineStageFlagBits src_stage=ZERO_BIT, 
        VkPipelineStageFlagBits dst_stage=ZERO_BIT
    );

    void CreateTextureSampler();
    
private:
    // didn't realize it would be this many vectors
    VulkanDevice* m_device;
    std::vector<VkImageView> m_image_views;
    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_image_memories;
    std::vector<VkFormat> m_format;
    std::vector<VkSampler> m_texture_samplers;

    void createImage(
        uint32_t, uint32_t, 
        VkFormat, VkImageTiling, 
        VkImageUsageFlags, VkMemoryPropertyFlags,
        VkImage*, VkDeviceMemory*);

    void copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);

};