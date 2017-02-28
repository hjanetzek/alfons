// Ported from fontstash
//
// Atlas based on Skyline Bin Packer by Jukka Jylänki

#include "atlas.h"
#include "font.h"

#include "logger.h"

#include <algorithm>

namespace alfons {

Atlas::Atlas(int w, int h) {
    reset(w, h);
}

void Atlas::expand(int w, int h) {
    // Insert node for empty space
    if (w > width) {
        nodes.insert(nodes.end(), {width, 0, w - width});
    }
    width = w;
    height = h;
}

void Atlas::reset(int w, int h) {
    width = w;
    height = h;

    // Init root node.
    nodes.clear();
    nodes.push_back({0, 0, w});

    glyphMap.clear();
}

void Atlas::addSkylineLevel(uint32_t idx, int x, int y, int w, int h) {

    // Insert new node
    nodes.insert(nodes.begin() + idx, {x, y + h, w});

    // Delete skyline segments that fall under the shadow of the new segment.
    for (size_t i = idx + 1; i < nodes.size(); i++) {

        int shrink = (nodes[i - 1].x + nodes[i - 1].width) - nodes[i].x;
        if (shrink > 0) {

            int nw = nodes[i].width - shrink;
            if (nw > 0) {
                nodes[i].x += shrink;
                nodes[i].width = nw;
                break;
            } else {
                nodes.erase(nodes.begin() + i);
                i--;
            }
        } else { break; }
    }

    // Merge same height skyline segments that are next to each other.
    for (size_t i = 0; i < nodes.size() - 1; i++) {
        if (nodes[i].y == nodes[i + 1].y) {
            nodes[i].width += nodes[i + 1].width;
            nodes.erase(nodes.begin() + i + 1);
            i--;
        }
    }
}

/// Checks if there is enough space at the location of skyline span 'i', and
/// return the max height of all skyline spans under that at that location,
/// (think tetris block being dropped at that position). Or -1 if no space
/// found.
int Atlas::rectFits(uint32_t i, int w, int h) {

    if (nodes[i].x + w > width) { return -1; }

    int spaceLeft = w;
    int y = nodes[i].y;
    while (spaceLeft > 0) {
        if (i == nodes.size()) { return -1; }

        y = std::max(y, nodes[i].y);
        if (y + h > height) { return -1; }

        spaceLeft -= nodes[i].width;
        ++i;
    }
    return y;
}

bool Atlas::addRect(int w, int h, int* rx, int* ry) {
    int besth = height, bestw = width;
    int bestx = -1, besty = -1, besti = -1;

    // Bottom left fit heuristic.
    for (size_t i = 0; i < nodes.size(); i++) {
        int y = rectFits(i, w, h);
        if (y != -1) {
            if ((y + h < besth) ||
                ((y + h == besth) && (nodes[i].width < bestw))) {
                besti = i;
                bestw = nodes[i].width;
                besth = y + h;
                bestx = nodes[i].x;
                besty = y;
            }
        }
    }

    if (besti == -1) { return false; }

    // Perform the actual packing.
    addSkylineLevel(besti, bestx, besty, w, h);

    *rx = bestx;
    *ry = besty;

    return true;
}

bool GlyphAtlas::getGlyph(const Font& _font, const GlyphKey& _key, AtlasGlyph& _entry) {
    AtlasID id = 0;

    for (auto& a : m_atlas) {
        auto it = a.glyphMap.find(_key);
        if (it != a.glyphMap.end()) {
            //a->usage++;
            _entry.atlas = id;
            _entry.glyph = &it->second;
            return true;
        }
        id++;
    }
    return createGlyph(_font, _key, _entry);
}

bool GlyphAtlas::createGlyph(const Font& _font, const GlyphKey& _key, AtlasGlyph& _entry) {

    if (_key.codepoint == 0) { return false; }

    auto& fontFace = _font.face(_key.font);

    const auto* gd = fontFace.createGlyph(_key.codepoint);
    if (!gd) { return false; }

    unsigned int pad = m_padding;
    int w = gd->x1 - gd->x0;
    int h = gd->y1 - gd->y0;
    int texW = w + pad * 2;
    int texH = h + pad * 2;

    if (texW > m_textureSize || texH > m_textureSize) {
        return false;
    }

    int x, y;
    Atlas* atlas = nullptr;

    AtlasID id = 0;
    for (auto& a : m_atlas) {
        if (a.addRect(texW, texH, &x, &y)) {
            atlas = &a;
            break;
        }
        id++;
    }
    if (!atlas) {
        m_atlas.emplace_back(m_textureSize, m_textureSize);
        atlas = &m_atlas.back();
        m_textureCb.addTexture(id, m_textureSize, m_textureSize);

        if (!atlas->addRect(texW, texH, &x, &y)) {
            // TODO  glyph does not fit into atlas size - check beforehand
            return false;
        }
    }

    m_textureCb.addGlyph(id, x, y, w, h, gd->getBuffer(), pad);

    auto glyphItem = atlas->glyphMap.emplace(_key, Glyph(x, y, w + pad * 2, h + pad * 2,
                                                        glm::vec2(gd->x0, gd->y0) - float(pad),
                                                        glm::vec2(w, h) + float(pad * 2)));
    _entry.atlas = id;
    _entry.glyph = &(glyphItem.first->second);

    return true;
}

void GlyphAtlas::clear(AtlasID _atlasId) {
    if (_atlasId >= m_atlas.size()) { return; }

    m_atlas[_atlasId].reset(m_textureSize, m_textureSize);
}


}
