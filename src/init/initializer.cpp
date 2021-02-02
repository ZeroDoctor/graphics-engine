#include "initializer.hpp"

#include "vertex.hpp"

// Infos
VkApplicationInfo init::application_info(VulkanConfiguration* config)
{
    VkApplicationInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = config->application_name;
    info.applicationVersion = config->application_version;
    info.apiVersion = config->api_version;
    info.pEngineName = config->engine_name;
    info.engineVersion = config->engine_version;
    return info;
}

VkInstanceCreateInfo init::instance_info(
        VkApplicationInfo* appInfo, 
        const char* const* extensions, 
        size_t extensions_count,
        VkDebugUtilsMessengerCreateInfoEXT* debug_info
    )
{
    VkInstanceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = appInfo;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions_count);
    info.ppEnabledExtensionNames = extensions;
    if(enable_validation_layers) 
    {
        info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        info.ppEnabledLayerNames = reinterpret_cast<const char* const*>(validation_layers.data());
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) debug_info;
    } else {
        info.enabledLayerCount = 0;
        info.pNext = nullptr;
    }
    return info;
}

VkDeviceQueueCreateInfo init::device_queue_info(uint32_t queue_family_index, float* priority) 
{
    VkDeviceQueueCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    info.queueFamilyIndex = queue_family_index;
    info.queueCount = 1;
    info.pQueuePriorities = priority;
    return info;
}

VkDeviceCreateInfo init::device_info(
        VkDeviceQueueCreateInfo* queue_infos,
        size_t queue_info_count,
        VkPhysicalDeviceFeatures* device_features
    ) 
{
    VkDeviceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pQueueCreateInfos = queue_infos;
    info.queueCreateInfoCount = static_cast<uint32_t>(queue_info_count);
    info.pEnabledFeatures = device_features;
    if(enable_validation_layers) {
        info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        info.ppEnabledLayerNames = reinterpret_cast<const char* const*>(validation_layers.data());
    } else {
        info.enabledLayerCount = 0;
    }
    return info;
}

VkCommandPoolCreateInfo init::command_pool_info(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) 
{
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queue_family_index;
    info.flags = flags;
    return info;
}

VkCommandBufferAllocateInfo init::command_buffer_allocate_info(VkCommandPool command_pool, uint32_t count) 
{
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = command_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = count;
    return info;
}

VkDebugUtilsMessengerCreateInfoEXT init::debug_messenger_info(PFN_vkDebugUtilsMessengerCallbackEXT debug_callback)
{
    VkDebugUtilsMessengerCreateInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    info.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debug_callback;
    return info;
}

VkBufferCreateInfo init::buffer_info(VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return info;
}

VkMemoryAllocateInfo init::memory_allocate_info(VkMemoryRequirements memory_requirements, uint32_t memory_type)
{
    VkMemoryAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.allocationSize = memory_requirements.size;
    info.memoryTypeIndex = memory_type;
    return info;
}

VkFramebufferCreateInfo init::frame_buffer_info(
        VkRenderPass render_pass, 
        VkImageView* attachments, 
        uint32_t width, uint32_t height
    ) 
{
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = render_pass;
    info.attachmentCount = 1;
    info.pAttachments = attachments;
    info.width = width;
    info.height = height;
    info.layers = 1;
    return info;
}

VkCommandBufferBeginInfo init::command_buffer_begin_info(VkCommandBufferUsageFlags flags) 
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = flags;
    info.pInheritanceInfo = nullptr;
    return info;
}

VkRenderPassBeginInfo init::render_pass_begin_info(
        VkRenderPass render_pass, 
        VkRect2D rect, 
        VkFramebuffer frame_buffer,
        VkClearValue* clear_value,
        uint32_t clear_count
    )
{
    VkRenderPassBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderArea = rect;
    info.renderPass = render_pass;
    info.framebuffer = frame_buffer;
    info.clearValueCount = clear_count;
    info.pClearValues = clear_value;
    info.pNext = NULL;
    return info;
}

VkImageViewCreateInfo init::image_view_info(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    /*
        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    */
    info.subresourceRange = {};
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    return info;
}

