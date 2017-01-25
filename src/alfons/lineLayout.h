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
#include <glm/vec4.hpp>

#include <vector>
#include <memory>

namespace alfons {
class Font;

enum class Alignment {
    middle,
    left,
    right,
    top,
    baseline,
    bottom,
    block
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
            // Linebreak values, determine which breaks
            // are possible *after* this glyph.
            unsigned char mustBreak : 1;
            unsigned char canBreak : 1;
            unsigned char noBreak : 1;

            unsigned char isSpace : 1;
        };
    };

    float advance;

    uint32_t codepoint;
    glm::vec2 position;

    Shape() : face(0), flags(0), advance(0), codepoint(0) {}

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
    hb_direction_t m_direction = HB_DIRECTION_INVALID;
    FontFace::Metrics m_metrics;

    float m_advance = 0;
    float m_middleLineFactor = 1;
    float m_scale = 1;

    bool m_missingGlyphs = false;

public:

    LineLayout() {}

    LineLayout(std::shared_ptr<Font> _font)
        : m_font(std::move(_font)) {}

    LineLayout(std::shared_ptr<Font> _font, std::vector<Shape> _shapes,
               FontFace::Metrics _metrics, hb_direction_t _direction)
        : m_font(std::move(_font)),
          m_shapes(std::move(_shapes)),
          m_direction(_direction),
          m_metrics(_metrics),
          m_advance(0),
          m_middleLineFactor(1.0),
          m_scale(1) {

        for (auto& shape : m_shapes) {
            m_advance += shape.advance;
        }
    }

    void addShapes(const std::vector<Shape>& _shapes) {
        for (auto& shape : _shapes) {
            m_advance += shape.advance;
        }
        m_shapes.insert(m_shapes.end(), _shapes.begin(), _shapes.end());
    }

    std::vector<Shape>& shapes() { return m_shapes; }
    const std::vector<Shape>& shapes() const { return m_shapes; }

    const Font& font() const { return *m_font; }

    FontFace::Metrics& metrics() { return m_metrics; }

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
        case Alignment::middle:
            return m_middleLineFactor *
                (m_metrics.ascent - m_metrics.descent) * m_scale;
        case Alignment::top:
            return +ascent();
        case Alignment::bottom:
            return -descent();
        default:
            return 0;
        }
    }

    float offsetX(Alignment align) const {
        switch (align) {
        case Alignment::middle:
            return -0.5f * advance();

        case Alignment::right:
            return -advance();

        default:
            return 0;
        }
    }

    // Default-value is 0, otherwise getOffsetY() for "ALIGN_MIDDLE"
    // will return middleLineFactor * (getAscent() - getDescent())
    void setMiddleLineFactor(float factor) { m_middleLineFactor = factor; }

    void setMissingGlyphs() { m_missingGlyphs = true; }
    bool missingGlyphs() const { return m_missingGlyphs; }
};

}
