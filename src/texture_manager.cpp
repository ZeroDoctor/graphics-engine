#include "texture_manager.hpp"

AtlasTexture::AtlasTexture(TextureAtlas* owner, Area* area) 
{
    m_owner = owner;
    m_area = area;
}

VkDescriptorSet AtlasTexture::GetDescriptorSet() { return m_owner->GetDescriptorSet(); }

TextureWindow AtlasTexture::GetTextureWindow() 
{
    float x0 = float(m_area->x) / float(m_owner->GetWidth());
    float y0 = float(m_area->y) / float(m_owner->GetHeight());
    float x1 = x0 + float(m_area->width) / float(m_owner->GetWidth());
    float y1 = y0 + float(m_area->height) / float(m_owner->GetHeight());
    return TextureWindow{x0, y0, x1, y1};
}

int AtlasTexture::GetWidth() { return m_area->width; }
int AtlasTexture::GetHeight() { return m_area->height; }

TextureAtlas::TextureAtlas() 
{
    m_width = 0;
    m_height = 0;
    m_image_view = nullptr;
}

TextureAtlas::~TextureAtlas() 
{
    if(m_image_view != nullptr) {
        delete m_image_view;
    }
}

void TextureAtlas::Init(VulkanImageView* image_view, uint32_t width, uint32_t height) 
{
    m_width = width;
    m_height = height;
    
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    image_view->GenerateTextureImage(width, height, format, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    VkImageAspectFlags flags[] = {VK_IMAGE_ASPECT_COLOR_BIT};
    image_view->CreateImageView(flags);
    image_view->CreateTextureSampler();
    m_sampler = image_view->GetSamplers()[0];
    m_image = image_view->GetImages()[0];
    m_image_memory = image_view->GetImageMemories()[0];
    m_image_view = image_view;

    m_allocator.Initialize(width, height);

}

AtlasTexture* TextureAtlas::Add(const std::string& filename) 
{
    int width;
    int height;
    int channels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if(!pixels) {
        printff("failed to load texture image\n");
    }

    auto result = Add(static_cast<uint32_t>(width), static_cast<uint32_t>(height), pixels);

    stbi_image_free(pixels);

    return result;
}

AtlasTexture* TextureAtlas::Add(uint32_t width, uint32_t height, uint8_t* pixels) 
{
    auto area = m_allocator.Allocate(width, height);
    if(!area) {
        return nullptr;
    }

    m_image_view->LoadImage(width, height, pixels);

    return new AtlasTexture(this, area);
}

VkDescriptorSet TextureAtlas::GetDescriptorSet() { return m_descriptor_set; }
TextureWindow TextureAtlas::GetTextureWindow() {
    return TextureWindow{0.0f, 0.0f, 1.0f, 1.0f};
}

int TextureAtlas::GetWidth() { return m_width; }
int TextureAtlas::GetHeight() { return m_height; }
