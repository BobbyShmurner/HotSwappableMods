#include "Utils/ModUtils.hpp"
#include "Utils/JNIUtils.hpp"

#include "Utils/HiddenModConfigUtils.hpp"

#include <dlfcn.h>
#include <dirent.h>

Logger& getLogger();

const char* ModUtils::m_ModPath;
std::list<std::string>* ModUtils::m_OddLibNames;
std::list<std::string>* ModUtils::m_CoreMods;
std::list<std::string>* ModUtils::m_LoadedMods;

JavaVM* ModUtils::m_Jvm;

std::list<std::string> ModUtils::GetDirContents(const char* dirPath) {
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

void ModUtils::ToggleMod(std::string name) {
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);

	for (std::string modFileName : modFileNames) {
		if (!IsFileName(modFileName)) continue;
		if (strcmp(modFileName.c_str(), GetFileName(name).c_str())) continue;

		if (IsDisabled(modFileName)) rename(string_format("%s/%s", m_ModPath, modFileName.c_str()).c_str(), string_format("%s/%s.so", m_ModPath, GetLibName(name).c_str()).c_str());
		else rename(string_format("%s/%s", m_ModPath, modFileName.c_str()).c_str(), string_format("%s/%s.disabled", m_ModPath, GetLibName(name).c_str()).c_str());
	}
}

void ModUtils::SetModsActive(std::list<std::string>* mods) {
	getLogger().info("Setting mod activites");
	for (std::string modFileName : *mods) {
		std::string modName = GetModID(modFileName);

		getLogger().info("Setting Activity For \"%s\"", modName.c_str());
		ToggleMod(modName);
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
	return (std::find(m_LoadedMods->begin(), m_LoadedMods->end(), GetFileName(name)) != m_LoadedMods->end());
}

bool ModUtils::IsCoreMod(std::string name) {
	return (std::find(m_CoreMods->begin(), m_CoreMods->end(), GetLibName(name)) != m_CoreMods->end());
}

// Mod Name = Mod Name
// Lib Name = libmodname
// File Name = libmodname.so / libmodname.disabled

// Mod Names, Lib Names and File Names can all convert between eachother

// Name Tests

bool ModUtils::IsModID(std::string name) {
	return !IsLibName(name) && !IsFileName(name);
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

void ModUtils::CacheJVM() {
	JNIEnv* env = Modloader::getJni();
	env->GetJavaVM(&m_Jvm);
}

void ModUtils::CollectCoreMods() {
	// TODO: Get Core Mods From BMBF, and phase out HiddenModConfigUtils
}

void ModUtils::CollectLoadedMods() {
	for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
		m_LoadedMods->emplace_front(modPair.second.name);
	}
}

void ModUtils::CollectOddLibs() {
	m_OddLibNames->clear();
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);

	for (std::string modFileName : modFileNames) {
		if (!IsFileName(modFileName)) continue;

		if (strcmp(modFileName.substr(0, 3).c_str(), "lib")) {
			getLogger().info("Mod \"%s\" does not start with \"lib\"! Adding to m_OddLibNames", modFileName.c_str());

			if (IsDisabled(modFileName)) m_OddLibNames->emplace_front(modFileName.substr(0, modFileName.size() - 9));
			else m_OddLibNames->emplace_front(modFileName.substr(0, modFileName.size() - 3));
		}
	}
}

std::string ModUtils::GetFileNameFromDir(std::string libName) {
	std::list<std::string> modFileNames = GetDirContents(m_ModPath);

	for (std::string modFileName : modFileNames) {
		if (!strcmp(GetLibName(modFileName).c_str(), libName.c_str())) return modFileName;
	}

	return {"Null"};
}

std::string ModUtils::GetFileNameFromModID(std::string modID) {
	return Modloader::getMods().at(modID).name;
}

void ModUtils::Init() {
	m_OddLibNames = new std::list<std::string>();
	m_LoadedMods = new std::list<std::string>();
	m_ModPath = "/sdcard/Android/data/com.beatgames.beatsaber/files/mods/";
	m_CoreMods = HiddenModConfigUtils::GetCoreMods();
	
	CollectLoadedMods();
}

void __attribute__((constructor)) ModUtils::OnDlopen() {
	__android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Getting m_Jvm...");
	CacheJVM();

	if (m_Jvm != nullptr) __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Successfully cached m_Jvm!");
	else __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to cache m_Jvm, m_Jvm is a nullptr!");
}