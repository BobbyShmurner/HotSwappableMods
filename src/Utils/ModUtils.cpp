#include "Utils/ModUtils.hpp"
#include "Utils/JNIUtils.hpp"
#include "Utils/HiddenModConfigUtils.hpp"

#include "UnityEngine/Application.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"

#include <dlfcn.h>
#include <dirent.h>
#include <stdio.h>
#include <sstream>
#include <fstream>

Logger& getLogger();

const char* ModUtils::m_ModPath;
const char* ModUtils::m_LibPath;
const char* ModUtils::m_GameVersion;

std::list<std::string>* ModUtils::m_OddLibNames;
std::list<std::string>* ModUtils::m_CoreMods;
std::list<std::string>* ModUtils::m_LoadedMods;

std::unordered_map<std::string, std::string>* ModUtils::m_ModVersions;

JavaVM* ModUtils::m_Jvm;

std::list<std::string> ModUtils::GetDirContents(std::string dirPath) {
	DIR* dir = opendir(dirPath.c_str());
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

void ModUtils::SetModActive(std::string name, bool active) {
	getLogger().info("%s mod \"%s\"", active ? "Enabling" : "Disabling", GetLibName(name).c_str());

	std::string path;
	if (IsModALibrary(name)) path = m_LibPath;
	else path = m_ModPath;

	std::list<std::string> fileNames = GetDirContents(path);

	for (std::string fileName : fileNames) {
		if (!IsFileName(fileName)) continue;
		if (strcmp(fileName.c_str(), GetFileName(name).c_str())) continue;

		if (active) rename(string_format("%s/%s", path.c_str(), fileName.c_str()).c_str(), string_format("%s/%s.so", path.c_str(), GetLibName(name).c_str()).c_str());
		else rename(string_format("%s/%s", path.c_str(), fileName.c_str()).c_str(), string_format("%s/%s.disabled", path.c_str(), GetLibName(name).c_str()).c_str());

		return;
	}
}

void ModUtils::SetModsActive(std::list<std::string>* mods, bool active) {
	getLogger().info("%s a list of mods", active ? "Enabling" : "Disabling");

	for (std::string modFileName : *mods) {
		SetModActive(modFileName, active);
	}
}

void ModUtils::ToggleMod(std::string name) {
	SetModActive(name, IsDisabled(name));
}

void ModUtils::ToggleMods(std::list<std::string>* mods) {
	getLogger().info("Toggling a list of mods");

	for (std::string modFileName : *mods) {
		ToggleMod(modFileName);
	}
}

bool ModUtils::IsDisabled(std::string name) {
	std::string fileName = GetFileName(name);

	return fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled");
}

bool ModUtils::IsOddLibName(std::string name) {
	return (std::find(m_OddLibNames->begin(), m_OddLibNames->end(), name) != m_OddLibNames->end());
}

bool ModUtils::IsModLoaded(std::string name) {
	return (std::find(m_LoadedMods->begin(), m_LoadedMods->end(), GetFileName(name)) != m_LoadedMods->end());;
}

bool ModUtils::IsCoreMod(std::string name) {
	return (std::find(m_CoreMods->begin(), m_CoreMods->end(), GetFileName(name)) != m_CoreMods->end());
}

bool ModUtils::IsModALibrary(std::string name) {
	std::string fileName = GetFileName(name);
	std::list<std::string> libFiles = GetDirContents(m_LibPath);

	return (std::find(libFiles.begin(), libFiles.end(), fileName) != libFiles.end());
}

// Mod Name = Mod Name
// Lib Name = libmodname
// File Name = libmodname.so / libmodname.disabled

// Mod Names, Lib Names and File Names can all convert between eachother

// Name Tests

bool ModUtils::IsModID(std::string name) {
	std::string fileName = GetFileName(name);
	std::unordered_map<std::string, const Mod> mods = Modloader::getMods();

	for (std::pair<std::string, const Mod> modPair : mods) {
		if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return true;
	}

	return false;
}

bool ModUtils::IsLibName(std::string name) {
	return ((!IsFileName(name)) && (name.length() > 3 && !strcmp(name.substr(0, 3).c_str(), "lib"))) || IsOddLibName(name);
}

bool ModUtils::IsFileName(std::string name) {
	return (name.length() > 9 && !strcmp(name.substr(name.size() - 9).c_str(), ".disabled")) || (name.length() > 3 && !strcmp(name.substr(name.size() - 3).c_str(), ".so"));
}

// Name Conversions

std::string ModUtils::GetModID(std::string name) {
	std::string fileName = GetFileName(name);
	std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
	
	for (std::pair<std::string, const Mod> modPair : mods) {
		if (!strcmp(fileName.c_str(), modPair.second.name.c_str())) return modPair.first;
	}

	return GetLibName(name);
}

std::string ModUtils::GetLibName(std::string name) {
	if (IsLibName(name)) return name;

	std::string fileName;

	if (IsFileName(name)) fileName = name;
	else fileName = GetFileNameFromModID(name);

	if (fileName == "Null") fileName = GetFileNameFromDir(name);
	if (fileName == "Null") return "Null";

	if (IsDisabled(fileName)) return fileName.substr(0, fileName.size() - 9);
	else return fileName.substr(0, fileName.size() - 3);
}

std::string ModUtils::GetFileName(std::string name) {
	if (IsFileName(name)) return name;
	std::string libName;

	if (IsLibName(name)) libName = name;
	else libName = GetLibName(name);

	return GetFileNameFromDir(libName);
}

std::string ModUtils::GetModVersion(std::string name) {
	std::string fileName = GetFileName(name);
	if (m_ModVersions->find(fileName) == m_ModVersions->end()) return "Unknown";

	return m_ModVersions->at(fileName);
}

std::list<std::string> ModUtils::GetLoadedModsFileNames() {
	return *m_LoadedMods;
}

std::list<std::string> ModUtils::GetCoreMods() {
	return *m_CoreMods;
}

std::list<std::string> ModUtils::GetOddLibNames() {
	return *m_OddLibNames;
}

// Thanks for Laurie for the original code snippet: 
std::optional<std::string> ModUtils::GetModError(std::string name) {
	std::string fileName = GetFileName(name);
	std::string filePath = Modloader::getDestinationPath() + fileName;
	
	dlerror(); // Clear Existing Errors
	dlopen(filePath.c_str(), RTLD_LOCAL | RTLD_NOW);

	char* error = dlerror();
	return error ? std::optional(std::string(error).substr(15)) : std::nullopt;
}

std::string ModUtils::GetModsFolder() {
	return m_ModPath;
}

std::string ModUtils::GetLibsFolder() {
	return m_LibPath;
}

std::string ModUtils::GetGameVersion() {
	return m_GameVersion;
}

JNIEnv* ModUtils::GetJNIEnv() {
	JNIEnv* env;

	JavaVMAttachArgs args;
	args.version = JNI_VERSION_1_6;
	args.name = NULL;
	args.group = NULL;

	m_Jvm->AttachCurrentThread(&env, &args);

	return env;
}

void ModUtils::RestartBS() {
	getLogger().info("-- STARTING RESTART --");

	JNIEnv* env = GetJNIEnv();

	jstring packageName = env->NewStringUTF("com.beatgames.beatsaber");

	// // Get Context Shit (courtesy of sc2bad)
	// GET_JCLASS(env, activityThreadClass, "android/app/ActivityThread", jclass);
	// GET_JCLASS(env, ActivityClass, "android/content/Context", NOTHING);

	// CALL_STATIC_JOBJECT_METHOD(env, activityThread, activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;", jobject);
	// CALL_JOBJECT_METHOD(env, AppActivity, activityThread, "getApplication", "()Landroid/app/Application;", NOTHING);

	// Get Activity Shit (courtesy of NOT sc2bad (cus he's bad))
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
	CALL_STATIC_VOID_METHOD(env, processClass, killProcess, "(I)V", pid);
}

bool ModUtils::RemoveDuplicateMods() {
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);
	std::list<std::string> libFileNames = GetDirContents(m_LibPath);

	std::list<std::string> fileNames = modFileNames;
	fileNames.merge(libFileNames);

	bool removedDuplicate = false;

	for (std::string file : fileNames) {
		if (!IsDisabled(file)) continue;

		std::string enabledName = GetLibName(file) + ".so";
		if (std::find(fileNames.begin(), fileNames.end(), enabledName) != fileNames.end()) {
			std::string path = m_ModPath;
			if (IsModALibrary(file)) path = m_LibPath;

			remove(string_format("%s/%s", path.c_str(), file.c_str()).c_str());
			getLogger().info("Removed Duplicated File \"%s\"", file.c_str());

			removedDuplicate = true;
		}
	}

	return removedDuplicate;
}

