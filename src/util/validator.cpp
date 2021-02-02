#include "validator.hpp"

// TODO: create proper error checker
void ErrorCheck(VkResult result, std::string msg) 
{
    if(result != VK_SUCCESS) {
        switch (result)
        {
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            printfw("EXTENSION_NOT_PRESENT %s\n", msg.c_str());
            return;
            break;
        default:
            break;
        }

        printff((std::string("Failed to ") + msg + "\n").c_str());
    }    
}



VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* pcreate_info, const VkAllocationCallbacks* pallocator, VkDebugUtilsMessengerEXT* pdebug_messenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pcreate_info, pallocator, pdebug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* pallocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr){
        func(instance, debug_messenger, pallocator);
    }
}