#pragma once
#include "../sdk/datamodel/instance.hpp"
#include <vector>
#include "../cheat/explorer/explorer.hpp"
#include "../cheat/explorer/decompiler/decompiler.h"
#include "../../libraries/dear-imgui/TextEditor.h"
#include <atomic>
#include <mutex>
#include <cstdint>
#include "../utils/math/math.hpp"
#include "../../libraries/dear-imgui/imgui.h"

struct decompiled_script_t {
    std::string title;
    std::string code;
    bool open = true;
    TextEditor editor;

    decompiled_script_t() {
        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
        editor.SetPalette(TextEditor::GetDarkPalette());
        editor.SetShowWhitespaces(false);
    }
};

namespace globals {

    namespace services {
        inline rbx::instance_t game;
        inline std::unique_ptr<explorer::explorer_t> explorer_instance = std::make_unique<explorer::explorer_t>();
        inline std::unique_ptr<decompiler::decompiler_t> decompiler_instance = std::make_unique<decompiler::decompiler_t>();
        inline std::vector<decompiled_script_t> decompiled_scripts;
        inline std::vector<decompiled_script_t> disassembled_scripts;
    }

    namespace cache_settings {
        inline float cache_speed = 250.0f;
        inline int max_players_cached = 32;
        inline bool cache_display_names = true;
        inline bool cache_tools = true;
        inline bool cache_health = true;
        inline bool cache_rig_type = true;
    }

    namespace decompiler_settings {
        inline decompiler::backend selected = decompiler::backend::lunaux;
    }

    inline bool focused = true;
}
