#pragma once

#include "build_order.hpp"
#include "instance.hpp"
#include "physical_device.hpp"
#include "pipeline.hpp"

class VulkanPhysicalDevice;

class VulkanDevice 
{
public:
    VulkanDevice(){}
    VulkanDevice(VulkanInstance*, VulkanPhysicalDevice*);
    ~VulkanDevice();
    VkDevice GetDevice();
    VulkanPhysicalDevice* GetPhysicalDevice();
    VulkanInstance* GetInstance();
    VkQueue GetComputeQueue();
    VkQueue GetGraphicsQueue();
    VkCommandPool& GetComputeCommandPool();
    VkCommandPool& GetGraphicsCommandPool();
    void SetComputeCommand(VkCommandBuffer*, uint32_t);
    void FreeComputeCommand(VkCommandBuffer*, uint32_t);
    VkCommandBuffer BeginSingleCommand(); // move this in VulkanDevice?
    void EndSingleCommand(VkCommandBuffer, uint32_t flag=0); 
    void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer*, VkDeviceMemory*, void* data=nullptr);
    void CopyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
    void SubmitWork(VkCommandBuffer, VkQueue);
    uint32_t FindMemoryType(uint32_t, VkMemoryPropertyFlags);
    bool GetSupportedDepthFormat(VkFormat* depthFormat);
private:
    VkDevice m_device;
    VkQueue m_compute_queue=NULL;
    VkQueue m_graphics_queue=NULL;
    VulkanInstance* m_instance;
    VulkanPhysicalDevice* m_physical_device;
    VkCommandPool m_ccompute_pool;
    VkCommandPool m_cgraphics_pool;

    void createFrameBuffers(
        std::vector<VkImageView>, 
        VkRenderPass render_pass,
        uint32_t width, uint32_t height
    );
    void createCommandPool(VkCommandPool*, uint32_t, VkCommandPoolCreateFlags);
    
};