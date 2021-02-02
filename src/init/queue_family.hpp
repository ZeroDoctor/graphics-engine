#pragma once

#include "build_order.hpp"

struct QueueFamilyIndices 
{
    uint32_t graphics_indices = UINT32_MAX;
    uint32_t compute_indices = UINT32_MAX;
};