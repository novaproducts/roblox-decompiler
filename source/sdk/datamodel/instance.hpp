#pragma once
#include <cstdint>
#include <string>
#ifndef instance_HPP
#define instance_HPP
#include <vector>
#include "../../utils/math/math.hpp"
namespace rbx {
    class instance_t {
    public:
        std::uint64_t address;
        instance_t() : address(0) {}
        instance_t(std::uint64_t addr) : address(addr) {}
        std::string get_name() const;
        std::string get_class_name() const;
        std::vector<rbx::instance_t> get_children();
        std::uint64_t find_first_child(const std::string& child_name);
        std::uint64_t find_first_child_by_class(const std::string& class_name);
        std::uint64_t get_parent();
        std::uint64_t get_primitive();
        rbx::vector3 get_move_direction();
        void set_size(const rbx::vector3& size);
        void set_can_collide(bool can_collide);
        void set_transparency(float transparency);
        void set_mesh_id(const std::string& mesh_id);
        std::string get_mesh_id();
        std::string get_animation_id();
        void set_animation_id(const std::string& animation_id);
        void set_emote_id(const std::string& emote_id);
        void set_velocity(const rbx::vector3& velocity);
        bool read_bool_value();
        void write_bool_value(bool value);

    };
}
#endif //instance_HPP