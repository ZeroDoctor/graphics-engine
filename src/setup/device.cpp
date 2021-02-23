#include "device.hpp"

VulkanDevice::VulkanDevice(VulkanInstance* instance, VulkanPhysicalDevice* physical_device)
{
    m_instance = instance;
    m_physical_device = physical_device;

    uint32_t compute_index = m_physical_device->GetQueueFamily().compute_index;
    uint32_t graphics_index = m_physical_device->GetQueueFamily().graphics_index;
    uint32_t present_index = m_physical_device->GetQueueFamily().present_index;
    std::set<uint32_t> unique_queue_indices = {compute_index, graphics_index};
    if(m_physical_device->HasSwapchainEnabled()) {
        unique_queue_indices.insert(present_index);
    }

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float graphics_priority = 1.0f;
    for(uint32_t queue_family : unique_queue_indices)
    {
        queue_create_infos.push_back(
            init::device_queue_info(queue_family, &graphics_priority)
        );
    }

    std::vector<const char*> device_extensions = m_physical_device->device_extensions;
    VkDeviceCreateInfo device_info = init::device_info(
        queue_create_infos.data(), queue_create_infos.size(), 
        &m_physical_device->GetFeatures(), device_extensions
    );

    ErrorCheck(vkCreateDevice(
        m_physical_device->GetDevice(),
        &device_info,
        nullptr,
        &m_device
    ), "Create Device");

    printfi("Creating compute queue\n");
    vkGetDeviceQueue(
        m_device,
        compute_index,
        0,
        &m_compute_queue
    );

    if(graphics_index < UINT32_MAX) {
        printfi("Creating graphics queue\n");
        vkGetDeviceQueue(
            m_device,
            graphics_index,
            0,
            &m_graphics_queue
        );
    } else {
        printfw("Failed to find suitable graphics indices");
    }

    if(present_index < UINT32_MAX) {
        printfi("Creating present queue\n");
        vkGetDeviceQueue(
            m_device,
            present_index,
            0,
            &m_present_queue
        );
    } else {
        printfw("Failed to find suitable presentation indices");
    }

    createCommandPool(&m_ccompute_pool, compute_index, 0);
    createCommandPool(&m_cgraphics_pool, graphics_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

VulkanDevice::~VulkanDevice()
{
    printfi("-- Destroying Command Pools...\n");
    vkDestroyCommandPool(m_device, m_cgraphics_pool, nullptr);
    vkDestroyCommandPool(m_device, m_ccompute_pool, nullptr);
    
    printfi("-- Destroying Logcial Device...\n");
    vkDestroyDevice(m_device, nullptr);

    if(m_physical_device != nullptr) {
        printfi("-- Destroying Physical Device...\n");
        delete(m_physical_device);
    }
}

VkDevice VulkanDevice::GetDevice() { return m_device; }
VulkanPhysicalDevice* VulkanDevice::GetPhysicalDevice() { return m_physical_device; }
VulkanInstance* VulkanDevice::GetInstance() { return m_instance; }
VkQueue VulkanDevice::GetComputeQueue() { return m_compute_queue; }
VkQueue VulkanDevice::GetGraphicsQueue() { return m_graphics_queue; }
VkQueue VulkanDevice::GetPresentQueue() { return m_present_queue; }
VkCommandPool& VulkanDevice::GetComputeCommandPool() { return m_ccompute_pool; }
VkCommandPool& VulkanDevice::GetGraphicsCommandPool() { return m_cgraphics_pool; }

void VulkanDevice::SetComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
    VkCommandBufferAllocateInfo buffer_allocate_info = init::command_buffer_allocate_info(m_cgraphics_pool, count);
    ErrorCheck(vkAllocateCommandBuffers(
        m_device,
        &buffer_allocate_info,
        buffers
    ), "Allocate Command Buffers");
}

void VulkanDevice::FreeComputeCommand(VkCommandBuffer* buffers, uint32_t count)
{
    vkFreeCommandBuffers(
        m_device,
        m_cgraphics_pool,
        count,
        buffers
    );
}

VkCommandBuffer VulkanDevice::BeginSingleCommand()
{
    VkCommandBufferAllocateInfo alloc_info = init::command_buffer_allocate_info(
        m_cgraphics_pool,
        1
    );

    VkCommandBuffer command_buffer;
    ErrorCheck(vkAllocateCommandBuffers(
        m_device, 
        &alloc_info, 
        &command_buffer
    ), "Allocate Command Buffers");

    VkCommandBufferBeginInfo begin_info = init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    ErrorCheck(vkBeginCommandBuffer(
        command_buffer, 
        &begin_info
    ), "Begin Command Buffer");

    return command_buffer;
}

// using a flag for now
void VulkanDevice::EndSingleCommand(VkCommandBuffer command_buffer, uint32_t flag)
{
    ErrorCheck(vkEndCommandBuffer(command_buffer), "End Command Buffer");

    if(flag != 0) {
        SubmitWork(command_buffer, m_graphics_queue);
        return;
    }

    VkSubmitInfo submit_info = init::submit_info(1, &command_buffer);

    if(m_graphics_queue == NULL) {
        printff("debug --> graphics queue not set\n");
    }
    ErrorCheck(vkQueueSubmit(
        m_graphics_queue, 
        1, 
        &submit_info, 
        VK_NULL_HANDLE
    ), "Submit to Queue");
    ErrorCheck(vkQueueWaitIdle(m_graphics_queue), "Idle Queue");

    vkFreeCommandBuffers(m_device, m_cgraphics_pool, 1, &command_buffer);
}

void VulkanDevice::CreateBuffer(
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        VkBuffer* buffer, 
        VkDeviceMemory* buffer_memory,
        void* data
    ) 
{
    VkBufferCreateInfo buffer_info = init::buffer_info(size, usage);

    ErrorCheck(vkCreateBuffer(
        m_device, 
        &buffer_info, 
        nullptr,
        buffer
    ), "Create Buffer");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(
        m_device,
        *buffer,
        &mem_requirements
    );

    VkMemoryAllocateInfo alloc_info = init::memory_allocate_info(
        mem_requirements,
        FindMemoryType(mem_requirements.memoryTypeBits, properties)
    );

    ErrorCheck(vkAllocateMemory(
        m_device,
        &alloc_info,
        nullptr,
        buffer_memory
    ), "Allocate Buffer Memory");

    if (data != nullptr) {
        void *mapped;
        ErrorCheck(vkMapMemory(
            m_device, 
            *buffer_memory, 
            0, size, 0, 
            &mapped
        ), "Map Memory");
        memcpy(mapped, data, size);
        vkUnmapMemory(m_device, *buffer_memory);
    }

    ErrorCheck(vkBindBufferMemory(
        m_device,
        *buffer,
        *buffer_memory,
        0
    ), "Bind Buffer Memory");
}

void VulkanDevice::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) 
{
    VkCommandBuffer command_buffer = BeginSingleCommand();
    
    VkBufferCopy copy_region = {};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    EndSingleCommand(command_buffer, 1);

    vkFreeCommandBuffers(m_device, m_cgraphics_pool, 1, &command_buffer);
}

