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
        : m_uri(_uri), m_data(std::make_shared<Data>()) {}

    InputSource(LoadSourceHandle _loadSource)
        : m_data(std::make_shared<Data>(_loadSource)) {}

    InputSource(const std::vector<char>& _data)
        : m_data(std::make_shared<Data>(_data)) {}

    InputSource(std::vector<char>&& _data)
        : m_data(std::make_shared<Data>(std::move(_data))) {}

    InputSource(const char* data, size_t len)
        : m_data(std::make_shared<Data>(std::vector<char>{data, data + len})) {}

    const std::string& uri() const { return m_uri; }

    const std::vector<char>& buffer() const {
        return m_data->buffer;
    }

    bool isUri() const { return !m_uri.empty(); }

    bool hasSourceCallback() { return m_data && bool(m_data->loadSource); }

    bool resolveSource() {
        if (!m_data || !bool(m_data->loadSource)) {
            return false;
        }

        if (!m_data->buffer.empty()) {
            return true;
        }

        m_data->buffer = m_data->loadSource();

        if (m_data->buffer.empty()) {
            return false;
        }

        return true;
    }

    bool isValid() {
        if (!m_uri.empty())  { return true; }

        if (m_data) {
            if (!m_data->buffer.empty()) { return true; }


            if (resolveSource()) {
                return true;
            }
        }
        return false;
    }

protected:
    std::string m_uri = "";

    struct Data {
        Data() {}
        Data(std::vector<char> buffer) : buffer(buffer), loadSource(nullptr) {}
        Data(LoadSourceHandle source) : buffer(), loadSource(source) {}

        std::vector<char> buffer;
        LoadSourceHandle loadSource;
    };

    std::shared_ptr<Data> m_data;
};
}
