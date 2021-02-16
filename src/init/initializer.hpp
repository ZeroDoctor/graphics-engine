#pragma once

#include "build_order.hpp"
#include <xcb/xcb.h>
#include "vulkan_config.hpp"
#include <vulkan/vulkan_xcb.h>

namespace init
{
    // Infos
    VkApplicationInfo application_info(VulkanConfiguration*);
    VkInstanceCreateInfo instance_info(VkApplicationInfo*,const char* const*,size_t,VkDebugUtilsMessengerCreateInfoEXT*);
    
    VkDeviceQueueCreateInfo device_queue_info(uint32_t, float*);
    VkDeviceCreateInfo device_info(VkDeviceQueueCreateInfo*, size_t, VkPhysicalDeviceFeatures*, std::vector<const char*>);
    
    VkCommandPoolCreateInfo command_pool_info(uint32_t, VkCommandPoolCreateFlags flags=0);
    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool, uint32_t);
    
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info(PFN_vkDebugUtilsMessengerCallbackEXT);

    VkBufferCreateInfo buffer_info(VkDeviceSize, VkBufferUsageFlags);
    VkMemoryAllocateInfo memory_allocate_info(VkMemoryRequirements,uint32_t);
    VkFramebufferCreateInfo frame_buffer_info(VkRenderPass, VkImageView*, uint32_t, uint32_t);
    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags);
    VkRenderPassBeginInfo render_pass_begin_info(VkRenderPass, VkRect2D, VkFramebuffer, VkClearValue*, uint32_t);

    VkImageViewCreateInfo image_view_info(VkImage, VkFormat);
    VkImageCreateInfo image_info(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags);
    VkSamplerCreateInfo sampler_info();

    VkShaderModuleCreateInfo shader_module_info(const char*, size_t);
    VkPipelineShaderStageCreateInfo pipline_shader_stage_info(VkShaderModule, VkShaderStageFlagBits);
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_info(
        VkVertexInputAttributeDescription*, uint32_t,
        VkVertexInputBindingDescription*, uint32_t
    );
    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_info();
    VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_info(VkBool32, VkBool32, VkCompareOp);
    VkPipelineViewportStateCreateInfo pipeline_viewport_state_info(VkViewport*, VkRect2D*);
    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_info(
        VkPipelineMultisampleStateCreateFlags flags=0
    );
    VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_info(
        VkPipelineMultisampleStateCreateFlags flags=0
    ); // Disabled
    // VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_info();
    VkPipelineColorBlendStateCreateInfo pipeline_colorblend_state_info(VkPipelineColorBlendAttachmentState*);
    VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_info(VkDynamicState*);
    VkPipelineLayoutCreateInfo pipeline_layout_info(VkPipelineRasterizationStateCreateFlags flags=0); // Low-Key Disabled
    VkGraphicsPipelineCreateInfo graphics_pipeline_info(
        VkPipelineShaderStageCreateInfo*, uint32_t,
        VkPipelineVertexInputStateCreateInfo*,
        VkPipelineInputAssemblyStateCreateInfo*,
        VkPipelineViewportStateCreateInfo*,
        VkPipelineRasterizationStateCreateInfo*,
        VkPipelineMultisampleStateCreateInfo*,
        VkPipelineColorBlendStateCreateInfo*,
        VkPipelineDynamicStateCreateInfo*,
        VkPipelineLayout,
        VkRenderPass
    );

    VkRenderPassCreateInfo render_pass_info(
        std::array<VkAttachmentDescription, 2>, 
        VkSubpassDescription*, 
        std::array<VkSubpassDependency, 2>
    );

    VkSubmitInfo submit_info(uint32_t, VkCommandBuffer*);
    VkFenceCreateInfo fence_info(VkFenceCreateFlags flags=0);

#if defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR surface_info(xcb_connection_t* connection, xcb_window_t window);   
#endif
    
    // Attachments
    VkPipelineColorBlendAttachmentState pipeline_colorblend_state();
    VkAttachmentDescription description(VkFormat, VkImageLayout);

    // Reference

    // Description

    // Dependency

    // Image Copy
    VkImageCopy image_copy(uint32_t, uint32_t);
    VkSwapchainCreateInfoKHR swapchain_info(VkSurfaceKHR, uint32_t, VkSurfaceFormatKHR, VkExtent2D);

} // namespace Initializers
