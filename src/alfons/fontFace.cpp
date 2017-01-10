/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "alfons.h"
#include "fontFace.h"
#include "lineLayout.h"
#include "logger.h"

#include <hb-ft.h>

// See http://www.microsoft.com/typography/otspec/name.htm for a list of some
// possible platform-encoding pairs.  We're interested in 0-3 aka 3-1 - UCS-2.
// Otherwise, fail. If a font has some unicode map, but lacks UCS-2 - it is a
// broken or irrelevant font. What exactly Freetype will select on face load
// (it promises most wide unicode, and if that will be slower that UCS-2 -
// left as an excercise to check.)

static FT_Error force_ucs2_charmap(FT_Face face) {
    for (int i = 0; i < face->num_charmaps; i++) {
        auto platform_id = face->charmaps[i]->platform_id;
        auto encoding_id = face->charmaps[i]->encoding_id;

        if (((platform_id == 0) && (encoding_id == 3)) ||
            ((platform_id == 3) && (encoding_id == 1))) {
            return FT_Set_Charmap(face, face->charmaps[i]);
        }
    }

    return -1;
}

static const FT_ULong SPACE_SEPARATORS[] = {
    0x0020, 0x00A0, 0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
    0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202F, 0x205F, 0x3000};

static const size_t SPACE_SEPARATORS_COUNT = sizeof(SPACE_SEPARATORS) / sizeof(FT_ULong);

namespace alfons {

FontFace::FontFace(FreetypeHelper& _ft, FaceID _faceId,
                   const Descriptor& _descriptor, float _baseSize)
    : m_ft(_ft), m_id(_faceId), m_descriptor(_descriptor),
      m_baseSize(_baseSize * _descriptor.scale),
      m_loaded(false),
      m_invalid(false),
      m_ftFace(nullptr),
      m_hbFont(nullptr) {
}

FontFace::~FontFace() {
    unload();
}

bool FontFace::isSpace(hb_codepoint_t codepoint) const {
    for (auto cp : m_spaceSeparators) {
        if (cp == codepoint) { return true; }
    }
    return false;
}

hb_codepoint_t
FontFace::getCodepoint(FT_ULong charCode) const {
    if (m_ftFace) {
        return (hb_codepoint_t)FT_Get_Char_Index(m_ftFace, charCode);
    } else {
        return 0;
    }
}

std::string
FontFace::getFullName() const {
    if (m_ftFace) {
        return std::string(m_ftFace->family_name) + " " + m_ftFace->style_name;
    } else {
        return "";
    }
}

bool FontFace::load() {
    if (m_loaded) {  return true; }
    if (m_invalid) { return false; }

    if (!m_descriptor.source.isValid()) {
        m_invalid = true;
        return false;
    }

    FT_Error error;

    if (m_descriptor.source.hasSourceCallback()) {
        if (!m_descriptor.source.resolveSource()) {
            LOGE("Invalid data loaded by source callback");
            m_invalid = true;
            return false;
        }
    }

    if (m_descriptor.source.isUri()) {
        error = FT_New_Face(m_ft.getLib(),
                            m_descriptor.source.uri().c_str(),
                            m_descriptor.faceIndex, &m_ftFace);
        if (error) {
            LOGE("Missing font: error: %d %s", error, m_descriptor.source.uri());
            m_invalid = true;
            return false;
        }

    } else {
        auto& buffer = m_descriptor.source.buffer();
        error = FT_New_Memory_Face(m_ft.getLib(), reinterpret_cast<const FT_Byte*>(buffer.data()),
                                   buffer.size(), m_descriptor.faceIndex, &m_ftFace);
        if (error) {
            LOGE("Coul not create font: error: %d", error);
            m_invalid = true;
            return false;
        }
    }

    if (force_ucs2_charmap(m_ftFace)) {
        LOGE("Font is broken or irrelevant...");
        // ...but DroisSansJapan still seems to work!
        // FT_Done_Face(m_ftFace);
        // m_ftFace = nullptr;
        // return false;
    }

    // Docs for Pixels, points and device resolutions
    // http://www.freetype.org/freetype2/docs/glyphs/glyphs-2.html
    // - 1 point equals 1/72th of an inch
    // - pixel_size = point_size * resolution_dpi / 72
    // - coordinates are expressed in 1/64th of a pixel
    //   (also known as 26.6 fixed-point numbers).

    // This basiclly sets the pixel size since dpi == 72
    int dpi = 72;
    FT_Set_Char_Size(m_ftFace,
                     m_baseSize * 64, // char_width in 26.6 fixed-point
                     m_baseSize * 64, // char_height in 26.6 fixed-point
                     dpi,       // horizontal_resolution
                     dpi);      // vertical_resolution

    // This must take place after ftFace is properly scaled and transformed
    m_hbFont = hb_ft_font_create(m_ftFace, nullptr);

    m_metrics.height = m_ftFace->size->metrics.height / 64.f;
    m_metrics.ascent = m_ftFace->size->metrics.ascender / 64.f;
    m_metrics.descent = -m_ftFace->size->metrics.descender / 64.f;

    m_metrics.lineThickness = m_ftFace->underline_thickness / 64.f;
    m_metrics.underlineOffset = -m_ftFace->underline_position / 64.f;

    // auto os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(ftFace, ft_sfnt_os2));
    // if (os2 && (os2->version != 0xFFFF) && (os2->yStrikeoutPosition != 0)) {
    //     metrics.strikethroughOffset = FT_MulFix(os2->yStrikeoutPosition,
    //                                             ftFace->size->metrics.y_scale) *
    //                                   scale.y;
    // } else {
    //     metrics.strikethroughOffset = 0.5f * (metrics.ascent - metrics.descent);
    // }

    // The following is necessary because the codepoints provided by
    // freetype for space-separators are somehow not unicode values, e.g.
    // depending on the font, the codepoint for a "regular space" can be
    // 2 or 3 (instead of 32)
    if (m_spaceSeparators.empty()) {
        for (size_t i = 0; i < SPACE_SEPARATORS_COUNT; i++) {
            auto codepoint = static_cast<hb_codepoint_t>(
                FT_Get_Char_Index(m_ftFace, SPACE_SEPARATORS[i]));

            if (codepoint) {
                if (std::find(m_spaceSeparators.begin(),
                              m_spaceSeparators.end(),
                              codepoint) == m_spaceSeparators.end()) {
                    m_spaceSeparators.push_back(codepoint);
                }
            }
        }
    }

    LOGI("LOADED Font: %s size: %d", getFullName(), m_baseSize);

    m_loaded = true;
    return true;
}

void FontFace::unload() {
    if (m_loaded) {
        m_loaded = false;

        hb_font_destroy(m_hbFont);
        m_hbFont = nullptr;

        FT_Done_Face(m_ftFace);
        m_ftFace = nullptr;
    }
}

const GlyphData* FontFace::createGlyph(hb_codepoint_t codepoint) const {

    if (!m_loaded)
        return nullptr;

    // FIXME mutex

    return m_ft.loadGlyph(m_ftFace, codepoint);
}


}
