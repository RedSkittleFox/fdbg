#pragma once
#ifndef FDBG_DBG_SOURCE_VIEW_H_
#define FDBG_DBG_SOURCE_VIEW_H_

#include <future>
#include <fstream>
#include <unordered_map>
#include <vector>

class source_view
{
    bool m_enabled;
    // Precached files  - file name - lines of code -
    std::unordered_map<std::string, std::vector<std::string>> m_files;
public:
    static source_view& instance();

public:
    void update();
};

#endif