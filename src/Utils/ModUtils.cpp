#include "Utils/ModUtils.hpp"
#include "DataTypes/MainConfig.hpp"
#include "main.hpp"

namespace ModUtils {
	const char* ModPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
	std::list<std::string>* OddLibNames = new std::list<std::string>();

	bool AlwaysDisplayLibNames = false;

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
		std::list<std::string> modFileNames = GetDirContents(ModPath);

		for (std::string modFileName : modFileNames) {
			if (!strcmp(GetLibName(modFileName).c_str(), libName.c_str())) return modFileName;
		}

		return {"Null"};
	}

	void ToggleMod(std::string modName) {
		std::list<std::string> modFileNames = GetDirContents(ModPath);

		for (std::string modFileName : modFileNames) {
			if (!IsFileName(modFileName)) continue;
			if (strcmp(modFileName.c_str(), GetFileName(modName).c_str())) continue;

			if (IsDisabled(modFileName)) rename(string_format("%s/%s", ModPath, modFileName.c_str()).c_str(), string_format("%s/%s.so", ModPath, GetLibName(modName).c_str()).c_str());
			else rename(string_format("%s/%s", ModPath, modFileName.c_str()).c_str(), string_format("%s/%s.disabled", ModPath, GetLibName(modName).c_str()).c_str());
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

    void GetOddLibNames() {
		OddLibNames->clear();
		std::list<std::string> modFileNames = GetDirContents(ModPath);

		for (std::string modFileName : modFileNames) {
			if (!IsFileName(modFileName)) continue;

			if (strcmp(modFileName.substr(0, 3).c_str(), "lib")) {
				getLogger().info("Mod \"%s\" does not start with \"lib\"! Adding to OddLibNames", modFileName.c_str());

				if (IsDisabled(modFileName)) OddLibNames->emplace_front(modFileName.substr(0, modFileName.size() - 9));
				else OddLibNames->emplace_front(modFileName.substr(0, modFileName.size() - 3));
			}
		}
	}

	bool IsDisabled(std::string name) {
		std::string fileName = GetFileName(name);

		return fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled");
	}

	bool IsOddLibName(std::string name) {
		return (std::find(OddLibNames->begin(), OddLibNames->end(), name) != OddLibNames->end());
	}

	void UpdateAlwaysDisplayLibNames(bool value) {
		AlwaysDisplayLibNames = value;
		getMainConfig().AlwaysShowFileNames.SetValue(value);
	}

    // Mod Display Name = Mod Name
    // Mod Name = modname
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother,
    // But a Mod Display Name can only convert to a File Name

    // Name Tests

    bool IsModName(std::string name) {
		return !IsLibName(name) && !IsFileName(name);
	}

    bool IsLibName(std::string name) {
		return ((!IsFileName(name)) && (name.length() > 3 && !strcmp(name.substr(0, 3).c_str(), "lib"))) || IsOddLibName(name);
	}

    bool IsFileName(std::string name) {
		return (name.length() > 9 && !strcmp(name.substr(name.size() - 9).c_str(), ".disabled")) || (name.length() > 3 && !strcmp(name.substr(name.size() - 3).c_str(), ".so"));
	}

    // Name Conversions

    std::string GetModName(std::string name) {
		if (IsModName(name)) return name;
		std::string libName;
		std::string modName;

		if (IsFileName(name)) libName = GetLibName(name);
		else libName = name;

		
		if (IsOddLibName(libName)) return libName;
		else return libName.substr(3);
	}

    std::string GetLibName(std::string name) {
		if (IsLibName(name)) return name;

		if (IsFileName(name)) {
			if (IsDisabled(name)) return name.substr(0, name.size() - 9);
			else return name.substr(0, name.size() - 3);
		} else {
			return "lib" + name;
		}
	}

    std::string GetFileName(std::string name) {
		if (IsFileName(name)) return name;
		std::string libName;

		if (IsLibName(name)) libName = name;
		else libName = GetLibName(name);

		return GetFileNameFromDir(libName);
	}

	std::string GetModDisplayName(std::string name) {
		if (AlwaysDisplayLibNames) return GetLibName(name);

		std::string fileName = GetFileName(name);
		std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
		
		for (std::pair<std::string, const Mod> modPair : mods) {
			if (!strcmp(name.c_str(), modPair.second.name.c_str())) return modPair.first;
		}

		return name;
	}

	std::string GetFileNameFromDisplayName(std::string displayName) {
		if (AlwaysDisplayLibNames) return GetFileName(displayName);

		return Modloader::getMods().at(displayName).name;
	}
}