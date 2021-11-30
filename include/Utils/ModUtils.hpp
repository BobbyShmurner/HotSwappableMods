#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>

#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

namespace ModUtils {
	std::list<std::string> GetDirContents(const char* dirPath);
    std::string GetFileNameFromDir(std::string libName);
    
    void ToggleMod(std::string modName);
    void SetModsActive(std::list<std::string>* mods);

    bool IsDisabled(std::string name);

    // Mod Name = modname
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Name Tests

    bool IsModName(std::string name);
    bool IsLibName(std::string name);
    bool IsFileName(std::string name);

    // Name Conversions

    std::string GetModName(std::string name);
    std::string GetLibName(std::string name);
    std::string GetFileName(std::string name);
}