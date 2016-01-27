#pragma once

#include "alfons/alfons.h"
#include "alfons/atlas.h"

#include <cstdint>
#include <vector>

#define USE_SDF

namespace alfons {

struct Vertex {
    int16_t x;
    int16_t y;

    int16_t u;
    int16_t v;

    uint32_t state;

    Vertex(short x, short y, short u, short v, uint32_t state)
        : x(x), y(y), u(u), v(v), state(state) {}
};

struct QuadBatch {

    std::vector<unsigned char> texData;
    uint16_t width, height;
    uint16_t dirtyRect[4];
    bool dirty;

    std::vector<Vertex> vertices;

    void clear() { vertices.clear(); }

    void pack() { vertices.shrink_to_fit(); }

    // return number of quads
    int size() const { return vertices.size() >> 2; }

    bool empty() const { return vertices.empty(); }

    void add(const Rect& _rect, const Glyph& _glyph, uint32_t _state) {
        short x1 = _rect.x1 * 4.0f;
        short y1 = _rect.y1 * 4.0f;
        short x2 = _rect.x2 * 4.0f;
        short y2 = _rect.y2 * 4.0f;

        vertices.emplace_back(x1, y1, _glyph.u1, _glyph.v1, _state);
        vertices.emplace_back(x2, y1, _glyph.u2, _glyph.v1, _state);
        vertices.emplace_back(x2, y2, _glyph.u2, _glyph.v2, _state);
        vertices.emplace_back(x1, y2, _glyph.u1, _glyph.v2, _state);
    }

    void add(const Quad& _quad, const Glyph& _glyph, uint32_t _state) {
        vertices.emplace_back(_quad.x1 * 4.0f, _quad.y1 * 4.0f, _glyph.u1, _glyph.v1, _state);
        vertices.emplace_back(_quad.x2 * 4.0f, _quad.y2 * 4.0f, _glyph.u1, _glyph.v2, _state);
        vertices.emplace_back(_quad.x3 * 4.0f, _quad.y3 * 4.0f, _glyph.u2, _glyph.v2, _state);
        vertices.emplace_back(_quad.x4 * 4.0f, _quad.y4 * 4.0f, _glyph.u2, _glyph.v1, _state);
    }
};

class Renderer : public MeshCallback, public TextureCallback {

public:
    void draw();
    bool init();
    void dispose();

    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void setAttributeIds(int pos, int tex) {
        attributePos = pos;
        attributeTex = tex;
    }

    void clearQuads() {
        for (auto &batch : batches) {
            batch.vertices.clear();
        }
    }

    void setTextState(uint32_t _state) { m_state = _state; }

    void drawGlyph(const Quad& box, const AtlasGlyph& glyph) override;
    void drawGlyph(const Rect& rect, const AtlasGlyph& glyph) override;

    void addTexture(AtlasID id, uint16_t textureWidth, uint16_t textureHeight) override;
    void addGlyph(AtlasID, uint16_t gx, uint16_t gy, uint16_t gw, uint16_t gh,
                  const unsigned char* src, uint16_t padding) override;

    std::vector<QuadBatch> batches;

private:

    int attributePos = 0;
    int attributeTex = 1;

    uint idxCapacity = 0;

    uint vbo[2];
    uint32_t m_state = 0;

    std::vector<uint> textures;

    void getIndices(uint capacity);
    void drawVertices(QuadBatch& quads);

#ifdef USE_SDF
    std::vector<unsigned char> tmp_buffer;
#endif

};
}
