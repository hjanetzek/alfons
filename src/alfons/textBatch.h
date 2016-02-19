/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

#include "lineLayout.h"
#include "quadMatrix.h"

#include "path/lineSampler.h"

#include <set>
#include <map>
#include <memory>
#include <limits>

namespace alfons {

struct MeshCallback;
struct AtlasGlyph;
class GlyphAtlas;

struct LineDesc {
    glm::vec4 aabb = {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min()
    };
    glm::vec2 offset = glm::vec2(0.f);
};

class TextBatch {
public:
    TextBatch(GlyphAtlas& _atlas, MeshCallback& _mesh);

    void setClip(const Rect& clipRect);
    void setClip(float x1, float y1, float x2, float y2);

    void clearClip() { m_hasClip = false; }

    /* Use current QuadMatrix for transform */
    glm::vec4 drawTransformedShape(const Font& font, const Shape& shape, const glm::vec2& position,
                              float sizeRatio);

    /* Use current QuadMatrix for transform */
    inline glm::vec4 drawTransformedShape(const Font& font, const Shape& shape, float x, float y,
                                      float sizeRatio) {
        return drawTransformedShape(font, shape, glm::vec2(x, y), sizeRatio);
    }

    glm::vec4 drawShape(const Font& font, const Shape& shape, const glm::vec2& position,
                   float sizeRatio);

    LineDesc draw(const LineLayout& line, LineDesc lineDesc);

    LineDesc draw(const LineLayout& line, float x, float y) {
        LineDesc desc;
        desc.offset = {x, y};
        return draw(line, desc);
    }

    LineDesc draw(const LineLayout& line, LineDesc lineDesc, float width);

    LineDesc draw(const LineLayout& line, size_t start, size_t end, LineDesc lineDesc);

    float draw(const LineLayout& line, const LineSampler& path,
               float offsetX = 0, float offsetY = 0);

    QuadMatrix& matrix() { return m_matrix; }

protected:
    GlyphAtlas& m_atlas;
    MeshCallback& m_mesh;

    bool m_hasClip = false;
    Rect m_clip;

    QuadMatrix m_matrix;

    inline bool clip(Rect& rect) const;
    inline bool clip(Quad& quad) const;

    inline void setupRect(const Shape& shape, const glm::vec2& position,
                          float sizeRatio, Rect& rect, AtlasGlyph& glyph);
};
}
