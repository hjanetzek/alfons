#include "glfwloop.h"

#include "alfons/fontManager.h"
#include "alfons/textBatch.h"
#include "alfons/textShaper.h"
#include "alfons/font.h"
#include "alfons/path/lineSampler.h"
#include "alfons/renderer/renderer.h"

#include "logger.h"
#include "demoRenderer.h"

#define TEXT_SIZE 16
#define DEFAULT "DroidSans.ttf"
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

void onSetup(int w, int h) {

    renderer.init();

    font = fontMan.addFont(DEFAULT, TEXT_SIZE);
    font->addFace(fontMan.getFontFace(InputSource(FONT_JA), TEXT_SIZE));
    font->addFace(fontMan.getFontFace(InputSource(FALLBACK), TEXT_SIZE));

    l.push_back(shaper.shape(font, "Eß hatte aber alle Welt önerlei Zünge und Spräche."));
    l.push_back(shaper.shape(font, "وَكَانَتِ الارْضُ كُلُّهَا لِسَانا وَاحِدا وَلُغَةً وَاحِدًَ.")); // ar
    l.push_back(shaper.shape(font, "ΚΑΙ ολόκληρη η γη ήταν μιας γλώσσας, και μιας φωνής.")); // el
    l.push_back(shaper.shape(font, "And the whole earth was of one language, and of one speech."));
    l.push_back(shaper.shape(font, "ERA entonces toda la tierra de una lengua y unas mismas palabras."));
    l.push_back(shaper.shape(font, "Toute la terre avait une seule langue et les mêmes mots."));
    //l.push_back(shaper.shape(font, "nוַיְהִי כָל-הָאָרֶץ, שָׂפָה אֶחָת, וּדְבָרִים, אֲחָדִים.")); //he
    //l.push_back(shaper.shape(font, "सारी पृथ्वी पर एक ही भाषा, और एक ही बोली थी।")); // hi
    l.push_back(shaper.shape(font, "全地は同じ発音、同じ言葉であった。")); //ja
    l.push_back(shaper.shape(font, "온 땅의 구음이 하나이요 언어가 하나이었더라")); //ko
    l.push_back(shaper.shape(font, "На всей земле был один язык и одно наречие."));
    l.push_back(shaper.shape(font, "那時、天下人的口音言語、都是一樣。")); //zh-tw
}

void onDraw(int width, int height) {

    batch.setClip(0,0, width, height);
    renderer.beginFrame(width, height);

    glm::vec2 offset(20, 20);

    for (auto& layout : l) {
        auto adv = batch.draw(layout, offset, std::max(width - 40, 10));
        offset.y = adv.y;
    }

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
