#pragma once
#include <vector>

#include <string>
#include "../../../source/sdk/datamodel/instance.hpp"
namespace explorer
{
    void run();

    struct node_t final
    {
        node_t* parent;
        rbx::instance_t self;
        std::vector<node_t*> children;

        std::string get_path()
        {
            if (!parent)
            {
                return self.get_name();
            }

            return parent->get_path() + "." + self.get_name();
        }
    };

    class explorer_t final
    {
    public:
        void cache();
        void render_node(explorer::node_t* node);

        node_t* root;
        node_t* selected_node = nullptr;
    };
}