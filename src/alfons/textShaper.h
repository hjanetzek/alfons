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

#include "lineLayout.h"
#include "font.h"
#include "langHelper.h"

#include <vector>
#include <memory>

#include <unicode/unistr.h>

struct hb_buffer_t;
namespace alfons {

struct TextLine;
struct TextRun;
class TextItemizer;

class TextShaper {
public:
    TextShaper();
    ~TextShaper();


    LineLayout shapeICU(std::shared_ptr<Font>& font, const UnicodeString& text,
                     hb_language_t langHint = HB_LANGUAGE_INVALID,
                     hb_direction_t direction = HB_DIRECTION_INVALID);

    LineLayout shape(std::shared_ptr<Font>& font, const std::string& text,
                     hb_language_t langHint = HB_LANGUAGE_INVALID,
                     hb_direction_t direction = HB_DIRECTION_INVALID);

    LineLayout shape(std::shared_ptr<Font>& font, const std::string& text,
                     const std::string& langHint,
                     hb_direction_t direction = HB_DIRECTION_INVALID) {
        return shape(font, text, hb_language_from_string(langHint.c_str(), -1),
                     direction);
    }

protected:

    bool shape(std::shared_ptr<Font>& font, const TextLine& line,
               const std::vector<TextRun>& range, LineLayout& layout);

    bool shape(std::shared_ptr<Font>& font, TextLine& line, LineLayout& layout);

    bool processRun(const FontFace& face, const TextRun& run, FontFace::Metrics& _lineMetrics);

    LangHelper m_langHelper;
    std::unique_ptr<TextItemizer> m_itemizer;
    std::unique_ptr<TextLine> m_textLine;

    hb_buffer_t* m_hbBuffer;

    std::vector<Shape> m_shapes;
    // Storage for additional Glyphs in a cluster.
    // https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Characters_grapheme_clusters_and_glyphs
    std::vector<std::vector<Shape>> m_clusters;

    std::vector<uint8_t> m_glyphAdded;
    std::vector<char> m_linebreaks;

};

}