// using a fence to sync queue
void VulkanDevice::SubmitWork(VkCommandBuffer cmd_buffer, VkQueue queue)
{
    if(m_graphics_queue == NULL) printff("graphics queue is not set!\n");

    VkSubmitInfo submit_info = init::submit_info(1, &cmd_buffer);
    VkFenceCreateInfo fence_info = init::fence_info();
    VkFence fence;

    printfi("Submiting Work with Fence...\n");
    ErrorCheck(vkCreateFence(
        m_device, &fence_info, nullptr, &fence
    ), "Create Fence");
    ErrorCheck(vkQueueSubmit(
        m_graphics_queue, 1, &submit_info, fence
    ), "Queue Submit");
    ErrorCheck(vkWaitForFences(
        m_device, 1, &fence, VK_TRUE, UINT64_MAX
    ), "Wait For Fence");

    vkDestroyFence(m_device, fence, nullptr);
    printfi("Done Submiting Work...\n");
}

uint32_t VulkanDevice::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(
        m_physical_device->GetDevice(),
        &memory_properties
    );

    for(uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if(type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    printff("Failed to find suitable memory type!\n");
    return 0;
}

bool VulkanDevice::GetSupportedDepthFormat(VkFormat* depthFormat) 
{
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for(auto f : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(m_physical_device->GetDevice(), f, &formatProps);

        if(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            *depthFormat = f;
            return true;
        }
    }

    return false;
}

void VulkanDevice::createCommandPool(VkCommandPool* command_pool, uint32_t indices, VkCommandPoolCreateFlags flag) 
{
    VkCommandPoolCreateInfo compute_pool_info = init::command_pool_info(indices, flag);

    ErrorCheck(vkCreateCommandPool(
        m_device,
        &compute_pool_info,
        nullptr,
        command_pool
    ), "Create Command Pool");
}

