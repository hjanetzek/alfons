#pragma once

#include "fontManager.h"
#include "textShaper.h"
#include "textBatch.h"
#include "atlas.h"
#include "textRenderer.h"

namespace alfons {

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

}
