#include <algorithm>

#include <fdbg/win32_helpers/windows.hpp>

std::string get_process_name(DWORD pid_)
{
    static std::array<char, MAX_PATH> process_name;

    HANDLE hp = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false,
        pid_);

    if (hp == 0)
        return std::string();

    HMODULE hm;
    DWORD cbn;

    if (!EnumProcessModules(hp, &hm, sizeof(hm), &cbn))
        return std::string();

    GetModuleBaseName(hp, hm, process_name.data(), process_name.size() / sizeof(char));
    CloseHandle(hp);

    return std::string(process_name.data());
}

std::vector<std::pair<DWORD, std::string>> enum_proceseses()
{
    static constexpr size_t enum_count = 2048;
    std::vector<DWORD> data(enum_count);

    DWORD read_size;
    if (!EnumProcesses(data.data(), enum_count, &read_size))
    {
        // Something went wrong. Report this?
        return std::vector<std::pair<DWORD, std::string>>();
    }

    data.resize(read_size < enum_count ? read_size : enum_count);

    {
        std::vector<std::pair<DWORD, std::string>> ret(data.size());
        std::transform(std::begin(data), std::end(data), std::begin(ret),
            [](DWORD pid__) -> std::pair<DWORD, std::string> { return { pid__, get_process_name(pid__) }; });

        std::erase_if(ret, [](const auto& e__)
            {
                return (e__.first == 0 || e__.second == std::string());
            });

        ret.shrink_to_fit();
        std::sort(std::begin(ret), std::end(ret), [](const auto& lhs__, const auto& rhs__) {return lhs__.second < rhs__.second; });

        return ret;
    }
}
