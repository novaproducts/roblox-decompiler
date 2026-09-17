#pragma once
#include <memory>
#include <d3d11.h>

struct gfx_context_t {
    HWND hwnd = nullptr;
    WNDCLASSEX wnd_class = {};
    ID3D11Device* d3d_device = nullptr;
    ID3D11DeviceContext* d3d_context = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    IDXGISwapChain* swapchain = nullptr;
};

class graphics_renderer_t {
public:
    graphics_renderer_t();
    ~graphics_renderer_t();

    bool is_active = true;

    void begin_frame();
    void draw_interface();
    void finish_frame();

    bool init_device();
    bool init_window();
    bool init_imgui();
    void init_class_icons();

    bool cap_fps = true;

    std::unique_ptr<gfx_context_t> ctx;
};