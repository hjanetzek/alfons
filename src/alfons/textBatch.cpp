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

LineMetrics NO_METRICS;

TextBatch::TextBatch(GlyphAtlas& _atlas, MeshCallback& _mesh)
    : m_atlas(_atlas), m_mesh(_mesh) {
    m_clip.x1 = 0;
    m_clip.y1 = 0;
    m_clip.x2 = 0;
    m_clip.y2 = 0;
}

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
                          const glm::vec2& _position, float _scale,
                          LineMetrics& _metrics) {

    AtlasGlyph atlasGlyph;
    if (!m_atlas.getGlyph(_font, {_shape.face, _shape.codepoint}, atlasGlyph)) {
        return;
    }

    Rect rect;
    setupRect(_shape, _position, _scale, rect, atlasGlyph);

    if (m_hasClip && clip(rect)) {
        return;
    }

    m_mesh.drawGlyph(rect, atlasGlyph);

    if (&_metrics != &NO_METRICS) {
        _metrics.addExtents({rect.x1, rect.y1, rect.x2, rect.y2});
    }
}

void TextBatch::drawTransformedShape(const Font& _font, const Shape& _shape,
                                     const glm::vec2& _position, float _scale,
                                     LineMetrics& _metrics) {
    AtlasGlyph atlasGlyph;
    if (!m_atlas.getGlyph(_font, {_shape.face, _shape.codepoint}, atlasGlyph)) {
        return;
    }

    Rect rect;
    setupRect(_shape, _position, _scale, rect, atlasGlyph);

    Quad quad;
    m_matrix.transformRect(rect, quad);

    if (m_hasClip && clip(quad)) {
        return;
    }

    m_mesh.drawGlyph(quad, atlasGlyph);

    // FIXME: account for matrix transform
    // return glm::vec4(atlasGlyph.glyph->u1,
    //     atlasGlyph.glyph->u2,
    //     atlasGlyph.glyph->v1,
    //     atlasGlyph.glyph->v2);
}

glm::vec2 TextBatch::draw(const LineLayout& _line, glm::vec2 _position, LineMetrics& _metrics) {
    return draw(_line, 0, _line.shapes().size(), _position, _metrics);
}

glm::vec2 TextBatch::drawShapeRange(const LineLayout& _line, size_t _start, size_t _end,
                                    glm::vec2 _position, LineMetrics& _metrics) {

    for (size_t j = _start; j < _end; j++) {
        auto& c = _line.shapes()[j];
        if (!c.isSpace) {
            drawShape(_line.font(), c, _position, _line.scale(), _metrics);
        }

        _position.x += _line.advance(c);
    }
    return _position;
}

glm::vec2 TextBatch::draw(const LineLayout& _line, size_t _start, size_t _end,
                          glm::vec2 _position, LineMetrics& _metrics) {

    float startX = _position.x;

    for (size_t j = _start; j < _end; j++) {
        auto& c = _line.shapes()[j];
        if (!c.isSpace) {
            drawShape(_line.font(), c, _position, _line.scale(), _metrics);
        }

        _position.x += _line.advance(c);
        if (c.mustBreak) {
            _position.x = startX;
            _position.y += _line.height();
        }
    }

    _position.y += _line.height();

    return _position;
}

glm::vec2 TextBatch::draw(const LineLayout& _line, glm::vec2 _position, float _width, LineMetrics& _metrics) {

    if (_line.shapes().empty()) { return _position; }

    float lineWidth = 0;
    float startX = _position.x;

    float adv = 0;
    size_t shapeCount = 0;

    float lastWidth = 0;
    size_t lastShape = 0;
    size_t startShape = 0;

    for (auto& shape : _line.shapes()) {

        if (!shape.cluster) {
            shapeCount++;
            lineWidth += _line.advance(shape);
            continue;
        }

        shapeCount++;
        lineWidth += _line.advance(shape);

        // is break - or must break?
        if (shape.canBreak || shape.mustBreak) {
            lastShape = shapeCount;
            lastWidth = lineWidth;
        }

        if (lastShape != 0 && (lineWidth > _width || shape.mustBreak)) {
            auto& endShape = _line.shapes()[lastShape-1];
            if (endShape.isSpace) {
                lineWidth -= _line.advance(endShape);
                lastWidth -= _line.advance(endShape);
            }

            adv = std::max(adv, drawShapeRange(_line, startShape, lastShape,
                                               _position, _metrics).x);

            lineWidth -= lastWidth;

            startShape = lastShape;
            lastShape = 0;

            _position.y += _line.height();
            _position.x = startX;
        }
    }

    if (startShape < shapeCount) {
        adv = std::max(adv, drawShapeRange(_line, startShape, shapeCount,
                                           _position, _metrics).x);
        _position.y += _line.height();
    }

    _position.x = adv;
    return _position;
}

float TextBatch::draw(const LineLayout& _line, const LineSampler& _path,
                      float _offsetX, float _offsetY) {

    bool reverse = false; //(line.direction() == HB_DIRECTION_RTL);
    float direction = reverse ? -1 : 1;
    // float sampleSize = 0.1 * line.height();

    auto& font = _line.font();
    float scale = _line.scale();

    glm::vec2 position;
    float angle;

    for (auto& shape : DirectionalRange(_line.shapes(), reverse)) {
        //float half = 0.5f * line.advance(shape) * direction;
        float half = 0.5f * shape.advance * scale * direction;
        _offsetX += half;

        if (!shape.isSpace) {
            _path.get(_offsetX, position, angle);

            m_matrix.setTranslation(position);

            //m_matrix.rotateZ(path.offset2SampledAngle(offsetX, sampleSize));
            m_matrix.rotateZ(angle);

            drawTransformedShape(font, shape, -half, _offsetY, scale);
        }
        _offsetX += half;
    }
    return _offsetX;
}

}
