/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "textShaper.h"

#include <hb.h>
#include "linebreak.h"
#include "scrptrun.h"
#include "unicode/unistr.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"

#include "logger.h"

#define FT_INV_SCALE (1.f/64.f)

namespace alfons {

struct TextRun {
    size_t start = 0;
    size_t end = 0;

    hb_script_t script = HB_SCRIPT_INVALID;
    hb_language_t language = HB_LANGUAGE_INVALID;
    hb_direction_t direction = HB_DIRECTION_INVALID;

    TextRun() {}

    size_t length() const { return end - start; }

    TextRun(size_t _start, size_t _end, hb_script_t _script,
            hb_language_t _language, hb_direction_t _direction)
        : start(_start),
          end(_end),
          script(_script),
          language(_language),
          direction(_direction) { }
};

struct TextLine {

    template <typename T>
    struct Item {
        size_t start;
        size_t end;
        T data;

        Item(size_t _start, size_t _end, T _data)
            : start(_start), end(_end), data(_data) {}
    };

    using ScriptAndLanguageItem = Item<std::pair<hb_script_t, hb_language_t>>;
    using DirectionItem = Item<hb_direction_t>;
    using LineItem = Item<int>;

    // Input
    UnicodeString* text;
    hb_language_t langHint;
    hb_direction_t overallDirection;

    // Intermediate
    std::vector<ScriptAndLanguageItem> scriptLangItems;
    std::vector<DirectionItem> directionItems;

    // Result
    std::vector<TextRun> runs;

    void set(UnicodeString& _input,
             hb_language_t _langHint = HB_LANGUAGE_INVALID,
             hb_direction_t _overallDirection = HB_DIRECTION_INVALID) {
        runs.clear();
        directionItems.clear();
        scriptLangItems.clear();

        text = &_input;
        langHint = _langHint;
        overallDirection = _overallDirection;
    }
};

class TextItemizer {
public:

    TextItemizer(const LangHelper& _langHelper);
    ~TextItemizer();

    void processLine(TextLine& _line);

protected:
    const LangHelper& langHelper;
    UBiDi* bidi;
    int bidiBufferSize = 0;

