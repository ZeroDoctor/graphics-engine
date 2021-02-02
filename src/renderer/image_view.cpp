#include "image_view.hpp"

VulkanImageView::VulkanImageView(VulkanDevice* device)
{
    m_device = device;
}

VulkanImageView::~VulkanImageView()
{
    printfi("-- Destorying Image View...\n");
    for(auto img : m_images) {
        vkDestroyImage(m_device->GetDevice(), img, nullptr);
    }

    for(auto imgm : m_image_memories) {
        vkFreeMemory(m_device->GetDevice(), imgm, nullptr);
    }
    
    for(auto iv : m_image_views) {
        vkDestroyImageView(m_device->GetDevice(), iv, nullptr);
    }

    // Check Usage
    if(m_images.size() <= 0) {
        printfw("VkImage was not used\n");
    }
    
    if(m_image_memories.size() <= 0) {
        printfw("Memory for VkImage was not used\n");
    }

    if(m_image_views.size() <= 0) {
        printfw("VkImageView was not used\n");
    }
}

std::vector<VkImage> VulkanImageView::GetImages() { return m_images; }
std::vector<VkDeviceMemory> VulkanImageView::GetImageMemories() { return m_image_memories; }
std::vector<VkImageView> VulkanImageView::GetImageViews() { return m_image_views; }
std::vector<VkSampler> VulkanImageView::GetSamplers() { return m_texture_samplers; }

// ? Could stream line this in function LoadImage()
void VulkanImageView::CreateImageView(VkImageAspectFlags* flags)
{

    if(m_format.size() <= 0) {
        printff("Format not found create Image before ImageView\n");
    }
    
    m_image_views.resize(m_images.size());
    
    for(size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo info = init::image_view_info(m_images[i], m_format[i]);
        info.subresourceRange.aspectMask = flags[i];
        printfi("Creating %d Image View...\n", i+1);
        ErrorCheck(vkCreateImageView(
            m_device->GetDevice(),
            &info,
            nullptr,
            &m_image_views[i]
        ), "Create Image Views");
    }
}

