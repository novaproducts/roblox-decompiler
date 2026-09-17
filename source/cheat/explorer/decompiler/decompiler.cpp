#include "decompiler.h"
#include "../bytecode/bytecode.hpp"
#include "../bytecode/luau_disasm.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
#include "../../../globals/globals.hpp"
#include "../../../utils/memory/memory.h"
#include "../../../utils/output/output.h"
#include "../../offsets.hpp"

namespace
{
    constexpr const char* lunaux_url = "https://lunaux-decompiler.dev/api/decompile";
    constexpr const char* konstant_url = "https://api.plusgiant5.com/konstant/decompile";
    constexpr const char* bytefall_url = "https://decompiler.bytefall.dev/decompile";

    constexpr std::uint64_t min_user_ptr = 0x10000ull;
    constexpr std::uint64_t max_user_ptr = 0x00007FF000000000ull;
    constexpr std::size_t min_blob_bytes = 8;
    constexpr std::size_t max_blob_bytes = 64ull * 1024 * 1024;

    std::optional<std::string> decode_blob(const std::vector<char>& bytes)
    {
        if (bytes.size() < min_blob_bytes)
            return std::nullopt;
        const std::string out = Bytecode::decompress(std::string(bytes.data(), bytes.size()));
        if (!out.empty())
            return out;
        if (bytes[0] == 'R' && bytes[1] == 'S' && bytes[2] == 'B' && bytes[3] == '1') {
            const std::uint32_t size = static_cast<std::uint32_t>(bytes[4])
                | (static_cast<std::uint32_t>(bytes[5]) << 8)
                | (static_cast<std::uint32_t>(bytes[6]) << 16)
                | (static_cast<std::uint32_t>(bytes[7]) << 24);
            if (size >= min_blob_bytes && size < (1u << 27) && bytes.size() > min_blob_bytes) {
                std::string data(size, '\0');
                const std::size_t written = ZSTD_decompress(data.data(), size, bytes.data() + 8, bytes.size() - 8);
                if (!ZSTD_isError(written) && written == size)
                    return data;
            }
        }
        return std::nullopt;
    }

    class winhttp_handle final
    {
    public:
        explicit winhttp_handle(HINTERNET value = nullptr) : value(value) {}
        ~winhttp_handle() { reset(); }
        winhttp_handle(winhttp_handle&& other) noexcept : value(other.value) { other.value = nullptr; }
        winhttp_handle& operator=(winhttp_handle&& other) noexcept
        {
            if (this != &other) {
                reset();
                value = other.value;
                other.value = nullptr;
            }
            return *this;
        }
        winhttp_handle(const winhttp_handle&) = delete;
        winhttp_handle& operator=(const winhttp_handle&) = delete;
        HINTERNET get() const { return value; }
        explicit operator bool() const { return value != nullptr; }
    private:
        void reset()
        {
            if (value) {
                WinHttpCloseHandle(value);
                value = nullptr;
            }
        }
        HINTERNET value = nullptr;
    };
}

bool decompiler::decompiler_t::decompile_script(std::uint64_t script, script_type type)
{
    const std::optional<std::string> bytecode = decompress_script(script, type);
    if (!bytecode)
        return false;
    std::string source = fetch_source(*bytecode);
    if (source.empty())
        return false;
    if (source.back() != '\n')
        source.push_back('\n');
    decompiled_script_t entry;
    entry.title = "Script_" + std::to_string(script);
    entry.code = source;
    entry.editor.SetText(source);
    globals::services::decompiled_scripts.push_back(std::move(entry));
    return true;
}

bool decompiler::decompiler_t::disassemble_script(std::uint64_t script, script_type type)
{
    const std::optional<std::string> bytecode = decompress_script(script, type);
    if (!bytecode)
        return false;
    std::string listing = luau_deasm::disassemble(*bytecode);
    if (listing.empty())
        return false;
    const bool failed = listing.rfind("!! disassemble failed", 0) == 0;
    decompiled_script_t entry;
    entry.title = failed ? "DisasmFail_" + std::to_string(script) : "Disasm_" + std::to_string(script);
    entry.code = listing;
    entry.editor.SetText(listing);
    globals::services::disassembled_scripts.push_back(std::move(entry));
    return !failed;
}

