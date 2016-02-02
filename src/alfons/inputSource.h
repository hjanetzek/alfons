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

    InputSource(const std::string& _uri) : m_uri(_uri) {}
    InputSource(std::vector<unsigned char> _data) : m_buffer(std::move(_data)) {}
    InputSource(unsigned char* data, size_t len) : m_buffer(data, data + len) {}

    const std::string& uri() const { return m_uri; }
    const std::vector<unsigned char> buffer() const { return m_buffer; }

    bool isUri() const { return !m_uri.empty(); }

    // TODO - check valid?
    bool operator!() { return true; }

protected:
    std::string m_uri;
    std::vector<unsigned char> m_buffer;
};
}
