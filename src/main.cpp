#include "main.hpp"

#include "Utils/ModUtils.hpp"

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

ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

extern HMUI::NoTransitionsButton* BackButton;
extern std::list<std::string>* modsToToggle;

std::list<std::string> NoNoMods = { "libHotSwappableMods", "libmodutils" }; // These cant be disabled no matter what

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
}

MAKE_HOOK_MATCH(OnBackButton, &UnityEngine::UI::Button::Press, void, UnityEngine::UI::Button* self) {
	OnBackButton(self);

	if (((UnityEngine::UI::Button*)BackButton) == self) {
		if (modsToToggle->size() == 0) return;
		ClearModsToToggle(); // This clears the mods to enable/disable list in the ModListViewController
	}
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
	il2cpp_functions::Init();
	getMainConfig().Init(modInfo);
	ModUtils::Init();

	getLogger().info("Installing hooks...");
	INSTALL_HOOK(getLogger(), OnBackButton);
	getLogger().info("Installed all hooks!");

	getLogger().info("Setting Up QuestUI...");
	QuestUI::Init();
	QuestUI::Register::RegisterModSettingsViewController<HotSwappableMods::SettingsViewController*>(modInfo);
	QuestUI::Register::RegisterMainMenuModSettingsViewController<HotSwappableMods::ModListViewController*>(modInfo);
	getLogger().info("Setup QuestUI!");

	if (getMainConfig().RemoveDuplicatesAtStartup.GetValue()) {
		getLogger().info("Checking for duplicate files...");

		if (ModUtils::RemoveDuplicateMods()) {
			getLogger().info("Duplicate Mods found! Restarting...");
			ModUtils::RestartBS();
		} else {
			getLogger().info("No Duplicate Mods Found");
		}
	}
}