/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "lineSampler.h"

#include "alfons/path/splinePath.h"
#include "alfons/utils.h"

#include "glm/gtx/norm.hpp"

namespace alfons {

static float boundf(float value, float range) {
    float bound = fmodf(value, range);
    return (bound < 0) ? (bound + range) : bound;
}

LineSampler::LineSampler(int capacity)
    : mode(Mode::tangent) {
    if (capacity > 0) {
        extendCapacity(capacity);
    }
}

LineSampler::LineSampler(const std::vector<glm::vec2>& points)
    : mode(Mode::tangent) {
    add(points);
}

void LineSampler::add(const std::vector<glm::vec2>& newPoints) {
    extendCapacity(newPoints.size());

    for (auto& point : newPoints) {
        add(point);
    }
}

void LineSampler::add(const glm::vec2& point) {
    if (points.empty()) {
        lengths.push_back(0);
    } else {
        glm::vec2 delta = point - points.back();

        // Ignore zero-length segments
        if (delta == glm::vec2(0))
            return;

        lengths.push_back(lengths.back() + glm::length(delta));
    }
    points.push_back(point);
}

void LineSampler::clear() {
    points.clear();
    lengths.clear();
}

int LineSampler::size() const {
    return points.size();
}

bool LineSampler::empty() const {
    return points.empty();
}

float LineSampler::getLength() const {
    if (!points.empty()) {
        return lengths.back();
    } else {
        return 0;
    }
}

Rect LineSampler::getBounds() const {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();

    for (auto& point : points) {
        if (point.x < minX)
            minX = point.x;
        if (point.y < minY)
            minY = point.y;

        if (point.x > maxX)
            maxX = point.x;
        if (point.y > maxY)
            maxY = point.y;
    }

    return Rect{minX, minY, maxX, maxY};
}

void LineSampler::close() {
    if (size() > 2) {
        if (points.front() != points.back()) {
            add(points.front());
        }
    }
}

bool LineSampler::isClosed() const {
    return (size() > 2) && (points.front() == points.back());
}

void LineSampler::setMode(Mode mode) {
    this->mode = mode;
}

LineSampler::Mode LineSampler::getMode() const {
    return mode;
}

LineSampler::Value LineSampler::offset2Value(float offset) const {
    Value value;

    float length = getLength();
    if (length <= 0) {
        value.angle = 0;
        value.offset = 0;
        value.index = 0;
        return value;
    }

    if ((mode == Mode::loop) || (mode == Mode::modulo)) {
        offset = boundf(offset, length);
    } else {
        if (offset <= 0) {
            if (mode == Mode::bounded) {
                offset = 0;
            }
        } else if (offset >= length) {
            if (mode == Mode::bounded) {
                offset = length;
            }
        }
    }

    int index = search(lengths, offset, 1, size());

    auto& p0 = points[index];
    auto& p1 = points[index + 1];

    float ratio = (offset - lengths[index]) / (lengths[index + 1] - lengths[index]);

    value.position = p0 + (p1 - p0) * ratio;
    value.angle = atan2(p1.y - p0.y, p1.x - p0.x);
    value.offset = offset;
    value.index = index;

    return value;
}

bool LineSampler::get(float _offset, glm::vec2& _position, float& _angle) const {
    float length = getLength();

    if (length > 0) {
        if ((mode == Mode::loop) || (mode == Mode::modulo)) {
            _offset = boundf(_offset, length);
        } else {
            if (_offset <= 0) {
                if (mode == Mode::bounded) {
                    _offset = 0;
                }
            } else if (_offset >= length) {
                if (mode == Mode::bounded) {
                    _offset = length;
                }
            }
        }

        int index = search(lengths, _offset, 1, size());
        auto& p0 = points[index];
        auto& p1 = points[index + 1];

        float ratio = (_offset - lengths[index]) / (lengths[index + 1] - lengths[index]);
        _position = p0 + (p1 - p0) * ratio;

        _angle =  atan2(p1.y - p0.y, p1.x - p0.x);

        return true;
    } else {
        _position = glm::vec2{0};
        _angle = 0;
        return false;
    }
}
glm::vec2 LineSampler::offset2Position(float offset) const {
    float length = getLength();

    if (length > 0) {
        if ((mode == Mode::loop) || (mode == Mode::modulo)) {
            offset = boundf(offset, length);
        } else {
            if (offset <= 0) {
                if (mode == Mode::bounded) {
                    return points.front();
                }
            } else if (offset >= length) {
                if (mode == Mode::bounded) {
                    return points.back();
                }
            }
        }

        int index = search(lengths, offset, 1, size());
        auto& p0 = points[index];
        auto& p1 = points[index + 1];

        float ratio = (offset - lengths[index]) / (lengths[index + 1] - lengths[index]);
        return p0 + (p1 - p0) * ratio;
    } else {
        return glm::vec2(0);
    }
}

float LineSampler::offset2Angle(float offset) const {
    float length = getLength();

    if (length > 0) {
        if ((mode == Mode::loop) || (mode == Mode::modulo)) {
            offset = boundf(offset, length);
        } else {
            if (offset <= 0) {
                if (mode == Mode::bounded) {
                    offset = 0;
                }
            } else if (offset >= length) {
                if (mode == Mode::bounded) {
                    offset = length;
                }
            }
        }

        int index = search(lengths, offset, 1, size());
        auto& p0 = points[index];
        auto& p1 = points[index + 1];

        return atan2(p1.y - p0.y, p1.x - p0.x);
    } else {
        return 0;
    }
}

float LineSampler::offset2SampledAngle(float offset, float sampleSize) const {
    glm::vec2 gradient = offset2Gradient(offset, sampleSize);

    /*
     * We use an epsilon value to avoid degenerated results in some cases
     * (E.g. close to 180 degree diff. between two segments)
     */
    if (glm::length2(gradient) > 1) {
        return atan2(gradient.y, gradient.x);
    } else {
        return offset2Angle(offset);
    }
}

glm::vec2 LineSampler::offset2Gradient(float offset, float sampleSize) const {
    glm::vec2 pm = offset2Position(offset - sampleSize * 0.5f);
    glm::vec2 pp = offset2Position(offset + sampleSize * 0.5f);

    return (pp - pm) * 0.5f;
}

/*
   * Returns false if closest-point is farther than 'threshold' distance
   *
   * Reference: "Minimum Distance between a Point and a Line" BY Paul Bourke
   * http://paulbourke.net/geometry/pointlineplane
   */
bool LineSampler::findClosestPoint(const glm::vec2& input, float threshold, ClosePoint& output) const {

    float sqMin = threshold * threshold;

    int end = size();
    int index = -1;
    glm::vec2 position;
    float offset;

    for (int i = 0; i < end; i++) {
        int i0, i1;

        if (i == end - 1) {
            i0 = i - 1;
            i1 = i;
        } else {
            i0 = i;
            i1 = i + 1;
        }

        auto& p0 = points[i0];
        auto& p1 = points[i1];

        glm::vec2 delta = p1 - p0;
        float length = lengths[i1] - lengths[i0];
        float u = glm::dot(delta, (input - p0)) / (length * length);

        if (u >= 0 && u <= 1) {
            glm::vec2 p = p0 + u * delta;
            float mag = glm::length2(p - input);

            if (mag < sqMin) {
                sqMin = mag;
                index = i0;

                position = p;
                offset = lengths[index] + u * length;
            }
        } else {
            float mag0 = glm::length2(p0 - input);
            float mag1 = glm::length2(p1 - input);

            if ((mag0 < sqMin) && (mag0 < mag1)) {
                sqMin = mag0;
                index = i0;

                position = points[i0];
                offset = lengths[index];
            } else if ((mag1 < sqMin) && (mag1 < mag0)) {
                sqMin = mag1;
                index = i1;

                position = points[i1];
                offset = lengths[index];
            }
        }
    }

    if (index != -1) {
        output.position = position;
        output.offset = offset;
        output.distance = sqrt(sqMin);

        return true;
    }

    return false;
}

/* Reference: "Minimum Distance between a Point and a Line" BY Paul Bourke
 * http://paulbourke.net/geometry/pointlineplane
 */
LineSampler::ClosePoint LineSampler::closestPointFromSegment(const glm::vec2& input, int segmentIndex) const {
    LineSampler::ClosePoint output;

    if ((segmentIndex >= 0) && (segmentIndex + 1 < size())) {
        int i0 = segmentIndex;
        int i1 = segmentIndex + 1;

        auto& p0 = points[i0];
        auto& p1 = points[i1];

        glm::vec2 delta = p1 - p0;
        float length = lengths[i1] - lengths[i0];
        float u = glm::dot(delta, (input - p0)) / (length * length);

        if (u >= 0 && u <= 1) {
            glm::vec2 p = p0 + u * delta;
            float mag = glm::length2(p - input);

            output.position = p;
            output.offset = lengths[i0] + u * length;
            output.distance = sqrt(mag);
        } else {
            float mag0 = glm::length2(p0 - input);
            float mag1 = glm::length2(p1 - input);

            if (mag0 < mag1) {
                output.position = p0;
                output.offset = lengths[i0];
                output.distance = sqrt(mag0);
            } else {
                output.position = p1;
                output.offset = lengths[i1];
                output.distance = sqrt(mag1);
            }
        }
    } else {
        output.distance = std::numeric_limits<float>::max();
        output.offset = 0;
    }

    return output;
}

void LineSampler::extendCapacity(int amount) {
    int newCapacity = size() + amount;
    points.reserve(newCapacity);
    lengths.reserve(newCapacity);
}

void LineSampler::sampleSpline(const SplinePath &path, SplineType type, float tolerance) {
    points.clear();
    lengths.clear();

    path.flush(type, points, tolerance);

    if (points.size() < 2) { return; }


    lengths.push_back(0);

    size_t end = points.size();
    for (size_t i = 1; i < end; ) {
        glm::vec2 delta = points[i] - points[i-1];

        // Ignore zero-length segments
        if (delta == glm::vec2(0)) {
            points.erase(points.begin() + i);
            end -= 1;
            continue;
        }
        lengths.push_back(lengths.back() + glm::length(delta));

        i++;
    }

    if (path.isClosed()) {
        close();
        setMode(LineSampler::Mode::loop);
    }

}

}
