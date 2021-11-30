#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <algorithm>

#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

namespace ModUtils {
	std::list<std::string> GetDirContents(const char* dirPath);
    std::string GetFileNameFromDir(std::string libName);
    
    void ToggleMod(std::string modName);
    void SetModsActive(std::list<std::string>* mods);

    void GetOddLibNames();

    bool IsDisabled(std::string name);
    bool IsOddLibName(std::string name);

    void UpdateAlwaysDisplayLibNames(bool value);

    // Mod Display Name = Mod Name
    // Mod Name = modname
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother,
    // But a Mod Display Name can only convert to a File Name

    // Name Tests

    bool IsModName(std::string name);
    bool IsLibName(std::string name);
    bool IsFileName(std::string name);

    // Name Conversions

    std::string GetModName(std::string name);
    std::string GetLibName(std::string name);
    std::string GetFileName(std::string name);

    std::string GetModDisplayName(std::string name);
    std::string GetFileNameFromDisplayName(std::string name);
}