std::optional<std::string> decompiler::decompiler_t::decompress_script(std::uint64_t script, script_type type)
{
    const std::uint64_t bytecode_offset = (type == ModuleScript) ? Offsets::ModuleScript::ByteCode
        : (type == Script) ? Offsets::Script::ByteCode : Offsets::LocalScript::ByteCode;
    const std::uint64_t container = memory->read<std::uint64_t>(script + bytecode_offset);
    if (!(container > min_user_ptr && container < max_user_ptr)) {
        g_logger.error("[decompress] invalid bytecode container=%llX", static_cast<unsigned long long>(container));
        return std::nullopt;
    }
    auto read_exact = [](std::uint64_t address, std::size_t size) -> std::vector<char> {
        if (!(address > min_user_ptr && address < max_user_ptr) || size < min_blob_bytes || size > max_blob_bytes)
            return {};
        std::vector<char> buffer(size);
        if (Luck_ReadVirtualMemory(memory->get_process_handle(), reinterpret_cast<void*>(address), buffer.data(), static_cast<ULONG>(buffer.size()), nullptr) != 0)
            return buffer;
        std::vector<std::uint64_t> words((buffer.size() + 7) / 8);
        for (std::size_t i = 0; i < words.size(); i++)
            words[i] = memory->read<std::uint64_t>(address + static_cast<std::uint64_t>(i) * 8);
        std::memcpy(buffer.data(), words.data(), buffer.size());
        return buffer;
    };
    std::vector<std::uint64_t> window(0x400 / 8);
    const std::vector<char> window_data = read_exact(container, window.size() * 8);
    if (window_data.empty())
        return std::nullopt;
    std::memcpy(window.data(), window_data.data(), window_data.size());
    std::string found;
    auto try_pair = [&](std::uint64_t address) {
        if (!found.empty() || !(address > min_user_ptr && address < max_user_ptr))
            return;
        const std::uint64_t pointer = memory->read<std::uint64_t>(address);
        if (!(pointer > min_user_ptr && pointer < max_user_ptr))
            return;
        const std::uint64_t raw_size = memory->read<std::uint64_t>(address + 0x10);
        const std::size_t size = raw_size > 0xFFFFFFFFull ? static_cast<std::uint32_t>(raw_size) : static_cast<std::size_t>(raw_size);
        if (size < min_blob_bytes || size > max_blob_bytes)
            return;
        const std::optional<std::string> decoded = decode_blob(read_exact(pointer, size));
        if (decoded) {
            g_logger.info("[decompress] found=%zu bytes at=%llX", decoded->size(), static_cast<unsigned long long>(pointer));
            found = *decoded;
        }
    };
    auto scan_pointer = [&](std::uint64_t value) {
        if (!found.empty() || !(value > min_user_ptr && value < max_user_ptr))
            return;
        try_pair(value + 0x18);
        try_pair(value + 0x10);
    };
    for (std::size_t i = 0; i < window.size() && found.empty(); i++) {
        if (window[i] > min_user_ptr && window[i] < max_user_ptr)
            try_pair(container + static_cast<std::uint64_t>(i) * 8);
    }
    for (std::size_t i = 0; i < window.size() && found.empty(); i++) {
        if (window[i] > min_user_ptr && window[i] < max_user_ptr)
            scan_pointer(window[i]);
    }
    if (type == Script) {
        for (std::uint64_t offset = 0x100; offset < 0x280 && found.empty(); offset += 8) {
            const std::uint64_t pointer = memory->read<std::uint64_t>(script + offset);
            if (!(pointer > min_user_ptr && pointer < max_user_ptr))
                continue;
            try_pair(script + offset);
            scan_pointer(pointer);
        }
    }
    if (found.empty()) {
        g_logger.error("[decompress] no valid bytecode for=%llX", static_cast<unsigned long long>(script));
        return std::nullopt;
    }
    return found;
}

std::string decompiler::decompiler_t::fetch_source(const std::string& bytecode)
{
    const backend selected = globals::decompiler_settings::selected;
    const char* endpoint = selected == backend::konstant ? konstant_url : lunaux_url;
    const std::string encoded = to_base64(bytecode.data(), bytecode.size());
    std::string source;
    if (selected == backend::konstant) {
        source = post(endpoint, bytecode.data(), bytecode.size(), L"Content-Type: text/plain\r\n");
        if (!source.empty())
            source = strip_lines(source, 6);
    }
    else {
        const std::string body = "{\"bytecode\":\"" + encoded + "\"}";
        source = post(endpoint, body.data(), body.size(), L"Content-Type: application/json\r\n");
    }
    if (source.empty() || source.find_first_not_of(" \t\r\n") == std::string::npos || source[source.find_first_not_of(" \t\r\n")] == '{') {
        g_logger.warning("[decompile] bad response from %s, falling back to ByteFall", endpoint);
        const std::string body = "{\"script\":\"" + encoded + "\"}";
        source = post(bytefall_url, body.data(), body.size(), L"Content-Type: application/json\r\n");
        if (!source.empty())
            source = strip_lines(source, 2);
    }
    return source;
}

