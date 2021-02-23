#include "render_manager.hpp"

void RenderManager::Init(std::string app_name) 
{
    // app configurations
    VulkanConfiguration config;
    config.application_name = app_name.c_str();
    config.application_version = VK_MAKE_VERSION(1,0,0);
    
    // device setup
    m_instance = new VulkanInstance(config);
    m_physical_device = VulkanPhysicalDevice::GetPhysicalDevice(m_instance, nullptr, false);
    m_device = new VulkanDevice(m_instance, m_physical_device);
}

void RenderManager::Setup(uint32_t width, uint32_t height, VkFormat src_format) 
{
    VkFormat depth_format;
    if(!m_device->GetSupportedDepthFormat(&depth_format)) {
        printff("Could not find depth supported physical device");
    }

    // create "screen"
    m_screen_view = new VulkanImageView(m_device);
    m_depth_view = new VulkanImageView(m_device);
    m_screen_view->GenerateImage(width, height, src_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    m_depth_view->GenerateImage(width, height, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageAspectFlags flags[] = {
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
    };
    m_screen_view->CreateImageView(flags);
    m_depth_view->CreateImageView(flags);

    m_pipeline = new VulkanGraphicsPipline(
        m_device, width, height
    );

    m_width = width;
    m_height = height;
    m_src_format = src_format;
    m_depth_format = depth_format;
}

VulkanImageView* RenderManager::Draw( std::vector<Vertex> vertices, std::vector<uint16_t> indices) 
{
    // vertex buffer setup
    std::unique_ptr<VulkanVertexBuffer> vertex_buffer(new VulkanVertexBuffer(
        m_device, vertices, indices
    ));

    uint32_t command_count = 1;
    VkCommandBuffer* command = new VkCommandBuffer[command_count];
    
    // graphics pipeline
    {
        // Create Pipeline
        m_pipeline->CreateShaderModule("./../src/shader/vert.spv", "./../src/shader/frag.spv");
        m_pipeline->CreateRenderPass(m_src_format, m_depth_format, false);
        m_pipeline->CreateFrameBuffers(1, m_screen_view->GetImageViews(), &m_depth_view->GetImageViews()[0]);
        m_pipeline->CreatePipelineLayout(m_width, m_height);

        // Start Drawing to buffer 
        m_pipeline->CreateCommandBuffers(command, command_count, vertex_buffer.get());
        
        // since we are only dealing with one command buffer
        m_device->SubmitWork(command[0], m_device->GetGraphicsQueue());
        vkDeviceWaitIdle(m_device->GetDevice());
    }

    m_device->FreeComputeCommand(command, command_count);

    return copyScreen(m_screen_view->GetImages()[0]);
}

VulkanImageView* RenderManager::copyScreen(VkImage src_image) 
{
    // generate image
    VulkanImageView* output_view = new VulkanImageView(m_device);
    output_view->GenerateImage(
        m_width, m_height, m_src_format, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_IMAGE_TILING_LINEAR, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    VkCommandBuffer copy_command = m_device->BeginSingleCommand();

    output_view->TransitionImageLayout(
        copy_command, output_view->GetImages()[0], 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );

    VkImageCopy image_copy_region = init::image_copy(m_width, m_height);
    vkCmdCopyImage(
        copy_command, 
        src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // should be in the first one
        output_view->GetImages()[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &image_copy_region
    );

    //copy screen image to offset image
    output_view->TransitionImageLayout(
        copy_command, output_view->GetImages()[0],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL
    );

    m_device->EndSingleCommand(copy_command, 1);

    return output_view;
}

void RenderManager::Close() 
{
    // this is probably okay, right? we don't need that much performance... ?
    if(m_pipeline != nullptr) { 
        delete m_pipeline; // might not need to delete pipeline, but we need to remove frame buffer from pipeline first
        m_pipeline = nullptr;
    }
    if(m_screen_view != nullptr) {
        delete m_screen_view;
        m_screen_view = nullptr;
    }
    if(m_depth_view != nullptr) {
        delete m_depth_view;
        m_depth_view = nullptr;
    }
}

void RenderManager::Wait() 
{
    vkQueueWaitIdle(m_device->GetGraphicsQueue());
}

void RenderManager::SaveImage( std::string filename, VulkanImageView* output_view)
{

    const char* imagedata;
    // TODO: place in imageview class? called CopyScreen?
    VkImageSubresource image_subresource = {};
    image_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSubresourceLayout subresource_layout;
    vkGetImageSubresourceLayout(
        m_device->GetDevice(), output_view->GetImages()[0], &image_subresource, &subresource_layout
    );
    
    vkMapMemory(
        m_device->GetDevice(), output_view->GetImageMemories()[0],
        0, VK_WHOLE_SIZE, 0, (void**)&imagedata
    );
    imagedata += subresource_layout.offset;

    // TODO: separate this in to save_file()?
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    file << "P6\n" << m_width << "\n" << m_height << "\n" << 255 << "\n";

    std::vector<VkFormat> formats_BGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
    const bool color_swizzle = (std::find(formats_BGR.begin(), formats_BGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formats_BGR.end());
    
    printfi("Saving Image as a file...\n");
    for(int32_t y = 0; y < m_height; y++) 
    {
        unsigned int *row = (unsigned int*) imagedata;
        for(int32_t x = 0; x < m_width; x++)
        {
            if(color_swizzle)
            {
                file.write((char*)row + 2, 1);
                file.write((char*)row + 1, 1);
                file.write((char*)row, 1);
            } else {
                file.write((char*)row, 3);
            }
            row++;
        }
        imagedata += subresource_layout.rowPitch;
    }
    file.close();

    printfv("Framebuffer image is saved!\n");

    vkUnmapMemory(m_device->GetDevice(), output_view->GetImageMemories()[0]);
}

RenderManager::~RenderManager() 
{
    if(m_pipeline != nullptr) delete m_pipeline;
    if(m_screen_view != nullptr) delete m_screen_view;
    if(m_device != nullptr) delete m_device;
    if(m_physical_device != nullptr) delete m_physical_device;
    if(m_instance != nullptr) delete m_instance;
}