    void itemizeScriptAndLanguage(TextLine& _line) const;
    void itemizeDirection(TextLine& _line);
    void mergeItems(TextLine& _line) const;
};

template <typename T>
typename T::const_iterator findEnclosingRange(const T& _items, uint32_t _position) {
    for (auto it = _items.begin(); it != _items.end(); ++it) {
        if ((it->start <= _position) && (it->end > _position))
            return it;
    }
    return _items.end();
}

auto icuScriptToHB(UScriptCode _script) -> hb_script_t {
    if (_script == USCRIPT_INVALID_CODE) {
        return HB_SCRIPT_INVALID;
    }

    return hb_script_from_string(uscript_getShortName(_script), -1);
}

auto icuDirectionToHB(UBiDiDirection _direction) -> hb_direction_t {
    return (_direction == UBIDI_RTL) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
}

TextItemizer::TextItemizer(const LangHelper& _langHelper)
    : langHelper(_langHelper), bidi(nullptr) {
}

TextItemizer::~TextItemizer() {
    if (bidiBufferSize > 0)
        ubidi_close(bidi);
}

void TextItemizer::processLine(TextLine& _line) {
    if (_line.scriptLangItems.empty())
        itemizeScriptAndLanguage(_line);

    if (_line.directionItems.empty())
        itemizeDirection(_line);

    mergeItems(_line);

    if (!_line.runs.empty()) {
        if (_line.langHint == HB_LANGUAGE_INVALID)
            _line.langHint = _line.runs.front().language;

        if (_line.overallDirection == HB_DIRECTION_INVALID)
            _line.overallDirection = _line.runs.front().direction;
    }
}

void TextItemizer::itemizeScriptAndLanguage(TextLine& _line) const {
    ScriptRun scriptRun(_line.text->getBuffer(), _line.text->length());

    while (scriptRun.next()) {
        auto start = scriptRun.getScriptStart();
        auto end = scriptRun.getScriptEnd();
        auto code = scriptRun.getScriptCode();

        auto script = icuScriptToHB(code);
        const char* lang = hb_language_to_string(_line.langHint);

        if (lang != nullptr && langHelper.matchLanguage(script, std::string(lang))) {
            _line.scriptLangItems.emplace_back(start, end, std::make_pair(script, _line.langHint));
            continue;

        }
        auto& language = langHelper.detectLanguage(script);

        _line.scriptLangItems.emplace_back(start, end, std::make_pair(script,
                hb_language_from_string(language.c_str(), -1)));
    }
}

void TextItemizer::itemizeDirection(TextLine& _line) {
    /*
     * If overallDirection is undefined:
     * The paragraph-level will be determined from the text.
     *
     * http://www.icu-project.org/apiref/icu4c/ubidi_8h.html#abdfe9e113a19dd8521d3b7ac8220fe11
     */
    UBiDiLevel paraLevel = (_line.overallDirection == HB_DIRECTION_INVALID)
        ? UBIDI_DEFAULT_LTR
        : ((_line.overallDirection == HB_DIRECTION_RTL) ? 1 : 0);

    auto length = _line.text->length();
    UErrorCode error = U_ZERO_ERROR;
    if (length == 0) {
        _line.directionItems.emplace_back(0, length, HB_DIRECTION_LTR);
        return;
    }

    if ((!bidi) || (length > bidiBufferSize)) {
        if (bidiBufferSize > 0) {
            ubidi_close(bidi);
            bidi = nullptr;
        }

        int size = length < 256 ? 256 : length;

        // FIXME maxRuns too large?
        bidi = ubidi_openSized(size, 10, &error);
        if (!U_SUCCESS(error)) {
            LOGE("UBIDI error alloc: %d (%d - %s)", size, error, u_errorName(error));
            _line.directionItems.emplace_back(0, length, HB_DIRECTION_LTR);
            bidi = nullptr;
            return;
        }
        bidiBufferSize = size;
    }

    ubidi_setPara(bidi, _line.text->getBuffer(), length, paraLevel, 0, &error);
    if (!U_SUCCESS(error)) {
        LOGE("UBIDI error setPara %d (%d - %s)", length, error, u_errorName(error));
        _line.directionItems.emplace_back(0, length, HB_DIRECTION_LTR);
        return;
    }

    auto direction = ubidi_getDirection(bidi);

    if (direction != UBIDI_MIXED) {
        _line.directionItems.emplace_back(0, length, icuDirectionToHB(direction));

    } else {
        auto count = ubidi_countRuns(bidi, &error);
        _line.directionItems.reserve(count);

        for (int i = 0; i < count; ++i) {
            int32_t start, length;
            direction = ubidi_getVisualRun(bidi, i, &start, &length);
            _line.directionItems.emplace_back(start, start + length,
                                              icuDirectionToHB(direction));
        }
    }
}

void TextItemizer::mergeItems(TextLine& _line) const {

    // Go through text directions
    for (auto& directionIt : _line.directionItems) {
        auto position = directionIt.start;
        auto end = directionIt.end;
        auto direction = directionIt.data;

        auto rtlIt = _line.runs.end();

        auto scriptIt = findEnclosingRange(_line.scriptLangItems, position);

        while (position < end) {
            TextRun run;
            run.start = position;
            run.end = std::min(scriptIt->end, end);

            run.script = scriptIt->data.first;
            run.language = scriptIt->data.second;

            run.direction = direction;

            if (direction == HB_DIRECTION_LTR) {
                _line.runs.push_back(run);
            } else {
                rtlIt = _line.runs.insert(rtlIt, run);
            }

            position = run.end;

            if (scriptIt->end == position) {
                // next script within direction
                ++scriptIt;
            }
        }
    }
}


TextShaper::TextShaper() :
    m_itemizer(std::make_unique<TextItemizer>(m_langHelper)),
    m_textLine(std::make_unique<TextLine>()),
    m_hbBuffer(hb_buffer_create()) {

    static bool lineBreakInitialized = false;

    if (!lineBreakInitialized) {
        lineBreakInitialized = true;
        init_linebreak();
    }
}

TextShaper::~TextShaper() {
    hb_buffer_destroy(m_hbBuffer);
}

bool TextShaper::processRun(const FontFace& _face, const TextRun& _run,
                            FontFace::Metrics& _lineMetrics) {

    hb_shape(_face.hbFont(), m_hbBuffer, NULL, 0);

    auto glyphCount = hb_buffer_get_length(m_hbBuffer);
    const auto* glyphInfos = hb_buffer_get_glyph_infos(m_hbBuffer, NULL);
    const auto* glyphPositions = hb_buffer_get_glyph_positions(m_hbBuffer, NULL);

    bool missingGlyphs = false;
    bool addedGlyphs = false;

    for (size_t pos = 0; pos < glyphCount; pos++) {
        hb_codepoint_t codepoint = glyphInfos[pos].codepoint;
        uint32_t clusterId = glyphInfos[pos].cluster;

        auto id = clusterId;
        // Map cluster position to visual LTR order
        if (_run.direction == HB_DIRECTION_RTL) {
            id = _run.end - clusterId - 1;
        } else {
            id = clusterId - _run.start;
        }

        if (codepoint == 0) {
            if (m_linebreaks[clusterId] != LINEBREAK_MUSTBREAK) {
                missingGlyphs = true;
            }
            continue;
        }

        if (m_glyphAdded[id] && m_shapes[id].face != _face.id()) {
            // cluster found, with another font (e.g. 'space')
            continue;
        }

        auto offset = glm::vec2(glyphPositions[pos].x_offset * FT_INV_SCALE,
                                -glyphPositions[pos].y_offset * FT_INV_SCALE);

        float advance = glyphPositions[pos].x_advance * FT_INV_SCALE;

        if (m_glyphAdded[id]) {
            m_glyphAdded[id] = 2;

            if (m_clusters.size() < m_shapes.size()) {
                m_clusters.resize(m_shapes.size());
            }
            m_clusters[id].emplace_back(_face.id(), codepoint, offset, advance, 0);

        } else {
            addedGlyphs = true;
            m_glyphAdded[id] = 1;

            uint8_t breakmode = m_linebreaks[clusterId];

             uint8_t flags = 1 |           // cluster start
                //((breakmode == LINEBREAK_MUSTBREAK) ? 2 : 0) | // must break
                ((breakmode == 1) ? 4 : 0) | // can break
                ((breakmode == 2) ? 8 : 0) | // no break
                (_face.isSpace(codepoint) ? 16 : 0);

            m_shapes[id] = Shape(_face.id(), codepoint, offset, advance, flags);
        }
    }

    if (addedGlyphs) {
        auto setMax = [](float& a, float b){ if (b > a) a = b; };
        auto &metrics = _face.metrics();
        setMax(_lineMetrics.height, metrics.height);
        setMax(_lineMetrics.ascent, metrics.ascent);
        setMax(_lineMetrics.descent, metrics.descent);
        setMax(_lineMetrics.lineThickness, metrics.lineThickness);
        setMax(_lineMetrics.underlineOffset, metrics.underlineOffset);
    }

    return !missingGlyphs;
}

bool TextShaper::shape(std::shared_ptr<Font>& _font, const TextLine& _line,
                       const std::vector<TextRun>& _range, LineLayout& _layout) {

    if (_range.empty()) { return true; }

    hb_language_t defaultLang = hb_language_get_default();

    int numChars = _line.text->length();

    std::vector<Shape> shapes;
    shapes.reserve(numChars);

    for (const TextRun& run : _range) {
        size_t length = run.end - run.start;

        m_shapes.assign(length, {});
        m_glyphAdded.assign(length, 0);

        bool missingGlyphs = true;

        for (auto& face : _font->getFontSet(run.language)) {

            if (!face->load()) { continue; }

            // Setup harfbuzz buffer with current TextRun
            // TODO check why the contents must be set again!
            // - check if it is possible to only reset the hb_buffer position
            // - or determine the font used for each run in advance.
            hb_buffer_clear_contents(m_hbBuffer);
            hb_buffer_add_utf16(m_hbBuffer, (const uint16_t*)_line.text->getBuffer(),
                                _line.text->length(),
                                run.start, run.end - run.start);

            hb_buffer_set_script(m_hbBuffer, run.script);
            hb_buffer_set_direction(m_hbBuffer, run.direction);
            if (run.language == HB_LANGUAGE_INVALID) {
                hb_buffer_set_language(m_hbBuffer, defaultLang);
            } else {
                hb_buffer_set_language(m_hbBuffer, run.language);
            }

            if (processRun(*face, run, _layout.metrics())) {
                missingGlyphs = false;
                break;
            }
        }

        if (missingGlyphs) { _layout.setMissingGlyphs(); }

        for (size_t i = 0; i < length; i++) {
            if (m_glyphAdded[i] && m_shapes[i].codepoint != 0) {

                shapes.push_back(m_shapes[i]);

                if (m_glyphAdded[i] == 2) {
                    for (auto& shape : m_clusters[i]) {
                        shapes.push_back(shape);
                    }
                    m_clusters[i].clear();
                }
            }
        }
    }

    if (shapes.empty()) { return false; }

    _layout.addShapes(shapes);

    return true;
}

LineLayout TextShaper::shape(std::shared_ptr<Font>& _font, const std::string& _text,
                             hb_language_t _langHint, hb_direction_t _direction) {

    auto text = UnicodeString::fromUTF8(_text);
    return shapeICU(_font, text, _langHint, _direction);
}

LineLayout TextShaper::shapeICU(std::shared_ptr<Font>& _font, const UnicodeString& _text,
                             hb_language_t _langHint, hb_direction_t _direction) {
    LineLayout layout(_font);

    int numChars = _text.length();

    const char* language = nullptr;
    if (_langHint != HB_LANGUAGE_INVALID) {
        language = hb_language_to_string(_langHint);
    }

    m_linebreaks.resize(numChars);
    set_linebreaks_utf16((const uint16_t*)_text.getBuffer(),
                         numChars, language,
                         m_linebreaks.data());

    auto &line = *m_textLine;
    int start = 0;

    for (int pos = 0; pos < numChars; pos++) {
        if (m_linebreaks[pos] != 0) { continue; }

        auto cur = _text.tempSubStringBetween(start, pos+1);

        line.set(cur, _langHint, _direction);
        m_itemizer->processLine(line);

        shape(_font, line, line.runs, layout);
        start = pos + 1;

    }

    return layout;
}

}
