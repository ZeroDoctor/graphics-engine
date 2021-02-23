#include "physical_device.hpp"

const std::vector<const char*> VulkanPhysicalDevice::device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance* instance, VkPhysicalDevice device, QueueFamilyIndices queue_family, bool swapchain_needed) 
{
    m_instance = instance;
    m_device = device;
    m_queue_family = queue_family;
    m_swapchain_needed = swapchain_needed;

    vkGetPhysicalDeviceProperties(
        m_device,
        &m_properties
    );

    vkGetPhysicalDeviceFeatures(
        m_device,
        &m_features
    );

    vkGetPhysicalDeviceMemoryProperties(
        m_device,
        &m_memory_properties
    );

    std::string device_type = "Unknown";
    switch(m_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: device_type = "Other"; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: device_type = "Integrated GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: device_type = "Discrete GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: device_type = "Virtual GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: device_type = "CPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM: break;
    }

    // TODO: only use in verbose mode
    std::cout << "------------------ Device Properties ------------------\n";

    std::cout << "device_type: " << device_type << "\n";
    std::cout << "device_name: " << m_properties.deviceName << "\n";
    std::cout << "driver_version: " << m_properties.driverVersion << "\n";
    std::cout << "vender_id: " << m_properties.vendorID << "\n";
    std::cout << "geometry_shader: " << ((m_features.geometryShader == 1) ? "true" : "false") << "\n";
    std::cout << "fill_mode_nonsolid: " << ((m_features.fillModeNonSolid == 1) ? "true" : "false")  << "\n";
    std::cout << "wide_lines: " << ((m_features.wideLines) == 1 ? "true" : "false") << "\n";
    std::cout << "sampler_anisotropy: " << ((m_features.samplerAnisotropy) == 1 ? "true" : "false") << "\n";
    std::cout << "heap_count: " << m_memory_properties.memoryHeapCount << "\n";

    std::cout << "------------------------------------------------------\n";

    if(!m_features.samplerAnisotropy) { // needed for fonts
        printff("Support for sampler anisotropy is required. Hint: check if physical device supports sampler anisotropy.\n");
    }

}

VulkanPhysicalDevice::~VulkanPhysicalDevice() 
{

}

VulkanPhysicalDevice* VulkanPhysicalDevice::GetPhysicalDevice(VulkanInstance* instance, VulkanSurface* surface, bool swapchain_needed) 
{
    std::vector<VkPhysicalDevice> devices = getAvailablePhysicalDevice(instance);

    VkPhysicalDevice secondary_device = VK_NULL_HANDLE;
    QueueFamilyIndices secondary_queue;

    for(auto &d : devices)
    {
        QueueFamilyIndices queue_family;
        if(hasPhysicalDeviceSupport(&d, &queue_family, surface, swapchain_needed))
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(d, &properties);

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                printfi("Found Discrete GPU\n");
                return new VulkanPhysicalDevice(instance, d, queue_family, swapchain_needed);
            } else {
                secondary_device = d;
                secondary_queue = queue_family;
            }
        }
    }

    // just a redundant check
    if(secondary_device == VK_NULL_HANDLE) return nullptr;

    return new VulkanPhysicalDevice(instance, secondary_device, secondary_queue, swapchain_needed);
}

VkPhysicalDevice& VulkanPhysicalDevice::GetDevice() { return m_device; }
QueueFamilyIndices& VulkanPhysicalDevice::GetQueueFamily() { return m_queue_family; }
VkPhysicalDeviceFeatures& VulkanPhysicalDevice::GetFeatures() { return m_features; }
VkPhysicalDeviceMemoryProperties& VulkanPhysicalDevice::GetMemoryProperties() { return m_memory_properties; }
bool VulkanPhysicalDevice::HasSwapchainEnabled() { return m_swapchain_needed; }

std::vector<VkPhysicalDevice> VulkanPhysicalDevice::getAvailablePhysicalDevice(VulkanInstance* instance) 
{
    uint32_t device_count = 0;

    vkEnumeratePhysicalDevices (
        instance->GetInstance(),
        &device_count,
        nullptr
    );

    if(device_count == 0) {
        printff("Failed to find vulkan supported physical device");
    }

    std::vector<VkPhysicalDevice> devices(device_count);

    vkEnumeratePhysicalDevices (
        instance->GetInstance(),
        &device_count,
        devices.data()
    );

    // TODO: could find best suitable device i.e. Dedicated GPU

    return devices;
}

bool VulkanPhysicalDevice::hasPhysicalDeviceSupport(VkPhysicalDevice* device, QueueFamilyIndices* queue_family, VulkanSurface* surface, bool swapchain_needed)
{
    bool support_family = hasSupportQueueFamily(device, queue_family, surface);
    bool support_swapchain = hasDeviceSwapChainSupport(*device, device_extensions);

    if(support_swapchain) {
        SwapChainSupportDetails details = querySwapChainSupport(*device, surface->GetSurface());
        support_swapchain = !details.formats.empty() && !details.present_modes.empty();
    }

    if(support_family) {
        printfi("Found supported queue family...\n");
    }

    if(support_swapchain && swapchain_needed) {
        printfi("Found supported swap chain logical device...\n");
    } else if(!support_swapchain && swapchain_needed) {
        printfe("Failed to find supported swap chain logical device...\n");
    } else {
        printfw("Found non-supported swap chain logical device...\n");
    }

    bool supported = support_family && !(!support_swapchain && swapchain_needed);
    printfi("Device Supported: %s\n", supported ? "true" : "false");
    return supported;
}

bool VulkanPhysicalDevice::hasSupportQueueFamily(VkPhysicalDevice* device, QueueFamilyIndices* queue_indices, VulkanSurface* surface) 
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        *device,
        &queue_family_count,
        nullptr
    );

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        *device,
        &queue_family_count,
        queue_families.data()
    ); 

    for(uint32_t i = 0; i < queue_families.size(); i++)
    {   
        VkQueueFamilyProperties qf = queue_families[i];
        if(qf.queueCount > 0) 
        {
            if(qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queue_indices->graphics_index = i;
            }
            
            if(qf.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queue_indices->compute_index = i;
            }

            if(surface != nullptr) 
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, surface->GetSurface(), &present_support);
                if(present_support) {
                    queue_indices->present_index = i;
                    printfi("Found present index at: %d\n", i);
                }
            }
        }

        if(queue_indices->graphics_index < UINT32_MAX && 
            queue_indices->compute_index < UINT32_MAX && 
            (surface != nullptr && queue_indices->present_index < UINT32_MAX)) return true;
    }

    return false;
}

bool VulkanPhysicalDevice::hasDeviceSwapChainSupport(VkPhysicalDevice device, const std::vector<const char*> device_extensions)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
        device, 
        nullptr, 
        &extension_count, 
        nullptr
    );

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        device, 
        nullptr, 
        &extension_count, 
        available_extensions.data()
    );

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for(const auto& e : available_extensions) {
        required_extensions.erase(e.extensionName);
    }

    return required_extensions.empty();
}

SwapChainSupportDetails VulkanPhysicalDevice::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) 
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if(format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if(present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}