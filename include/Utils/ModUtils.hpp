#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <algorithm>

#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

class ModUtils {
public:
	static std::list<std::string> GetDirContents(const char* dirPath);
	static std::string GetFileNameFromDir(std::string libName);

	static void ToggleMod(std::string modName);
	static void SetModsActive(std::list<std::string>* mods);
    static void GetOddLibNames();

	static bool IsDisabled(std::string name);
	static bool IsOddLibName(std::string name);
	static bool IsModLoaded(std::string name);
	static bool IsCoreMod(std::string name);

    // Mod Name = Mod Name
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother

    // Name Tests

    static bool IsModName(std::string name);
    static bool IsLibName(std::string name);
    static bool IsFileName(std::string name);

    // Name Conversions

	static std::string GetModName(std::string name);
    static std::string GetLibName(std::string name);
    static std::string GetFileName(std::string name);

	static std::string GetFileNameFromDisplayName(std::string displayName);
	static std::string GetModID(std::string name);
	static std::list<std::string>* GetLoadedModsFileNames();

	static std::list<std::string>* GetCoreMods();
	static std::optional<std::string> GetModError(std::string name);

	static void RestartBS();
	static void Init();
private:
	static const char* m_ModPath;
	static std::list<std::string>* m_OddLibNames;
	static std::list<std::string>* m_CoreMods;
	static std::list<std::string>* m_LoadedMods;

	static JavaVM* m_Jvm;

	static void CollectCoreMods();
	static void CollectLoadedMods();

	static void CacheJVM();
	static void __attribute__((constructor)) DlOpened();
};