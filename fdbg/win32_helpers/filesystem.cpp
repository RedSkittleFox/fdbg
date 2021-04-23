#include <fdbg/nativefiledialog/nfd.h>
#include <fdbg/win32_helpers/filesystem.hpp>

// TODO: Stop using native file dialog!
std::string get_file_path(const std::string& filter_)
{
    std::string ret("");
    nfdchar_t* out = nullptr;
    auto res = NFD_OpenDialog(filter_.c_str(), "C:\\Users\\fox\\source\\repos\\fdbg\\bin\\Debug", &out);

    if (res)
    {
        if (out != nullptr)
            ret = std::string(out);

        free(out);
    }

    return ret;
}

std::string get_folder_path()
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

// Modified https://docs.microsoft.com/en-us/windows/win32/memory/obtaining-a-file-name-from-a-file-handle
std::string get_file_name_from_handle(HANDLE file_)
{
    BOOL res = false;
    TCHAR filename[MAX_PATH + 1];
    HANDLE file_map;

    // Get the file size.
    DWORD file_size_hi = 0;
    DWORD file_size_lo = GetFileSize(file_, &file_size_hi);

    if (file_size_lo == 0 && file_size_hi == 0)
    {
        return std::string();
    }

    // Create a file mapping object.
    file_map = CreateFileMapping(file_,
        NULL,
        PAGE_READONLY,
        0,
        1,
        NULL);

    if (file_map)
    {
        // Create a file mapping to get the file name.
        void* mem = MapViewOfFile(file_map, FILE_MAP_READ, 0, 0, 1);

        if (mem)
        {
            if (GetMappedFileName(GetCurrentProcess(),
                mem,
                filename,
                MAX_PATH))
            {

                // Translate path with device name to drive letters.
                TCHAR temp_str[MAX_PATH];
                temp_str[0] = '\0';

                if (GetLogicalDriveStrings(MAX_PATH - 1, temp_str))
                {
                    TCHAR szName[MAX_PATH];
                    TCHAR drive[3] = TEXT(" :");
                    BOOL found = false;
                    TCHAR* p = temp_str;

                    do
                    {
                        // Copy the drive letter to the template string
                        *drive = *p;

                        // Look up each device name
                        if (QueryDosDevice(drive, szName, MAX_PATH))
                        {
                            size_t name_length = strlen(szName);

                            if (name_length < MAX_PATH)
                            {
                                found = strncmp(filename, szName, name_length) == 0
                                    && *(filename + name_length) == char('\\');

                                if (found)
                                {
                                    // Reconstruct pszFilename using szTempFile
                                    // Replace device path with DOS path
                                    TCHAR temp_file[MAX_PATH];
                                    StringCchPrintf(temp_file,
                                        MAX_PATH,
                                        TEXT("%s%s"),
                                        drive,
                                        filename + name_length);
                                    StringCchCopyN(filename, MAX_PATH + 1, temp_file, strlen(temp_file));
                                }
                            }
                        }

                        // Go to the next NULL character.
                        while (*p++);
                    } while (!found && *p); // end of string
                }
            }
            res = true;
            UnmapViewOfFile(mem);
        }

        CloseHandle(file_map);
    }
    return std::string(filename);
}
