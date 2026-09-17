#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace decompiler
{
    enum script_type { LocalScript = 0, ModuleScript = 1, Script = 2 };

    enum class backend { lunaux = 0, konstant = 1 };

    class decompiler_t final
    {
    public:
        bool decompile_script(std::uint64_t script, script_type type);
        bool disassemble_script(std::uint64_t script, script_type type);

    private:
        std::optional<std::string> decompress_script(std::uint64_t script, script_type type);
        std::string fetch_source(const std::string& bytecode);
        std::string post(const char* endpoint, const void* data, std::size_t size, const wchar_t* content_type);
        static std::string to_base64(const void* data, std::size_t size);
        static std::string strip_lines(const std::string& text, int count);
    };
}