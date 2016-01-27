#include "glfwloop.h"

#include "alfons/fontManager.h"
#include "alfons/textBatch.h"
#include "alfons/textShaper.h"
#include "alfons/font.h"
#include "alfons/path/lineSampler.h"
#include "alfons/renderer/renderer.h"

#include "logger.h"
#include "demoRenderer.h"

#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#define TEXT_SIZE 20
#define DEFAULT "NotoSans-Regular.ttf"
#define FONT_AR "NotoNaskh-Regular.ttf"
#define FONT_HE "NotoSansHebrew-Regular.ttf"
#define FONT_HI "NotoSansDevanagari-Regular.ttf"
#define FONT_JA "DroidSansJapanese.ttf"
#define FALLBACK "DroidSansFallback.ttf"

using namespace alfons;

DemoRenderer renderer;
FontManager fontMan;
GlyphAtlas atlas(renderer, 256);
TextBatch batch(atlas, renderer);
TextShaper shaper;
std::shared_ptr<Font> font;
std::vector<LineLayout> l;

NVGcontext* vg = nullptr;

void onSetup(int w, int h) {

    renderer.init();

    font = fontMan.addFont(DEFAULT, TEXT_SIZE);
    font->addFace(fontMan.getFontFace(InputSource(FONT_AR), TEXT_SIZE));
    font->addFace(fontMan.getFontFace(InputSource(FONT_HE), TEXT_SIZE));
    font->addFace(fontMan.getFontFace(InputSource(FONT_HI), TEXT_SIZE));
    font->addFace(fontMan.getFontFace(InputSource(FONT_JA), TEXT_SIZE));
    font->addFace(fontMan.getFontFace(InputSource(FALLBACK), TEXT_SIZE));

    l.push_back(shaper.shape(font, "Eß hatte aber alle Welt einerlei Zünge und Sprache."));
    l.push_back(shaper.shape(font, "وَكَانَتِ الارْضُ كُلُّهَا لِسَانا وَاحِدا وَلُغَةً وَاحِدًَ.")); // ar
    l.push_back(shaper.shape(font, "ΚΑΙ ολόκληρη η γη ήταν μιας γλώσσας, και μιας φωνής.")); // el
    l.push_back(shaper.shape(font, "And the whole earth was of one language, and of one speech."));
    l.push_back(shaper.shape(font, "ERA entonces toda la tierra de una lengua y unas mismas palabras."));
    l.push_back(shaper.shape(font, "Toute la terre avait une seule langue et les mêmes mots."));
    l.push_back(shaper.shape(font, "nוַיְהִי כָל-הָאָרֶץ, שָׂפָה אֶחָת, וּדְבָרִים, אֲחָדִים.")); //he
    l.push_back(shaper.shape(font, "सारी पृथ्वी पर एक ही भाषा, और एक ही बोली थी।")); // hi
    l.push_back(shaper.shape(font, "全地は同じ発音、同じ言葉であった。")); //ja
    l.push_back(shaper.shape(font, "온 땅의 구음이 하나이요 언어가 하나이었더라")); //ko
    l.push_back(shaper.shape(font, "На всей земле был один язык и одно наречие."));
    l.push_back(shaper.shape(font, "那時、天下人的口音言語、都是一樣。")); //zh-tw

    // BIDI
    l.push_back(shaper.shape(font, "محور 26 يوليو 42 يوليو end"));
    l.push_back(shaper.shape(font, "start محور 26 يوليو 42 يوليو end"));

    vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

}

void onDraw(GLFWwindow *window, int width, int height) {

    nvgBeginFrame(vg, width, height, 1);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,255));

    batch.setClip(0,0, width, height);

    glm::vec2 offset(20, 20);

    for (auto& line : l) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, offset.x, offset.y);
        nvgLineTo(vg, offset.x + line.advance(), offset.y);
        nvgStroke(vg);

        auto adv = batch.draw(line, offset, std::max(width - 40, 10));
        offset.y = adv.y;
    }

    nvgEndFrame(vg);

    renderer.beginFrame(width, height);

    float inner = 0.1;
    float outer = 0.3;
    float outerCenter = 0.5 - inner;

    renderer.setColor({1.0,1.0,1.0,1.0});
    renderer.setWidth(outerCenter - outer, outerCenter + outer);
    renderer.draw();

    renderer.setColor({0.0,0.0,0.0,1.0});
    renderer.setWidth(0.5 - inner, 0.5 + inner);
    renderer.draw();

    renderer.clearQuads();

}

int main() {
    glfwLoop();
    renderer.dispose();
    return 0;
}
