#import <Foundation/Foundation.h>
#import "appleFontConverter/FontConverter.h"
#include <hb-coretext.h>

#include "appleFontFace.h"
#include "logger.h"

#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#endif


namespace alfons {

AppleFontFace::AppleFontFace(FreetypeHelper& _ft, FaceID _faceId, const Descriptor& _descriptor,
                             float _baseSize)
    : FontFace(_ft, _faceId, _descriptor, _baseSize) {}

bool AppleFontFace::load() {

    if (m_loaded) { return true; }
    if (m_invalid) { return false; }

    if (!m_descriptor.source.isValid() || !m_descriptor.source.isSystemFont()) {
        m_invalid = true;
        return false;
    }

    auto &fontName = m_descriptor.source.uri();

    CFStringRef name = CFStringCreateWithCString(nullptr, fontName.c_str(), kCFStringEncodingUTF8);
    CGFontRef cgFont = CGFontCreateWithFontName(name);
    CFRelease(name);

#if TARGET_OS_OSX
    if (!cgFont) {
        // On macOS 10.12+ some default system font names start with `.AppleSystemUIFont`, for such fonts we need
        // NSFont to get CGFontRef
        NSString* nsFontName = [NSString stringWithUTF8String:fontName.c_str()];
        NSFont *font = [NSFont fontWithName:nsFontName size:1.0];
        if (font) {
            cgFont = CTFontCopyGraphicsFont(CTFontRef(font), nullptr);
        }
    }
#endif

    if (!cgFont) {
        LOGE("Cannot create font with name '%s'", fontName.c_str());
        m_invalid = true;
        return false;
    }

    if (!m_descriptor.source.hasData()) {
        std::vector<char> fontData = [FontConverter fontDataForCGFont:cgFont];
        if (fontData.empty()) {
            LOGE("Font data read from apple system font '%s' is empty.", fontName.c_str());
            m_invalid = true;
            return false;
        }

        // Use raw system fontData to create freetype face for glyph rendering
        m_descriptor.source.setData(fontData);
    } else {
        LOGD("Reusing converted font data");
    }

    auto &buffer = m_descriptor.source.buffer();
    FT_Error error = FT_New_Memory_Face(m_ft.getLib(), reinterpret_cast<const FT_Byte *>(buffer.data()),
                                        buffer.size(), m_descriptor.faceIndex, &m_ftFace);

    if (error) {
        LOGE("Could not create font: error: %d", error);
        m_invalid = true;
        return false;
    }

    if (force_ucs2_charmap(m_ftFace)) {
        LOGE("Font is broken or irrelevant...");
        // ...but DroisSansJapan still seems to work!
        // FT_Done_Face(m_ftFace);
        // m_ftFace = nullptr;
        // return false;
    }

    int dpi = 72;
    FT_Set_Char_Size(m_ftFace,
                     m_baseSize * 64, // char_width in 26.6 fixed-point
                     m_baseSize * 64, // char_height in 26.6 fixed-point
                     dpi,       // horizontal_resolution
                     dpi);      // vertical_resolution

    // Set font metrics from cgFont and ctFont
    CTFontRef ctFont = CTFontCreateWithGraphicsFont(cgFont, m_baseSize, nullptr, nullptr);

    // Create harfbuzz font context using CTFont (New API available in hb 1.7.2)
    m_hbFont = hb_coretext_font_create(ctFont);

    hb_font_set_scale(m_hbFont,
                      (static_cast<uint64_t>(m_ftFace->size->metrics.x_scale) *
                       static_cast<uint64_t>(m_ftFace->units_per_EM)) >> 16,
                      (static_cast<uint64_t>(m_ftFace->size->metrics.y_scale) *
                       static_cast<uint64_t>(m_ftFace->units_per_EM)) >> 16);
    hb_font_set_ppem(m_hbFont, m_ftFace->size->metrics.x_ppem, m_ftFace->size->metrics.y_ppem);

    m_metrics.height = CGFontGetCapHeight(cgFont) / (float)::CGFontGetUnitsPerEm(cgFont) * m_baseSize;
    m_metrics.ascent = CGFontGetAscent(cgFont) / (float)::CGFontGetUnitsPerEm(cgFont) * m_baseSize;
    m_metrics.descent = -CGFontGetDescent(cgFont) / (float)::CGFontGetUnitsPerEm(cgFont) * m_baseSize;

    m_metrics.lineThickness = CTFontGetUnderlineThickness(ctFont);
    m_metrics.underlineOffset = -CTFontGetUnderlinePosition(ctFont);

    CFRelease(ctFont);
    CGFontRelease(cgFont);

    LOGI("LOADED Apple System Font: %s size: %d", getFullName(), m_baseSize);

    m_loaded = true;
    return true;
}

}
