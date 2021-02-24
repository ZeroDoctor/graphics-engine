#include "render_manager.hpp"

void RenderManager::Init(RenderSettings settings) 
{
    // app configurations
    VulkanConfiguration config;
    config.application_name = settings.app_name.c_str();
    config.application_version = VK_MAKE_VERSION(1,0,0);
    
    // device setup
    m_instance = new VulkanInstance(config);
    if(!settings.headless) {
        m_surface = new VulkanSurface(m_instance, settings.win_settings, settings.width, settings.height);
    }
    m_physical_device = VulkanPhysicalDevice::GetPhysicalDevice(m_instance, m_surface, !settings.headless);
    m_device = new VulkanDevice(m_instance, m_physical_device);
    m_render_settings = settings;
}

void RenderManager::Setup() 
{
    VkFormat depth_format;
    if(!m_device->GetSupportedDepthFormat(&depth_format)) {
        printff("Could not find depth supported physical device\n");
    }

    m_depth_view = new VulkanImageView(m_device);
    m_depth_view->GenerateImage(
        m_render_settings.width, m_render_settings.height, 
        depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    VkImageAspectFlags flags[] = {
         VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
    };
    m_depth_view->CreateImageView(flags);

    // create "screen"
    if(m_render_settings.headless)
    {
        m_screen_view = new VulkanImageView(m_device);
        m_screen_view->GenerateImage(
            m_render_settings.width, m_render_settings.height, m_render_settings.src_format, 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
        );
        VkImageAspectFlags flags[] = {
            VK_IMAGE_ASPECT_COLOR_BIT
        };
        m_screen_view->CreateImageView(flags);
    }
    // or else
    if(!m_render_settings.headless)
    {
        m_swapchain = new VulkanSwapChain(
            m_instance, m_physical_device, m_device, 
            m_surface, m_render_settings.width, m_render_settings.height
        );
        m_swapchain_views = m_swapchain->GetImageViews();
    }
    
    m_pipeline = new VulkanGraphicsPipline(
        m_device, m_render_settings.width, m_render_settings.height
    );

    m_depth_format = depth_format;
}

void RenderManager::Draw(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
{
    if(m_swapchain_views.size() <= 0) {
        printfw("Failed to find swapchain image view\n");
        return;
    }

    if(m_pipeline == nullptr) {
        printfw("Failed to find pipeline. Must create pipeline layout first before draw\n");
        return;
    }

    m_vertex_buffer = new VulkanVertexBuffer(
        m_device, vertices, indices
    );

    m_command_count = m_swapchain_views.size();
    m_command = new VkCommandBuffer[m_command_count];
    {
        m_pipeline->CreateShaderModule("./../src/shader/vert.spv", "./../src/shader/frag.spv");
        m_pipeline->CreateRenderPass(m_swapchain->GetFormat(), m_depth_format, true);
        m_pipeline->CreateFrameBuffers(m_swapchain_views.size(), m_swapchain_views, &m_depth_view->GetImageViews()[0]);
        m_pipeline->CreatePipelineLayout(m_render_settings.width, m_render_settings.height);
        m_pipeline->CreateCommandBuffers(m_command, m_command_count, m_vertex_buffer);
    }

    createSyncObjects();
}

void RenderManager::WinLoop() 
{
    printfi("RUNNING WINDOW LOOP\n");
    std::function<bool()> render_func = std::bind(&RenderManager::render, this);
    if(!m_render_settings.headless)
    {
        m_surface->MainLoop(m_device->GetDevice(), render_func);
    }
}

VulkanImageView* RenderManager::DrawHeadless( std::vector<Vertex> vertices, std::vector<uint16_t> indices) 
{
    if(m_screen_view == nullptr) {
        printfw("Failed to find screen image view\n");
        return nullptr;
    }

    if(m_pipeline == nullptr) {
        printfw("Failed to find pipeline. Must create pipeline layout first before draw\n");
        return nullptr;
    }

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
        m_pipeline->CreateRenderPass(m_render_settings.src_format, m_depth_format, false);
        m_pipeline->CreateFrameBuffers(1, m_screen_view->GetImageViews(), &m_depth_view->GetImageViews()[0]);
        m_pipeline->CreatePipelineLayout(m_render_settings.width, m_render_settings.height);

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
        m_render_settings.width, m_render_settings.height, m_render_settings.src_format, 
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

    VkImageCopy image_copy_region = init::image_copy(m_render_settings.width, m_render_settings.height);
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
    if(m_device == nullptr) return;
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
    file << "P6\n" << m_render_settings.width << "\n" << m_render_settings.height << "\n" << 255 << "\n";

    std::vector<VkFormat> formats_BGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
    const bool color_swizzle = (std::find(formats_BGR.begin(), formats_BGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formats_BGR.end());
    
    printfi("Saving Image as a file...\n");
    for(int32_t y = 0; y < m_render_settings.height; y++) 
    {
        unsigned int *row = (unsigned int*) imagedata;
        for(int32_t x = 0; x < m_render_settings.width; x++)
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

bool RenderManager::render()
{
    ErrorCheck(vkWaitForFences(
        m_device->GetDevice(), 1, 
        &m_in_flight_fences[m_current_frame], VK_TRUE, 
        UINT64_MAX
    ), "Wait For Fences");

    uint32_t image_index;
    ErrorCheck(vkAcquireNextImageKHR(
        m_device->GetDevice(), m_swapchain->GetSwapchain(), UINT64_MAX,
        m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index   
    ), "Acquire Next Image");

    if(m_image_in_flight[image_index] != VK_NULL_HANDLE) {
        ErrorCheck(vkWaitForFences(
            m_device->GetDevice(), 1, 
            &m_image_in_flight[image_index], VK_TRUE, 
            UINT64_MAX
        ), "Wait For Fences");
    }
    m_image_in_flight[image_index] = m_in_flight_fences[m_current_frame];

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore wait_semaphore[] = {m_image_available_semaphores[m_current_frame]};
    VkSemaphore signal_semaphore[] = {m_render_finished_semaphores[m_current_frame]};
    
    VkSubmitInfo submit_info = init::submit_info(m_command_count, m_command);
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphore;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphore;

    ErrorCheck(vkResetFences(
        m_device->GetDevice(), 1, &m_in_flight_fences[m_current_frame]
    ), "Reset Fences");

    ErrorCheck(vkQueueSubmit(
        m_device->GetGraphicsQueue(), 1, &submit_info, m_in_flight_fences[m_current_frame]
    ), "Sumbit Render Queue");

    Wait();

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphore;

    VkSwapchainKHR swapchain[] = {m_swapchain->GetSwapchain()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchain;
    present_info.pImageIndices = &image_index;

    ErrorCheck(vkQueuePresentKHR(m_device->GetPresentQueue(), &present_info), "Queue Present");

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

void RenderManager::createSyncObjects() 
{
    m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    m_image_in_flight.resize(m_swapchain->GetImages().size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info = init::fence_info(VK_FENCE_CREATE_SIGNALED_BIT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        ErrorCheck(vkCreateSemaphore(
            m_device->GetDevice(), &semaphore_info, 
            nullptr, &m_image_available_semaphores[i]
        ),"Create Image Semaphore");

        ErrorCheck(vkCreateSemaphore(
            m_device->GetDevice(), &semaphore_info,
            nullptr, &m_render_finished_semaphores[i]
        ), "Create Render Semaphore");

        ErrorCheck(vkCreateFence(
            m_device->GetDevice(), &fence_info, 
            nullptr, &m_in_flight_fences[i]
        ), "Create In Flight Fence");
    }
}

RenderManager::~RenderManager() 
{
    if(m_vertex_buffer != nullptr) delete m_vertex_buffer;

    if(m_command != nullptr) {
        m_device->FreeComputeCommand(m_command, m_command_count);
    }

    for(auto render : m_render_finished_semaphores)
        vkDestroySemaphore(m_device->GetDevice(), render, nullptr);
    for(auto image : m_image_available_semaphores)
        vkDestroySemaphore(m_device->GetDevice(), image, nullptr);
    for(auto in : m_in_flight_fences)
        vkDestroyFence(m_device->GetDevice(), in, nullptr);

    if(m_pipeline != nullptr) delete m_pipeline;
    if(m_screen_view != nullptr) delete m_screen_view;
    if(m_depth_view != nullptr) delete m_depth_view;
    if(m_swapchain != nullptr) delete m_swapchain;
    if(m_surface != nullptr) delete m_surface;
    if(m_device != nullptr) delete m_device;
    if(m_physical_device != nullptr) delete m_physical_device;
    if(m_instance != nullptr) delete m_instance;
}
