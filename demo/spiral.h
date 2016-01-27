/*
 * THE NEW CHRONOTEXT TOOLKIT: https://github.com/arielm/new-chronotext-toolkit
 * COPYRIGHT (C) 2012-2014, ARIEL MALKA ALL RIGHTS RESERVED.
 *
 * THE FOLLOWING SOURCE-CODE IS DISTRIBUTED UNDER THE MODIFIED BSD LICENSE:
 * https://github.com/arielm/new-chronotext-toolkit/blob/master/LICENSE.md
 */

#pragma once

#include "alfons/font.h"
#include "alfons/textBatch.h"
#include "alfons/utils.h"
#include "alfons/path/mathUtils.h"

#include <set>

using namespace alfons;

class TextSpiral {

public:
    float ox;
    float oy;
    float r1;
    float r2;
    float turns;

    std::vector<glm::vec2> vertices;

    void update(float ox, float oy, float r1, float r2, float turns, float DD1, float DD2) {
        this->ox = ox;
        this->oy = oy;
        this->r1 = r1;
        this->r2 = r2;
        this->turns = turns;

        //    vertices.clear();
        //
        //    float l = TWO_PI * turns;
        //    float L = PI * turns * (r1 + r2);
        //    float dr = (r2 - r1) / l;
        //    float DD = (DD2 - DD1) / l;
        //    float D = 0;
        //
        //    do
        //    {
        //        float r = math<float>::sqrt(r1 * r1 + 2 * dr * D);
        //        float d = (r - r1) / dr;
        //        D += DD1 + d * DD;
        //
        //        vertices.emplace_back(ox - math<float>::cos(d) * r, oy + math<float>::sin(d) * r);
        //    }
        //    while (D < L);
    }
    float drawText(alfons::TextBatch &batch, const alfons::LineLayout &line, float offsetX = 0, float offsetY = 0) {
        bool reverse = false; //(line.direction() == HB_DIRECTION_RTL);

        float l = TWO_PI * turns;
        float dr = (r2 - r1) / l;
        float adv = offsetX;

        auto& matrix = batch.matrix();

        for (auto &shape : DirectionalRange(line.shapes(), reverse)) {
            float half = 0.5f * line.advance(shape);
            adv += half;

            if (!(shape.isSpace))  {
                float r = sqrt(r1 * r1 + 2 * dr * adv);
                float d = (r - r1) / dr;

                matrix.setTranslation(ox - cos(d) * r,
                                      oy + sin(d) * r);

                matrix.rotateZ((reverse ? -1 : +1) * HALF_PI - d);

                batch.drawTransformedShape(line.font(), shape, -half, offsetY, line.scale());
            }

            adv += half;
        }
        return adv;

    }
};
