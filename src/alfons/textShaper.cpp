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

namespace alfons {


struct TextRun {
    size_t start;
    size_t end;

    hb_script_t script;
    hb_language_t language;
    hb_direction_t direction;

    TextRun() {}

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

    // Input
    UnicodeString* text;
    hb_language_t langHint;
    hb_direction_t overallDirection;

    //
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

    TextItemizer(LangHelper& _langHelper);
    ~TextItemizer();

    void processLine(TextLine& _line);

protected:
    LangHelper& langHelper;
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

TextItemizer::TextItemizer(LangHelper& _langHelper)
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
            //log("script run: %d to %d, lang:%s", start, end, lang);

            _line.scriptLangItems.emplace_back(start, end, std::make_pair(script, _line.langHint));
            continue;

        }
        auto& language = langHelper.detectLanguage(script);

        //log("script run: %d to %d, lang:%s", start, end, language.c_str());

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
        LOGE("UBIDI run length == 0...");
        _line.directionItems.emplace_back(0, length, HB_DIRECTION_LTR);
        return;
    }

    if ((!bidi) || (length > bidiBufferSize)) {
        if (bidiBufferSize > 0) {
            ubidi_close(bidi);
            bidi = nullptr;
        }

        int size = length < 256 ? 256 : length;
        LOGE("UBIDI alloc: %d", size);

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
        //log("dir run: %d to %d - not mixed", 0, length);

    } else {
        auto count = ubidi_countRuns(bidi, &error);
        _line.directionItems.reserve(count);

        for (int i = 0; i < count; ++i) {
            int32_t start, length;
            direction = ubidi_getVisualRun(bidi, i, &start, &length);
            _line.directionItems.emplace_back(start, start + length,
                                              icuDirectionToHB(direction));

            // log("dir run: %d to %d, rtl:%d", start, start + length,
            //     icuDirectionToHB(direction) == HB_DIRECTION_RTL);
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

        // log("merge run: %d to %d/%d, rtl:%d", position, scriptIt->end, end,
        //     directionIt.data == HB_DIRECTION_RTL);

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
    m_hbBuffer(hb_buffer_create()) {}

TextShaper::~TextShaper() {
    hb_buffer_destroy(m_hbBuffer);
}

bool TextShaper::processRun(const FontFace& _face, const TextRun& _run){

    hb_shape(_face.hbFont(), m_hbBuffer, NULL, 0);

    auto glyphCount = hb_buffer_get_length(m_hbBuffer);
    const auto* glyphInfos = hb_buffer_get_glyph_infos(m_hbBuffer, NULL);
    const auto* glyphPositions = hb_buffer_get_glyph_positions(m_hbBuffer, NULL);

    bool missingGlyphs = false;

    //log("run: %d to %d, rtl:%d", _run.start, _run.end, _run.direction == HB_DIRECTION_RTL);

    for (size_t pos = 0; pos < glyphCount; pos++) {
        auto codepoint = glyphInfos[pos].codepoint;
        auto clusterId = glyphInfos[pos].cluster;

        auto id = clusterId;
        // Map cluster position to visual LTR order
        if (_run.direction == HB_DIRECTION_RTL) {
            id = _run.end - clusterId - 1;
        } else {
            id = clusterId - _run.start;
        }

        if (codepoint == 0) {
            //log("missing glyphs");
            missingGlyphs = true;
            continue;
        }

        if (m_glyphAdded[id] && m_shapes[id].face != _face.id()) {
            // cluster found, with another font (e.g. 'space')
            LOGE("skip glyph %d/%d pos:%d", id, clusterId, pos);

            continue;
        }

        auto offset = glm::vec2(glyphPositions[pos].x_offset,
                                -glyphPositions[pos].y_offset) * _face.scale();

        float advance = glyphPositions[pos].x_advance * _face.scale().x;

        auto bufferPos = clusterId - _run.start;
        // log("add glyph %d/%d pos:%d linebreak:%d adv:%f",
        //     id, clusterId, pos, m_linebreaks[bufferPos], advance);

        if (m_glyphAdded[id]) {
            m_glyphAdded[id] = 2;

            if (m_clusters.size() < m_shapes.size()) {
                m_clusters.resize(m_shapes.size());
            }
            m_clusters[id].emplace_back(_face.id(), codepoint, offset, advance, 0);

        } else {
            m_glyphAdded[id] = 1;

            uint8_t breakmode = m_linebreaks[bufferPos];
            if (breakmode == 0) breakmode = 1;

            uint8_t flags = 1 |           // cluster start
                ((breakmode == 0) << 1) | // must break
                ((breakmode == 1) << 2) | // can break
                ((breakmode == 2) << 3) | // no break
                (_face.isSpace(codepoint) << 4);

            m_shapes[id] = Shape(_face.id(), codepoint, offset, advance, flags);
        }
    }

    return !missingGlyphs;
}

LineLayout TextShaper::shape(std::shared_ptr<Font>& _font, const TextLine& _line,
                             const std::vector<TextRun>& _range) {

    if (_range.empty()) {
        log("Eeek empty line");
        return LineLayout();
    }

    hb_language_t defaultLang = hb_language_get_default();

    int numChars = _range.back().end;

    std::vector<Shape> shapes;
    shapes.reserve(numChars);

    FontFace::Metrics lineMetrics;

    for (const TextRun& run : _range) {
        size_t length = run.end - run.start;
        if (length >= m_shapes.capacity()) {
            // log("resize glyph maps %d", length);
            m_shapes.resize(length + 16);
            m_glyphAdded.resize(m_shapes.capacity());
            m_linebreaks.reserve(m_shapes.capacity());
        }

        // Setup harfbuzz buffer with current TextRun

        hb_buffer_clear_contents(m_hbBuffer);

        hb_buffer_add_utf16(m_hbBuffer, _line.text->getBuffer(),
                            _line.text->length(),
                            run.start, run.end - run.start);

        hb_buffer_set_script(m_hbBuffer, run.script);
        hb_buffer_set_direction(m_hbBuffer, run.direction);

        const char* lang = nullptr;
        if (run.language == HB_LANGUAGE_INVALID) {
            hb_buffer_set_language(m_hbBuffer, defaultLang);
        } else {
            hb_buffer_set_language(m_hbBuffer, run.language);
            lang = hb_language_to_string(run.language);
        }

        set_linebreaks_utf16(_line.text->getBuffer() + run.start,
                             run.end - run.start, lang,
                             m_linebreaks.data());

        m_glyphAdded.assign(length, 0);

        for (auto& face : _font->getFontSet(run.language)) {

            if (!face->load()) { continue; }

            if (processRun(*face, run)) {
                auto setMax = [](float& a, float b){ if (b > a) a = b; };
                auto &metrics = face->metrics();
                setMax(lineMetrics.height, metrics.height);
                setMax(lineMetrics.ascent, metrics.ascent);
                setMax(lineMetrics.descent, metrics.descent);
                setMax(lineMetrics.lineThickness, metrics.lineThickness);
                setMax(lineMetrics.underlineOffset, metrics.underlineOffset);
                break;
            }
            //log("missing glyph for lang: %s", hb_language_to_string(run.language));

            // TODO check why the contents must be set again!
            // - check if it is possible to only reset the hb_buffer position
            // - or determine the font used for each run in advance.
            hb_buffer_clear_contents(m_hbBuffer);
            hb_buffer_add_utf16(m_hbBuffer, _line.text->getBuffer(),
                                _line.text->length(),
                                run.start, run.end - run.start);

            hb_buffer_set_script(m_hbBuffer, run.script);
            hb_buffer_set_direction(m_hbBuffer, run.direction);
            if (run.language == HB_LANGUAGE_INVALID) {
                hb_buffer_set_language(m_hbBuffer, defaultLang);
            } else {
                hb_buffer_set_language(m_hbBuffer, run.language);
            }
        }

        for (size_t i = 0; i < length; i++) {
            if (m_glyphAdded[i]) {
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

    return LineLayout(_font, shapes, lineMetrics, _line.overallDirection);
}

LineLayout TextShaper::shape(std::shared_ptr<Font>& _font,
                             const std::string& _text,
                             hb_language_t _langHint,
                             hb_direction_t _direction) {

    auto input = UnicodeString::fromUTF8(_text);

    m_textLine->set(input, _langHint, _direction);

    return shape(_font, *m_textLine);
}

LineLayout TextShaper::shape(std::shared_ptr<Font>& _font, UnicodeString& _text) {

    m_textLine->set(_text);

    return shape(_font, *m_textLine);
}

LineLayout TextShaper::shape(std::shared_ptr<Font>& _font, TextLine& _line) {

    m_itemizer->processLine(_line);

    return shape(_font, _line, _line.runs);
}

}
