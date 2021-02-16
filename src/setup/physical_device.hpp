#pragma once

#include "build_order.hpp"
#include "queue_family.hpp"
#include "instance.hpp"
#include "surface.hpp"

class VulkanInstance;

class VulkanPhysicalDevice
{
public:
    ~VulkanPhysicalDevice();
    VkPhysicalDevice& GetDevice();
    QueueFamilyIndices& GetQueueFamily();
    VkPhysicalDeviceProperties& GetProperties();
    VkPhysicalDeviceFeatures& GetFeatures();
    VkPhysicalDeviceMemoryProperties& GetMemoryProperties();

    static VulkanPhysicalDevice* GetPhysicalDevice(VulkanInstance*, VulkanSurface* surface=nullptr, bool swapchain_needed=true);
    
private:
    VulkanInstance* m_instance;
    VkPhysicalDevice m_device;
    QueueFamilyIndices m_queue_family;
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceMemoryProperties m_memory_properties;

    static const std::vector<const char*> device_extensions;

    VulkanPhysicalDevice(VulkanInstance*, VkPhysicalDevice, QueueFamilyIndices);
    static std::vector<VkPhysicalDevice> getAvailablePhysicalDevice(VulkanInstance*);
    static bool hasPhysicalDeviceSupport(VkPhysicalDevice*, QueueFamilyIndices*, VulkanSurface*, bool);
    static bool hasSupportQueueFamily(VkPhysicalDevice*, QueueFamilyIndices*, VulkanSurface*);
    static bool hasDeviceSwapChainSupport(VkPhysicalDevice, const std::vector<const char*>);
};