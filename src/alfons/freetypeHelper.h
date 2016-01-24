/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaptation to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

#include <ft2build.h>
#include <vector>

#include FT_GLYPH_H
#include FT_TRUETYPE_TABLES_H

namespace alfons {

//  WARNING:
//  If the GlyphData is not for "immediate consumption", invoke
//  getBufferCopy() otherwise, the data will become
//  corrupted with the next FT_Get_Glyph() operation.

struct GlyphData {
    int x0, y0, x1, y1;

    GlyphData()
        : x0(0), y0(0), x1(0), y1(0),
          ftGlyph(nullptr),
          ftSlot(nullptr) {}

    bool isValid() const { return bool(ftGlyph); }

    unsigned char* getBuffer() const {
        if (ftGlyph)
            return ftSlot->bitmap.buffer;

        return nullptr;
    }

    std::vector<unsigned char> getBufferCopy() {
        if (!ftGlyph)
            return {};

        int dataSize = (x1 - x0) * (y1 - y0);
        std::vector<unsigned char> data;
        data.resize(dataSize);

        memcpy(&data[0], ftSlot->bitmap.buffer, dataSize);
        FT_Done_Glyph(ftGlyph);
        ftGlyph = nullptr;

        return data;
    }

    bool loadGlyph(FT_Face ftFace, FT_UInt codepoint) {
        if (ftGlyph) {
            FT_Done_Glyph(ftGlyph);
            ftGlyph = nullptr;
        }

        if (codepoint == 0)
            return false;

        if (FT_Load_Glyph(ftFace, codepoint,
                          FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT) != 0)
            return false;

        ftSlot = ftFace->glyph;

        if (FT_Get_Glyph(ftSlot, &ftGlyph) != 0)
            return false;

        FT_Render_Glyph(ftSlot, FT_RENDER_MODE_NORMAL);

        if (ftSlot->bitmap.width <= 0 || ftSlot->bitmap.rows <= 0) {
            FT_Done_Glyph(ftGlyph);
            ftGlyph = nullptr;
            return false;
        }
        x0 = ftSlot->bitmap_left;
        x1 = x0 + ftSlot->bitmap.width;
        y0 = -ftSlot->bitmap_top;
        y1 = y0 + ftSlot->bitmap.rows;

        return true;
    }

    FT_Glyph ftGlyph;

    // Slot belongs to most recently used ftFace!
    FT_GlyphSlot ftSlot;
};

class FreetypeHelper {

    GlyphData glyphData;

    FT_Library library;

public:
    FreetypeHelper() {
        FT_Init_FreeType(&library);
    }

    ~FreetypeHelper() {
        FT_Done_FreeType(library);
    }

    FT_Library getLib() const { return library; }

    const GlyphData* loadGlyph(FT_Face ftFace, FT_UInt codepoint) {
        if (!glyphData.loadGlyph(ftFace, codepoint))
            return nullptr;

        return &glyphData;
    }

};

}
