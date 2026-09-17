#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <dwmapi.h>
#include <cstdio>
#include <chrono>
#include <thread>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <tuple>
#include <array>
#include <vector>
#include <d3d11.h>
#include <commdlg.h>
#include <shlwapi.h>
#include "overlay.hpp"
#include "../../../../libraries/dear-imgui/imgui.h"
#include "../../../../libraries/dear-imgui/imgui_internal.h"
#include "../../globals/globals.hpp"
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <freetype/imgui_freetype.h>
#include <cmath>
#include "menu/fonts/fonts.hpp"
#include "menu/class_icons.hpp"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "../../../libraries/dear-imgui/stb_image.h"
#include "../offsets.hpp"
#include "../../utils/memory/memory.h"
#include "../../utils/output/output.h"

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")

#pragma comment(lib, "d3d11.lib")
using namespace ImGui;

static ID3D11ShaderResourceView* create_srv_from_png(ID3D11Device* device, const std::uint8_t* png_data, std::uint32_t png_size) {
    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(png_data), static_cast<int>(png_size), &width, &height, &channels, 4);
    if (!pixels)
        return nullptr;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initial_data{};
    initial_data.pSysMem = pixels;
    initial_data.SysMemPitch = static_cast<UINT>(width * 4);

    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* view = nullptr;
    if (SUCCEEDED(device->CreateTexture2D(&desc, &initial_data, &texture)))
        device->CreateShaderResourceView(texture, nullptr, &view);
    if (texture)
        texture->Release();
    stbi_image_free(pixels);
    return view;
}

static std::vector<ID3D11ShaderResourceView*> g_class_icon_srvs;

