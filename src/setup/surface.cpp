#include "surface.hpp"

VulkanSurface::VulkanSurface(VulkanInstance* instance, WindowSettings settings, uint32_t width, uint32_t height) 
{
    m_width = width;
    m_height = height;
    m_instance = instance;
    m_settings = settings;
	printfi("Setting up Window...\n");
    setupWindow();
	printfi("Creating Surface...\n");
    initSurface();
}

VulkanSurface::~VulkanSurface()
{
	if(m_surface != NULL) {
		printfi("-- Destroying Surface...\n");
    	vkDestroySurfaceKHR(m_instance->GetInstance(), m_surface, nullptr);
	}
	
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(m_connection, m_window);
	if(m_connection != NULL) {
		xcb_disconnect(m_connection);
	}
#endif
}

void VulkanSurface::MainLoop(VkDevice device, std::function<bool()> func) 
{
	bool run = true;
#if defined(VK_USE_PLATFORM_XCB_KHR)
	if(m_connection == nullptr) {
		printfe("XCB connection is null\n");
		return;
	}
	xcb_flush(m_connection);
	while(run) {
		xcb_generic_event_t* event;
		while((event = xcb_poll_for_event(m_connection)))
		{
			run = handleEvent(event);
			free(event);
		}

		run = func() && run;
		if(!run) printfi("WINDOW IS CLOSING...\n");
	}
#endif

	vkDeviceWaitIdle(device);
}

VkSurfaceKHR VulkanSurface::GetSurface() 
{
	return m_surface;
}

#if defined(VK_USE_PLATFORM_XCB_KHR)
bool VulkanSurface::handleEvent(const xcb_generic_event_t* event) 
{
	switch(event->response_type & 0x07f)
	{
		case XCB_CLIENT_MESSAGE:
			if ((*(xcb_client_message_event_t*)event).data.data32[0] ==
				(*m_atom_wm_delete_window).atom) {
				printf("CLIENT MESSAGE EVENT\n");
				return false;
			}
		break;
		case XCB_DESTROY_NOTIFY:
			printf("DESTROY NOTIFY EVENT\n");
			return false;
		break;
		case XCB_KEY_PRESS:
		{
			const xcb_key_release_event_t* press_event = (const xcb_key_release_event_t*) event;
			switch(press_event->detail)
			{

			}
		}
		break;
		case XCB_KEY_RELEASE:
		{
			const xcb_key_release_event_t* release_event = (const xcb_key_release_event_t*) event;
			switch(release_event->detail)
			{
				case KEY_ESCAPE:
					printf("CLIENT ESCAPE RELEASE\n");
					return false;
				break;
			}
		}
		break;
		default:
		break;
	}

	return true;
}

static xcb_intern_atom_reply_t* intern_atom_helper(xcb_connection_t *conn, bool only_if_exists, const char *str) 
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
	return xcb_intern_atom_reply(conn, cookie, NULL);
}

void VulkanSurface::setupWindow()
{
    // * connect to X server
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    m_connection = xcb_connect(NULL, &scr);
    assert(m_connection);
    if(xcb_connection_has_error(m_connection))
    {
        printfe("Failed to find a compatible Vulkan ICD! (aka xcb connection failed)");
        fflush(stdout);
        exit(1);
    }

    setup = xcb_get_setup(m_connection);
    iter = xcb_setup_roots_iterator(setup);
    while(scr-- > 0) {
        xcb_screen_next(&iter);
    }
    m_screen = iter.data;

    // * generate window
    uint32_t value_mask, value_list[32];

    m_window = xcb_generate_id(m_connection);
    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_screen->black_pixel;

    value_list[1] =
		XCB_EVENT_MASK_KEY_RELEASE |
		XCB_EVENT_MASK_KEY_PRESS |
		XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_BUTTON_PRESS |
		XCB_EVENT_MASK_BUTTON_RELEASE;

	if (m_settings.fullscreen)
	{
		m_width = m_dest_width = m_screen->width_in_pixels;
		m_height = m_dest_height = m_screen->height_in_pixels;
	}

    xcb_create_window(m_connection,
		XCB_COPY_FROM_PARENT,
	    m_window, m_screen->root,
		0, 0, m_width, m_height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		m_screen->root_visual,
		value_mask, value_list
    );

	/* Magic code that will send notification when window is destroyed */
	xcb_intern_atom_reply_t* reply = intern_atom_helper(m_connection, true, "WM_PROTOCOLS");
	m_atom_wm_delete_window = intern_atom_helper(m_connection, false, "WM_DELETE_WINDOW");

	xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE,
		m_window, (*reply).atom, 4, 32, 1,
		&(*m_atom_wm_delete_window).atom);

	std::string windowTitle = m_settings.title;
	xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE,
		m_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
		m_settings.title.size(), windowTitle.c_str());

	free(reply);

	/**
	 * Set the WM_CLASS property to display
	 * title in dash tooltip and application menu
	 * on GNOME and other desktop environments
	 */
	std::string wm_class;
    std::string name = m_settings.title;
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
	wm_class = wm_class.insert(0, name);
	wm_class = wm_class.insert(name.size(), 1, '\0');
	wm_class = wm_class.insert(name.size() + 1, m_settings.title);
	wm_class = wm_class.insert(wm_class.size(), 1, '\0');
	xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, wm_class.size() + 2, wm_class.c_str());

	if (m_settings.fullscreen)
	{
		xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper(m_connection, false, "_NET_WM_STATE");
		xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper(m_connection, false, "_NET_WM_STATE_FULLSCREEN");
		xcb_change_property(m_connection,
				XCB_PROP_MODE_REPLACE,
				m_window, atom_wm_state->atom,
				XCB_ATOM_ATOM, 32, 1,
				&(atom_wm_fullscreen->atom));
		free(atom_wm_fullscreen);
		free(atom_wm_state);
	}

    // * display the window
	xcb_map_window(m_connection, m_window);
    xcb_flush(m_connection);
}

void VulkanSurface::initSurface()
{
    VkXcbSurfaceCreateInfoKHR info = init::surface_info(m_connection, m_window);

    ErrorCheck(
		vkCreateXcbSurfaceKHR(
            m_instance->GetInstance(), 
            &info, nullptr, 
            &m_surface
        ), "Create XCB Surface"
    );
}
#elif defined(VK_USE_PLATFORM_WIN32_KHR)	
void VulkanSurface::SetupWindow()
{

}

void VulkanSurface::InitSurface()
{

}
#endif