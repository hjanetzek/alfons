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


    std::shared_ptr<Font> addFont(std::string _name, Font::Properties _properties, InputSource _source = {});

    std::shared_ptr<Font> getFont(std::string _name, Font::Properties _properties);

#if 0
    std::shared_ptr<Font> addFont(std::string name, std::string path, float baseSize,
                                  Font::Style style = Font::Style::regular);

    std::shared_ptr<Font> addFontFromMemory(std::string name, unsigned char* blob, size_t dataSize,
                                            float size, Font::Style style = Font::Style::regular);

    Sets the font fallback stack, ordered by priorities (vector.begin() more important than vector.end())
    void setFallbackStack(std::vector<std::shared_ptr<Font>> fontfallbacks);

    Get the font for a given name, returns nullptr if no font for the given name & properties is found
    std::shared_ptr<Font> getFont(const std::string& name,
                                  const Font::Properties& props = Font::Properties(0, Font::Style::regular));

    std::shared_ptr<Font> getCachedFont(InputSource source,
                                        const Font::Properties& properties);
#endif

    void unload(Font& font);

    void unload();

    std::shared_ptr<FontFace> addFontFace(const FontFace::Descriptor& descriptor, float baseSize);


private:
    FreetypeHelper m_ftHelper;

    FaceID m_maxFontId = 0;

#if 0
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
#endif

    // Path and Properties
    using FontKey = std::pair<std::string, Font::Properties>;

    std::map<FontKey, std::shared_ptr<Font>> m_fonts;

    std::vector<std::shared_ptr<FontFace>> m_faces;

};
}
