#include "font_manager.hpp"

Glyph::Glyph(FT_Face face, FT_UInt ix, TextureAtlas* ta) 
{
    m_face = face;
    m_glyph_index = ix;
    auto err = FT_Load_Glyph(m_face, m_glyph_index, FT_LOAD_DEFAULT);
    if(err) {
        printff("failed to load glyph");
    }

    FT_GlyphSlot glyph_slot = m_face->glyph;
    err = FT_Render_Glyph(glyph_slot, FT_RENDER_MODE_NORMAL);
    if(err) {
        printff("failed to render glyph");
    }

    m_left = glyph_slot->bitmap_left;
    m_top = glyph_slot->bitmap_top;
    m_width = static_cast<int>(glyph_slot->metrics.width / 64);
    m_height = static_cast<int>(glyph_slot->metrics.height / 64);
    m_advance = static_cast<int>(glyph_slot->advance.x / 64);

    createTextureFromBitmap(ta);
}

Glyph::~Glyph() 
{
    
}

ITexture* Glyph::GetTexture() { return m_texture; }
int Glyph::GetAdvance() { return m_advance; }

void Glyph::createTextureFromBitmap(TextureAtlas* ta) 
{
     FT_GlyphSlot glyph_slot = m_face->glyph;

    if(glyph_slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY || glyph_slot->bitmap.num_grays != 256) {
        throw std::runtime_error("unsupported pixel mode");
    }

    auto width = glyph_slot->bitmap.width;
    auto height = glyph_slot->bitmap.rows;
    auto bufferSize = width*height*4;

    if(bufferSize == 0) {
        return;
    }

    std::vector<uint8_t> buffer(bufferSize);

    uint8_t* src = glyph_slot->bitmap.buffer;
    uint8_t* startOfLine = src;
    int dst = 0;

    for(int y = 0; y < height; ++y) {
        src = startOfLine;
        for(int x = 0; x < width; ++x) {
            auto value = *src;
            src++;

            buffer[dst++] = 0xff;
            buffer[dst++] = 0xff;
            buffer[dst++] = 0xff;
            buffer[dst++] = value;
        }
        startOfLine += glyph_slot->bitmap.pitch;
    }
    m_texture = ta->Add(width, height, buffer.data());
}

Font::Font(FT_Face& face, TextureAtlas* texture_atlas) 
{
    m_face = face;
    m_texture_atlas = texture_atlas;
}

Font::~Font() 
{
    printfi("-- Destroying Glyphs\n");
    for(auto& g : m_glyphs) {
        if(g.second != nullptr) {
            delete g.second;
            g.second = nullptr;
        }
    }
}

Glyph* Font::GetGlyph(uint16_t c) 
{
    auto found_it = m_glyphs.find(c);
    if(found_it != m_glyphs.end()) {
        return found_it->second;
    }

    auto glyph_index = FT_Get_Char_Index(m_face, c);
    auto glyph = new Glyph(m_face, glyph_index, m_texture_atlas);

    m_glyphs[c] = glyph;
    return glyph; 
}

void Font::Draw() 
{
    
}

FontManager::FontManager(TextureAtlas* ta) 
{
    m_texture_atlas = ta;
    auto err = FT_Init_FreeType(&m_library);
    if(err) {
        printff("couldn't initialize FreeType library\n");
    }
}

Font* FontManager::GetFont(std::string& fontname, int pt) 
{
    FT_Face face;
    auto err = FT_New_Face(m_library, fontname.c_str(), 0, &face);
    if(!err) {
        err = FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(pt));
        if(err) {
            printff("couldn't set pixel sizes\n");
        }
        return new Font(face, m_texture_atlas);
    }
    printff("couldn't load font\n");
}
