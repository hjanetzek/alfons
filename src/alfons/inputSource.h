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
#include <vector>
#include <functional>

namespace alfons {

using LoadSourceHandle = std::function<std::vector<char>()>;

class InputSource {
public:

    InputSource() {}

    InputSource(const std::string& _uri)
        : m_uri(_uri) {}

    InputSource(LoadSourceHandle _loadSource)
        : m_loadSource(_loadSource) {}

    InputSource(std::vector<char> _data)
        : m_buffer(std::make_shared<std::vector<char>>(std::move(_data))) {}

    InputSource(const char* data, size_t len)
        : m_buffer(std::make_shared<std::vector<char>>(data, data + len)) {}

    const std::string& uri() const { return m_uri; }
    const auto buffer() const { return m_buffer; }

    bool isUri() const { return !m_uri.empty(); }

    bool hasSourceCallback() { return bool(m_loadSource); }

    bool resolveSource() {
        if (m_buffer) {
            return true;
        }

        auto buffer = m_loadSource();

        if (buffer.size() == 0) {
            return false;
        }

        m_buffer = std::make_shared<std::vector<char>>(std::move(buffer));
        return true;
    }

    bool isValid() {
        return (m_buffer && !m_buffer->empty()) || !m_uri.empty() || bool(m_loadSource);
    }

protected:
    std::string m_uri = "";
    std::shared_ptr<std::vector<char>> m_buffer;
    LoadSourceHandle m_loadSource = nullptr;
};
}
