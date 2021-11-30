#include "Utils/ModUtils.hpp"
#include "main.hpp"

namespace ModUtils {
	const char* modPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";

	std::list<std::string> GetDirContents(const char* dirPath) {
		DIR* dir = opendir(dirPath);
		dirent* dp;
		std::list<std::string> files; 

		if (dir == nullptr) return files;

		while ((dp = readdir(dir)) != NULL) {
			if (strlen(dp->d_name) > 3) {
				files.emplace_front(std::string(dp->d_name));
			}
		}

		return files;
	}

	std::string GetFileNameFromDir(std::string libName) {
		std::list<std::string> modFileNames = GetDirContents(modPath);

		for (std::string modFileName : modFileNames) {
			if (!strcmp(GetLibName(modFileName).c_str(), libName.c_str())) return modFileName;
		}

		return {"Null"};
	}

	void ToggleMod(std::string modName) {
		std::list<std::string> modFileNames = GetDirContents(modPath);

		for (std::string modFileName : modFileNames) {
			if (!IsFileName(modFileName)) continue;
			if (strcmp(modFileName.c_str(), GetFileName(modName).c_str())) continue;

			if (IsDisabled(modFileName)) rename(string_format("%s/%s", modPath, modFileName.c_str()).c_str(), string_format("%s/lib%s.so", modPath, modName.c_str()).c_str());
			else rename(string_format("%s/%s", modPath, modFileName.c_str()).c_str(), string_format("%s/lib%s.disabled", modPath, modName.c_str()).c_str());
		}
	}

	void SetModsActive(std::list<std::string>* mods) {
		getLogger().info("Setting mod activites");
		for (std::string modFileName : *mods) {
			std::string modName = GetModName(modFileName);

			getLogger().info("Setting Activity For \"%s\"", modName.c_str());
			ToggleMod(modName);
		}
	}

	bool IsDisabled(std::string name) {
		std::string fileName = GetFileName(name);

		return fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled");
	}

    // Mod Name = modname
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Name Tests

    bool IsModName(std::string name) {
		return !IsLibName(name) && !IsFileName(name);
	}

    bool IsLibName(std::string name) {
		return (!IsFileName(name)) && (name.length() > 3 && !strcmp(name.substr(0, 3).c_str(), "lib"));
	}

    bool IsFileName(std::string name) {
		return (name.length() > 9 && !strcmp(name.substr(name.size() - 9).c_str(), ".disabled")) || (name.length() > 3 && !strcmp(name.substr(name.size() - 3).c_str(), ".so"));
	}

    // Name Conversions

    std::string GetModName(std::string name) {
		if (IsModName(name)) { getLogger().info("\"%s\" Already Mod Name! Returning", name.c_str()); return name; }
		std::string libName;

		if (IsFileName(name)) libName = GetLibName(name);
		else libName = name;

		getLogger().info("Returning \"%s\" as Mod Name", libName.substr(3).c_str());
		return libName.substr(3);
	}

    std::string GetLibName(std::string name) {
		if (IsLibName(name)) { getLogger().info("\"%s\" Already Lib Name! Returning", name.c_str()); return name; }

		if (IsFileName(name)) {
			if (IsDisabled(name)) { getLogger().info("Returning \"%s\" as LibName", name.substr(0, name.size() - 9).c_str()); return name.substr(0, name.size() - 9); }
			else { getLogger().info("Returning \"%s\" as LibName", name.substr(0, name.size() - 3).c_str()); return name.substr(0, name.size() - 3); }
		} else {
			getLogger().info("Returning \"%s\" as LibName", ("lib" + name).c_str());
			return "lib" + name;
		}
	}

    std::string GetFileName(std::string name) {
		if (IsFileName(name)) { getLogger().info("\"%s\" Already File Name! Returning", name.c_str()); return name; }
		std::string libName;

		if (IsLibName(name)) libName = name;
		else libName = GetLibName(name);

		getLogger().info("Returning \"%s\" as FileName", GetFileNameFromDir(libName).c_str());
		return GetFileNameFromDir(libName);
	}
}