static const std::uint8_t k_luaapp_icon_png[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7A, 0x7A, 0xF4, 0x00, 0x00, 0x05,
    0x04, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0xCD, 0x97, 0x5D, 0x4C, 0x5B,
    0x65, 0x1C, 0xC6, 0xC9, 0x64, 0xC0, 0x6C, 0xCB, 0xF7, 0x00, 0x61, 0x50,
    0x4A, 0xA1, 0xA5, 0x80, 0x74, 0x2B, 0x52, 0xBE, 0x0A, 0xA5, 0x14, 0x4A,
    0xF9, 0x5A, 0xE9, 0x98, 0x63, 0x03, 0x46, 0xCB, 0xE7, 0x18, 0xA2, 0x35,
    0x31, 0x71, 0x31, 0xC6, 0x54, 0xAF, 0x9C, 0x99, 0x86, 0x79, 0xBD, 0x28,
    0xD1, 0x0B, 0x8D, 0x66, 0x86, 0x98, 0x18, 0x13, 0xD8, 0x66, 0x07, 0xCA,
    0x00, 0xF9, 0x72, 0x82, 0x8C, 0xEF, 0xF2, 0xB1, 0xC9, 0xE2, 0x85, 0x5C,
    0xEE, 0xF2, 0x6F, 0xFE, 0xEF, 0x7B, 0xCE, 0xE9, 0x29, 0xF4, 0x88, 0x01,
    0x89, 0xBE, 0xC9, 0x93, 0x96, 0xDE, 0xBC, 0xBF, 0xF7, 0x79, 0x9E, 0xFF,
    0x7B, 0x0E, 0x7E, 0x7E, 0xFF, 0xE7, 0x15, 0x20, 0x49, 0x56, 0x87, 0xCB,
    0xED, 0xCE, 0x13, 0x61, 0xA7, 0xF5, 0xFF, 0x09, 0x80, 0x4C, 0xFF, 0xB5,
    0x5B, 0x61, 0x1E, 0x06, 0x54, 0x4A, 0xF9, 0x30, 0x9C, 0xCA, 0xFE, 0xD8,
    0x15, 0x2A, 0x3D, 0xEF, 0x40, 0xB0, 0x23, 0xDF, 0x3C, 0x50, 0x92, 0xAC,
    0x4E, 0xAD, 0xFC, 0x09, 0x50, 0xCA, 0x0A, 0x56, 0x3F, 0x82, 0xC2, 0x8C,
    0x1A, 0x06, 0xB9, 0xF1, 0xFB, 0x9D, 0xE8, 0x8C, 0xB7, 0xFA, 0x24, 0xB1,
    0x66, 0x9B, 0x7F, 0x50, 0x8C, 0xF4, 0x5F, 0x07, 0x08, 0x97, 0x5D, 0x70,
    0xA4, 0x55, 0x3D, 0x00, 0x15, 0xA7, 0x11, 0x50, 0x55, 0x8E, 0x40, 0x2A,
    0x11, 0x05, 0x62, 0x61, 0x52, 0xCA, 0x87, 0x20, 0xB1, 0xF0, 0x2B, 0x77,
    0xA4, 0xB2, 0xA7, 0x57, 0x74, 0x52, 0x67, 0x39, 0xE6, 0x2F, 0x0A, 0x39,
    0x34, 0x40, 0x42, 0xF6, 0x07, 0xFD, 0xE9, 0x35, 0x63, 0x90, 0x5E, 0x33,
    0xCA, 0x29, 0xAD, 0x1A, 0xE5, 0x01, 0xF2, 0x05, 0x92, 0x6C, 0xBA, 0x0F,
    0xF2, 0x32, 0x17, 0xC4, 0xE5, 0xDC, 0x9A, 0x09, 0x95, 0xD9, 0x9C, 0x41,
    0xA1, 0xEA, 0x83, 0xF5, 0xE7, 0x45, 0xCB, 0x38, 0xF0, 0x95, 0x71, 0x16,
    0x35, 0x06, 0x19, 0x0C, 0x94, 0x07, 0xC6, 0x1B, 0x04, 0xBB, 0x92, 0x6C,
    0x1A, 0x02, 0x79, 0xD9, 0x7D, 0x48, 0x2A, 0x75, 0x81, 0xCC, 0x78, 0x0F,
    0xA4, 0x25, 0x77, 0xE1, 0x64, 0xC6, 0x7B, 0xFD, 0xE2, 0x38, 0xAB, 0xE3,
    0xB8, 0x28, 0x69, 0xFF, 0xFE, 0x88, 0x22, 0x35, 0x7A, 0xB5, 0x75, 0x02,
    0x38, 0xD5, 0x4E, 0x40, 0x66, 0xED, 0xCF, 0x44, 0x1E, 0x20, 0x04, 0x19,
    0xF3, 0x01, 0x42, 0xDD, 0xF0, 0x86, 0xF8, 0x81, 0x40, 0x24, 0x14, 0xDF,
    0x81, 0x78, 0xFD, 0x20, 0xC4, 0x68, 0x3F, 0x73, 0xFF, 0x2D, 0x48, 0x8C,
    0xAA, 0xD3, 0x79, 0xA6, 0x6E, 0x0A, 0x88, 0xCE, 0x4D, 0xC2, 0x69, 0x56,
    0xD6, 0x49, 0x02, 0xC4, 0x87, 0xA1, 0x20, 0x7B, 0x21, 0xAE, 0x7D, 0xB8,
    0x08, 0x6F, 0xDE, 0x58, 0x00, 0x7D, 0xD3, 0x98, 0xC7, 0x09, 0x03, 0x85,
    0x38, 0xA5, 0x1F, 0x00, 0x49, 0x42, 0x93, 0x53, 0x10, 0x40, 0x55, 0xFA,
    0xE5, 0x4C, 0xD6, 0xF9, 0x69, 0x40, 0x69, 0x58, 0xD5, 0x4D, 0x33, 0x40,
    0x53, 0x04, 0x86, 0x82, 0x4C, 0x40, 0xA6, 0x85, 0x07, 0x52, 0x3D, 0x4A,
    0xFA, 0x81, 0x45, 0x7D, 0xFC, 0xF4, 0x19, 0xB0, 0x6B, 0x6B, 0xFB, 0x19,
    0x7C, 0xFA, 0xCD, 0x26, 0x24, 0x96, 0x50, 0x88, 0xF8, 0xE2, 0x41, 0x08,
    0x0C, 0xC9, 0xF4, 0xDD, 0x8D, 0xE7, 0x8E, 0x4B, 0x42, 0x5E, 0xBA, 0xF0,
    0x0B, 0xEC, 0x56, 0xD6, 0xCB, 0x33, 0x44, 0x7C, 0x18, 0x0F, 0x88, 0xB7,
    0x1B, 0xA5, 0xAD, 0x53, 0xDC, 0xE6, 0x7C, 0x08, 0x8C, 0x02, 0x21, 0x12,
    0x0C, 0x77, 0x40, 0xF0, 0xF4, 0x61, 0x71, 0x06, 0x8B, 0xF6, 0xE2, 0xAF,
    0xE0, 0xD1, 0x43, 0xD0, 0xD6, 0x3F, 0x84, 0x6C, 0x14, 0x1F, 0x86, 0x80,
    0x30, 0x8E, 0x60, 0x34, 0xB5, 0x13, 0x1C, 0xC4, 0xF5, 0x5B, 0xEE, 0x3D,
    0x00, 0xB7, 0x07, 0xB6, 0xB9, 0x3E, 0x44, 0x9F, 0xF9, 0xC8, 0x25, 0x08,
    0x20, 0xD5, 0x5C, 0xEB, 0xCD, 0xBD, 0x34, 0x0B, 0xB9, 0x0D, 0xB3, 0x40,
    0x3E, 0x2F, 0xCD, 0x42, 0x0E, 0x11, 0x03, 0xC4, 0xC0, 0xB0, 0x20, 0xE8,
    0x08, 0xE7, 0x06, 0x03, 0x71, 0x6F, 0xF4, 0xCF, 0x3D, 0x00, 0x57, 0xDF,
    0x9D, 0xE3, 0x4A, 0x19, 0x1C, 0x5F, 0xE7, 0x10, 0x04, 0xD0, 0xD4, 0x0E,
    0xBA, 0xF3, 0x1B, 0x7F, 0x03, 0x56, 0x79, 0x44, 0x73, 0x90, 0xD7, 0x30,
    0x47, 0xA0, 0x08, 0x0C, 0x0B, 0x82, 0x8E, 0xF8, 0x80, 0xC8, 0x6F, 0x98,
    0x82, 0xB7, 0x6F, 0xAE, 0x42, 0xFF, 0xDD, 0x3F, 0xB8, 0x2E, 0x64, 0xD5,
    0x3D, 0x60, 0x26, 0xC3, 0x05, 0x01, 0x62, 0xB9, 0xEF, 0x09, 0x08, 0x14,
    0xC5, 0x4A, 0x0B, 0x9A, 0xE6, 0xA1, 0xE0, 0xF2, 0x23, 0xD0, 0x31, 0x2A,
    0xB8, 0x3C, 0x0F, 0xF8, 0x5B, 0x3E, 0x0A, 0x61, 0x78, 0x20, 0xE8, 0x08,
    0x71, 0x83, 0x0F, 0x61, 0x9D, 0xE4, 0x3A, 0xC1, 0x4E, 0x87, 0xB1, 0x65,
    0x92, 0x4C, 0x06, 0xDE, 0x11, 0x32, 0xC3, 0x77, 0x3B, 0x82, 0xA7, 0x8F,
    0x92, 0x5B, 0x6D, 0x85, 0xCD, 0x0B, 0x50, 0x68, 0xE3, 0x09, 0xFF, 0x6E,
    0x5E, 0x00, 0x5D, 0x33, 0xC2, 0x3C, 0xA2, 0x30, 0x0C, 0x88, 0x10, 0x04,
    0x5B, 0x4C, 0xB6, 0x94, 0x38, 0x9E, 0xF4, 0xA2, 0x1A, 0x06, 0x7C, 0x7E,
    0x08, 0x02, 0x28, 0x75, 0xD7, 0xFB, 0x8A, 0xEC, 0x4B, 0xA0, 0x47, 0xB5,
    0x30, 0x9F, 0xF6, 0x25, 0x28, 0xB2, 0x2F, 0x42, 0x91, 0x6D, 0x91, 0x07,
    0x82, 0x8E, 0x30, 0x6E, 0xB0, 0x10, 0x6C, 0x27, 0xEA, 0xA6, 0xBD, 0xFA,
    0x40, 0x2F, 0x2B, 0x3A, 0x9A, 0xE8, 0x42, 0x70, 0xAC, 0xD9, 0x26, 0x08,
    0xA0, 0x6B, 0x9C, 0xDA, 0x29, 0x6E, 0x5D, 0x06, 0x43, 0xEB, 0x0A, 0x4F,
    0xCB, 0x50, 0xDC, 0xB2, 0xCC, 0x01, 0xB1, 0x20, 0xE8, 0x06, 0x0B, 0x41,
    0x9C, 0x60, 0x8B, 0xE9, 0xCB, 0x85, 0x6A, 0x8F, 0x0B, 0xFE, 0x27, 0x04,
    0x9E, 0x9C, 0xE2, 0x70, 0x95, 0xDA, 0xD0, 0xB6, 0x02, 0x25, 0x6D, 0xAB,
    0x50, 0xD2, 0xBE, 0x0A, 0x46, 0x46, 0xF8, 0x1D, 0x7F, 0x43, 0x18, 0x02,
    0xE2, 0x05, 0x41, 0xE3, 0xC8, 0x45, 0x08, 0x26, 0x0A, 0x76, 0x32, 0xBC,
    0x5D, 0xA0, 0x5D, 0x90, 0x1B, 0x6E, 0xBB, 0x05, 0x4F, 0x1F, 0x9F, 0x6E,
    0x77, 0x18, 0xDB, 0xD7, 0xA0, 0xB4, 0x03, 0xE5, 0x86, 0xB2, 0x4E, 0x37,
    0x94, 0xA2, 0x3A, 0xDC, 0x60, 0xEC, 0x58, 0xA3, 0x30, 0x6D, 0x2B, 0x80,
    0x0E, 0xA1, 0x1B, 0x08, 0x41, 0xE2, 0x68, 0x9A, 0x27, 0x53, 0x42, 0xA3,
    0xF0, 0xE1, 0x82, 0x85, 0xBA, 0x80, 0x00, 0xD1, 0x69, 0x8E, 0x5E, 0x41,
    0x00, 0x4D, 0xD5, 0x17, 0x2E, 0xDC, 0xB0, 0xEC, 0xCA, 0x3A, 0x98, 0x50,
    0x5D, 0x1B, 0x60, 0xBA, 0x82, 0x5A, 0x87, 0xB2, 0xCE, 0x75, 0x0A, 0xD2,
    0xBE, 0xE6, 0x81, 0xB0, 0x2F, 0x91, 0x92, 0xE2, 0xA4, 0xF8, 0x74, 0x81,
    0x19, 0x4B, 0x1A, 0xC3, 0x38, 0x89, 0x41, 0x12, 0x5D, 0x68, 0x11, 0x04,
    0xC0, 0x13, 0xE3, 0x66, 0xE5, 0x5D, 0x1B, 0x60, 0xBE, 0xBA, 0xC9, 0xA9,
    0x1C, 0x45, 0x60, 0x58, 0x88, 0x55, 0xC0, 0xA8, 0x30, 0x8E, 0xDD, 0x2E,
    0x60, 0x17, 0xF8, 0x13, 0x41, 0x01, 0x3C, 0x31, 0x1C, 0xF3, 0x17, 0xFB,
    0x7E, 0x51, 0x09, 0x7B, 0x21, 0x47, 0x4F, 0x37, 0xDF, 0x04, 0x73, 0xF7,
    0x16, 0x54, 0x74, 0x3F, 0x86, 0xCA, 0x57, 0xA8, 0x2A, 0xBA, 0xB7, 0x28,
    0x48, 0xD7, 0x06, 0xE3, 0x04, 0xBA, 0xB0, 0xEA, 0x71, 0x81, 0xE9, 0x02,
    0x5E, 0x58, 0xFC, 0x18, 0xC8, 0x48, 0x9E, 0xA3, 0x31, 0x20, 0x80, 0x5C,
    0xFF, 0xF9, 0x8C, 0xE0, 0xE9, 0x93, 0xB2, 0x5E, 0x77, 0xD2, 0x93, 0x6F,
    0x91, 0x4D, 0xAB, 0x7A, 0x9E, 0x40, 0xF5, 0xAB, 0xBF, 0x43, 0x15, 0xAA,
    0xE7, 0x09, 0x01, 0x42, 0x08, 0x8C, 0x04, 0x63, 0x22, 0x2E, 0xB4, 0xAE,
    0xF8, 0x88, 0x81, 0x8E, 0xE4, 0xEE, 0x1E, 0x20, 0x40, 0x94, 0xB2, 0x5D,
    0xF8, 0xF1, 0x2B, 0x89, 0x48, 0x53, 0x17, 0x35, 0x8E, 0xB8, 0xF1, 0xB4,
    0xEC, 0xE6, 0x35, 0xAF, 0x6D, 0x13, 0xE1, 0x77, 0x84, 0x42, 0x38, 0xEC,
    0x05, 0x46, 0x85, 0x5D, 0xC0, 0x18, 0xB8, 0x32, 0x32, 0x23, 0xC9, 0xF5,
    0x80, 0xB9, 0x13, 0xF8, 0x45, 0x14, 0x45, 0x68, 0xF6, 0x7F, 0x35, 0x93,
    0x44, 0xA6, 0xA9, 0x65, 0xEA, 0x76, 0x47, 0x76, 0x55, 0x5F, 0xFF, 0x59,
    0xC7, 0x53, 0x40, 0x51, 0x00, 0x74, 0x61, 0x8B, 0x8B, 0x81, 0x05, 0xE0,
    0x7A, 0xF0, 0x0F, 0x00, 0xFC, 0x0E, 0xB2, 0x22, 0xE2, 0xF2, 0xF4, 0x0A,
    0xED, 0x1B, 0x4E, 0x5D, 0xFD, 0xC0, 0xCC, 0x1E, 0x07, 0x5A, 0x7D, 0x3B,
    0xA0, 0xF5, 0x01, 0x20, 0xD5, 0xDE, 0xE8, 0xF7, 0x3B, 0xEC, 0xF2, 0x0F,
    0x08, 0x0E, 0x89, 0x4A, 0x34, 0x59, 0x14, 0x79, 0xEF, 0xF4, 0xE6, 0xD7,
    0x0F, 0xB9, 0xF9, 0x1D, 0x28, 0xD8, 0xA7, 0x03, 0x91, 0xF2, 0x8B, 0xC2,
    0x8F, 0xDF, 0x83, 0xAE, 0x20, 0x71, 0x9C, 0x34, 0x3A, 0xD9, 0x6A, 0x53,
    0x14, 0xBC, 0xDF, 0x97, 0x53, 0x3F, 0xBE, 0xC3, 0x4D, 0x41, 0x3D, 0x7F,
    0x0A, 0xE8, 0x1B, 0x53, 0x50, 0x48, 0xCA, 0xD1, 0xFF, 0x27, 0xF5, 0x7C,
    0x98, 0x52, 0x1D, 0xA3, 0x6C, 0x72, 0xA4, 0x1A, 0x3E, 0x71, 0xF1, 0xEF,
    0x01, 0x95, 0xE9, 0x5B, 0xE1, 0xEB, 0xD7, 0xEF, 0x08, 0x97, 0x38, 0x32,
    0x4B, 0x8F, 0x6F, 0xD7, 0x41, 0x21, 0x8A, 0xA3, 0x3F, 0xFD, 0x61, 0xD6,
    0x5F, 0x6F, 0x10, 0x82, 0x73, 0xDB, 0x94, 0xA5, 0xF4, 0x00, 0x00, 0x00,
    0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82,
};
static const std::uint32_t k_luaapp_icon_size = sizeof(k_luaapp_icon_png);
static ID3D11ShaderResourceView* g_luaapp_icon_srv = nullptr;

