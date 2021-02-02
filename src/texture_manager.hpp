#pragma once

#include "build_order.hpp"
#include "image_view.hpp"
#include "area.hpp"

// TODO: cleanup later
struct TextureWindow {
    float x0;
    float y0;
    float x1;
    float y1;
};

struct ITexture {
    virtual VkDescriptorSet GetDescriptorSet() = 0;
    virtual TextureWindow GetTextureWindow() = 0;
    virtual int GetWidth() = 0;
    virtual int GetHeight() = 0;
};

class TextureAtlas;

class AtlasTexture : public ITexture {
public:
    explicit AtlasTexture(TextureAtlas* owner, Area* area);

    VkDescriptorSet GetDescriptorSet() override;
    TextureWindow GetTextureWindow() override;

    int GetWidth() override;
    int GetHeight() override;

protected:
    TextureAtlas* m_owner;
    Area* m_area;
};

class TextureAtlas : public ITexture {

public:
    TextureAtlas();
    ~TextureAtlas();

    void Init(VulkanImageView*, uint32_t, uint32_t);
    AtlasTexture* Add(const std::string& filename);
    AtlasTexture* Add(uint32_t, uint32_t, uint8_t*);

    VkDescriptorSet GetDescriptorSet() override;
    TextureWindow GetTextureWindow() override;
    int GetWidth() override;
    int GetHeight() override;

protected:
    uint32_t m_width;
    uint32_t m_height;

    VulkanImageView* m_image_view;
    VkImage m_image;
    VkDeviceMemory m_image_memory;
    VkSampler m_sampler;
    VkDescriptorSet m_descriptor_set;
    AreaAllocator m_allocator;
};

