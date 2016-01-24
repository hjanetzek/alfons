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

#include <glm/vec2.hpp>

namespace alfons {

struct Glyph {
    Glyph(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, glm::vec2 offset, glm::vec2 size)
        :  offset(offset), size(size) {
        u1 = x0;
        v1 = y0;
        u2 = x0 + x1;
        v2 = y0 + y1;
    }

    glm::vec2 offset;
    glm::vec2 size;

    uint16_t u1;
    uint16_t v1;
    uint16_t u2;
    uint16_t v2;
};
}
