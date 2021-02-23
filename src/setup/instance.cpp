#include "instance.hpp"


VulkanInstance::VulkanInstance(VulkanConfiguration& config)
{
    if(enable_validation_layers && !checkValidationLayerSupport()) {
        printff("validation layers requested, but not available!\n");
    }
    printfi("Getting Debug Info...\n");
    VkDebugUtilsMessengerCreateInfoEXT debug_info = init::debug_messenger_info(debugCallback);

    printfi("Getting App Info...\n");
    VkApplicationInfo app_info = init::application_info(&config);

    m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_extensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_XCB_KHR) 
    m_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
    m_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    
    printfi("Getting Instance Info...\n");
    VkInstanceCreateInfo instance_info = init::instance_info(&app_info, m_extensions.data(), m_extensions.size(), &debug_info);

    /* uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "available extensions: \n";
    for(const auto& e : extensions) {
        std::cout << "\tname: " << e.extensionName << " version: " << e.specVersion << "\n";
    } */
    
    printfi("Create Instance... \n");
    ErrorCheck(vkCreateInstance(&instance_info, NULL, &m_instance), "Create Instance");
    setupDebugMessenger();
}

VulkanInstance::~VulkanInstance()
{
    printfi("-- Destroying Validation Layer...\n");
    if(enable_validation_layers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
    }

    printfi("-- Destroying Instance...\n");
    vkDestroyInstance(m_instance, NULL);
}

VkInstance& VulkanInstance::GetInstance()
{
    return m_instance;
}

std::vector<const char*> getRequiredExtensions()
{
    // OPTIONAL TODO: add extension i.e. glfw, sdl, etc
    uint32_t type_extension_count = 0;
    const char** type_extensions;
    // glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions;
    // std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    if(enable_validation_layers) {
        //extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanInstance::setupDebugMessenger()
{
    if(!enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT info = init::debug_messenger_info(debugCallback);

    ErrorCheck(CreateDebugUtilsMessengerEXT(
        m_instance, 
        &info, 
        nullptr, 
        &m_debug_messenger
    ), "Create Deubg Utils Messenger");
}

bool VulkanInstance::checkValidationLayerSupport() 
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for(const char* name : validation_layers)
    {
        bool layer_found = false;

        for(const auto& prop : available_layers) 
        {
            if(strcmp(name, prop.layerName) == 0) 
            {
                layer_found = true;
                break;
            }
        }

        if(!layer_found) return false;
    } 

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* pcallback_data,
    void* puser_data) 
{
    switch(message_severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            // printfv("%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            printf("%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            printfw("%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printfe("%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            printfw("Now this is a werid one, but: \n\t%s\n", pcallback_data->pMessage);
            break;
    }

    switch(message_type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            // printfv("general type --> %s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            // printfw("validation type -->%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            printfw("performance type -->%s\n", pcallback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT:
            printfw("type --> Now this is a werid one,but: \n\t%s\n", pcallback_data->pMessage);
            break;
    }

    // std::cerr << pcallback_data->pMessage << std::endl;

    return VK_FALSE;
}