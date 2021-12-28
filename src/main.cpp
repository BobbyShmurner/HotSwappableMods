#include "main.hpp"

#include "modloader-utils/shared/ModloaderUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"
#include "UI/ViewControllers/SettingsViewController.hpp"

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

#include "Zenject/DiContainer.hpp"

#include "System/Action_1.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/SharedCoroutineStarter.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

extern bool ShouldRefreshList;
extern bool ShouldClearSettingsList;

extern HMUI::NoTransitionsButton* BackButton;
extern std::list<std::string>* modsToToggle;

extern UnityEngine::GameObject* SeperatorTemplate;

std::list<std::string> NoNoMods = { "libHotSwappableMods", "libbeatsaber-hook_3_3_4", "libbeatsaber-hook_3_4_2", "libbeatsaber-hook_3_4_4", "libcodegen", "libcustom-types", "libquestui" }; // These cant be disabled no matter what

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

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
	std::string modId = ID;

	// Thanks Laurie 👍 https://imgur.com/a/u6u0tXC
	srand(time(0));
	int randNo = rand() % 100;

	if (randNo == 0) {
		modId = "speedymodswapper2000omatic";
	}

	info.id = modId;
	info.version = VERSION;
	modInfo = info;
	
	getConfig().Load(); // Load the config file
	getLogger().info("Completed setup!");
	getLogger().info("super sekrit noomber: %i", randNo);
}

MAKE_HOOK_MATCH(OnSoftRestart, &GlobalNamespace::MenuTransitionsHelper::RestartGame, void, GlobalNamespace::MenuTransitionsHelper* self, System::Action_1<Zenject::DiContainer*>* finishCallback) {
	OnSoftRestart(self, finishCallback);

	ShouldRefreshList = true;
	ShouldClearSettingsList = true;
	
	SeperatorTemplate = nullptr;
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
	il2cpp_functions::Init();
	getMainConfig().Init(modInfo);
	ModloaderUtils::Init();

	getLogger().info("Installing hooks...");
	INSTALL_HOOK(getLogger(), OnSoftRestart);
	getLogger().info("Installed all hooks!");

	if (getMainConfig().RemoveDuplicatesAtStartup.GetValue()) {
		getLogger().info("Checking for duplicate files...");

		if (ModloaderUtils::RemoveDuplicateMods()) {
			getLogger().info("Duplicate Mods found! Restarting...");
			ModloaderUtils::RestartGame();
		} else {
			getLogger().info("No Duplicate Mods Found");
		}
	}

	getLogger().info("Setting Up QuestUI...");
	QuestUI::Init();
	QuestUI::Register::RegisterModSettingsViewController<HotSwappableMods::SettingsViewController*>(modInfo);
	QuestUI::Register::RegisterMainMenuModSettingsFlowCoordinator<HotSwappableMods::ModListFlowCoordinator*>(modInfo);
	getLogger().info("Setup QuestUI!");
}