void ModUtils::CacheJVM() {
	JNIEnv* env = Modloader::getJni();
	env->GetJavaVM(&m_Jvm);
}

void ModUtils::CollectCoreMods() {
	std::ifstream coreModsFile("/sdcard/BMBFData/core-mods.json");
	std::stringstream coreModsSS;
	coreModsSS << coreModsFile.rdbuf();

	rapidjson::Document coreModsDoc;
	coreModsDoc.Parse(coreModsSS.str());

	getLogger().info("Collecting Core Mods...");

	if (coreModsDoc.HasMember(m_GameVersion)) {
		const rapidjson::Value& versionInfo = coreModsDoc[m_GameVersion];
		const rapidjson::Value& coreModsList = versionInfo["mods"];

		for (rapidjson::SizeType i = 0; i < coreModsList.Size(); i++) { // rapidjson uses SizeType instead of size_t.
			const rapidjson::Value& coreModInfo = coreModsList[i];

			std::string fileName = GetFileName(coreModInfo["id"].GetString());

			m_CoreMods->emplace_front(fileName);
			getLogger().info("Found Core mod %s", fileName.c_str());
		}
	} else {
		getLogger().info("ERROR! No Core Mods Found For This Version!");
	}

	getLogger().info("Finished Collecting Core Mods!");
}

