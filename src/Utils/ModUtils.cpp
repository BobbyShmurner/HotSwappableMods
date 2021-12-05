#include "Utils/ModUtils.hpp"
#include "Utils/JNIUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include <dlfcn.h>
#include <dirent.h>

#include "main.hpp"

namespace ModUtils {
	const char* ModPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
	std::list<std::string>* OddLibNames = new std::list<std::string>();

	static JavaVM* jvm;

	bool AlwaysDisplayLibNames = false;

	std::list<std::string> GetDirContents(const char* dirPath) {
		DIR* dir = opendir(dirPath);
		dirent* dp;
		std::list<std::string> files; 

		if (dir == nullptr) return files;

		while ((dp = readdir(dir)) != NULL) {
			if (strlen(dp->d_name) > 3 && dp->d_type != DT_DIR) {
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

	bool IsModLoaded(std::string name) {
		std::list<std::string> loadedModsLibNames = GetLoadedModsFileNames();

		return (std::find(loadedModsLibNames.begin(), loadedModsLibNames.end(), GetFileName(name)) != loadedModsLibNames.end());
	}

	void UpdateAlwaysDisplayLibNames(bool value) {
		AlwaysDisplayLibNames = value;
		getMainConfig().AlwaysShowFileNames.SetValue(value);
	}

    // Mod Name = Mod Name
    // Lib Name = libmodname
    // File Name = libmodname.so / libmodname.disabled

    // Mod Names, Lib Names and File Names can all convert between eachother

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
		if (AlwaysDisplayLibNames) return GetLibName(name);

		std::string fileName = GetFileName(name);
		std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
		
		for (std::pair<std::string, const Mod> modPair : mods) {
			if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return modPair.first;
		}

		return GetLibName(name);
	}

    std::string GetLibName(std::string name) {
		if (IsLibName(name)) return name;

		std::string fileName;

		if (IsFileName(name)) fileName = name;
		else fileName = GetFileNameFromDisplayName(name);

		if (IsDisabled(fileName)) return fileName.substr(0, fileName.size() - 9);
		else return fileName.substr(0, fileName.size() - 3);

		return fileName;
	}

    std::string GetFileName(std::string name) {
		if (IsFileName(name)) return name;
		std::string libName;

		if (IsLibName(name)) libName = name;
		else libName = GetLibName(name);

		return GetFileNameFromDir(libName);
	}

	std::string GetFileNameFromDisplayName(std::string displayName) {
		if (AlwaysDisplayLibNames) return GetFileName(displayName);

		return Modloader::getMods().at(displayName).name;
	}

	std::string GetModID(std::string name) {
		std::string fileName = GetFileName(name);
		std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
		
		for (std::pair<std::string, const Mod> modPair : mods) {
			if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return modPair.second.info.id;
		}

		return {"Null"};
	}

	std::list<std::string> GetLoadedModsFileNames() {
		std::list<std::string> loadedMods;
		for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
			loadedMods.emplace_front(modPair.second.name);
		}

		return loadedMods;
	}

	// Thanks for Laurie for the original code snippet: 
	std::optional<std::string> GetModError(std::string name) {
		std::string fileName = GetFileName(name);
		std::string filePath = Modloader::getDestinationPath() + fileName;
		
		dlerror(); // Clear Existing Errors
		dlopen(filePath.c_str(), RTLD_LOCAL | RTLD_NOW);

		char* error = dlerror();
		return error ? std::optional(std::string(error).substr(15)) : std::nullopt;
	}

	void RestartBS() {
		getLogger().info("-- STARTING RESTART --");

		JNIEnv* env;

		JavaVMAttachArgs args;
		args.version = JNI_VERSION_1_6;
		args.name = NULL;
		args.group = NULL;

		jvm->AttachCurrentThread(&env, &args);

		jstring packageName = env->NewStringUTF("com.beatgames.beatsaber");

		// // Get Context Shit (courtesy of sc2bad)
		// GET_JCLASS(env, activityThreadClass, "android/app/ActivityThread", jclass);
		// GET_JCLASS(env, ActivityClass, "android/content/Context", NOTHING);

		// CALL_STATIC_JOBJECT_METHOD(env, activityThread, activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;", jobject);
		// CALL_JOBJECT_METHOD(env, AppActivity, activityThread, "getApplication", "()Landroid/app/Application;", NOTHING);

		// Get Activity Shit (courtesy of NOT sc2bad)
		GET_JCLASS(env, unityPlayerClass, "com/unity3d/player/UnityPlayer", jclass);

		GET_STATIC_JFIELD(env, appActivity, unityPlayerClass, "currentActivity", "Landroid/app/Activity;", jobject);
		GET_JCLASS(env, activityClass, "android/app/Activity", jclass);

		// Get Package Manager
		CALL_JOBJECT_METHOD(env, packageManager, appActivity, "getPackageManager", "()Landroid/content/pm/PackageManager;", jobject);
		GET_JOBJECT_JCLASS(env, packageManagerClass, packageManager, jclass);

		// Get Intent
		GET_JCLASS(env, intentClass, "android/content/Intent", jclass);
		CALL_JOBJECT_METHOD(env, intent, packageManager, "getLaunchIntentForPackage", "(Ljava/lang/String;)Landroid/content/Intent;", jobject, packageName);

		// Set Intent Flags
		CALL_JOBJECT_METHOD(env, setFlagsSuccess, intent, "setFlags", "(I)Landroid/content/Intent;", jobject, 536870912);

		// Get Component Name
		CALL_JOBJECT_METHOD(env, componentName, intent, "getComponent", "()Landroid/content/ComponentName;", jobject);

		// Create Restart Intent
		CALL_STATIC_JOBJECT_METHOD(env, restartIntent, intentClass, "makeRestartActivityTask", "(Landroid/content/ComponentName;)Landroid/content/Intent;", jobject, componentName);

		// Restart Game
		CALL_VOID_METHOD(env, appActivity, startActivity, "(Landroid/content/Intent;)V", restartIntent);

		GET_JCLASS(env, processClass, "android/os/Process", jclass);

		CALL_STATIC_JINT_METHOD(env, pid, processClass, "myPid", "()I", jint);
		getLogger().info("-- RESTARTING --");
		CALL_STATIC_VOID_METHOD(env, processClass, killProcess, "(I)V", pid);
	}

	void CacheJVM() {
		JNIEnv* env = Modloader::getJni();
		env->GetJavaVM(&jvm);
	}

	void __attribute__((constructor)) DlOpened() {
		__android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Getting JVM...");
		CacheJVM();

		if (jvm != nullptr) __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Successfully cached JVM!");
		else __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to cache JVM, JVM is a nullptr!");
	}
}