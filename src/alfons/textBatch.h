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

// Declared as a struct for possible other informations about the line
struct LineMetrics {
    glm::vec4 aabb = {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::min()
    };
    float height() {
        if (aabb[1] != std::numeric_limits<float>::max()) {
            return aabb[3] - aabb[1];
        }
        // Not initialized
        return 0;
    }

    void addExtents(glm::vec4 other) {
        aabb.x = std::min(aabb.x, other.x);
        aabb.y = std::min(aabb.y, other.y);
        aabb.z = std::max(aabb.z, other.z);
        aabb.w = std::max(aabb.w, other.w);
    }
};

extern LineMetrics NO_METRICS;

class TextBatch {
public:
    TextBatch(GlyphAtlas& _atlas, MeshCallback& _mesh);

    void setClip(const Rect& clipRect);
    void setClip(float x1, float y1, float x2, float y2);

    void clearClip() { m_hasClip = false; }

    /* Use current QuadMatrix for transform */
    void drawTransformedShape(const Font& font, const Shape& shape, const glm::vec2& position,
                              float scale, LineMetrics& metrics = NO_METRICS);

    /* Use current QuadMatrix for transform */
    inline void drawTransformedShape(const Font& font, const Shape& shape, float x, float y,
                                     float scale, LineMetrics& metrics = NO_METRICS) {
        drawTransformedShape(font, shape, glm::vec2(x, y), scale, metrics);
    }

    void drawShape(const Font& font, const Shape& shape, const glm::vec2& position,
                   float scale, LineMetrics& metrics = NO_METRICS);

    glm::vec2 drawShapeRange(const LineLayout& _line, size_t _start, size_t _end,
                             glm::vec2 _position, LineMetrics& _metrics = NO_METRICS);

    glm::vec2 draw(const LineLayout& line, glm::vec2 position, LineMetrics& metrics = NO_METRICS);

    glm::vec2 draw(const LineLayout& line, float x, float y, LineMetrics& metrics = NO_METRICS) {
        return draw(line, {x, y}, metrics);
    }

    glm::vec2 draw(const LineLayout& line, glm::vec2 position, float width,
                   LineMetrics& metrics = NO_METRICS);

    glm::vec2 draw(const LineLayout& line, size_t start, size_t end, glm::vec2 position,
                   LineMetrics& metrics = NO_METRICS);

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
