#pragma once
#ifndef FDBG_FILEMIN_H_
#define FDBG_FILEMIN_H_

#include <string>
#include <fdbg/nativefiledialog/nfd.h>

inline std::string get_file_path(const std::string& filter_)
{
    std::string ret("");
    nfdchar_t* out = nullptr;
    auto res = NFD_OpenDialog(filter_.c_str(), "C:\\Users\\fox\\source\\repos\\fdbg\\bin\\Debug", &out);

    if (res)
    {
        if(out != nullptr)
            ret = std::string(out);

        free(out);
    }
    
    return ret;
}

inline std::string get_folder_path()
{
    std::string ret("");
    nfdchar_t* out = nullptr;
    auto res = NFD_PickFolder("C:\\Users\\fox\\source\\repos\\fdbg\\bin\\Debug", &out);

    if (res)
    {
        if (out != nullptr)
            ret = std::string(out);
        free(out);
    }

    return ret;
}

#endif