#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <algorithm>

#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

class ModUtils {
public:
	/**
	 * @brief Get all the files that are contained in a specified directory
	 * 
	 * @param dirPath The Path to get the contents of
	 * @return A list of all the files in the directory
	 */
	static std::list<std::string> GetDirContents(const char* dirPath);

	/**
	 * @brief Toggles a mod to either be enabled or disabled
	 * 
	 * @param name The mod to toggle
	 */
	static void ToggleMod(std::string name);

	/**
	 * @brief Sets a list of mods to be toggled
	 * 
	 * @param mods The list of mods to be toggles
	 */
	static void SetModsActive(std::list<std::string>* mods);

	/**
	 * @brief Checks if a mod is disabled for not
	 * 
	 * @param name The mod to check
	 * @return Returns true if disabled
	 */
	static bool IsDisabled(std::string name);

	/**
	 * @brief Checks if a mod is an odd lib or not
	 * @details An "Odd Lib" is a mod that doesnt start with "lib"
	 * @details This just makes it easy to test if a name is a lib
	 * 
	 * @param name The mod to check
	 * @return Returns true if mod is an odd lib
	 */
	static bool IsOddLibName(std::string name);

	/**
	 * @brief Checks if a mod is loaded
	 * @details A mod is considered loaded if it has been dlopened by ModLoader
	 * 
	 * @param name The mod to check
	 * @return Returns true if the mod is loaded
	 */
	static bool IsModLoaded(std::string name);

	/**
	 * @brief Checks if a mod is a core mod
	 * 
	 * @param name The mod to check
	 * @return Returns true if the mod is a core mod
	 */
	static bool IsCoreMod(std::string name);

    // Mod Id = Mod Name
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother

    // Name Tests

	/**
	 * @brief Checks if a mod name is a Mod ID
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a Mod ID
	 */
    static bool IsModID(std::string name);

	/**
	 * @brief Checks if a mod name is a Lib Name
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a Lib Name
	 */
    static bool IsLibName(std::string name);

	/**
	 * @brief Checks if a mod name is a File Name
	 * 
	 * @param name The name to check
	 * @return Returns true if the name is a File Name
	 */
    static bool IsFileName(std::string name);

    // Name Conversions

	/**
	 * @brief Gets the Mod ID of a mod
	 * 
	 * @param name The mod to get the id of
	 * @return The mod's Mod ID. If the mod isn't loaded the mod's Lib Name will be returned instead
	 */
	static std::string GetModID(std::string name);

	/**
	 * @brief Gets the Lib Name of a mod
	 * @details The File Name is just the file name without the file extention
	 * 
	 * @param name The mod to get the lib name of
	 * @return The mod's Lib Name.
	 */
    static std::string GetLibName(std::string name);

	/**
	 * @brief Gets the File Name of a mod
	 * @details The File Name is just the lib name with the file extention
	 * 
	 * @param name The mod to get the file name of
	 * @return The mod's File Name.
	 */
    static std::string GetFileName(std::string name);

	/**
	 * @brief Get a list of all the loaded mods
	 * 
	 * @return Returns a list of loaded mod file names
	 */
	static std::list<std::string> GetLoadedModsFileNames();

	/**
	 * @brief Get a list of all the loaded core mods
	 * 
	 * @return Returns a list of the lib names for the loaded core mods
	 */
	static std::list<std::string> GetCoreMods();

	/**
	 * @brief Get a list of all the "Odd Libs"
	 * 
	 * @return Returns a list of Odd Lib Names
	 */
	static std::list<std::string> GetOddLibNames();

	/**
	 * @brief Get's the error for a mod
	 * @details The mod is dlopened, and then then the error is fetched using dlerror
	 * 
	 * @param name The name of the mod to test for an error
	 * @return Returns the error if there was one, else returns null
	 */
	static std::optional<std::string> GetModError(std::string name);

	/**
	 * @brief Restarts Beat Saber
	 */
	static void RestartBS();

	static void Init();
private:
	static const char* m_ModPath;
	static JavaVM* m_Jvm;

	static std::list<std::string>* m_OddLibNames;
	static std::list<std::string>* m_CoreMods;
	static std::list<std::string>* m_LoadedMods;

	static void CollectCoreMods();
	static void CollectLoadedMods();
	static void CollectOddLibs();

	static std::string GetFileNameFromDir(std::string libName);
	static std::string GetFileNameFromModID(std::string modID);

	static void CacheJVM();
	static void __attribute__((constructor)) DlOpened();
};