#pragma once

#include "build_order.hpp"
#include "device.hpp"
#include "vertex_buffer.hpp"

struct Vertex;
class VulkanDevice;
class VulkanVertexBuffer;

class VulkanGraphicsPipline
{
public:
    VulkanGraphicsPipline(VulkanDevice*, uint32_t, uint32_t);
    ~VulkanGraphicsPipline();
    void CreateShaderModule(std::string, std::string);
    void CreatePipelineLayout(uint32_t, uint32_t);
    void CreateRenderPass(VkFormat, VkFormat, bool);
    void CreateFrameBuffers(uint32_t, std::vector<VkImageView>, VkImageView* depth_view=nullptr); 
    void CreateCommandBuffers(VkCommandBuffer*, uint32_t, VulkanVertexBuffer*);
    
private:
    uint32_t  m_screen_width;
    uint32_t m_screen_height;
    VulkanDevice* m_device;
    // TODO: create VkShaderModule std::vector
    VkShaderModule m_vert_module=0;
    VkShaderModule m_frag_module=0;
    
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    VkPipelineCache m_cache = NULL;
    VkPipelineLayout m_pipeline_layout=NULL;
    VkPipeline m_graphics_pipeline=NULL;

    VkRenderPass m_render_pass=NULL;
    std::vector<VkFramebuffer> m_frame_buffers;
    
    VkCommandBuffer* m_command_buffers;
    uint32_t m_command_buffer_count=0;
    
    std::vector<VkImageView> m_imageviews;
    VkDescriptorSetLayout m_descriptor_set_layout=VK_NULL_HANDLE;
    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    
    void createDescriptorPool();
    void createDescriptorSets(VkSampler, VkImageView);
    void createDescriptorLayout();
    VkShaderModule createShaderModule(VulkanDevice*, const std::vector<char>);
};