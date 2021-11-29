#include "Utils/DirUtils.hpp"
#include "main.hpp"

namespace DirUtils {
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

	void ToggleMod(std::string modName) {
		const char* modPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
		DIR* dir = opendir(modPath);
		dirent* dp;
		bool isEnabled = true;

		if (dir == nullptr) return;

		while ((dp = readdir(dir)) != NULL) {
			std::string fileName = std::string(dp->d_name);
			if (fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled") && !strcmp(fileName.substr(0, fileName.size() - 9).c_str(), modName.c_str())) {
				isEnabled = false;
			} else if (fileName.length() > 3 && !strcmp(fileName.substr(fileName.size() - 3).c_str(), ".so") && !strcmp(fileName.substr(0, fileName.size() - 3).c_str(), modName.c_str())) {
				isEnabled = true;
			} else continue;

			getLogger().info("Found file \"%s\" for mod \"%s\"", dp->d_name, modName.c_str());

			if (isEnabled) {
				rename(string_format("%s/%s", modPath, dp->d_name).c_str(), string_format("%s/%s.disabled", modPath, modName.c_str()).c_str());
				getLogger().info("Disabled \"%s\"", modName.c_str());
			} else {
				rename(string_format("%s/%s", modPath, dp->d_name).c_str(), string_format("%s/%s.so", modPath, modName.c_str()).c_str());
				getLogger().info("Enabled \"%s\"", modName.c_str());
			}
		}
	}

	void SetModsActive(std::list<std::string>* mods) {
		getLogger().info("Setting mod activites");
		for (std::string modName : *mods) {
			getLogger().info("Setting Activity For \"%s\"", modName.c_str());
			ToggleMod(modName);
		}
	}
}