#include "pipeline.hpp"

VulkanGraphicsPipline::VulkanGraphicsPipline(
        VulkanDevice* device, 
        VkFormat format,
        uint32_t width, uint32_t height
    ) 
{
    m_screen_width = width;
    m_screen_height = height;
    m_device = device;
    //CreateRenderPass(format); // inits m_render_pass
}

VulkanGraphicsPipline::~VulkanGraphicsPipline()
{
    if(m_render_pass != NULL) {
        printfi("-- Destroying Render Pass Pipeline...\n");
        vkDestroyRenderPass(m_device->GetDevice(), m_render_pass, nullptr);
    }

    // TODO: create free function for objects below
    for(auto fb : m_frame_buffers) {
        printfi("-- Destroying %d Framebuffer Pipeline...\n", m_frame_buffers.size());
        vkDestroyFramebuffer(m_device->GetDevice(), fb, nullptr);
    }

    if(m_command_buffer_count != 0)
    {
        // FreeCommandBuffers
        vkFreeCommandBuffers(
            m_device->GetDevice(), 
            m_device->GetGraphicsCommandPool(),
            m_command_buffer_count,
            m_command_buffers
        );
    }

    if(m_pipeline_layout != NULL) {
        printfi("-- Destroying Graphic Layout Pipeline...\n");
        vkDestroyPipelineLayout(m_device->GetDevice(), m_pipeline_layout, nullptr);
    }
    if(m_graphics_pipeline != NULL) {
        printfi("-- Destroying Graphic Pipeline...\n");
        vkDestroyPipeline(m_device->GetDevice(), m_graphics_pipeline, nullptr);
    }
    if(m_cache != NULL) {
        printfi("-- Destroying Graphic Cache Pipeline...\n");
        vkDestroyPipelineCache(m_device->GetDevice(), m_cache, nullptr);
    }
    
    // Destroy Shader Modules in the event of a failed render pass / pipeline
    if(m_vert_module != 0) {
        printfi("-- Destroying Vertex Module Pipeline...\n");
        vkDestroyShaderModule(m_device->GetDevice(), m_vert_module, nullptr);
    }
    if(m_frag_module != 0) {
        printfi("-- Destroying Fragmentation Module Pipeline...\n");
        vkDestroyShaderModule(m_device->GetDevice(), m_frag_module, nullptr);
    }

    printfi("-- Destorying Descriptor Layout\n"); // not needed to destory the layout everytime
    vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_descriptor_set_layout, nullptr);
}

// TODO: make it more generic
void VulkanGraphicsPipline::CreateShaderModule(std::string vert_path, std::string frag_path) 
{
    std::vector<char> vert_shader = read_shader(vert_path);
    std::vector<char> frag_shader = read_shader(frag_path);

    m_vert_module = createShaderModule(m_device, vert_shader);
    m_frag_module = createShaderModule(m_device, frag_shader);

    VkPipelineShaderStageCreateInfo vert_stage_info = init::pipline_shader_stage_info(
        m_vert_module,
        VK_SHADER_STAGE_VERTEX_BIT
    );

    VkPipelineShaderStageCreateInfo frag_stage_info = init::pipline_shader_stage_info(
        m_frag_module,
        VK_SHADER_STAGE_FRAGMENT_BIT
    );

    m_shader_stages.push_back(vert_stage_info);
    m_shader_stages.push_back(frag_stage_info);
}

// TODO: Cleanup
void VulkanGraphicsPipline::createDescriptorPool() 
{
    VkDescriptorPoolSize pool_size = {};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1; // the number of images being rendered

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1; // number of objects in pool
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = 1; // the number of images being rendered

    ErrorCheck(vkCreateDescriptorPool(
        m_device->GetDevice(),
        &pool_info,
        nullptr,
        &m_descriptor_pool
    ), "Descriptor Pool"); 
}

