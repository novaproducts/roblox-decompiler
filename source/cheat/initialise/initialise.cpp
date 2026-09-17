#include <cstdint>
#include <thread>
#include "../../utils/output/output.h"
#include "../../utils/memory/memory.h"
#include "../../globals/globals.hpp"
#include "../offsets.hpp"
#include "../overlay/overlay.hpp"

void overlay_thread() {
    graphics_renderer_t render;
    if (!render.init_window()) return;
    if (!render.init_device()) return;
    render.init_class_icons();
    if (!render.init_imgui()) return;
    render.begin_frame();
}

auto main(int32_t argc, char* argv[]) -> int {
    memory->attach_to_process("RobloxPlayerBeta.exe");
    memory->find_module_address("RobloxPlayerBeta.exe");
    auto module = memory->get_module_address();

    if (module == 0) {
        g_logger.error("failed to resolve module base");
        return 1;
    }

    g_logger.setup("nut");
    g_logger.info("attached to roblox");

    auto fake_datamodel = memory->read<std::uint64_t>(module + Offsets::FakeDataModel::Pointer);
    if (fake_datamodel == 0) {
        g_logger.error("failed to find fake datamodel");
        return 1;
    }

    auto datamodel = memory->read<std::uint64_t>(fake_datamodel + Offsets::FakeDataModel::RealDataModel);
    if (datamodel == 0) {
        g_logger.error("failed to find real datamodel");
        return 1;
    }

    g_logger.info("found datamodel 0x%llX", static_cast<unsigned long long>(datamodel));

    auto workspace = memory->read<std::uint64_t>(datamodel + Offsets::DataModel::Workspace);
    g_logger.info("workspace 0x%llX", static_cast<unsigned long long>(workspace));

    auto world = memory->read<std::uint64_t>(workspace + Offsets::Workspace::World);
    g_logger.info("world 0x%llX", static_cast<unsigned long long>(world));

    globals::services::game = rbx::instance_t(datamodel);
    globals::services::explorer_instance->cache();

    std::thread(overlay_thread).detach();

    while (true) {
        Sleep(INFINITE);
    }

    return 0;
}