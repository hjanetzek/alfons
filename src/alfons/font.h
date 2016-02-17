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

#include "fontFace.h"

#include <set>
#include <map>
#include <memory>
#include <vector>

namespace alfons {

class Font {

public:
    using Faces = std::vector<std::shared_ptr<FontFace>>;

    enum class Style {
        regular,
        bold,
        italic,
        bold_italic
    };

    struct Properties {
        float baseSize;
        Style style;

        Properties(float _baseSize, Style _style = Style::regular)
            : baseSize(_baseSize), style(_style) {}

        bool operator<(const Properties& rhs) const {
            return std::tie(baseSize, style) < std::tie(rhs.baseSize, rhs.style);
        }
    };

    Font(const Properties& properties);

    float size() const { return m_properties.baseSize; }

    bool addFace(std::shared_ptr<FontFace> face, hb_language_t lang = HB_LANGUAGE_INVALID);

    static Style styleStringToEnum(const std::string& style);
    static std::string styleEnumToString(Style style);

    const Faces& getFontSet(hb_language_t lang) const;

    const FontFace& face(FaceID _faceId) const {
        for (size_t id = 0; id < m_faces.size(); id++) {
            if (m_faces[id]->id() == _faceId) { return *m_faces[id]; }
        }
        assert(false);
        return *m_faces[0];
    };

    auto& faces() const { return m_faces; };

    bool hasFaces() const { return !m_faces.empty(); };

    int maxFaceId() const { return m_faces.size()-1; };

    void addFaces(const Font& _other);

    const std::map<hb_language_t, Faces>& fontFaceMap() { return m_fontFaceMap; }

protected:
    Properties m_properties;

    Faces m_faces;
    std::map<hb_language_t, Faces> m_fontFaceMap;
};
}
