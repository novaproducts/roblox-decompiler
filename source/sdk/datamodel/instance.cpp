#include "instance.hpp"
#include "../../utils/memory/memory.h"
#include "../../cheat/offsets.hpp"
#include "../../utils/output/output.h"
#include <cstring>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
    std::shared_mutex g_class_cache_mutex;
    std::unordered_map<std::uint64_t, std::string> g_class_cache;

    std::shared_mutex g_desc_cache_mutex;
    std::unordered_map<std::uint64_t, std::uint64_t> g_desc_cache;

    bool is_valid_ptr(std::uint64_t address) {
        return address > 0x10000 && address < 0x7FFFFFFFFFFF;
    }

    std::uint64_t class_descriptor_for(std::uint64_t address) {
        if (!address) {
            return 0;
        }

        {
            std::shared_lock<std::shared_mutex> lock(g_desc_cache_mutex);
            auto it = g_desc_cache.find(address);
            if (it != g_desc_cache.end()) {
                return it->second;
            }
        }

        std::uint64_t desc = memory->read<std::uint64_t>(address + Offsets::Instance::ClassDescriptor);
        if (desc) {
            std::unique_lock<std::shared_mutex> lock(g_desc_cache_mutex);
            g_desc_cache[address] = desc;
        }
        return desc;
    }

    std::string resolve_class_name(std::uint64_t descriptor) {
        if (!descriptor) {
            return {};
        }

        {
            std::shared_lock<std::shared_mutex> lock(g_class_cache_mutex);
            auto it = g_class_cache.find(descriptor);
            if (it != g_class_cache.end()) {
                return it->second;
            }
        }

        std::uint64_t class_ptr = memory->read<std::uint64_t>(descriptor + Offsets::Instance::ClassName);
        std::string cn = class_ptr ? memory->read_string(class_ptr) : std::string{};

        {
            std::unique_lock<std::shared_mutex> lock(g_class_cache_mutex);
            g_class_cache[descriptor] = cn;
        }
        return cn;
    }
}

std::string rbx::instance_t::get_name() const {
    if (!this->address) {
        return {};
    }

    std::uint64_t container = memory->read<std::uint64_t>(this->address + Offsets::Instance::NameContainer);
    if (!is_valid_ptr(container)) {
        return {};
    }

    return memory->read_string(container + Offsets::Instance::Name);
}

std::string rbx::instance_t::get_class_name() const {
    return resolve_class_name(class_descriptor_for(this->address));
}

std::vector<rbx::instance_t> rbx::instance_t::get_children() {
    std::vector<rbx::instance_t> children;

    if (!this->address || !is_valid_ptr(this->address)) {
        return children;
    }

    std::uint64_t child_ptr = memory->read<std::uint64_t>(this->address + Offsets::Instance::ChildrenStart);
    if (!is_valid_ptr(child_ptr)) {
        return children;
    }

    std::uint64_t start = memory->read<std::uint64_t>(child_ptr);
    std::uint64_t end = memory->read<std::uint64_t>(child_ptr + Offsets::Instance::ChildrenEnd);

    if (!start || !end || start >= end) {
        return children;
    }

    const std::size_t diff = static_cast<std::size_t>(end - start);
    if (diff > 160000) {
        return children;
    }

    const std::size_t count = diff / 16;
    if (count > 10000) {
        return children;
    }

    thread_local std::vector<std::uint8_t> buf;
    if (buf.size() < diff) {
        buf.resize(diff);
    }

    Luck_ReadVirtualMemory(memory->get_process_handle(), reinterpret_cast<void*>(start), buf.data(), static_cast<ULONG>(diff), nullptr);

    children.reserve(count);
    for (std::size_t i = 0; i < diff; i += 16) {
        std::uint64_t child = 0;
        std::memcpy(&child, buf.data() + i, sizeof(std::uint64_t));
        if (child) {
            children.emplace_back(rbx::instance_t(child));
        }
        if (children.size() >= 10000) {
            break;
        }
    }
    return children;
}

std::uint64_t rbx::instance_t::find_first_child(const std::string& child_name)
{
    for (const auto& child : this->get_children()) {
        if (child.get_name() == child_name) {
            return child.address;
        }
    }
    return 0;
}

std::uint64_t rbx::instance_t::find_first_child_by_class(const std::string& class_name)
{
    for (const auto& child : this->get_children()) {
        if (resolve_class_name(class_descriptor_for(child.address)) == class_name) {
            return child.address;
        }
    }
    return 0;
}

