#include "gl.h"

#include "renderer.h"
#include "logger.h"

#ifdef USE_SDF
#define SDF_IMPLEMENTATION
#include "sdf.h"
#endif

#define ATTRIBUTE_POS 0
#define ATTRIBUTE_TEX 1

#define MAX_QUADS 8192
#define MAX_INDICES MAX_QUADS / 4 * 6

namespace alfons {

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::drawGlyph(const Quad& _quad, const AtlasGlyph& _glyph) {
    batches[_glyph.atlas].add(_quad, *_glyph.glyph, m_state);

}

void Renderer::drawGlyph(const Rect& _rect, const AtlasGlyph& _glyph) {
    batches[_glyph.atlas].add(_rect, *_glyph.glyph, m_state);
}

void Renderer::addTexture(AtlasID atlas, uint16_t textureWidth, uint16_t textureHeight) {
    if (atlas >= batches.size()) {
        batches.resize(atlas+1);
    }
    batches[atlas].texData.resize(textureWidth * textureHeight);
    batches[atlas].width = textureWidth;
    batches[atlas].height = textureHeight;
}

void Renderer::addGlyph(AtlasID atlas, uint16_t gx, uint16_t gy, uint16_t gw, uint16_t gh,
                        const unsigned char* src, uint16_t pad) {

    // LOGD("addGlyph %d", atlas);

    auto &texData = batches[atlas].texData;
    auto &dirtyRect = batches[atlas].dirtyRect;
    auto width = batches[atlas].width;
    uint stride = width;

    unsigned char* dst = &texData[(gx + pad) + (gy + pad) * stride];

    uint pos = 0;
    for (uint y = 0; y < gh; y++) {
        for (uint x = 0; x < gw; x++) {
            dst[x + (y * stride)] = src[pos++];
        }
    }

    dst = &texData[gx + gy * width];
    gw += pad * 2;
    gh += pad * 2;

// if (pad > 0) {
//   for (uint y = 0; y < gh; y++) {
//     dst[y * stride] = 0;
//     dst[gw - 1 + y * stride] = 0;
//   }
//   for (uint x = 0; x < gw; x++) {
//     dst[x] = 0;
//     dst[x + (gh - 1) * stride] = 0;
//   }
// }

#ifdef USE_SDF
    size_t bytes = gw * gh * sizeof(float) * 3;
    if (tmp_buffer.size() < bytes) {
        LOGD("realloc: %d", (int)bytes);
        tmp_buffer.resize(bytes);
    }

    sdfBuildDistanceFieldNoAlloc(dst, width, 3,
                                 dst,
                                 gw, gh, width,
                                 &tmp_buffer[0]);
#endif

    dirtyRect[0] = std::min(dirtyRect[0], gx);
    dirtyRect[1] = std::min(dirtyRect[1], gy);
    dirtyRect[2] = std::max(dirtyRect[2], uint16_t(gx + gw));
    dirtyRect[3] = std::max(dirtyRect[3], uint16_t(gy + gh));
    batches[atlas].dirty = true;
}

bool Renderer::init() {
    glGenBuffers(2, vbo);
    getIndices(MAX_QUADS);
    return true;
}

void Renderer::dispose() {
    glDeleteBuffers(1, vbo);
    glDeleteTextures(textures.size(), textures.data());
}

static void glCheckError(const char* str) {
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("Error %08x %d after %s", err, err, str);
        return;
    }
}

void Renderer::getIndices(uint32_t capacity) {

    if (capacity > idxCapacity) {
        idxCapacity = capacity;

        std::vector<uint16_t> indices;
        indices.reserve(capacity * 6);

        int offset = 0;
        for (uint32_t i = 0; i < capacity; i++) {
            indices.push_back(offset + 0);
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
            indices.push_back(offset + 0);
            offset += 4;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);

        uint16_t* pointer = reinterpret_cast<uint16_t*>(indices.data());

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 6 * capacity,
                     pointer, GL_STATIC_DRAW);
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);
    }
}

static uint32_t loadTexture(QuadBatch& batch, uint32_t texId) {

    bool useMipmap = false;

    bool init = (texId == 0);

    if (init) {
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

#if defined(GL_GENERATE_MIPMAP)
        if (useMipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_GENERATE_MIPMAP)
        if (useMipmap) {
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        }
#endif
    } else {
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 batch.width, batch.height, 0, GL_ALPHA,
                 GL_UNSIGNED_BYTE, &batch.texData[0]);

#if defined(GL_GENERATE_MIPMAP)
    if (useMipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    }
#endif

    return texId;
}

void Renderer::drawVertices(QuadBatch& quads) {

    const short* pointer = reinterpret_cast<const short*>(quads.vertices.data());

    uint32_t numIndices = 6 * quads.size();

    for (uint32_t idx = 0; idx < numIndices; idx += MAX_INDICES) {
        // * (24 shorts per quad) / 6 indices == 4)
        uint32_t offset = idx * 4;

        glVertexAttribPointer(attributePos, 2,
                              GL_SHORT, GL_FALSE, 12,
                              pointer + offset);

        glVertexAttribPointer(attributeTex, 4, GL_SHORT,
                              GL_FALSE, 12, pointer + 2 + offset);

        uint32_t curIndices = numIndices - idx;
        if (curIndices > MAX_INDICES) {
            curIndices = MAX_INDICES;
        }

        glDrawElements(GL_TRIANGLES, curIndices,
                       GL_UNSIGNED_SHORT, 0);
    }
}


static void debugQuad(short size, short x, short y) {
    int s = size * 4;

    short texQuad[] = {
        (short)(s * x), (short)(s * y), 0, 0, 0, 0,
        (short)(s * x + s), (short)(s * y), size, 0, 0, 0,
        (short)(s * x + s), (short)(s * y + s), size, size, 0, 0,
        (short)(s * x), (short)(s * y + s), 0, size, 0, 0};
    glVertexAttribPointer(ATTRIBUTE_POS, 2, GL_SHORT, GL_FALSE, 12, texQuad);
    glVertexAttribPointer(ATTRIBUTE_TEX, 2, GL_SHORT, GL_FALSE, 12, texQuad + 2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void Renderer::draw() {

    uint32_t t;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[0]);

    AtlasID id = 0;

    for (auto& batch : batches) {
        if (textures.size() <= id) {
            t = loadTexture(batch, 0);
            textures.push_back(t);
            LOGD("new texture %d => %d", id, t);
        } else if (batch.dirty) {
            t = textures[id];
            batch.dirty = false;
            loadTexture(batch, t);
            LOGD("update texture %d => %d", id, t);
        } else {
            t = textures[id];
            glBindTexture(GL_TEXTURE_2D, t);
        }

        drawVertices(batch);

        // debugQuad(512, id, 0);
        id++;
    }
}
}
