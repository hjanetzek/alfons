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

#if 0
#include "geom/matrixAffine2.h"
#include "geom/rect.h"
#endif

#include "glm/vec2.hpp"

#include <vector>
#include <algorithm>
#include <math.h>

namespace alfons {
static const float D2R = M_PI / 180.0;
static const float R2D = 180.0 / M_PI;

static const float PI = M_PI;
static const float TWO_PI = M_PI * 2.0;
static const float HALF_PI = M_PI * 0.5;


inline float boundf(float value, float range) {
    float bound = fmodf(value, range);
    return (bound < 0) ? (bound + range) : bound;
}

inline int bound(int value, int range) {
    int bound = value % range;
    return (bound < 0) ? (bound + range) : bound;
}

/*
 * S-SHAPED CROSS-FADE CURVE: 3 * (t ^ 2) - 2 * (t ^ 3)
 */
inline float ease(float t) { return (t * t * (3 - 2 * t)); }

#if 0
/*
 * Based on quake
 * http://betterexplained.com/articles/understanding-quakes-fast-inverse-square-root/
 */
inline float fastSqrt(float x) {
    int i = *(int*)&x;              // store floating-point bits in integer
    i = 0x5f3759d5 - (i >> 1);      // initial guess for Newton's method
    float r = *(float*)&i;          // convert new bits into float
    r *= (1.5f - 0.5f * x * r * r); // One round of Newton's method
    return r * x;
}
#endif

inline int nextPowerOfTwo(int x) {
    int result = 1;

    while (result < x) {
        result <<= 1;
    }

    return result;
}

inline bool isPowerOfTwo(int x) { return (x > 0) && !(x & (x - 1)); }

/*
 * Reference for the 4 following functions:
 * http://stackoverflow.com/a/253874/50335
 */
inline bool approximatelyEqual(float a, float b, float epsilon) {
    return fabsf(a - b) <= ((fabsf(a) < fabsf(b) ? fabsf(b) : fabsf(a)) * epsilon);
}

inline bool essentiallyEqual(float a, float b, float epsilon) {
    return fabsf(a - b) <= ((fabsf(a) > fabsf(b) ? fabsf(b) : fabsf(a)) * epsilon);
}

inline bool definitelyGreaterThan(float a, float b, float epsilon) {
    return (a - b) > ((fabsf(a) < fabsf(b) ? fabsf(b) : fabsf(a)) * epsilon);
}

inline bool definitelyLessThan(float a, float b, float epsilon) {
    return (b - a) > ((fabsf(a) < fabsf(b) ? fabsf(b) : fabsf(a)) * epsilon);
}

class MathUtils {
public:
#if 0
    static bool isRectNull(const Rectf& rect);
    static bool compareRects(const Rectf& r1, const Rectf& r2);

    static void transformVertices(const std::vector<glm::vec2>& source,
                                  std::vector<glm::vec2>& target,
                                  const MatrixAffine2f& matrix);
    static Rectf getBoundingBox(const std::vector<glm::vec2>& polygon);
#endif
    static float getShortestDistance(
        const glm::vec2& point, const std::vector<glm::vec2>& polygon,
        bool close = false,
        float threshold = std::numeric_limits<float>::max());

    static bool isPointInside(const glm::vec2& point,
                              const std::vector<glm::vec2>& polygon);
    static bool isPointInside(const glm::vec2& point,
                              const std::vector<std::vector<glm::vec2>>& polygons);

};
}
