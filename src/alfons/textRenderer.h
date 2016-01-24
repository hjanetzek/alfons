#pragma once

#include "glyph.h"
#include "quad.h"
#include "atlas.h"

namespace alfons {

using AtlasID = size_t;

struct TextRenderer {
    virtual void drawGlyph(const Quad& quad, const AtlasGlyph& glyph) = 0;
    virtual void drawGlyph(const Rect& rect, const AtlasGlyph& glyph) = 0;
};

struct RenderDummy : public TextRenderer, public TextureCallback {
    virtual void drawGlyph(const Quad& quad, const AtlasGlyph& glyph) {}
    virtual void drawGlyph(const Rect& rect, const AtlasGlyph& glyph) {}
    virtual void addTexture(AtlasID id, uint32_t textureWidth, uint32_t textureHeight) {}
    virtual void addGlyph(AtlasID id, uint gx, uint gy, uint gw, uint gh, const unsigned char* src, uint padding) {}
};

}
