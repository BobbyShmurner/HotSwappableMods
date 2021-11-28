#pragma once

#include "modloader/shared/modloader.hpp"

class PublicMod {
    public:
        PublicMod(std::string_view name_, std::string_view path, ModInfo info_, void *handle_) : name(name_), pathName(path), info(info_), handle(handle_) {}
        const std::string name;
        const std::string pathName;
        const ModInfo info;
        bool get_loaded() const;
        bool operator==(const Mod& m) const {
            return info.id == m.info.id && info.version == m.info.version;
        }
        void init_mod();
        void load_mod();
        bool loaded = false;
        void *handle = nullptr;
        bool init_loaded = false;
        void (*init_func)(void) = NULL;
        bool load_loaded = false;
        void (*load_func)(void) = NULL;
};