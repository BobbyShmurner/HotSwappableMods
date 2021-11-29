#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>

#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

namespace DirUtils {
	std::list<std::string> GetDirContents(const char* dirPath);
    void ToggleMod(std::string modName);
    void SetModsActive(std::list<std::string>* mods);
}