std::uint64_t rbx::instance_t::get_parent()
{
    if (!this->address) {
        return 0;
    }
    return memory->read<std::uint64_t>(this->address + Offsets::Instance::Parent);
}




std::uint64_t rbx::instance_t::get_primitive()
{
	rbx::instance_t* self = static_cast<rbx::instance_t*>(this);
	instance_t parent = instance_t(memory->read<std::uint64_t>(self->address + Offsets::BasePart::Primitive));
	return parent.address;
}

void rbx::instance_t::set_size(const rbx::vector3& size) {
    if (!this->address) return;
    std::uint64_t primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (!primitive) return;
    memory->write<rbx::vector3>(primitive + Offsets::Primitive::Size, size);
}

void rbx::instance_t::set_can_collide(bool can_collide) {
    if (!this->address) return;
    std::uint64_t primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (!primitive) return;
    
    std::uint8_t flags = memory->read<std::uint8_t>(primitive + Offsets::Primitive::Flags);
    const std::uint8_t mask = static_cast<std::uint8_t>(Offsets::PrimitiveFlags::CanCollide);
    
    if (can_collide) {
        flags |= mask;
    } else {
        flags &= ~mask;
    }
    
    memory->write<std::uint8_t>(primitive + Offsets::Primitive::Flags, flags);
}

void rbx::instance_t::set_transparency(float transparency) {
    if (!this->address) return;
    memory->write<float>(this->address + Offsets::BasePart::Transparency, transparency);
}

void rbx::instance_t::set_mesh_id(const std::string& mesh_id) {
	memory->write_string(this->address + Offsets::MeshPart::MeshId, mesh_id);

}

std::string rbx::instance_t::get_mesh_id() {
    if (!this->address) {
        return "";
    }
    
    std::string class_name = this->get_class_name();
    std::uintptr_t mesh_id_offset = 0;
    
    if (class_name == "MeshPart") {
        mesh_id_offset = Offsets::MeshPart::MeshId;
    } else if (class_name == "SpecialMesh") {
        mesh_id_offset = Offsets::SpecialMesh::MeshId;
    } else {
        return "";
    }
    
    std::uint64_t string_ptr = memory->read<std::uint64_t>(this->address + mesh_id_offset);
    if (!string_ptr) {
        return "";
    }
    
    std::string result = memory->read_string(string_ptr);
    
    if (result.empty() || result == "Unknown") {
        return "";
    }
    
    if (result.find("rbxassetid://") == 0) {
        size_t id_start = 13;
        size_t id_end = result.find_first_not_of("0123456789", id_start);
        if (id_end == std::string::npos) {
            id_end = result.length();
        }
        if (id_start < id_end) {
            return result.substr(id_start, id_end - id_start);
        }
    }
    
    if (result.find("http://") == 0 || result.find("https://") == 0) {
        size_t id_start = result.find_last_of('/');
        if (id_start != std::string::npos && id_start + 1 < result.length()) {
            std::string id_part = result.substr(id_start + 1);
            size_t id_end = id_part.find_first_not_of("0123456789");
            if (id_end == std::string::npos) {
                return id_part;
            }
            return id_part.substr(0, id_end);
        }
    }
    
    if (result.length() > 0 && result[0] >= '0' && result[0] <= '9') {
        size_t id_end = result.find_first_not_of("0123456789");
        if (id_end == std::string::npos) {
            return result;
        }
        return result.substr(0, id_end);
    }
    
    return result;
}

std::string rbx::instance_t::get_animation_id() {
    if (!this->address) return "";
    std::uint64_t string_ptr = memory->read<std::uint64_t>(this->address + Offsets::Misc::AnimationId);
    if (string_ptr) {
        return memory->read_string(string_ptr);
    }
    return "";
}

void rbx::instance_t::set_animation_id(const std::string& animation_id) {
	memory->write_string(this->address + Offsets::Misc::AnimationId, animation_id);

}



void rbx::instance_t::set_emote_id(const std::string& emoteId)
{
}
void rbx::instance_t::set_velocity(const rbx::vector3& velocity) {
    if (!this->address) return;
    std::uint64_t primitive = memory->read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (primitive) {
        memory->write<rbx::vector3>(primitive + Offsets::Primitive::AssemblyLinearVelocity, velocity);
    }
}

bool rbx::instance_t::read_bool_value() {
    if (!this->address) return false;
    return memory->read<bool>(this->address + Offsets::Misc::Value);
}

void rbx::instance_t::write_bool_value(bool value) {
    memory->write<bool>(this->address + Offsets::Misc::Value, value);
}
