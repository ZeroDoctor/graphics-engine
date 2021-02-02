#pragma once

#include "build_order.hpp"
#include "texture_manager.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

class Glyph {
public:
    Glyph(FT_Face, FT_UInt, TextureAtlas*);
    ~Glyph();

    ITexture* GetTexture();
    int GetAdvance();

private:
    FT_Face m_face;
    FT_UInt m_glyph_index;
    int m_left;
    int m_top;
    int m_width;
    int m_height;
    int m_advance;
    ITexture* m_texture;

    void createTextureFromBitmap(TextureAtlas* ta);
};

class Font {
public:
    Font(FT_Face&, TextureAtlas*);
    ~Font();

    Glyph* GetGlyph(uint16_t);

    void Draw();

private:
    FT_Face m_face;
    std::map<uint16_t, Glyph*> m_glyphs;
    TextureAtlas* m_texture_atlas;
};

class FontManager {

public:
    FontManager(TextureAtlas*);
    ~FontManager();
    Font* GetFont(std::string& fontname, int pt);

private:
    FT_Library m_library;
    TextureAtlas* m_texture_atlas;
};