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

#include "freetypeHelper.h"
#include "glyph.h"
#include "inputSource.h"

#include <glm/vec2.hpp>

#include "hb.h"

#include <vector>
#include <memory>
#include <tuple>

namespace alfons {

class Alfons;
struct GlyphData;
struct Shape;

using FaceID = uint16_t;

class FontFace {
public:
    struct Descriptor {
        InputSource source;
        int faceIndex;
        float scale;

        Descriptor(InputSource source, int faceIndex = 0, float scale = 1)
            : source(source),
              faceIndex(faceIndex),
              scale(scale) {}
    };

    struct Key {
        std::string uri;
        int faceIndex;
        float baseSize;

        Key(const Descriptor& descriptor, float baseSize)
            : uri(descriptor.source.uri()),
              faceIndex(descriptor.faceIndex),
              baseSize(baseSize * descriptor.scale) {}

        bool operator<(const Key& rhs) const {
            return tie(uri, faceIndex, baseSize) <
                   tie(rhs.uri, rhs.faceIndex, rhs.baseSize);
        }
    };

    struct Metrics {
        float height = 0;
        float ascent = 0;
        float descent = 0;
        float underlineOffset = 0;
        float strikethroughOffset = 0;
        float lineThickness = 0;

        Metrics& operator*(float s) {
            height *= s;
            ascent *= s;
            descent *= s;
            lineThickness *= s;
            underlineOffset *= s;
            strikethroughOffset *= s;
            return *this;
        }
    };


    FontFace(FreetypeHelper& _ft, FaceID faceId,
             const Descriptor& descriptor, float baseSize);

    ~FontFace();

    bool isSpace(hb_codepoint_t codepoint) const;
    hb_codepoint_t getCodepoint(FT_ULong charCode) const;
    std::string getFullName() const;

    bool load();
    void unload();

    const GlyphData* createGlyph(hb_codepoint_t codepoint) const;

    FaceID id() const { return m_id; }

    hb_font_t* hbFont() const  { return m_hbFont; }

    const Metrics& metrics() const { return m_metrics; }

    const std::vector<hb_script_t>& scripts() const {
        return m_scripts;
    }
    const std::vector<hb_language_t>& languages() const {
        return m_languages;
    }

protected:

    FreetypeHelper& m_ft;

    const FaceID m_id;

    Descriptor m_descriptor;
    float m_baseSize;

    Metrics m_metrics;
    bool m_loaded;
    bool m_invalid;

    FT_Face m_ftFace;
    hb_font_t* m_hbFont;

    std::vector<hb_codepoint_t> m_spaceSeparators;

    std::vector<hb_script_t> m_scripts;
    std::vector<hb_language_t> m_languages;
};

}
