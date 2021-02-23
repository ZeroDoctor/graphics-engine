#pragma once

#include "build_order.hpp"
#include "vulkan_config.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "physical_device.hpp"
#include "device.hpp"
#include "image_view.hpp"
#include "pipeline.hpp"
#include "vertex_buffer.hpp"

struct RenderSettings {
    bool headless = false;
    bool tessellation = false;
};


class RenderManager {
public:
    RenderManager(){}
    ~RenderManager();

    void Init(std::string);
    void Setup(uint32_t, uint32_t, VkFormat src_format=VK_FORMAT_R8G8B8A8_UNORM);
    VulkanImageView* Draw(std::vector<Vertex>, std::vector<uint16_t>);
    void Close();
    void Wait();

    void SaveImage(std::string, VulkanImageView*);

private:
    VulkanInstance* m_instance;
    VulkanPhysicalDevice* m_physical_device;
    VulkanDevice* m_device;
    VulkanImageView* m_screen_view=nullptr;
    VulkanImageView* m_depth_view=nullptr;
    VulkanGraphicsPipline* m_pipeline;

    uint32_t m_width;
    uint32_t m_height;
    VkFormat m_src_format;
    VkFormat m_depth_format;

    VulkanImageView* copyScreen(VkImage);
};