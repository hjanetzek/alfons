/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "fontManager.h"
#include "logger.h"

#include <memory>
#include <assert.h>

namespace alfons {

std::shared_ptr<Font> FontManager::addFont(std::string path, float size) {
    hasDefaultFont = true;
    defaultFontName = "default";
    defaultFontStyle = Font::Style::regular;
    m_globalMap[make_pair(defaultFontName, defaultFontStyle)] = std::make_pair(path, 0);

    auto properties = Font::Properties(size, Font::Style::regular);

    return getFont(defaultFontName, properties);
}

std::shared_ptr<Font> FontManager::getFont(const std::string& name,
                                           const Font::Properties& prop) {

    auto key = make_tuple(name, prop);
    auto cached = m_shortcuts.find(key);

    if (cached != m_shortcuts.end())
        return cached->second;

    // auto alias = m_aliases.find(name);
    // std::string nameOrAlias = (alias == m_aliases.end()) ? name : alias->second;
    // auto it = globalMap.find({nameOrAlias, style});

    auto it = m_globalMap.find({name, prop.style});

    if (it != m_globalMap.end()) {
        float baseSize = prop.baseSize;

        // Trying to use the base-size attribute at the font-config level/
        if (baseSize == 0) { baseSize = it->second.second; }

        std::string& uri = it->second.first;
        auto font = getCachedFont(InputSource::get(uri),
                                  Font::Properties(baseSize, prop.style));

        // Allowing caching upon further access.
        m_shortcuts[key] = font;
        return font;
    }

    // Basic system for handling undefined font names and styles.
    if (hasDefaultFont) {
        if (name != defaultFontName) {
            auto font = getFont(defaultFontName, prop);
            m_shortcuts[key] = font;
            return font;
        }
        if (prop.style != defaultFontStyle) {

            auto font = getFont(defaultFontName, {prop.baseSize, defaultFontStyle});
            m_shortcuts[key] = font;
            return font;
        }
    }

    // Does not occur, unless no "Default font" is defined.
    return nullptr;
}

std::shared_ptr<Font> FontManager::getCachedFont(InputSource source,
                                                 const Font::Properties& prop) {

    auto key = make_pair(source.uri(), prop);
    auto it = m_fonts.find(key);

    if (it != m_fonts.end()) { return it->second; }
    log("getCachedFont: %s", source.uri());

    auto font = std::make_shared<Font>(prop);
    m_fonts[key] = font;

    auto descriptor = FontFace::Descriptor(source, 0, 1, false);

    font->addFace(getFontFace(descriptor, prop.baseSize));

    return font;
}

void FontManager::unload(Font& font) {

    // for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
    //     if (it->second == font) { m_shortcuts.erase(it); }
    // }
    // for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
    //     if (it->second == font) { m_fonts.erase(it); }
    // }

    std::set<FaceID> inUse;

    for (auto& font : m_fonts) {
        for (auto& entry : font.second->fontFaceMap()) {
            for (auto& face : entry.second) {
                inUse.insert(face->id());
            }
        }
    }

    for (auto& face : m_faces) {
        if (!inUse.count(face->id())) {
            face->unload();
        }
    }
}

void FontManager::unload() {
    // layoutCache.clear();

    for (auto& face : m_faces) {
        face->unload();
    }
}

std::shared_ptr<FontFace> FontManager::getFontFace(const FontFace::Descriptor& descriptor, float baseSize) {

    // FontFace::Key key(descriptor, baseSize);
    // for (const auto& face : faces) {
    //     if (face.)
    // }
    // auto it = faces.find(key);
    // if (it != faces.end()) {
    //     return it->second.faceId;
    // }
    auto face = std::make_shared<FontFace>(m_ftHelper, m_maxFontId++, descriptor, baseSize);

    m_faces.push_back(face);

    return face;
}




}
