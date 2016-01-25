/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#pragma once

#include "alfons.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <array>

namespace alfons {

class QuadMatrix {
public:
    union {
        glm::mat4 m;

        struct {
            float m00, m10, m20, m30;
            float m01, m11, m21, m31;
            float m02, m12, m22, m32;
            float m03, m13, m23, m33;
        };
    };

    QuadMatrix();

    void load(const glm::mat4& matrix);

    void setIdentity();

    void push();
    void pop();

    inline void setTranslation(const glm::vec2& t) { setTranslation(t.x, t.y, 0); }
    inline void setTranslation(const glm::vec3& t) {
        setTranslation(t.x, t.y, t.z);
    }
    void setTranslation(float x, float y, float z = 0);

    inline void translate(const glm::vec2& t) { translate(t.x, t.y, 0); }
    inline void translate(const glm::vec3& t) { translate(t.x, t.y, t.z); }
    void translate(float x, float y, float z = 0);

    inline void scale(float s) { scale(s, s, s); }
    void scale(float x, float y, float z = 1);

    void rotateX(float a);
    void rotateY(float a);
    void rotateZ(float a);
    void rotateXY(float sx, float sy);

    glm::vec3 transform(float x, float y) const;

    void transformRect(const Rect& rect, Quad& out) const;

protected:
    std::vector<glm::mat4> stack;
};
}
