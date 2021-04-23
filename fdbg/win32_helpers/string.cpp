#include <fdbg/win32_helpers/string.hpp>

size_t wstr_to_str(char* dest_, const wchar_t* src_, size_t dest_len_)
{
    size_t i;
    wchar_t code;

    i = 0;

    while (src_[i] != '\0' && i < (dest_len_ /*- 1*/)) {
        code = src_[i];
        if (code < 128)
            dest_[i] = char(code);
        else {
            dest_[i] = '?';
            if (code >= 0xD800 && code <= 0xD8FF)
                // lead surrogate, skip the next code unit, which is the trail
                i++;
        }
        i++;
    }

    /* dest_[i] = '\0'; */

    return i - 1;
}

std::string to_string_from_wstring(const std::wstring& str_)
{
    std::string ret(str_.size(), ' ');
    wstr_to_str(ret.data(), str_.c_str(), ret.size());
    return ret;
}
