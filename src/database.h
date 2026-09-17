#pragma once

#include "table.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>
#include <mutex>

class Database
{
public:
    Database() = default;

    bool insert(const std::string& tableName, int id, std::string name)
    {
        if (tableName == "A")
        {
            return m_tableA.insert(id, std::move(name));
        }
        else if (tableName == "B")
        {
            return m_tableB.insert(id, std::move(name));
        }
        return false;
    }

    void truncate(const std::string& tableName)
    {
        if (tableName == "A")
        {
            m_tableA.truncate();
        }
        else if (tableName == "B")
        {
            m_tableB.truncate();
        }
    }

    std::vector<std::string> intersect() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto dataA = m_tableA.getAllData();
        auto dataB = m_tableB.getAllData();

        std::vector<std::string> result;
        result.reserve(std::min(dataA.size(), dataB.size()));

        auto itA = dataA.begin();
        auto itB = dataB.begin();

        while (itA != dataA.end() && itB != dataB.end())
        {
            if (itA->first < itB->first)
            {
                ++itA;
            }
            else if (itA->first > itB->first)
            {
                ++itB;
            }
            else
            {
                result.push_back(std::to_string(itA->first) + "," + itA->second + "," + itB->second);
                ++itA;
                ++itB;
            }
        }
        return result;
    }

    std::vector<std::string> symmetricDifference() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        const auto& dataA = m_tableA.getAllData();
        const auto& dataB = m_tableB.getAllData();

        std::vector<std::string> result;
        result.reserve(dataA.size() + dataB.size());

        auto itA = dataA.begin();
        auto itB = dataB.begin();
        auto endA = dataA.end();
        auto endB = dataB.end();

        while (itA != endA || itB != endB)
        {
            if (itA == endA)
            {
                result.push_back(std::to_string(itB->first) + ",," + itB->second);
                ++itB;
            }
            else if (itB == endB)
            {
                result.push_back(std::to_string(itA->first) + "," + itA->second + ",");
                ++itA;
            }
            else
            {
                if (itA->first < itB->first) 
                {
                    result.push_back(std::to_string(itA->first) + "," + itA->second + ",");
                    ++itA;
                }
                else if (itB->first < itA->first) 
                {
                    result.push_back(std::to_string(itB->first) + ",," + itB->second);
                    ++itB;
                }
                else 
                {
                    ++itA;
                    ++itB;
                }
            }
        }
        return result;
    }

private:
    Table m_tableA;
    Table m_tableB;
    mutable std::mutex m_mutex;
};