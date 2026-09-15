#pragma once

#include "row.h"
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <algorithm>

class Table
{
public:
    Table() = default;

    bool insert(int id, std::string name)
    {
        std::lock_guard<std::shared_mutex> lock(m_mutex);
        if (m_data.count(id))
        {
            return false;
        }
        m_data.emplace(id, Row(id, std::move(name)));
        return true;
    }

    void truncate()
    {
        std::lock_guard<std::shared_mutex> lock(m_mutex);
        m_data.clear();
    }

    std::vector<std::pair<int, std::string>> getAllData() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        std::vector<std::pair<int, std::string>> result;
        result.reserve(m_data.size());
        for (const auto& pair : m_data)
        {
            result.push_back({ pair.first, pair.second.name });
        }
        return result;
    }

    bool isEmpty() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_data.empty();
    }

    size_t size() const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_data.size();
    }

    const Row* getRow(int id) const
    {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_data.find(id);
        if (it != m_data.end())
        {
            return &it->second;
        }
        return nullptr;
    }

private:
    std::map<int, Row> m_data;
    mutable std::shared_mutex m_mutex;
};