static std::uint16_t find_class_icon_index(const char* class_name) {
    if (!class_name)
        return k_class_icon_fallback;
    std::uint32_t lo = 0;
    std::uint32_t hi = k_class_name_mapping_count;
    while (lo < hi) {
        std::uint32_t mid = lo + (hi - lo) / 2;
        int cmp = _stricmp(class_name, k_class_name_to_icon[mid].class_name);
        if (cmp == 0)
            return k_class_name_to_icon[mid].icon_index;
        if (cmp < 0)
            hi = mid;
        else
            lo = mid + 1;
    }
    return k_class_icon_fallback;
}

static ID3D11ShaderResourceView* find_class_icon(const char* class_name) {
    std::uint16_t idx = find_class_icon_index(class_name);
    if (idx >= g_class_icon_srvs.size())
        return nullptr;
    return g_class_icon_srvs[idx];
}

static bool menu_opened = false;
static float menu_alpha = 0.0f;
static bool last_running_state = false;
const int menu_key = VK_F1;
static ImFont* tahoma_font = nullptr;
static ImFont* console_font = nullptr;
ID3D11ShaderResourceView* background{ };
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    graphics_renderer_t* instance = reinterpret_cast<graphics_renderer_t*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_SIZE:
        if (instance && instance->ctx && instance->ctx->swapchain && wParam != SIZE_MINIMIZED) {
            if (instance->ctx->rtv)
                instance->ctx->rtv->Release();

            instance->ctx->swapchain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* back_buffer = nullptr;
            instance->ctx->swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

            if (back_buffer) {
                instance->ctx->d3d_device->CreateRenderTargetView(back_buffer, NULL, &instance->ctx->rtv);
                back_buffer->Release();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_MOUSEACTIVATE:
        return MA_ACTIVATE;
    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

graphics_renderer_t::graphics_renderer_t() {
    this->ctx = std::make_unique<gfx_context_t>();
}

graphics_renderer_t::~graphics_renderer_t() {
}

bool graphics_renderer_t::init_window() {
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "Chrome_WidgetWin_1";

    if (!RegisterClassExA(&wc))
        return false;

    this->ctx->hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        "main",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (!this->ctx->hwnd) {
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    SetWindowLongPtrA(this->ctx->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (!SetLayeredWindowAttributes(this->ctx->hwnd, RGB(0, 0, 0), 255, LWA_ALPHA))
        return false;

    MARGINS margins = { -1 };
    if (DwmExtendFrameIntoClientArea(this->ctx->hwnd, &margins) != S_OK)
        return false;

    ShowWindow(this->ctx->hwnd, SW_SHOW);
    UpdateWindow(this->ctx->hwnd);

    return true;
}

bool graphics_renderer_t::init_device() {
    if (!this->ctx->hwnd)
        return false;

    DXGI_SWAP_CHAIN_DESC swap_desc{};
    swap_desc.BufferCount = 1;
    swap_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_desc.OutputWindow = this->ctx->hwnd;
    swap_desc.SampleDesc.Count = 1;
    swap_desc.SampleDesc.Quality = 0;
    swap_desc.Windowed = TRUE;
    swap_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL feature_level;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        levels,
        2,
        D3D11_SDK_VERSION,
        &swap_desc,
        &this->ctx->swapchain,
        &this->ctx->d3d_device,
        &feature_level,
        &this->ctx->d3d_context
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            0,
            levels,
            2,
            D3D11_SDK_VERSION,
            &swap_desc,
            &this->ctx->swapchain,
            &this->ctx->d3d_device,
            &feature_level,
            &this->ctx->d3d_context
        );
    }

    if (FAILED(hr) || !this->ctx->d3d_device || !this->ctx->d3d_context)
        return false;

    ID3D11Texture2D* back_buffer = nullptr;
    hr = this->ctx->swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

    if (FAILED(hr) || !back_buffer)
        return false;

    hr = this->ctx->d3d_device->CreateRenderTargetView(back_buffer, nullptr, &this->ctx->rtv);
    back_buffer->Release();

    return !FAILED(hr);
}

bool graphics_renderer_t::init_imgui() {
    using namespace ImGui;
    CreateContext();
    StyleColorsDark();

    if (!ImGui_ImplWin32_Init(this->ctx->hwnd))
        return false;

    if (!this->ctx->d3d_device || !this->ctx->d3d_context)
        return false;

    if (!ImGui_ImplDX11_Init(this->ctx->d3d_device, this->ctx->d3d_context))
        return false;

    ImFontConfig font_config;
    font_config.OversampleH = 2;
    font_config.OversampleV = 2;
    font_config.PixelSnapH = true;
    font_config.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    font_config.FontDataOwnedByAtlas = false;
    
    ImGuiIO& io = ImGui::GetIO();
    tahoma_font = io.Fonts->AddFontFromMemoryTTF(tahoma_hex, sizeof(tahoma_hex), 13.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
    if (!tahoma_font) {
        tahoma_font = io.Fonts->AddFontDefault();
    }
    io.FontDefault = tahoma_font;

    console_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 12.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
    if (!console_font)
        console_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consolab.ttf", 12.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
    if (!console_font)
        console_font = tahoma_font;

    return true;
}

void graphics_renderer_t::init_class_icons() {
    if (!g_class_icon_srvs.empty())
        return;
    if (!this->ctx || !this->ctx->d3d_device)
        return;

    g_class_icon_srvs.reserve(k_class_icon_count);
    for (std::uint32_t i = 0; i < k_class_icon_count; i++) {
        const class_icon_offset_t& off = k_class_icon_offsets[i];
        if (off.offset + off.size > k_class_icon_blob_size) {
            g_class_icon_srvs.push_back(nullptr);
            continue;
        }
        g_class_icon_srvs.push_back(create_srv_from_png(this->ctx->d3d_device, k_class_icon_blob + off.offset, off.size));
    }

    g_luaapp_icon_srv = create_srv_from_png(this->ctx->d3d_device, k_luaapp_icon_png, k_luaapp_icon_size);
}


void graphics_renderer_t::begin_frame() {
    MSG msg{};

    const std::chrono::milliseconds frame_time(1000 / 60);
    auto last_frame_time = std::chrono::high_resolution_clock::now();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (this->cap_fps) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (GetAsyncKeyState(menu_key) & 1)
            menu_opened = !menu_opened;
        float animation_speed = 8.f;
        menu_alpha = ImClamp(menu_alpha + (ImGui::GetIO().DeltaTime * animation_speed * (menu_opened ? 1.f : -1.f)), 0.f, 1.f);

        HWND target = FindWindowA(nullptr, "Roblox");
        if (!target || IsIconic(target)) {
            MoveWindow(this->ctx->hwnd, 0, 0, 0, 0, true);
        }
        else {
            RECT client_rect;
            if (GetClientRect(target, &client_rect)) {
                POINT client_to_screen_pos = { client_rect.left, client_rect.top };
                ClientToScreen(target, &client_to_screen_pos);
                MoveWindow(this->ctx->hwnd, client_to_screen_pos.x, client_to_screen_pos.y, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, true);
            }
        }
            
        if (last_running_state != menu_opened) {
            if (menu_opened) {
                SetWindowLong(this->ctx->hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
                SetForegroundWindow(this->ctx->hwnd);
                SetFocus(this->ctx->hwnd);
            }
            else {
                SetWindowLong(this->ctx->hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
                HWND target = FindWindowA(nullptr, "Roblox");
                if (target) {
                    SetForegroundWindow(target);
                    SetFocus(target);
                }
            }
            last_running_state = menu_opened;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

      
        draw_interface();

        finish_frame();
    }
}

void graphics_renderer_t::finish_frame() {
    using namespace ImGui;
    Render();
    const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
    this->ctx->d3d_context->OMSetRenderTargets(1, &this->ctx->rtv, nullptr);
    this->ctx->d3d_context->ClearRenderTargetView(this->ctx->rtv, clear_color);
    ImGui_ImplDX11_RenderDrawData(GetDrawData());
    this->ctx->swapchain->Present(0, 0);
}

static int selected_decompiled = -1;
static int selected_disassembled = -1;
static std::uint64_t selected_instance = 0;

static void render_explorer_tab()
{
    static const std::array<const char*, 20> services = {
        "Workspace", "Lighting", "Players", "ReplicatedFirst", "ReplicatedStorage",
        "ServerScriptService", "ServerStorage", "SoundService", "StarterGui",
        "StarterPack", "StarterPlayer", "Teams",
        "TextChatService", "VoiceChatService", "PhysicsService", "InsertService",
        "AnalyticsService", "TestService", "GuiService", "Chat"
    };

    ImGui::BeginChild("instances_pane", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true);
    {
        std::function<void(rbx::instance_t&, bool)> draw_node;
        draw_node = [&](rbx::instance_t& node, bool top_level) {
            if (node.address == 0)
                return;
            std::string name = node.get_name();
            std::string cls = node.get_class_name();
            if (name == "Ugc" && cls == "DataModel")
                name = "LuaApp";
            auto children = node.get_children();
            if (top_level) {
                std::vector<rbx::instance_t> ordered;
                for (const char* service : services)
                {
                    for (auto& c : children)
                    {
                        if (c.get_class_name() == service)
                        {
                            ordered.push_back(c);
                            break;
                        }
                    }
                }
                children = std::move(ordered);
            }
            bool has_children = !children.empty();
            ImGuiTreeNodeFlags tflags = has_children ? 0 : ImGuiTreeNodeFlags_Leaf;
            ImGui::PushID(static_cast<int>(node.address));
            ImGuiID tree_id = ImGui::GetID("tree");
            bool open = ImGui::TreeNodeBehavior(tree_id, tflags, "", nullptr);
            if (ImGui::IsItemClicked())
                selected_instance = node.address;
            ImGui::SameLine();

            ID3D11ShaderResourceView* icon = find_class_icon(cls.c_str());
            if (name == "LuaApp" && g_luaapp_icon_srv)
                icon = g_luaapp_icon_srv;
            if (icon) {
                ImGui::Image(icon, ImVec2(16, 16));
                ImGui::SameLine();
            }

            std::string label = name + " [" + cls + "]";
            if (ImGui::Selectable(label.c_str(), selected_instance == node.address, 0, ImVec2(0, 16))) {
                selected_instance = node.address;
                if (has_children && !open) {
                    open = true;
                    ImGui::TreeNodeSetOpen(tree_id, true);
                }
            }
            if (open) {
                for (auto& child : children)
                    draw_node(child, false);
                ImGui::TreePop();
            }
            ImGui::PopID();
        };
        auto& dm = globals::services::game;
        if (dm.address != 0)
            draw_node(dm, true);
        else
            ImGui::TextDisabled("No game instance");
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("properties_pane", ImVec2(0, 0), true);
    {
        ID3D11ShaderResourceView* prop_icon = find_class_icon("Properties");
        if (prop_icon) {
            ImGui::Image(prop_icon, ImVec2(16, 16));
            ImGui::SameLine();
        }
        ImGui::Text("Explorer Properties");
        ImGui::Separator();
        if (selected_instance != 0) {
            rbx::instance_t inst(selected_instance);
            std::string name = inst.get_name();
            std::string cls = inst.get_class_name();
            ID3D11ShaderResourceView* inst_icon = find_class_icon(cls.c_str());
            if (inst_icon) {
                ImGui::Image(inst_icon, ImVec2(16, 16));
                ImGui::SameLine();
            }
            ImGui::Text("Class: %s", cls.c_str());
            ImGui::Text("Name: %s", name.c_str());
            ImGui::Separator();
            if (cls == "LocalScript" || cls == "ModuleScript" || cls == "Script") {
                decompiler::script_type t = (cls == "ModuleScript") ? decompiler::ModuleScript
                    : (cls == "Script") ? decompiler::Script : decompiler::LocalScript;
                if (ImGui::Button("Decompile"))
                    globals::services::decompiler_instance->decompile_script(selected_instance, t);
                ImGui::SameLine();
                if (ImGui::Button("Disassemble"))
                    globals::services::decompiler_instance->disassemble_script(selected_instance, t);
               // ImGui::TextDisabled("Result goes to the Decompiled/Disassembled tabs");
            }
        }
        else {
            ImGui::TextDisabled("Select an instance to inspect");
        }
    }
    ImGui::EndChild();
}

void graphics_renderer_t::draw_interface() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (menu_alpha <= 0.1f)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha);
    ImGui::SetNextWindowSize(ImVec2(860, 620), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("Scripts", nullptr, ImGuiWindowFlags_NoCollapse)) {
        if (ImGui::BeginTabBar("ScriptTabs")) {
            if (ImGui::BeginTabItem("Explorer")) {
                render_explorer_tab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Decompiled Scripts")) {
                ImGui::BeginChild("decompiled_list", ImVec2(260, 0), true);
                for (int i = 0; i < (int)globals::services::decompiled_scripts.size(); i++) {
                    auto& script = globals::services::decompiled_scripts[i];
                    if (ImGui::Selectable(script.title.c_str(), selected_decompiled == i))
                        selected_decompiled = i;
                }
                if (globals::services::decompiled_scripts.empty())
                    ImGui::TextDisabled("No decompiled scripts yet");
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("decompiled_view", ImVec2(0, 0), true);
                if (selected_decompiled >= 0 && selected_decompiled < (int)globals::services::decompiled_scripts.size()) {
                    if (console_font) ImGui::PushFont(console_font);
                    globals::services::decompiled_scripts[selected_decompiled].editor.Render("code", ImVec2(0, 0), false);
                    if (console_font) ImGui::PopFont();
                }
                else
                    ImGui::TextDisabled("Select a script to view its contents");
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Disassembled Scripts")) {
                ImGui::BeginChild("disassembled_list", ImVec2(260, 0), true);
                for (int i = 0; i < (int)globals::services::disassembled_scripts.size(); i++) {
                    auto& script = globals::services::disassembled_scripts[i];
                    if (ImGui::Selectable(script.title.c_str(), selected_disassembled == i))
                        selected_disassembled = i;
                }
                if (globals::services::disassembled_scripts.empty())
                    ImGui::TextDisabled("No disassembled scripts yet");
                ImGui::EndChild();
                ImGui::SameLine();
                ImGui::BeginChild("disassembled_view", ImVec2(0, 0), true);
                if (selected_disassembled >= 0 && selected_disassembled < (int)globals::services::disassembled_scripts.size()) {
                    if (console_font) ImGui::PushFont(console_font);
                    globals::services::disassembled_scripts[selected_disassembled].editor.Render("asm", ImVec2(0, 0), false);
                    if (console_font) ImGui::PopFont();
                }
                else
                    ImGui::TextDisabled("Select a script to view its disassembly");
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}