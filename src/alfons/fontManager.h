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

#include "font.h"
#include "inputSource.h"

namespace alfons {

class FontManager {

public:

    ~FontManager() {}

    std::shared_ptr<Font> addFont(std::string path, float baseSize);

    std::shared_ptr<Font> getFont(const std::string& name,
                                  const Font::Properties& props = Font::Properties(0, Font::Style::regular));

    std::shared_ptr<Font> getCachedFont(InputSource source,
                                        const Font::Properties& properties);

    void unload(Font& font);

    void unload();

    std::shared_ptr<FontFace> getFontFace(const FontFace::Descriptor& descriptor, float baseSize);


private:
    FreetypeHelper m_ftHelper;

    FaceID m_maxFontId = 0;

    bool hasDefaultFont = false;
    Font::Style defaultFontStyle = Font::Style::regular;
    std::string defaultFontName;

    // Font face name and style
    using FontFaceKey = std::pair<std::string, Font::Style>;

    // Path and base-size
    using FontFaceEntry = std::pair<std::string, float>;

    std::map<FontFaceKey, FontFaceEntry> m_globalMap;

    // Font name, Style and Properties
    using ShortcutKey = std::tuple<std::string, Font::Properties>;

    //
    std::map<ShortcutKey, std::shared_ptr<Font>> m_shortcuts;

    // // Alias -> font name
    // std::map<std::string, std::string> m_aliases;

    // Path and Properties
    using FontKey = std::pair<std::string, Font::Properties>;

    std::map<FontKey, std::shared_ptr<Font>> m_fonts;

    std::vector<std::shared_ptr<FontFace>> m_faces;

};
}
