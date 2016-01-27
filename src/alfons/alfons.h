#pragma once

#include <stddef.h>
#include <stdint.h>

namespace alfons {

struct AtlasGlyph;
using AtlasID = size_t;

struct Quad {
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;
};

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;
};

struct MeshCallback {
    virtual void drawGlyph(const Quad& quad, const AtlasGlyph& glyph) = 0;
    virtual void drawGlyph(const Rect& rect, const AtlasGlyph& glyph) = 0;
};

struct TextureCallback {
    virtual void addTexture(AtlasID id, uint16_t textureWidth, uint16_t textureHeight) = 0;
    virtual void addGlyph(AtlasID id, uint16_t gx, uint16_t gy, uint16_t gw, uint16_t gh,
                          const unsigned char* src, uint16_t padding) = 0;
};

#if 0
class Alfons : protected MeshCallback, protected TextureCallback {
public:
    Alfons();

protected:

    // virtual void addTexture(AtlasID id, uint32_t textureWidth, uint32_t textureHeight) = 0;
    // virtual void addGlyph(AtlasID id, uint gx, uint gy, uint gw, uint gh,
    //                       const unsigned char* src, uint padding) = 0;

    GlyphAtlas m_atlas;
    TextBatch m_batcher;
    TextShaper m_shaper;
    FontManager m_fontManager;
};
#endif

}
