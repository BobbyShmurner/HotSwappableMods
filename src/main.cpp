#include "main.hpp"
#include "Utils/JNIUtils.hpp"
#include "DataTypes/PublicMod.hpp"

#include <dlfcn.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include "modloader\shared\modloader.hpp"

#include "custom-types/shared/coroutine.hpp"

#include "UnityEngine/WaitForEndOfFrame.hpp"

#include "UnityEngine/AndroidJNIHelper.hpp"
#include "UnityEngine/AndroidJavaClass.hpp"
#include "UnityEngine/AndroidJavaObject.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/SharedCoroutineStarter.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static JavaVM* jvm;

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

custom_types::Helpers::Coroutine DestroyBSQ() {
    co_yield reinterpret_cast<System::Collections::IEnumerator*>(CRASH_UNLESS(UnityEngine::WaitForEndOfFrame::New_ctor()));
    HookTracker::RemoveHooks();

    co_return;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

void LogHooks() {
    static const std::unordered_map<const void*, std::list<HookInfo>>* hooks = HookTracker::GetHooks();
    int i = 0;
    for (std::pair<const void*, std::list<HookInfo>> hookPair : *hooks) {
        for (HookInfo hook : hookPair.second) {
            getLogger().info("%i: %s -> %p", i, hook.name.c_str(), hook.destination);
        }
        i++;
    }
}

void CacheJVM() {
    JNIEnv* env = Modloader::getJni();
    env->GetJavaVM(&jvm);
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
    CALL_STATIC_VOID_METHOD(env, processClass, killProcess, "(I)V", pid);
}

MAKE_HOOK_MATCH(testHookOfPureGamer, &GlobalNamespace::NoteController::Init, void, GlobalNamespace::NoteController* self, GlobalNamespace::NoteData* noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float endRotation, float uniformScale) {
    testHookOfPureGamer(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration,  jumpGravity, endRotation, uniformScale);
    
    RestartBS();
}

void __attribute__((constructor)) DlOpened() {
    __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Getting JVM");
    CacheJVM();

    if (jvm != nullptr) __android_log_print(ANDROID_LOG_VERBOSE, "HotSwappableMods", "Successfully cached JVM!");
    else __android_log_print(ANDROID_LOG_ERROR, "HotSwappableMods", "Failed to cache JVM, JVM is a nullptr!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), testHookOfPureGamer);
    getLogger().info("Installed all hooks!");

    std::string modPath = string_format("/sdcard/Android/data/%s/files/mods/", Modloader::getApplicationId().c_str());
    getLogger().info("Mod Path: %s", modPath.c_str());

    DIR* dir = opendir(modPath.c_str());
    dirent* dp;
    if (dir == nullptr) {
        getLogger().info("Couldnt open the mod dir \"%s\"", modPath.c_str());
    } else {
        while ((dp = readdir(dir)) != NULL) {
            if (!strcmp(dp->d_name, "libBetterSaberQolors.so")) {
                rename(string_format("%s/%s", modPath.c_str(), dp->d_name).c_str(), string_format("%s/%s", modPath.c_str(), "libBetterSaberQolors.disabled").c_str());
                getLogger().info("Renamed \"%s\" To Dissabled", dp->d_name);
            } else if (!strcmp(dp->d_name, "libBetterSaberQolors.disabled")) {
                rename(string_format("%s/%s", modPath.c_str(), dp->d_name).c_str(), string_format("%s/%s", modPath.c_str(), "libBetterSaberQolors.so").c_str());
                getLogger().info("Renamed \"%s\" To Enabled", dp->d_name);
            }
        }
        closedir(dir);
    }
}