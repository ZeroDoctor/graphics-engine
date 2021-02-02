#include "vertex_buffer.hpp"


VulkanVertexBuffer::VulkanVertexBuffer(VulkanDevice* device, std::vector<Vertex> verts, std::vector<uint16_t> indices)
{
    m_device = device;
    m_verts = verts;
    m_indices = indices;

    createVertexBuffer(verts, &m_vertex_buffer, &m_vertex_buffer_memory);
    createIndexBuffer(indices, &m_index_buffer, &m_index_buffer_memory);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    printfi("-- Destroying Index Buffer...\n");
    vkDestroyBuffer(m_device->GetDevice(), m_index_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), m_index_buffer_memory, nullptr);

    printfi("-- Destorying Vertex Buffer...\n");
    vkDestroyBuffer(m_device->GetDevice(), m_vertex_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), m_vertex_buffer_memory, nullptr);
}

VulkanDevice* VulkanVertexBuffer::GetVulkanDevice() { return m_device; }
VkBuffer VulkanVertexBuffer::GetVertexBuffer() { return m_vertex_buffer; }
VkDeviceMemory VulkanVertexBuffer::GetVertexBufferMemory() { return m_vertex_buffer_memory; }

VkBuffer VulkanVertexBuffer::GetIndexBuffer() { return m_index_buffer;}
VkDeviceMemory VulkanVertexBuffer::GetIndexDeviceMemory() { return m_index_buffer_memory;}

std::vector<Vertex> VulkanVertexBuffer::GetVerts() { return m_verts; }
std::vector<uint16_t> VulkanVertexBuffer::GetIndices() { return m_indices; }

void VulkanVertexBuffer::createVertexBuffer(
        std::vector<Vertex> vect, VkBuffer* buffer, 
        VkDeviceMemory* buffer_memory
    ) 
{
    printfi("Creating Vertex Buffer of size %d...\n", vect.size());

    VkDeviceSize size = sizeof(vect[0]) * vect.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    m_device->CreateBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory, vect.data()
    );

    m_device->CreateBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory
    );

    m_device->CopyBuffer(staging_buffer, *buffer, size);

    vkDestroyBuffer(m_device->GetDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), staging_buffer_memory, nullptr);
}

void VulkanVertexBuffer::createIndexBuffer(std::vector<uint16_t> vect, VkBuffer* buffer, VkDeviceMemory* buffer_memory) 
{
    printfi("Creating Index Buffer of size %d...\n", vect.size());

    VkDeviceSize size = sizeof(vect[0]) * vect.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    m_device->CreateBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer, &staging_buffer_memory, vect.data()
    );

    m_device->CreateBuffer(
        size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, buffer_memory
    );

    m_device->CopyBuffer(staging_buffer, *buffer, size);

    vkDestroyBuffer(m_device->GetDevice(), staging_buffer, nullptr);
    vkFreeMemory(m_device->GetDevice(), staging_buffer_memory, nullptr);
}
