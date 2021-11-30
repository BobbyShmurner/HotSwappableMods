#include "main.hpp"
#include "Utils/JNIUtils.hpp"
#include "Utils/ModUtils.hpp"
#include "DataTypes/PublicMod.hpp"

#include "DataTypes/MainConfig.hpp"

#include "ViewControllers/ModListViewController.hpp"
#include "ViewControllers/SettingsViewController.hpp"

#include <dlfcn.h>
#include <string.h>
#include <dirent.h>
#include <list>

#include "questui/shared/QuestUI.hpp"

#include "modloader\shared\modloader.hpp"

#include "custom-types/shared/coroutine.hpp"

#include "UnityEngine/WaitForEndOfFrame.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/AndroidJNIHelper.hpp"
#include "UnityEngine/AndroidJavaClass.hpp"
#include "UnityEngine/AndroidJavaObject.hpp"

#include "HMUI/NoTransitionsButton.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/SharedCoroutineStarter.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

static JavaVM* jvm;

extern HMUI::NoTransitionsButton* BackButton;
extern std::list<std::string>* modsToToggle;

DEFINE_CONFIG(MainConfig);

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

// Used for restart test
MAKE_HOOK_MATCH(RestartTest, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    RestartTest(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    RestartBS();
}

MAKE_HOOK_MATCH(OnBackButton, &UnityEngine::UI::Button::Press, void, UnityEngine::UI::Button* self) {
    OnBackButton(self);

    if (((UnityEngine::UI::Button*)BackButton) == self) {
        if (modsToToggle->size() == 0) return;
        ModUtils::SetModsActive(modsToToggle);
        RestartBS();
    }
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
    getMainConfig().Init(modInfo);

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), OnBackButton);
    getLogger().info("Installed all hooks!");

    getLogger().info("Setting Up QuestUI...");
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController<HotSwappableMods::SettingsViewController*>(modInfo);
    QuestUI::Register::RegisterMainMenuModSettingsViewController<HotSwappableMods::ModListViewController*>(modInfo);
    getLogger().info("Setup QuestUI!");
}