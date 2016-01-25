/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "textBatch.h"
#include "font.h"
#include "alfons.h"
#include "atlas.h"
#include "utils.h"

#include "logger.h"

#include <glm/vec2.hpp>

namespace alfons {

TextBatch::TextBatch(GlyphAtlas& _atlas, MeshCallback& _mesh)
    : m_atlas(_atlas), m_mesh(_mesh) {}

void TextBatch::setClip(const Rect& _clipRect) {
    m_clip = _clipRect;
    m_hasClip = true;
}

void TextBatch::setClip(float x1, float y1, float x2, float y2) {
    m_clip.x1 = x1;
    m_clip.y1 = y1;
    m_clip.x2 = x2;
    m_clip.y2 = y2;

    m_hasClip = true;
}

bool TextBatch::clip(Rect& _rect) const {
    if (_rect.x1 > m_clip.x2 || _rect.x2 < m_clip.x1 ||
        _rect.y1 > m_clip.y2 || _rect.y2 < m_clip.y1) {
        return true;
    }
    return false;
}

bool TextBatch::clip(Quad& _quad) const {
    return ((_quad.x1 > m_clip.x2 && _quad.x2 > m_clip.x2 &&
             _quad.x3 > m_clip.x2 && _quad.x4 > m_clip.x2) ||
            (_quad.y1 > m_clip.y2 && _quad.y2 > m_clip.y2 &&
             _quad.y3 > m_clip.y2 && _quad.y4 > m_clip.y2) ||
            (_quad.x1 < m_clip.x1 && _quad.x2 < m_clip.x1 &&
             _quad.x3 < m_clip.x1 && _quad.x4 < m_clip.x1) ||
            (_quad.y1 < m_clip.y1 && _quad.y2 < m_clip.y1 &&
             _quad.y3 < m_clip.y1 && _quad.y4 < m_clip.y1));
    return false;

}

void TextBatch::setupRect(const Shape& _shape, const glm::vec2& _position,
                          float _sizeRatio, Rect& _rect, AtlasGlyph& _atlasGlyph) {

    glm::vec2 ul = _position + (_shape.position + _atlasGlyph.glyph->offset) * _sizeRatio;
    _rect.x1 = ul.x;
    _rect.y1 = ul.y;
    _rect.x2 = ul.x + _atlasGlyph.glyph->size.x * _sizeRatio;
    _rect.y2 = ul.y + _atlasGlyph.glyph->size.y * _sizeRatio;
}

void TextBatch::drawShape(const Font& _font, const Shape& _shape,
                          const glm::vec2& _position, float _sizeRatio) {
    AtlasGlyph atlasGlyph;
    if (!m_atlas.getGlyph(_font, {_shape.face, _shape.codepoint}, atlasGlyph)) {
        return;
    }

    Rect rect;
    setupRect(_shape, _position, _sizeRatio, rect, atlasGlyph);

    if (m_hasClip && clip(rect)) {
        return;
    }

    m_mesh.drawGlyph(rect, atlasGlyph);
}

void TextBatch::drawTransformedShape(const Font& _font, const Shape& _shape,
                                     const glm::vec2& _position, float _sizeRatio) {
    AtlasGlyph atlasGlyph;
    if (!m_atlas.getGlyph(_font, {_shape.face, _shape.codepoint}, atlasGlyph)) {
        return;
    }

    Rect rect;
    setupRect(_shape, _position, _sizeRatio, rect, atlasGlyph);

    Quad quad;
    m_matrix.transformRect(rect, quad);

    if (m_hasClip && clip(quad)) {
        return;
    }

    m_mesh.drawGlyph(quad, atlasGlyph);
}

glm::vec2 TextBatch::draw(const LineLayout& line, glm::vec2 offset) {
    return draw(line, 0, line.shapes().size(), offset);
}

glm::vec2 TextBatch::draw(const LineLayout& line, size_t start, size_t end,
                          glm::vec2 offset) {

    if (line.offsets.empty()) {
        float startX = offset.x;

        for (size_t j = start; j < end; j++) {
            auto& c = line.shapes()[j];
            if (!c.isSpace) {
                drawShape(line.font(), c, offset, line.scale());
            }

            offset.x += line.advance(c);
            if (c.mustBreak) {
                offset.x = startX;
                offset.y += line.height();
            }
        }
    } else {
        int i = 0;
        for (size_t j = start; j < end; j++) {
            auto& c = line.shapes()[j];
            if (!c.isSpace) {
                drawShape(line.font(), c, offset + line.offsets[i++], line.scale());
            }
        }
    }
    offset.y += line.height();
    return offset;
}

glm::vec2 TextBatch::draw(const LineLayout& line, glm::vec2 offset, float width) {

    float lineWidth = 0;
    int wordLength = 0;
    int wordStart = 0;
    float startX = offset.x;

    float adv = 0;

    for (auto& c : line.shapes()) {
        wordLength++;

        // is break - or must break?
        if (c.canBreak || c.mustBreak) {
            offset.x = draw(line, wordStart, wordStart + wordLength, offset).x;
            adv = std::max(adv, offset.x);

            wordStart += wordLength;
            wordLength = 0;
            lineWidth = offset.x - startX;

        } else {
            lineWidth += line.advance(c);
        }

        if (lineWidth > width) {
            // only go to next line if chars have been added on the current line
            if (offset.x > startX) {
                offset.y += line.height();
                offset.x = startX;
                lineWidth = 0;
            }
        }
    }
    if (wordLength > 0) {
        adv = std::max(adv, draw(line, wordStart, wordStart + wordLength, offset).x);
    }
    offset.y += line.height();
    offset.x = adv;
    return offset;
}

float TextBatch::draw(const LineLayout& line, const LineSampler& path,
                      float offsetX, float offsetY) {

    bool reverse = false; //(line.direction() == HB_DIRECTION_RTL);
    float direction = reverse ? -1 : 1;
    // float sampleSize = 0.1 * line.height();

    auto& font = line.font();
    float scale = line.scale();

    glm::vec2 position;
    float angle;

    for (auto& shape : DirectionalRange(line.shapes(), reverse)) {
        //float half = 0.5f * line.advance(shape) * direction;
        float half = 0.5f * shape.advance * scale * direction;
        offsetX += half;

        if (!shape.isSpace) {
            path.get(offsetX, position, angle);

            m_matrix.setTranslation(position);

            //m_matrix.rotateZ(path.offset2SampledAngle(offsetX, sampleSize));
            m_matrix.rotateZ(angle);

            drawTransformedShape(font, shape, -half, offsetY, scale);
        }
        offsetX += half;
    }
    return offsetX;
}

}
