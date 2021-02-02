#pragma once

#include "build_order.hpp"
#include "device.hpp"
#include "vertex.hpp"

class VulkanDevice;

class VulkanVertexBuffer
{
public:
    VulkanVertexBuffer(VulkanDevice* device, std::vector<Vertex>, std::vector<uint16_t>);
    ~VulkanVertexBuffer();
    
    VulkanDevice* GetVulkanDevice();
    VkBuffer GetVertexBuffer();
    VkDeviceMemory GetVertexBufferMemory();
    VkBuffer GetIndexBuffer();
    VkDeviceMemory GetIndexDeviceMemory();
    std::vector<Vertex> GetVerts();
    std::vector<uint16_t> GetIndices();
private:
    VulkanDevice* m_device;
    VkBuffer m_vertex_buffer;
    VkDeviceMemory m_vertex_buffer_memory;
    VkBuffer m_index_buffer;
    VkDeviceMemory m_index_buffer_memory;
    std::vector<Vertex> m_verts;
    std::vector<uint16_t> m_indices;

    void createVertexBuffer(std::vector<Vertex>, VkBuffer*, VkDeviceMemory*);
    void createIndexBuffer(std::vector<uint16_t>, VkBuffer*, VkDeviceMemory*);
};
