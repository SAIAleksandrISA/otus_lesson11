#pragma once

#include <string>
#include <utility>

struct Row
{
    int id;
    std::string name;

    Row(int id, std::string name) : id(id), name(std::move(name))
    {
    }

    Row() : id(-1)
    {
    }
};