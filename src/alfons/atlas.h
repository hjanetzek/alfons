#pragma once

#include "alfons.h"
#include "glyph.h"
#include <vector>
#include <memory>

#include <unordered_map>

namespace alfons {

struct GlyphKey {
    uint32_t font;
    uint32_t codepoint;

    GlyphKey(uint32_t font, uint32_t codepoint)
        : font(font), codepoint(codepoint) {}

    bool operator==(const GlyphKey& other) const {
        return (font == other.font && codepoint == other.codepoint);
    }
};
}

namespace std {
template <>
struct hash<alfons::GlyphKey> {
    std::size_t operator()(const alfons::GlyphKey& k) const {
        return ((std::hash<uint32_t>()(k.font) ^
                 std::hash<uint32_t>()(k.codepoint) << 1)) >> 1;
    }
};
}

namespace alfons {

class Atlas {
    struct Node {
        int x, y, width;
    };

public:
    Atlas(int w, int h);

    bool addRect(int rw, int rh, int* rx, int* ry);

    void expand(int w, int h);

    void reset(int w, int h);

    void addSkylineLevel(uint32_t idx, int x, int y, int w, int h);

    int rectFits(uint32_t i, int w, int h);

    int width, height;
    std::vector<Node> nodes;

    std::unordered_map<GlyphKey, Glyph> glyphMap;
};

using AtlasID = size_t;

struct AtlasGlyph {
    AtlasID atlas;
    Glyph* glyph;
};

class LineLayout;
class Font;

class GlyphAtlas {

public:
    GlyphAtlas(TextureCallback& _textureCb, uint16_t _textureSize = 512, int _glyphPadding = 1)
        : m_textureSize(_textureSize),
          m_padding(_glyphPadding),
          m_textureCb(_textureCb) {}

    bool prepare(LineLayout& lineLayout);
    bool getGlyph(const Font& font, const GlyphKey& key, AtlasGlyph& entry);

    bool createGlyph(const Font& font, const GlyphKey& key, AtlasGlyph& entry);

    void clear(AtlasID atlasId);
private:
    std::vector<Atlas> m_atlas;

    int m_textureSize;
    int m_padding;

    TextureCallback& m_textureCb;

};

}
