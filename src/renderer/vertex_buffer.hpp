#pragma once

#include "build_order.hpp"
#include "device.hpp"
struct Vertex{
    glm::vec4 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription binding_description = {};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        printfv("%d\n",binding_description.inputRate);
        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {        
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

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
