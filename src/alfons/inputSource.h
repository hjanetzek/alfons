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

#include <string>

namespace alfons {

class InputSource {
public:
    static auto get(std::string uri) -> InputSource {
        return InputSource {std::move(uri)};
    }

    InputSource(std::string uri) : m_uri(std::move(uri)) {}

    std::string uri() const { return m_uri; }
    bool operator!() { return true; }


protected:
    std::string m_uri;
};
}