void ModUtils::CollectLoadedMods() {
	for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
		m_LoadedMods->emplace_front(modPair.second.name);
	}

	// As Modloader only keeps track of loaded mods, not libs, we have to collect the ourself
	for (std::string fileName : GetDirContents(m_LibPath)) {
		if (GetModError(fileName) == std::nullopt) m_LoadedMods->emplace_front(fileName);
	}
}

void ModUtils::CollectModVersions() {
	for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
		m_ModVersions->emplace(modPair.second.name, modPair.second.info.version);
	}
}

void ModUtils::CollectOddLibs() {
	m_OddLibNames->clear();
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);
	std::list<std::string> libFileNames = GetDirContents(m_LibPath);

	std::list<std::string> fileNames = modFileNames;
	fileNames.merge(libFileNames);

	for (std::string fileName : fileNames) {
		if (!IsFileName(fileName)) continue;

		if (strcmp(fileName.substr(0, 3).c_str(), "lib")) {
			getLogger().info("Mod \"%s\" does not start with \"lib\"! Adding to m_OddLibNames", fileName.c_str());

			if (IsDisabled(fileName)) m_OddLibNames->emplace_front(fileName.substr(0, fileName.size() - 9));
			else m_OddLibNames->emplace_front(fileName.substr(0, fileName.size() - 3));
		}
	}
}

std::string ModUtils::GetFileNameFromDir(std::string libName, bool guessLibName) {
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);
	std::list<std::string> libFileNames = GetDirContents(m_LibPath);

	std::list<std::string> fileNames = modFileNames;
	fileNames.merge(libFileNames);

	if (guessLibName) libName = "lib" + libName;
		
	for (std::string fileName : fileNames) {
		if (!strcmp(GetLibName(fileName).c_str(), libName.c_str())) return fileName;
	}

	if (!guessLibName) {
		return GetFileNameFromDir(libName, true); // Just try gussing it xD
	}

	getLogger().info("Failed to get file name for \"%s\"!", libName.c_str());
	return {"Null"};
}

std::string ModUtils::GetFileNameFromModID(std::string modID) {
	if (!Modloader::getMods().contains(modID)) return {"Null"};

	return Modloader::getMods().at(modID).name;
}

void ModUtils::Init() {
	m_CoreMods = new std::list<std::string>();
	m_OddLibNames = new std::list<std::string>();
	m_LoadedMods = new std::list<std::string>();
	m_ModVersions = new std::unordered_map<std::string, std::string>();

	m_ModPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
	m_LibPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/libs/";

	m_GameVersion = to_utf8(csstrtostr(UnityEngine::Application::get_version())).c_str();
	
	CollectLoadedMods();
	CollectModVersions();
	CollectOddLibs();
	CollectCoreMods();
}

void __attribute__((constructor)) ModUtils::OnDlopen() {
	__android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Getting m_Jvm...");
	CacheJVM();

	if (m_Jvm != nullptr) __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Successfully cached m_Jvm!");
	else __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to cache m_Jvm, m_Jvm is a nullptr!");
}