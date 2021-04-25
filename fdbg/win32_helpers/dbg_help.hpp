#ifndef FDBG_WIN32_HELPERS_DBG_HELP_H_
#define FDBG_WIN32_HELPERS_DBG_HELP_H_
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <variant>
#include <unordered_map>

#include <fdbg/win32_helpers/windows.hpp>
#include <fdbg/win32_helpers/filesystem.hpp>
#include <fdbg/win32_helpers/string.hpp>

template<class T, size_t Size>
struct imagehlp_struct : public T
{
	std::array<char, Size> string_data;

	imagehlp_struct() noexcept 
    {
        std::bit_cast<T*>(this)->SizeOfStruct = sizeof(T); 
        std::bit_cast<T*>(this)->MaxNameLength = Size;
    };
    imagehlp_struct(const imagehlp_struct&) = default;
    imagehlp_struct(imagehlp_struct&&) noexcept = default;
    imagehlp_struct& operator=(const imagehlp_struct&) = default;
    imagehlp_struct& operator=(imagehlp_struct&&) noexcept = default;
    ~imagehlp_struct() noexcept = default;
};

template<class T>
struct imagehlp_struct<T, 0> : public T
{
    imagehlp_struct() noexcept
    {
        std::bit_cast<T*>(this)->SizeOfStruct = sizeof(T);
    };
    imagehlp_struct(const imagehlp_struct&) = default;
    imagehlp_struct(imagehlp_struct&&) noexcept = default;
    imagehlp_struct& operator=(const imagehlp_struct&) = default;
    imagehlp_struct& operator=(imagehlp_struct&&) noexcept = default;
    ~imagehlp_struct() noexcept = default;
};

using imagehlp_module64 = imagehlp_struct<IMAGEHLP_MODULE64, 0>;
using imagehlp_symbol64 = imagehlp_struct<IMAGEHLP_SYMBOL64, MAX_SYM_NAME>;
using imagehlp_line64 = imagehlp_struct<IMAGEHLP_LINE64, 0>;

CONTEXT current_context();

enum class imagehlp_symbol_type_fundamental_type
{
    NO_TYPE = 0,
    VOID_T = 1, // Windows.h defines VOID macro
    CHAR = 2,
    WCHAR = 3,
    INT = 6,
    UNSIGNED_INT = 7,
    FLOAT = 8,
    BCD = 9,
    BOOL = 10,
    LONG = 13,
    UNSIGNED_LONG = 14,
    CURRENCY = 25,
    DATE = 26,
    VARIANT = 27,
    COMPLEX = 28,
    BIT = 29,
    CSTR = 30,
    HRESULT = 31,
    CHAR16_T = 32,  // char16_t
    CHAR32_T = 33,  // char32_t
};

enum class imagehlp_symbol_type_udt_kind
{
    STRUCT,
    CLASS,
    UNION,
    INTERFACE
};

enum class imagehlp_symbol_location_type
{
    UNKNOWN,
    STATIC,
    TLS, // Thread Local Storage
    THIS_RELATIVE,
};

enum class imagehlp_symbol_data_kind
{
    UNKNOWN,
    LOCAL,
    STATIC_LOCAL,
    PARAM,
    OBJECT_PTR,
    FILE_STATIC,
    GLOBAL,
    MEMBER,
    STATIC_MEMBER,
    CONSTANT
};

struct imagehlp_symbol_type; // Forward declaration

struct imagehlp_symbol_variable
{
    std::string name;
    imagehlp_symbol_type* type;
    size_t address;
    imagehlp_symbol_data_kind data_kind;
};

struct imagehlp_symbol_function
{
    std::string name;
    imagehlp_symbol_type* type;
};

struct imagehlp_symbol_type
{
    size_t id;
    std::string name;
    size_t size;

    struct fundamental
    {
        imagehlp_symbol_type_fundamental_type type;
    };
    struct enumeration
    {
        // TODO: Possible values?
    };
    struct array
    {
        imagehlp_symbol_type* type;
        size_t size;
    };
    struct pointer
    {
        imagehlp_symbol_type* type;
    };
    struct reference
    {
        imagehlp_symbol_type* type;
    };
    struct type_def
    {
        imagehlp_symbol_type* type;
    };
    struct function
    {
        imagehlp_symbol_type* parent;
        imagehlp_symbol_type* return_type;
        std::vector<imagehlp_symbol_variable*> parameters;
    };
    struct user_defined
    {
        imagehlp_symbol_type_udt_kind kind;

        std::vector<imagehlp_symbol_variable*> variables;
        std::vector<imagehlp_symbol_function*> functions;
        std::vector<imagehlp_symbol_type*> base_classes;
    };

    std::variant<
        fundamental,
        enumeration,
        array,
        pointer,
        reference,
        type_def,
        function,
        user_defined
    > u;
};

std::vector<DWORD> imagehlp_get_children(DWORD id_, DWORD64 base_);
std::string imagehlp_get_name(DWORD id_, DWORD64 base_);
std::string imagehlp_get_fundamental_name(imagehlp_symbol_type_fundamental_type type_, size_t size_);

class imagehlp_symbol_manager
{
private:
    std::unordered_map<DWORD, imagehlp_symbol_type> m_types;
    std::unordered_map<DWORD, imagehlp_symbol_variable> m_variables;
    std::unordered_map<DWORD, imagehlp_symbol_function> m_functions;

public:
    static imagehlp_symbol_manager& instance();

private:
    imagehlp_symbol_type* register_type(DWORD id_, DWORD64 base_ = 0);
    imagehlp_symbol_variable* register_variable(DWORD id_, DWORD64 base_ = 0);
    imagehlp_symbol_function* register_function(DWORD id_, DWORD64 base_ = 0);

public:
    imagehlp_symbol_type* type(DWORD id_, DWORD64 base_ = 0);
    imagehlp_symbol_variable* varaible(DWORD id_, DWORD64 base_ = 0);
    imagehlp_symbol_function* function(DWORD id_, DWORD64 base_ = 0);
};

HANDLE dbg_process();
HANDLE dbg_thread();

#endif