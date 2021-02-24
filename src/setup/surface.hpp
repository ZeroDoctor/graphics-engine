#pragma once

#include "build_order.hpp"
#include <functional>
#include "instance.hpp"
#include "keycodes.hpp"

struct WindowSettings {
    bool validation=false;
    bool fullscreen=false;
    bool vsync=false;
    bool overlay=false;
    std::string title;
};

class VulkanSurface 
{
public:
    VulkanSurface(VulkanInstance*, WindowSettings, uint32_t, uint32_t);
    ~VulkanSurface();

    void MainLoop(VkDevice, std::function<bool()> func);
    VkSurfaceKHR GetSurface();

private:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_dest_width;
	uint32_t m_dest_height;

    VulkanInstance* m_instance;
    VkSurfaceKHR m_surface=NULL;
    WindowSettings m_settings;

#if defined(VK_USE_PLATFORM_XCB_KHR)
	bool m_quit = false;
	xcb_connection_t *m_connection=nullptr;
	xcb_screen_t *m_screen;
	xcb_window_t m_window;
	xcb_intern_atom_reply_t *m_atom_wm_delete_window;

    bool handleEvent(const xcb_generic_event_t*);
#endif

    void setupWindow();
    void initSurface();
};
