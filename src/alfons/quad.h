/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adaption to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

namespace alfons {

struct Quad {
    float x1;
    float y1;
    float x2;
    float y2;
    float x3;
    float y3;
    float x4;
    float y4;
};

struct Rect {
    float x1;
    float y1;
    float x2;
    float y2;

#if 0
    bool clip(const Rectf &clipRect, const glm::vec2 &textureFactor) {
        if ((x1 > clipRect.x2 ) || (x2 < clipRect.x1) || (y1 > clipRect.y2) || (y2 < clipRect.y1)) {
            return false;
        } else {
            if (x1 < clipRect.x1) {
                float dx = clipRect.x1 - x1;
                x1 += dx;
                u1 += dx / textureFactor.x;
            }

            if (x2 > clipRect.x2) {
                float dx = clipRect.x2 - x2;
                x2 += dx;
                u2 += dx / textureFactor.x;
            }

            if (y1 < clipRect.y1) {
                float dy = clipRect.y1 - y1;
                y1 += dy;
                v1 += dy / textureFactor.y;
            }

            if (y2 > clipRect.y2) {
                float dy = clipRect.y2 - y2;
                y2 += dy;
                v2 += dy / textureFactor.y;
            }

            return true;
        }
    }
#endif
};
}
