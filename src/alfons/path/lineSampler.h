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

#include "alfons/alfons.h"

#if 0
#include "geom/rect.h"
#include "geom/Path2d.h"
#include "geom/DataSource.h"
#include "geom/DataTarget.h"
#endif

#include <vector>

namespace alfons {

class LineSampler {
public:
    struct Value {
        glm::vec2 position;
        float angle;
        float offset;
        int index;
    };

    struct ClosePoint {
        glm::vec2 position; // POSITION OF CLOSEST-POINT ON PATH
        float offset;       // OFFSET OF CLOSEST-POINT ON PATH
        float distance;     // DISTANCE TO CLOSEST-POINT ON PATH
    };

    enum class Mode {
        bounded,
        loop,
        tangent,
        modulo,
    };

    LineSampler(int capacity = 0);
    LineSampler(const std::vector<glm::vec2>& points);
#if 0
    LineSampler(const Path2d &path, float approximationScale = 1.0f);
    LineSampler(DataSourceRef source);
    void read(DataSourceRef source);
    void write(DataTargetRef target);
#endif

    void add(const std::vector<glm::vec2>& points);
    void add(const glm::vec2& point);
    inline void add(float x, float y) { add(glm::vec2(x, y)); }

    const std::vector<glm::vec2>& getPoints() const;
    const std::vector<float>& getLengths() const;

    void clear();
    int size() const;
    bool empty() const;

    float getLength() const;
    Rect getBounds() const;

    void close();
    bool isClosed() const;

    void setMode(Mode mode);
    Mode getMode() const;

    Value offset2Value(float offset) const;
    glm::vec2 offset2Position(float offset) const;
    float offset2Angle(float offset) const;
    float offset2SampledAngle(float offset, float sampleSize) const;
    glm::vec2 offset2Gradient(float offset, float sampleSize) const;

    bool get(float _offset, glm::vec2& _position, float& _angle) const;

    bool findClosestPoint(const glm::vec2& input, float threshold,
                          ClosePoint& output) const;
    ClosePoint closestPointFromSegment(const glm::vec2& input,
                                       int segmentIndex) const;

protected:
    Mode mode;

    std::vector<glm::vec2> points;
    std::vector<float> lengths;

    void extendCapacity(int amount);
};
}
