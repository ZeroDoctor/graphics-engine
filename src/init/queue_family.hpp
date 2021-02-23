#pragma once

#include "build_order.hpp"

struct QueueFamilyIndices 
{
    uint32_t graphics_index = UINT32_MAX;
    uint32_t compute_index = UINT32_MAX;
    uint32_t present_index = UINT32_MAX;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};
