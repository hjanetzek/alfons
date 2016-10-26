/*
 * Based on The New Chronotext Toolkit
 * Copyright (C) 2014, Ariel Malka - All rights reserved.
 *
 * Adapted to Alfons
 * Copyright (C) 2015, Hannes Janetzek
 *
 * The following source-code is distributed under the Simplified BSD License.
 */

#include "splinePath.h"
#include "ASPC.h"

#include <cstring>

namespace alfons {
SplinePath::SplinePath(int capacity)
    : closed(false) {
    if (capacity > 0) {
        points.reserve(capacity);
    }
}

SplinePath::SplinePath(const std::vector<glm::vec2>& points)
    : closed(false) {
    add(points);
}

void SplinePath::add(const std::vector<glm::vec2>& newPoints) {
    points.reserve(size() + newPoints.size());

    for (auto& point : newPoints) {
        add(point);
    }
}

void SplinePath::add(const glm::vec2& point) {
    if (!points.empty() && point == points.back()) {
        return;
    }

    points.emplace_back(point);
}

const std::vector<glm::vec2>& SplinePath::getPoints() const {
    return points;
}

void SplinePath::clear() {
    points.clear();
}

int SplinePath::size() const {
    return points.size();
}

bool SplinePath::empty() const {
    return points.empty();
}

void SplinePath::close() {
    if (size() > 2) {
        closed = true;

        if (points.front() == points.back()) {
            points.pop_back();
        }
    }
}

bool SplinePath::isClosed() const {
    return closed;
}

void SplinePath::flush(SplineType type, std::vector<glm::vec2>& path, float tol) const {
    std::function<glm::vec2(float, glm::vec2*)> gamma;

    switch (type) {
    case SplineType::bspline:
        gamma = GammaBSpline;
        break;

    case SplineType::catmull_rom:
        gamma = GammaCatmullRom;
        break;

    default:
        return;
    }

    int size = points.size();

    if (size > 2) {
        ASPC aspc(gamma, path, tol);

        if (closed) {
            aspc.segment(points[size - 1], points[0], points[1], points[2]);
        } else {
            if (type == SplineType::bspline) {
                aspc.segment(points[0], points[0], points[0], points[1]);
            }

            aspc.segment(points[0], points[0], points[1], points[2]);
        }

        for (int i = 0; i < size - 3; i++) {
            aspc.segment(points[i], points[i + 1], points[i + 2], points[i + 3]);
        }

        if (closed) {
            aspc.segment(points[size - 3], points[size - 2], points[size - 1], points[0]);
            aspc.segment(points[size - 2], points[size - 1], points[0], points[1]);
        } else {
            aspc.segment(points[size - 3], points[size - 2], points[size - 1], points[size - 1]);
            aspc.segment(points[size - 2], points[size - 1], points[size - 1], points[size - 1]);

            if (type == SplineType::bspline) {
                aspc.segment(points[size - 1], points[size - 1], points[size - 1], points[size - 1]);
            }
        }
    }
}
}
