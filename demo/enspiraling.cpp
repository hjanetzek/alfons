#include "glfwloop.h"

#include "alfons/fontManager.h"
#include "alfons/textBatch.h"
#include "alfons/textShaper.h"
#include "alfons/font.h"
#include "alfons/path/splinePath.h"
#include "alfons/path/lineSampler.h"

#include "logger.h"
#include "alfons/renderer/renderer.h"

#include "demoRenderer.h"
#include "spiral.h"


#define TEXT_SIZE 28

#define DEFAULT "NotoSans-Regular.ttf"
#define FONT_AR "NotoNaskh-Regular.ttf"
#define FONT_JA "DroidSansJapanese.ttf"
#define FALLBACK "DroidSansFallback.ttf"

using namespace alfons;

DemoRenderer renderer;
FontManager fontMan;
TextShaper shaper;
GlyphAtlas atlas(renderer, 256);
TextBatch batch(atlas, renderer);
LineLayout layout;
TextSpiral spiral;
LineSampler peanutPath;
std::shared_ptr<Font> font;

double prevt = 0;

void onSetup(int w, int h) {

    font = fontMan.addFont("default", InputSource(DEFAULT), TEXT_SIZE);
    font->addFace(fontMan.addFontFace(InputSource(FONT_AR), TEXT_SIZE));
    font->addFace(fontMan.addFontFace(InputSource(FONT_JA), TEXT_SIZE));
    font->addFace(fontMan.addFontFace(InputSource(FALLBACK), TEXT_SIZE));

    layout = shaper.shape(font, "محور 26 يوليو 26 يوليو");

    SplinePath spline;
    float x = 300;
    float y = 300;
    spline.add(x-100, y-100);
    spline.add(x, y-25);
    spline.add(x+100, y-100);
    spline.add(x+200, y);
    spline.add(x+100, y+100);
    spline.add(x, y+25);
    spline.add(x-100, y+100);
    spline.add(x-200, y);
    spline.close();

    spline.flush(SplinePath::Type::bspline, peanutPath, 10);

    renderer.init();

    glfwSetTime(0);
    prevt = glfwGetTime();
}

void flushBatch(float scale) {
    float inner = 0.2 / (scale);
    if (inner > 0.3) { inner = 0.3; }
    float outer = 0.2 / scale;
    if (outer > 0.3) { outer = 0.3; }
    float outerCenter = 0.5 - inner;

    renderer.setColor({1.0,1.0,1.0,1.0});
    renderer.setWidth(outerCenter - outer, outerCenter + outer);
    renderer.draw();

    renderer.setColor({0.0,0.0,0.0,1.0});
    renderer.setWidth(0.5 - inner, 0.5 + inner);
    renderer.draw();

    renderer.clearQuads();

}
void onDraw(GLFWwindow *window, int fbWidth, int fbHeight) {
    double mx, my, t;
    int winWidth, winHeight;

    t = glfwGetTime();

    glfwGetCursorPos(window, &mx, &my);

    glfwGetWindowSize(window, &winWidth, &winHeight);
    // Calculate pixel ration for hi-dpi devices.
    // float pxRatio = float(fbWidth) / float(winWidth);

    renderer.beginFrame(fbWidth, fbWidth);

    float scale = std::max(float(my) / winHeight * 4, 0.5f);

    layout.setScale(scale);
    batch.draw(layout, {50,50}, 400);
    flushBatch(scale);


    scale = 1.0;
    layout.setScale(scale);
    double r2 = 300 + sin(t) * 300;
    spiral.update(winWidth/2, winHeight/2, 200, r2, 2, 0, 0);
    float adv = 0;
    adv = spiral.drawText(batch, layout, adv, 0);
    adv = spiral.drawText(batch, layout, adv, 0);
    adv = spiral.drawText(batch, layout, adv, 0);
    adv = spiral.drawText(batch, layout, adv, 0);
    adv = spiral.drawText(batch, layout, adv, 0);

    flushBatch(scale);


    scale = 2.0;
    layout.setScale(scale);
    batch.draw(layout, peanutPath, t*40.0, 0);
    flushBatch(scale);
}

int main() {
    glfwLoop();
    renderer.dispose();
    return 0;
}
