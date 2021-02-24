#pragma once

#include "build_order.hpp"
#include <functional>
#include "vulkan_config.hpp"
#include "instance.hpp"
#include "surface.hpp"
#include "physical_device.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "image_view.hpp"
#include "pipeline.hpp"
#include "vertex_buffer.hpp"

struct RenderSettings {
    bool headless=false;
    bool tessellation=false;
    uint32_t width=1280;
    uint32_t height=720;
    VkFormat src_format=VK_FORMAT_R8G8B8A8_UNORM;
    std::string app_name;
    WindowSettings win_settings;
};


class RenderManager {
public:
    RenderManager(){}
    ~RenderManager();

    void Init(RenderSettings);
    void Setup();
    void Draw(std::vector<Vertex>, std::vector<uint16_t>);
    void WinLoop();

    VulkanImageView* DrawHeadless(std::vector<Vertex>, std::vector<uint16_t>);
    void Close();
    void Wait();

    void SaveImage(std::string, VulkanImageView*);

private:
    RenderSettings m_render_settings;
    VulkanInstance* m_instance=nullptr;
    VulkanSurface* m_surface=nullptr;
    VulkanPhysicalDevice* m_physical_device=nullptr;
    VulkanDevice* m_device=nullptr;
    VulkanImageView* m_screen_view=nullptr;
    VulkanImageView* m_depth_view=nullptr;
    VulkanGraphicsPipline* m_pipeline=nullptr;
    VulkanSwapChain* m_swapchain=nullptr;
    VulkanVertexBuffer* m_vertex_buffer=nullptr;

    std::vector<VkImageView> m_swapchain_views;

    size_t m_current_frame = 0;
    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;
    std::vector<VkFence> m_image_in_flight;

    VkFormat m_depth_format;

    VkCommandBuffer* m_command=nullptr;
    uint32_t m_command_count;

    bool render();
    void createSyncObjects();
    VulkanImageView* copyScreen(VkImage);
};