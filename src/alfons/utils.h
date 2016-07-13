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

#include <set>
#include <vector>
#include <algorithm>
#include <glm/vec4.hpp>

namespace alfons {

template <typename T>
static int search(T* array, T value, int min, int max) {
    int mid = (min + max) >> 1;

    while (min < mid) {
        if (array[mid - 1] < value) {
            min = mid;
        } else if (array[mid - 1] > value) {
            max = mid;
        } else {
            min = max = mid;
        }
        mid = (min + max) >> 1;
    }

    return mid - 1;
}

template <typename T>
static inline int search(const std::vector<T>& array, float value, int min,
                         int max) {
    return search((T*)array.data(), value, min, max);
}

template <typename T>
struct Iterator {
    T& container;
    bool reverse;

    typedef typename T::const_iterator I;

    // Works for const and non-const std containers
    typedef typename std::iterator_traits<I>::reference R;

    struct InnerIterator {
        I i;
        bool reverse;

        InnerIterator(I i, bool reverse) : i(i), reverse(reverse) {}
        R operator*() { return *i; }
        I operator++() { return (reverse ? --i : ++i); }
        bool operator!=(const InnerIterator& o) { return i != o.i; }
    };

    Iterator(T& container, bool reverse)
        : container(container), reverse(reverse) {}
    InnerIterator begin() {
        return InnerIterator(reverse ? --container.end() : container.begin(),
                             reverse);
    }
    InnerIterator end() {
        return InnerIterator(reverse ? --container.begin() : container.end(),
                             reverse);
    }
};
template <typename T>
Iterator<T> DirectionalRange(T& container, bool reverse = false) {
    return Iterator<T>(container, reverse);
}

}
