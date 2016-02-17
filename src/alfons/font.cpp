/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "font.h"
#include "logger.h"

namespace alfons {

Font::Font(const Properties& properties)
    : m_properties(properties) {}

auto Font::addFace(std::shared_ptr<FontFace> _face, hb_language_t _lang) -> bool {

    if (_lang == HB_LANGUAGE_INVALID) {
        m_faces.push_back(_face);
        return true;
    }

    for (auto& face : m_fontFaceMap[_lang]) {
        if (face == _face) {
            log("Won't add font twice. %s", _lang);
            return false;
        }
    }

    m_fontFaceMap[_lang].push_back(_face);
    return true;
}

auto Font::getFontSet(hb_language_t lang) const -> const Faces& {
    if (lang == HB_LANGUAGE_INVALID) {
        return m_faces;
    }

    auto it = m_fontFaceMap.find(lang);
    if (it == m_fontFaceMap.end()) {
        return m_faces;
    }

    return it->second;
}

void Font::addFaces(const Font& _other) {
    m_faces.insert(m_faces.end(), _other.m_faces.begin(), _other.m_faces.end());
}

auto Font::styleStringToEnum(const std::string& style) -> Font::Style {
    if (style == "bold")
        return Font::Style::bold;
    if (style == "italic")
        return Font::Style::italic;
    if (style == "bold-italic")
        return Font::Style::bold_italic;

    return Font::Style::regular;
}

auto Font::styleEnumToString(Font::Style style) -> std::string {
    switch (style) {
    case Font::Style::bold:
        return "bold";
    case Font::Style::italic:
        return "italic";
    case Font::Style::bold_italic:
        return "bold-italic";
    case Font::Style::regular:
    default:
        return "regular";
    }
}
}
