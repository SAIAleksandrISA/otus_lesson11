#pragma once

#include "table.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <utility>

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
        return false; // Неизвестная таблица
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

    std::vector<std::string> symmetric_difference() const
    {
        auto dataA = m_tableA.getAllData();
        auto dataB = m_tableB.getAllData();

        std::vector<std::string> result;
        result.reserve(dataA.size() + dataB.size());

        auto itA = dataA.begin();
        auto itB = dataB.begin();

        while (itA != dataA.end() || itB != dataB.end())
        {
            int idA = (itA != dataA.end()) ? itA->first : -1;
            int idB = (itB != dataB.end()) ? itB->first : -1;

            if (idA != -1 && (idB == -1 || idA < idB))
            {
                result.push_back(std::to_string(idA) + "," + itA->second + ",");
                ++itA;
            }
            else if (idB != -1 && (idA == -1 || idB < idA))
            {
                result.push_back(std::to_string(idB) + ",," + itB->second);
                ++itB;
            }
            else if (idA != -1 && idB != -1 && idA == idB)
            {
                ++itA;
                ++itB;
            }
            else
            {
                if (idA == -1 && idB == -1) break;
            }
        }
        return result;
    }

private:
    Table m_tableA;
    Table m_tableB;
};