// TODO: Cleanup
void VulkanGraphicsPipline::createDescriptorSets(VkSampler sampler, VkImageView image_view) 
{
    std::vector<VkDescriptorSetLayout> layouts(1, m_descriptor_set_layout); // the number of images being rendered
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptor_pool;
    allocInfo.descriptorSetCount = 1;// the number of images being rendered
    allocInfo.pSetLayouts = layouts.data();

    m_descriptor_sets.resize(1); // the number of images being rendered
    ErrorCheck(vkAllocateDescriptorSets(
        m_device->GetDevice(), 
        &allocInfo, 
        m_descriptor_sets.data()
    ), "Allocate Descriptor Sets");

    for (size_t i = 0; i < 1; i++) { // the number of images being rendered
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = image_view;
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptor_sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanGraphicsPipline::createDescriptorLayout() 
{
    // creating sampler descriptor layout
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding}; // used array to enable uniforms later
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    ErrorCheck(vkCreateDescriptorSetLayout(
        m_device->GetDevice(),
        &layoutInfo, nullptr,
        &m_descriptor_set_layout
    ),"Descriptor Set Layout");
}

void VulkanGraphicsPipline::CreateRenderPass(VkFormat color_format, VkFormat depth_format) 
{
    printfi("Creating Render Pass...\n");

    std::array<VkAttachmentDescription, 2> attachment_descriptions;
    
    attachment_descriptions[0] = init::description(
        color_format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    );

    attachment_descriptions[1] = init::description(
        depth_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
    attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // TODO: create reference, descriptions and dependency init::functions
    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_reference;
    subpass_description.pDepthStencilAttachment = &depth_reference;

    
        /* //-- used for surface enabled logical devices
        std::array<VkSubpassDependency, 1> subpass_dependencies;

        subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependencies[0].dstSubpass = 0;
        subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[0].srcAccessMask = 0;
        subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; */

    
    std::array<VkSubpassDependency, 2> subpass_dependencies;
    
    subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependencies[0].dstSubpass = 0;
    subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpass_dependencies[1].srcSubpass = 0;
    subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // can't use init::render_pass_info()... nope you just can't... I don't know why...
    VkRenderPassCreateInfo render_info = {};
    render_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
    render_info.pAttachments = attachment_descriptions.data();
    render_info.subpassCount = 1;
    render_info.pSubpasses = &subpass_description;
    render_info.dependencyCount = static_cast<uint32_t>(subpass_dependencies.size());
    render_info.pDependencies = subpass_dependencies.data();

    ErrorCheck(vkCreateRenderPass(
        m_device->GetDevice(),
        &render_info,
        nullptr,
        &m_render_pass
    ), "Create Render Pass");
}

void VulkanGraphicsPipline::CreatePipelineLayout(uint32_t width, uint32_t height) 
{
    printfi("Creating Pipeline...\n");
    if(m_shader_stages.size() <= 0) {
        printff("Must Create Shader Module First");
    }

    createDescriptorLayout();

    // cache pipeline
    VkPipelineCacheCreateInfo cache_info = {};
    cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    ErrorCheck(vkCreatePipelineCache(
        m_device->GetDevice(), &cache_info, nullptr, &m_cache
    ), "Create Cache Pipeline");


    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = init::pipeline_input_assembly_state_info();

    // Vertex
    auto binding_description = Vertex::getBindingDescription();
    auto attribute_description = Vertex::getAttributeDescriptions();
    
    VkPipelineVertexInputStateCreateInfo vertex_input_info = init::pipeline_vertex_input_state_info(
        attribute_description.data(), static_cast<uint32_t>(attribute_description.size()), 
        &binding_description, 1
    );

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_info = init::pipeline_rasterization_state_info();

    // Color Blend
    VkPipelineColorBlendAttachmentState colorblend_attachment = init::pipeline_colorblend_state();
    VkPipelineColorBlendStateCreateInfo colorblend_info = init::pipeline_colorblend_state_info(&colorblend_attachment);

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = init::pipeline_depth_stencil_info(
        VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL
    );

    // Viewport
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_screen_width;
    viewport.height = (float) m_screen_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {m_screen_width, m_screen_height};

    VkPipelineViewportStateCreateInfo viewport_state_info = init::pipeline_viewport_state_info(&viewport, &scissor);

    // Multisampling - Disabled
    VkPipelineMultisampleStateCreateInfo multisampling_info = init::pipeline_multisample_state_info();

    VkDynamicState dynamic_state[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_info = init::pipeline_dynamic_state_info(dynamic_state);

    // Pipeline Layout - Low-Key Disabled (used for passing uniforms)
    // TODO: enable layout for uniforms
    VkPipelineLayoutCreateInfo pipeline_layout_info = init::pipeline_layout_info();
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_descriptor_set_layout;
    
    ErrorCheck(vkCreatePipelineLayout(
        m_device->GetDevice(),
        &pipeline_layout_info,
        nullptr,
        &m_pipeline_layout
    ), "Create Pipeline Layout");

    VkGraphicsPipelineCreateInfo pipeline_info = init::graphics_pipeline_info(
        m_shader_stages.data(), static_cast<uint32_t>(m_shader_stages.size()),
        &vertex_input_info,
        &input_assembly_info,
        &viewport_state_info,
        &rasterizer_info,
        &multisampling_info,
        &colorblend_info,
        &dynamic_state_info,
        m_pipeline_layout, m_render_pass
    );
    pipeline_info.pDepthStencilState = &depth_stencil_info;

    VkPipeline graphics_pipeline;
    ErrorCheck(vkCreateGraphicsPipelines(
        m_device->GetDevice(),
        VK_NULL_HANDLE, 1,
        &pipeline_info, nullptr,
        &graphics_pipeline
    ), "Create Graphics Pipelines");

    m_graphics_pipeline = graphics_pipeline;

    vkDestroyShaderModule(m_device->GetDevice(), m_vert_module, nullptr);
    vkDestroyShaderModule(m_device->GetDevice(), m_frag_module, nullptr);
    m_vert_module = 0;
    m_frag_module = 0;
}

void VulkanGraphicsPipline::CreateFrameBuffers(uint32_t count, std::vector<VkImageView> image_views) 
{
    printfi("Creating Frame Buffers %dx%d...\n", m_screen_width, m_screen_height);

    if(m_render_pass == NULL) {
        printfe("Must create render pass first before creating frame buffers\n");
        return;
    }

    m_frame_buffers.resize(count);
    // for now
    VkImageView* attachments = image_views.data();

    for (size_t i = 0; i < count; i++)
    {
        VkFramebufferCreateInfo frame_buffer_info = init::frame_buffer_info(
            m_render_pass,
            attachments,
            m_screen_width, m_screen_height
        );
        frame_buffer_info.attachmentCount = 2;

        printfi("Frame Buffer %d has been Set...\n",i);
        ErrorCheck(vkCreateFramebuffer(
            m_device->GetDevice(),
            &frame_buffer_info,
            nullptr,
            &m_frame_buffers[i]
        ), "Create Framebuffer");
    }   
}

void VulkanGraphicsPipline::CreateCommandBuffers(
        VkCommandBuffer* buffers,
        uint32_t count,
        VulkanVertexBuffer* vertex_buffer
    )
{
    printfi("Create Command Buffer...\n");

    if(m_render_pass == NULL) {
        printff("Can not create commmand buffers without render pass!");
    }
    if(m_frame_buffers[0] == NULL) {
        printff("Can not create command buffers without frame buffers!");
    } else if(m_frame_buffers.size() < count) {
        printfw("Something is werid here frame_buffer < command_buffer");
    }

    for (size_t i = 0; i < count; i++)
    {
        buffers[i] = nullptr;
        VkCommandBufferAllocateInfo alloc_info = init::command_buffer_allocate_info(
            m_device->GetGraphicsCommandPool(), 1
        );
        ErrorCheck(vkAllocateCommandBuffers(
            m_device->GetDevice(), &alloc_info, &(buffers[i])
        ), "Allocate Command Buffers");

        if(buffers[i] == nullptr) printff("Buffer is NULL\n");
        VkCommandBufferBeginInfo begin_info = init::command_buffer_begin_info(0);

        ErrorCheck(vkBeginCommandBuffer(
            buffers[i],
            &begin_info
        ), "Create Begin Command Buffer");

        VkClearValue clear_values[2]; 
        clear_values[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};
        VkRect2D render_area;
        render_area.offset = {0, 0};
        render_area.extent = {m_screen_width, m_screen_height};
        VkRenderPassBeginInfo render_pass_info = init::render_pass_begin_info(
            m_render_pass,
            render_area,
            m_frame_buffers[i],
            clear_values, 2
        );

        vkCmdBeginRenderPass(buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {};
            viewport.height = (float) m_screen_height;
            viewport.width = (float) m_screen_width;
            viewport.minDepth = (float) 0.0f;
            viewport.maxDepth = (float) 1.0f;
            vkCmdSetViewport(buffers[i], 0, 1, &viewport);

            VkRect2D scissor = {};
            scissor.extent = { m_screen_width, m_screen_height };
            vkCmdSetScissor(buffers[i], 0, 1, &scissor);

            vkCmdBindPipeline(buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
            vkCmdSetLineWidth(buffers[i], 1.0f);

            VkBuffer vertex_buffers[] = {vertex_buffer->GetVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(buffers[i], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(buffers[i], vertex_buffer->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(buffers[i], static_cast<uint32_t>(vertex_buffer->GetIndices().size()), 1, 0, 0, 0); // and... we finally made it
        
        vkCmdEndRenderPass(buffers[i]);
        ErrorCheck(vkEndCommandBuffer(
            buffers[i]
        ), "End Command Buffer");
    }
}

VkShaderModule VulkanGraphicsPipline::createShaderModule(VulkanDevice* device, const std::vector<char> shader_code) 
{
    VkShaderModuleCreateInfo info = init::shader_module_info(shader_code.data(), shader_code.size());

    VkShaderModule shader_module;
    ErrorCheck(vkCreateShaderModule(
        device->GetDevice(),
        &info,
        nullptr,
        &shader_module
    ), "Create Shader Module");

    return shader_module;
}