#pragma once

#include "build_order.hpp"
#include "vulkan_config.hpp"

class VulkanInstance
{
public:
    VulkanInstance(VulkanConfiguration&);
    ~VulkanInstance();

    VkInstance& GetInstance();

private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger; // could be in its own class
    std::vector<const char*> m_extensions;
    
    std::vector<const char*> getRequiredExtensions();
    void setupDebugMessenger();
    bool checkValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, 
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*);

};