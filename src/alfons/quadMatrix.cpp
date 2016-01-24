/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "quadMatrix.h"

#include "glm/gtx/norm.hpp"
#include "glm/gtx/transform.hpp"

#include <cmath>

namespace alfons {

QuadMatrix::QuadMatrix() {
    setIdentity();
}

void QuadMatrix::load(const glm::mat4& matrix) {
    m = matrix;
}

void QuadMatrix::setIdentity() {
    m = glm::mat4();
}

void QuadMatrix::push() {
    stack.push_back(m);
}

void QuadMatrix::pop() {
    if (!stack.empty()) {
        m = stack.back();
        stack.pop_back();
    }
}

void QuadMatrix::setTranslation(float x, float y, float z) {
    m = glm::mat4();
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
}

void QuadMatrix::translate(float x, float y, float z) {
    m = glm::translate(m, glm::vec3(x, y, z));
}

void QuadMatrix::scale(float x, float y, float z) {
    m = glm::scale(m, glm::vec3(x, y, z));
}

void QuadMatrix::rotateX(float a) {
    m = glm::rotate(m, a, glm::vec3(1, 0, 0));
}

void QuadMatrix::rotateY(float a) {
    m = glm::rotate(m, a, glm::vec3(0, 1, 0));
}

void QuadMatrix::rotateZ(float a) {
    float c = ::cos(a);
    float s = ::sin(a);

    float r00 = m00 * c + m01 * s;
    float r01 = m01 * c - m00 * s;

    float r10 = m10 * c + m11 * s;
    float r11 = m11 * c - m10 * s;

    float r20 = m20 * c + m21 * s;
    float r21 = m21 * c - m20 * s;

    float r30 = m30 * c + m31 * s;
    float r31 = m31 * c - m30 * s;

    m00 = r00;
    m01 = r01;
    m10 = r10;
    m11 = r11;
    m20 = r20;
    m21 = r21;
    m30 = r30;
    m31 = r31;
    //m = glm::rotate(m, a, glm::vec3(0, 0, 1));
}

void QuadMatrix::rotateXY(float sx, float sy) {

    float cx = ::sqrt(1.0f - sx * sx);
    float cy = ::sqrt(1.0f - sy * sy);

    float cxy = cx + cy;

    float r00 = m00 * cy - m02 * sy;
    float r01 = m01 * cx + m02 * sx;
    float r02 = m00 * sy + m02 * cxy - m01 * sx;

    float r10 = m10 * cy - m12 * sy;
    float r11 = m11 * cx + m12 * sx;
    float r12 = m10 * sy + m12 * cxy - m11 * sx;

    float r20 = m20 * cy - m22 * sy;
    float r21 = m21 * cx + m22 * sx;
    float r22 = m20 * sy + m22 * cxy - m21 * sx;

    float r30 = m30 * cy - m32 * sy;
    float r31 = m31 * cx + m32 * sx;
    float r32 = m30 * sy + m32 * cxy - m31 * sx;

    m00 = r00;
    m01 = r01;
    m02 = r02;
    m10 = r10;
    m11 = r11;
    m12 = r12;
    m20 = r20;
    m21 = r21;
    m22 = r22;
    m30 = r30;
    m31 = r31;
    m32 = r32;
}

glm::vec3 QuadMatrix::transform(float x, float y) const {
    float x00 = x * m00;
    float x10 = x * m10;
    float x20 = x * m20;

    float y01 = y * m01;
    float y11 = y * m11;
    float y21 = y * m21;

    return glm::vec3(x00 + y01 + m03, x10 + y11 + m13, x20 + y21 + m23);
}

void QuadMatrix::transformRect(const Rect& _rect, Quad& out) const {
    float x100 = _rect.x1 * m00;
    float x110 = _rect.x1 * m10;

    float y101 = _rect.y1 * m01;
    float y111 = _rect.y1 * m11;

    float x200 = _rect.x2 * m00;
    float x210 = _rect.x2 * m10;

    float y201 = _rect.y2 * m01;
    float y211 = _rect.y2 * m11;

    out.x1 = x100 + y101 + m03;
    out.y1 = x110 + y111 + m13;

    out.x2 = x100 + y201 + m03;
    out.y2 = x110 + y211 + m13;

    out.x3 = x200 + y201 + m03;
    out.y3 = x210 + y211 + m13;

    out.x4 = x200 + y101 + m03;
    out.y4 = x210 + y111 + m13;
}
}
