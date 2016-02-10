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
#include <memory>

namespace alfons {

class InputSource {
public:

    InputSource(const std::string& _uri)
        : m_uri(_uri) {}

    InputSource(std::vector<char> _data)
        : m_buffer(std::make_shared<std::vector<char>>(std::move(_data))) {}

    InputSource(const char* data, size_t len)
        : m_buffer(std::make_shared<std::vector<char>>(data, data + len)) {}

    const std::string& uri() const { return m_uri; }
    const auto buffer() const { return m_buffer; }

    bool isUri() const { return !m_uri.empty(); }

    // TODO - check valid?
    bool operator!() { return true; }

protected:
    std::string m_uri;
    std::shared_ptr<std::vector<char>> m_buffer;
};
}