void VulkanImageView::LoadImage(uint32_t width, uint32_t height, uint8_t* pixels) 
{
    printfi("Loading Image --> %dx%d\n", width, height);

    VkDeviceSize image_size = width * height * 4;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    m_device->CreateBuffer(
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory
    );

    void* data;
    vkMapMemory(m_device->GetDevice(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m_device->GetDevice(), staging_buffer_memory);

    VkCommandBuffer command_buffer = m_device->BeginSingleCommand();
    TransitionImageLayout(
        command_buffer, m_images[0],
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    m_device->EndSingleCommand(command_buffer);

    copyBufferToImage(
        staging_buffer, m_images[0], static_cast<uint32_t>(width), static_cast<uint32_t>(height)
    );

    command_buffer = m_device->BeginSingleCommand();
    TransitionImageLayout(
        command_buffer, m_images[0], 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    m_device->EndSingleCommand(command_buffer);

    vkDestroyBuffer(m_device->GetDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), staging_buffer_memory, nullptr);
    
}

void VulkanImageView::LoadImageFromFile(std::string path, VkFormat format)
{
    printfi("Loading Image From --> %s\n", path.c_str());
    VkImage image;
    VkDeviceMemory image_memory;
    m_format.push_back(format);

    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels) {
        printff("Failed to Load Image --> %s\n", path.c_str());
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    m_device->CreateBuffer(
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory
    );

    void* data;
    vkMapMemory(m_device->GetDevice(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m_device->GetDevice(), staging_buffer_memory);

    stbi_image_free(pixels);
    // color_format = VK_FORMAT_R8G8B8A8_SRGB
    createImage(
        static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height),
        format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &image, &image_memory
    );

    VkCommandBuffer command_buffer = m_device->BeginSingleCommand();
    TransitionImageLayout(
        command_buffer, image, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    m_device->EndSingleCommand(command_buffer);

    copyBufferToImage(
        staging_buffer, image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height)
    );

    command_buffer = m_device->BeginSingleCommand();
    TransitionImageLayout(
        command_buffer, image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    m_device->EndSingleCommand(command_buffer);

    vkDestroyBuffer(m_device->GetDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), staging_buffer_memory, nullptr);

    m_images.push_back(image);
    m_image_memories.push_back(image_memory);
}

void VulkanImageView::GenerateImage(
        uint32_t width, uint32_t height, VkFormat format, 
        VkImageUsageFlags usage, VkImageTiling tiling,
        VkMemoryPropertyFlags properties
    ) 
{
    printfi("Generating Image...\n");
    VkImage image;
    VkDeviceMemory image_memory;
    createImage(
        width, height,
        format, tiling,
        usage, properties,
        &image, &image_memory
    );

    m_format.push_back(format);
    m_images.push_back(image);
    m_image_memories.push_back(image_memory);
}

void VulkanImageView::GenerateTextureImage(
        uint32_t width, uint32_t height, VkFormat format, 
        VkImageUsageFlags usage, VkImageTiling tiling,
        VkMemoryPropertyFlags properties
    ) 
{
    printfi("Generating Texture Image...\n");
    VkImage image;
    VkDeviceMemory image_memory;
    createImage(
        width, height, format, 
        tiling, usage, properties,
        &image, &image_memory
    );

    VkCommandBuffer command = m_device->BeginSingleCommand();

    TransitionImageLayout(
        command, image, 
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    TransitionImageLayout(
        command, image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_device->EndSingleCommand(command);
}

void VulkanImageView::createImage(
        uint32_t width, uint32_t height, 
        VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage* image, VkDeviceMemory* image_memory
    )
{
    VkImageCreateInfo image_info = init::image_info(width, height, format, tiling, usage);

    ErrorCheck(vkCreateImage(
        m_device->GetDevice(),
        &image_info,
        nullptr,
        image
    ), "Create Image");

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(m_device->GetDevice(), *image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = init::memory_allocate_info(
        mem_requirements,
        m_device->FindMemoryType(mem_requirements.memoryTypeBits, properties)
    );

    ErrorCheck(vkAllocateMemory(
        m_device->GetDevice(),
        &alloc_info,
        nullptr,
        image_memory
    ), "Allocate Memory");

    ErrorCheck(vkBindImageMemory(
        m_device->GetDevice(), 
        *image, 
        *image_memory, 
        0
    ), "Bind Image to Memory");
}

void VulkanImageView::TransitionImageLayout(
        VkCommandBuffer command_buffer, VkImage image, 
        VkImageLayout old_layout, VkImageLayout new_layout,
        VkPipelineStageFlagBits src_stage, VkPipelineStageFlagBits dest_stage
    ) 
{
    // TODO: create image memory barrier in init.hpp
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    //barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
    };

    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        if(src_stage == ZERO_BIT) {
            src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }
        if(dest_stage == ZERO_BIT) {
            dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        
    } 
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        if(src_stage == ZERO_BIT) {
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        if(dest_stage == ZERO_BIT) {
            dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        if(src_stage == ZERO_BIT) {
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        if(dest_stage == ZERO_BIT) {
            dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
    } else {
        printff("Unsupported Layout Transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer, src_stage, dest_stage,
        0, 
        0, nullptr, 
        0, nullptr,
        1, &barrier
    );
}

void VulkanImageView::CreateTextureSampler() 
{
    VkSampler texture_sampler;

    VkSamplerCreateInfo sampler_info = init::sampler_info();
    ErrorCheck(vkCreateSampler(
        m_device->GetDevice(),
        &sampler_info,
        nullptr,
        &texture_sampler
    ),"Create Sampler");

    m_texture_samplers.push_back(texture_sampler);
}

void VulkanImageView::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
    VkCommandBuffer command_buffer = m_device->BeginSingleCommand();

    // TODO: create buffer image copy in init.hpp
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {
        VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1
    };
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        command_buffer, 
        buffer, 
        image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    m_device->EndSingleCommand(command_buffer);
}
