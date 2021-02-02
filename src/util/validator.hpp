#pragma once

#include "build_order.hpp"

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

void ErrorCheck(VkResult,std::string msg="To Do Something");

VkResult CreateDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);