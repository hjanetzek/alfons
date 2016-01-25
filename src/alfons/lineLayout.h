/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

#include "glyph.h"
#include "font.h"

#include <glm/vec2.hpp>

#include <vector>
#include <memory>

namespace alfons {
class Font;

enum Alignment {
    ALIGN_MIDDLE,
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_TOP,
    ALIGN_BASELINE,
    ALIGN_BOTTOM,
    ALIGN_BLOCK
};

struct Shape {
    // Reference to face within a Font.
    uint8_t face;
    union {
        uint8_t flags;
        struct {
            // Marks the start of a cluster:
            // When a codepoint uses multiple glyphs
            // only the first is set.
            unsigned char cluster : 1;
            // Linebreak values
            unsigned char mustBreak : 1;
            unsigned char canBreak : 1;
            unsigned char noBreak : 1;

            unsigned char isSpace : 1;
        };
    };

    float advance;

    uint32_t codepoint;
    glm::vec2 position;

    Shape() {}

    Shape(uint8_t faceID, uint32_t codepoint, glm::vec2 offset,
            float advance, uint8_t flags)
        : face(faceID),
          flags(flags),
          advance(advance),
          codepoint(codepoint),
          position(offset) {}
};

class LineLayout {
    std::shared_ptr<Font> m_font;
    std::vector<Shape> m_shapes;
    hb_direction_t m_direction;
    FontFace::Metrics m_metrics;

    float m_advance;
    float m_middleLineFactor;
    float m_scale;

public:
    // FIXME: For wrapped text
    std::vector<glm::vec2> offsets;

    LineLayout() {}

    LineLayout(std::shared_ptr<Font> _font,  std::vector<Shape> _shapes,
               FontFace::Metrics _metrics, hb_direction_t _direction)
        : m_font(std::move(_font)),
          m_shapes(std::move(_shapes)),
          m_direction(_direction),
          m_metrics(_metrics),
          m_advance(0),
          m_middleLineFactor(0),
          m_scale(1) {

            for (auto& shape : m_shapes) {
                if (shape.flags != 0) {
                    m_advance += shape.advance;
                }
            }
        }

    std::vector<Shape>& shapes() { return m_shapes; }
    const std::vector<Shape>& shapes() const { return m_shapes; }

    const Font& font() const { return *m_font; }

    glm::vec2 getOffset(Alignment alignX, Alignment alignY) const {
        return glm::vec2(offsetX(alignX), offsetY(alignY));
    }

    float scale() const {
        return m_scale;
    }

    hb_direction_t direction() const {
        return m_direction;
    }

    float height() const {
        return m_metrics.height * m_scale;
    }

    float ascent() const {
        return m_metrics.ascent * m_scale;
    }

    float descent() const {
        return m_metrics.descent * m_scale;
    }

    float lineThickness() const {
        return m_metrics.lineThickness * m_scale;
    }

    float advance() const {
        return m_advance * m_scale;
    }

    float advance(const Shape& shape) const {
        return shape.advance * m_scale;
    }

    void setScale(float scale) { m_scale = scale; }

    float scale() { return m_scale; }

    float offsetY(Alignment align) const {
        switch (align) {
        case ALIGN_MIDDLE:
            return m_middleLineFactor *
                (m_metrics.ascent - m_metrics.descent) * m_scale;
        case ALIGN_TOP:
            return +ascent();
        case ALIGN_BOTTOM:
            return -descent();
        default:
            return 0;
        }
    }

    float offsetX(Alignment align) const {
        switch (align) {
        case ALIGN_MIDDLE:
            return -0.5f * advance();

        case ALIGN_RIGHT:
            return -advance();

        default:
            return 0;
        }
    }

    // Default-value is 0, otherwise getOffsetY() for "ALIGN_MIDDLE"
    // will return middleLineFactor * (getAscent() - getDescent())
    void setMiddleLineFactor(float factor) { m_middleLineFactor = factor; }
};

}
