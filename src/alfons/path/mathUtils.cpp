/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "mathUtils.h"

#include "glm/gtx/norm.hpp"


namespace alfons {

#if 0
bool MathUtils::isRectNull(const Rectf& rect) {
    return (rect.x1 == 0) && (rect.y1 == 0) && (rect.x2 == 0) && (rect.y2 == 0);
}

bool MathUtils::compareRects(const Rectf& r1, const Rectf& r2) {
    return (r1.x1 == r2.x1) && (r1.y1 == r2.y1) && (r1.x2 == r2.x2) && (r1.y2 == r2.y2);
}

void MathUtils::transformVertices(const std::vector<glm::vec2>& source, std::vector<glm::vec2>& target,
                                  const MatrixAffine2f& matrix) {
    target.clear();
    target.reserve(source.size());

    for (auto& vertex : source) {
        target.push_back(matrix.transformPoint(vertex));
    }
}
  Rectf MathUtils::getBoundingBox(const vector<vec2> &polygon) {
    float minX = numeric_limits<float>::max();
    float minY = numeric_limits<float>::max();
    float maxX = numeric_limits<float>::min();
    float maxY = numeric_limits<float>::min();
        
    for (auto &point : polygon) {
      if (point.x < minX) minX = point.x;
      if (point.y < minY) minY = point.y;
            
      if (point.x > maxX) maxX = point.x;
      if (point.y > maxY) maxY = point.y;
    }
        
    return Rectf(minX, minY, maxX, maxY);
  }
#endif
/*
   * Returns numeric_limits<float>::max() if closest point is farther than threshold distance
   *
   * Reference: "Minimum Distance between a Point and a Line" BY Paul Bourke
   * http://paulbourke.net/geometry/pointlineplane
   */
float MathUtils::getShortestDistance(const glm::vec2& point, const std::vector<glm::vec2>& polygon,
                                     bool close, float threshold) {
    float min = threshold * threshold;

    int end = polygon.size();
    bool found = false;

    for (int i = 0; i < end; i++) {
        int i0, i1;

        if (i == end - 1) {
            if (close) {
                i0 = i;
                i1 = 0;
            } else {
                break;
            }
        } else {
            i0 = i;
            i1 = i + 1;
        }

        glm::vec2 p0 = polygon[i0];
        glm::vec2 p1 = polygon[i1];

        if (p0 != p1) {
            glm::vec2 delta = p1 - p0;
            float u = glm::length2(glm::dot(delta, point - p0) / delta);

            if (u >= 0 && u <= 1) {
                glm::vec2 p = p0 + u * delta;
                float mag = glm::length2(p - point);

                if (mag < min) {
                    min = mag;
                    found = true;
                }
            } else {
                float mag0 = glm::length2(p0 - point);
                float mag1 = glm::length2(p1 - point);

                if ((mag0 < min) && (mag0 < mag1)) {
                    min = mag0;
                    found = true;
                } else if ((mag1 < min) && (mag1 < mag0)) {
                    min = mag1;
                    found = true;
                }
            }
        }
    }

    //return found ? math<float>::sqrt(min) : std::numeric_limits<float>::max();
    return found ? sqrt(min) : std::numeric_limits<float>::max();
}

/*
   * The following 4 functions are based on code from cinder:
   *
   * https://github.com/cinder/Cinder/blob/ae580c2cb0fc44d0a99b233dbefdf736f7093209/src/cinder/Path2d.cpp#L754-768
   * https://github.com/cinder/Cinder/blob/ae580c2cb0fc44d0a99b233dbefdf736f7093209/src/cinder/Path2d.cpp#L824-857
   * https://github.com/cinder/Cinder/blob/ae580c2cb0fc44d0a99b233dbefdf736f7093209/src/cinder/Shape2d.cpp#L135-144
   *
   * Assertion: the implied winding is "odd" (aka "even-odd fill-rule")
   *
   * TODO:
   * Check what's necessary in order to handle "non-zero" winding
   * (which seems to be the default fill-rule in html canvas)
   * Reference: http://blogs.adobe.com/webplatform/2013/01/30/winding-rules-in-canvas/
   */

static float linearYatX(const glm::vec2 p[2], float x) {
    if (p[0].x == p[1].x) return p[0].y;
    return p[0].y + (p[1].y - p[0].y) * (x - p[0].x) / (p[1].x - p[0].x);
}

static size_t linearCrossings(const glm::vec2 p[2], const glm::vec2& pt) {
    if ((p[0].x < pt.x && pt.x <= p[1].x) || (p[1].x < pt.x && pt.x <= p[0].x)) {
        if (pt.y > linearYatX(p, pt.x)) return 1;
    }

    return 0;
}

bool MathUtils::isPointInside(const glm::vec2& point, const std::vector<glm::vec2>& polygon) {
    const size_t size = polygon.size();

    if (size <= 2) {
        return false;
    } else {
        size_t crossings = 0;

        for (size_t s = 0; s < size - 1; ++s) {
            crossings += linearCrossings(&(polygon[s]), point);
        }

        glm::vec2 temp[2];
        temp[0] = polygon[size - 1];
        temp[1] = polygon[0];
        crossings += linearCrossings(&(temp[0]), point);

        return (crossings & 1) == 1;
    }
}

bool MathUtils::isPointInside(const glm::vec2& point, const std::vector<std::vector<glm::vec2>>& polygons) {
    int numPathsInside = 0;

    for (auto& polygon : polygons) {
        if (isPointInside(point, polygon)) {
            numPathsInside++;
        }
    }

    return (numPathsInside % 2) == 1;
}
}