VkImageCreateInfo init::image_info(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
{
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.extent = {width, height, 1};
    info.mipLevels = 1;
    info.arrayLayers = 1;
    
    
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.usage = usage;
    info.tiling = tiling;

    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return info;
}

VkSamplerCreateInfo init::sampler_info()
{
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.anisotropyEnable = VK_TRUE;
    info.maxAnisotropy = 16.0f;
    info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    return info;
}

VkShaderModuleCreateInfo init::shader_module_info(const char* code, size_t code_count)
{
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code_count;
    info.pCode = reinterpret_cast<const uint32_t*>(code);
    return info;
}

VkPipelineShaderStageCreateInfo init::pipline_shader_stage_info(VkShaderModule module, VkShaderStageFlagBits flags)
{
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = flags;
    info.module = module;
    info.pName = "main";
    return info;
}

VkPipelineVertexInputStateCreateInfo init::pipeline_vertex_input_state_info(
    VkVertexInputAttributeDescription* attribute_description, uint32_t attribute_count,
    VkVertexInputBindingDescription* binding_description, uint32_t binding_count)
{
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = binding_count;
    info.pVertexBindingDescriptions = binding_description;

    info.vertexAttributeDescriptionCount = attribute_count;
    info.pVertexAttributeDescriptions = attribute_description;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo init::pipeline_input_assembly_state_info() 
{
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.flags = 0;
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}

VkPipelineDepthStencilStateCreateInfo init::pipeline_depth_stencil_info(
        VkBool32 depth_test, VkBool32 depth_write, VkCompareOp depth_compare_op
    )
{
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable = depth_test;
    info.depthWriteEnable = depth_write;
    info.depthCompareOp = depth_compare_op;
    info.back.compareOp = VK_COMPARE_OP_ALWAYS;
    return info;
}

VkPipelineViewportStateCreateInfo init::pipeline_viewport_state_info(VkViewport* viewport, VkRect2D* scissor)
{
    VkPipelineViewportStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.viewportCount = 1;
    info.pViewports = viewport;
    info.scissorCount = 1;
    info.pScissors = scissor;
    return info;
}

VkPipelineRasterizationStateCreateInfo init::pipeline_rasterization_state_info(
        VkPipelineRasterizationStateCreateFlags flags
    )
{
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.depthClampEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.lineWidth = 1.0f;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.flags = flags;
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;
    return info;
}

VkPipelineMultisampleStateCreateInfo init::pipeline_multisample_state_info(
        VkPipelineMultisampleStateCreateFlags flags
    ) 
{
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.sampleShadingEnable = VK_FALSE;
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.flags = flags;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendStateCreateInfo init::pipeline_colorblend_state_info(VkPipelineColorBlendAttachmentState* blend_attachment)
{
    VkPipelineColorBlendStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = blend_attachment;

    info.logicOpEnable = VK_FALSE;
    info.logicOp = VK_LOGIC_OP_COPY;
    info.blendConstants[0] = 0.0f;
    info.blendConstants[1] = 0.0f;
    info.blendConstants[2] = 0.0f;
    info.blendConstants[3] = 0.0f;
    return info;
}

VkPipelineDynamicStateCreateInfo init::pipeline_dynamic_state_info(VkDynamicState* dynamic_states)
{
    VkPipelineDynamicStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    info.dynamicStateCount = 2;
    info.pDynamicStates = dynamic_states;
    return info;
}

VkPipelineLayoutCreateInfo init::pipeline_layout_info(VkPipelineRasterizationStateCreateFlags flags)
{
    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkGraphicsPipelineCreateInfo init::graphics_pipeline_info(
        VkPipelineShaderStageCreateInfo* shader_stages_info, uint32_t shader_stages_count,
        VkPipelineVertexInputStateCreateInfo* vertex_info,
        VkPipelineInputAssemblyStateCreateInfo* input_assembly_info,
        VkPipelineViewportStateCreateInfo* viewport_state_info,
        VkPipelineRasterizationStateCreateInfo* rasterizer_info,
        VkPipelineMultisampleStateCreateInfo* multisampling_info,
        VkPipelineColorBlendStateCreateInfo* color_blending_info,
        VkPipelineDynamicStateCreateInfo* dynamic_info,
        VkPipelineLayout pipeline_layout,
        VkRenderPass render_pass
    )
{   
    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.stageCount = shader_stages_count;
    info.pStages = shader_stages_info;
    info.pVertexInputState = vertex_info;
    info.pInputAssemblyState = input_assembly_info;
    info.pViewportState = viewport_state_info;
    info.pRasterizationState = rasterizer_info;
    info.pMultisampleState = multisampling_info;
    info.pColorBlendState = color_blending_info;
    info.pDynamicState = dynamic_info;
    info.layout = pipeline_layout;
    info.renderPass = render_pass;
    info.subpass = 0;
    info.basePipelineHandle = VK_NULL_HANDLE;
    return info;
}

VkRenderPassCreateInfo init::render_pass_info(
        std::array<VkAttachmentDescription, 2> attachments,
        VkSubpassDescription* subpass, 
        std::array<VkSubpassDependency, 2> dependencies
    )
{
    printfi("%d\n", attachments[0].flags);
    printfi("%d\n", attachments[1].flags);
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = subpass;
    info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    info.pDependencies = dependencies.data();
    return info;
}

VkSubmitInfo init::submit_info(uint32_t buffer_count, VkCommandBuffer* command_buffer)
{
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = buffer_count;
    info.pCommandBuffers = command_buffer;
    return info;
}

VkFenceCreateInfo init::fence_info(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = flags;
    return info;
}


// Attachment
VkPipelineColorBlendAttachmentState init::pipeline_colorblend_state()
{
    VkPipelineColorBlendAttachmentState attachment = {};
    attachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = VK_FALSE;
    // not used
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
   
   return attachment;
}

VkAttachmentDescription init::description(VkFormat format, VkImageLayout layout) 
{
    VkAttachmentDescription attachment = {};
    //attachment.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = layout; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for surfaces
    return attachment;
}

VkImageCopy init::image_copy(uint32_t width, uint32_t height)
{
    VkImageCopy copy = {};
    copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.srcSubresource.layerCount = 1;
    copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.dstSubresource.layerCount = 1;
    copy.extent = {width, height, 1};
    return copy;
}