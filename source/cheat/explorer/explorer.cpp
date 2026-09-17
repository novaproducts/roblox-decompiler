#include "explorer.hpp"

#include <thread>
#include <chrono>
#include "../../../libraries/dear-imgui/imgui.h"
#include "../../../source/globals/globals.hpp"


void explorer::run()
{
    globals::services::explorer_instance->cache();
}

void explorer::explorer_t::cache()
{
    root = new node_t{};
    root->self = globals::services::game;

    auto build_tree = [](node_t* parent, rbx::instance_t& instance, auto& self_ref) -> void {
        node_t* child_node = new node_t{};
        child_node->parent = parent;
        child_node->self = instance;
        parent->children.push_back(child_node);

        for (rbx::instance_t& child_instance : instance.get_children())
        {
            self_ref(child_node, child_instance, self_ref);
        }
        };

    for (rbx::instance_t& instance : globals::services::game.get_children())
    {
        build_tree(root, instance, build_tree);
    }
}

void explorer::explorer_t::render_node(node_t* node)
{
    if (!node) return;

    char buf[128];
    snprintf(buf, sizeof(buf), "%s [%s]",
        node->self.get_name().c_str(),
        node->self.get_class_name().c_str()
    );

    ImGuiTreeNodeFlags flags = 0;

    if (node->children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (node == selected_node) flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx((void*)node, flags, "%s", buf);

    if (ImGui::IsItemClicked())
    {
        selected_node = node;

        if (!node->children.empty() && !open)
        {
            ImGui::TreeNodeEx((void*)node, flags | ImGuiTreeNodeFlags_DefaultOpen, "%s", buf);
        }
    }

    if (open)
    {
        for (node_t* child : node->children)
        {
            render_node(child);
        }

        if (!node->children.empty())
        {
            ImGui::TreePop();
        }
    }
}