std::string decompiler::decompiler_t::strip_lines(const std::string& text, int count)
{
    std::istringstream stream(text);
    std::string line;
    std::string output;
    int skipped = 0;
    while (std::getline(stream, line)) {
        if (skipped < count) {
            skipped++;
            continue;
        }
        output += line;
        output.push_back('\n');
    }
    return output;
}

std::string decompiler::decompiler_t::to_base64(const void* data, std::size_t size)
{
    static constexpr char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const auto* bytes = static_cast<const unsigned char*>(data);
    std::string output;
    output.reserve(((size + 2) / 3) * 4);
    std::size_t i = 0;
    while (i + 2 < size) {
        const unsigned int n = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2];
        output.push_back(table[(n >> 18) & 63]);
        output.push_back(table[(n >> 12) & 63]);
        output.push_back(table[(n >> 6) & 63]);
        output.push_back(table[n & 63]);
        i += 3;
    }
    if (i < size) {
        unsigned int n = bytes[i] << 16;
        output.push_back(table[(n >> 18) & 63]);
        if (i + 1 < size) {
            n |= bytes[i + 1] << 8;
            output.push_back(table[(n >> 12) & 63]);
            output.push_back(table[(n >> 6) & 63]);
            output.push_back('=');
        }
        else {
            output.push_back(table[(n >> 12) & 63]);
            output.push_back('=');
            output.push_back('=');
        }
    }
    return output;
}

std::string decompiler::decompiler_t::post(const char* endpoint, const void* data, std::size_t size, const wchar_t* content_type)
{
    URL_COMPONENTS url{};
    url.dwStructSize = sizeof(url);
    wchar_t host[256]{};
    wchar_t path[1024]{};
    wchar_t extra[1024]{};
    url.lpszHostName = host;
    url.dwHostNameLength = _countof(host);
    url.lpszUrlPath = path;
    url.dwUrlPathLength = _countof(path);
    url.lpszExtraInfo = extra;
    url.dwExtraInfoLength = _countof(extra);
    const std::wstring wide_endpoint(endpoint, endpoint + std::strlen(endpoint));
    if (!WinHttpCrackUrl(wide_endpoint.c_str(), 0, 0, &url)) {
        g_logger.error("[api] WinHttpCrackUrl failed err=%lu", GetLastError());
        return {};
    }
    const auto fail = [](const char* stage) -> std::string {
        g_logger.error("[api] %s failed err=%lu", stage, GetLastError());
        return {};
    };
    winhttp_handle session(WinHttpOpen(L"decompiler/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session)
        return fail("WinHttpOpen");
    DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
    WinHttpSetOption(session.get(), WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols));
    WinHttpSetTimeouts(session.get(), 60000, 60000, 60000, 60000);
    const std::wstring host_name(url.lpszHostName, url.dwHostNameLength);
    winhttp_handle connection(WinHttpConnect(session.get(), host_name.c_str(), url.nPort, 0));
    if (!connection)
        return fail("WinHttpConnect");
    std::wstring url_path(url.lpszUrlPath, url.dwUrlPathLength);
    if (url.dwExtraInfoLength > 0)
        url_path.append(url.lpszExtraInfo, url.dwExtraInfoLength);
    const DWORD flags = url.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
    winhttp_handle request(WinHttpOpenRequest(connection.get(), L"POST", url_path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags));
    if (!request)
        return fail("WinHttpOpenRequest");
    if (!WinHttpSendRequest(request.get(), content_type, static_cast<DWORD>(-1), const_cast<LPVOID>(data), static_cast<DWORD>(size), static_cast<DWORD>(size), 0))
        return fail("WinHttpSendRequest");
    if (!WinHttpReceiveResponse(request.get(), nullptr))
        return fail("WinHttpReceiveResponse");
    DWORD status = 0;
    DWORD status_bytes = sizeof(status);
    WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_bytes, WINHTTP_NO_HEADER_INDEX);
    std::string body;
    DWORD available = 0;
    do {
        if (!WinHttpQueryDataAvailable(request.get(), &available) || available == 0)
            break;
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request.get(), chunk.data(), available, &read))
            break;
        body.append(chunk.data(), read);
    } while (available > 0);
    if (status != 0 && status != 200) {
        g_logger.error("[api] status=%lu body=%.200s", status, body.c_str());
        return {};
    }
    return body;
}