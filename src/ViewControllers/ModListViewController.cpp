#include "ViewControllers/ModListViewController.hpp"

#include "Utils/BobbyUtils.hpp"
#include "Utils/ModUtils.hpp"
#include "Utils/HiddenModConfigUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/Toggle_ToggleEvent.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "HMUI/ScrollView.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/Touchable.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/NoTransitionsButton.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "GlobalNamespace/BoolSettingsController.hpp"

#include "Polyglot/LocalizedTextMeshProUGUI.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include <stdlib.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>

using namespace GlobalNamespace;
using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;
using namespace HMUI;
using namespace TMPro;
using namespace Polyglot;

extern ModInfo modInfo;

DEFINE_TYPE(HotSwappableMods, ModListViewController);

HMUI::NoTransitionsButton* BackButton;

std::unordered_map<std::string, bool>* modsEnabled = new std::unordered_map<std::string, bool>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::list<std::string>* modsToToggle = new std::list<std::string>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* coreModText;
TMPro::TextMeshProUGUI* coreModDesc;

UnityEngine::Color GetTextColor(bool isCurrentlyEnabled, bool toggleValue, bool isHiddenMod, bool isModLoaded) {
	if (isCurrentlyEnabled != toggleValue) {
			if (isHiddenMod) {
				if (!toggleValue) { // Is currently on, but will be disabled after restart
					return {1.0f, 0.0f, 0.0f, 1.0f};
				} else { // Is currently off, but will be enabled after restart
					return {0.0f, 1.0f, 0.0f, 1.0f};
				}
			}
			return {1.0f, 1.0f, 0.0f, 1.0f}; // Mod Will Change after restart
		}
		else {
			if (!toggleValue) { 
				if (isHiddenMod) return {1.0f, 0.0f, 0.0f, 1.0f}; // Is currently off, SHOULD be turned on tho
				else return {1.0f, 1.0f, 1.0f, 1.0f};
			} else { // Is currently on, lets leave it that way xD
				if (isModLoaded) return {1.0f, 1.0f, 1.0f, 1.0f};
				else return {1.0f, 0.5f, 0.0f, 1.0f};
			}
		}

	return {1.0f, 1.0f, 1.0f, 1.0f};
}

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, bool isActive, bool isHiddenMod) {
	std::string fileName = ModUtils::GetFileName(toggleName);

	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, isHiddenMod, fileName](bool value){
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color({1.0f, 1.0f, 0.0f, 1.0f});

		getLogger().info("Reached This Point!");	

		textMesh->set_color(GetTextColor(modsEnabled->at(fileName), value, isHiddenMod, ModUtils::IsModLoaded(fileName)));

		if (value != modsEnabled->at(fileName)) modsToToggle->emplace_front(fileName);
		else modsToToggle->remove(fileName);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(modsEnabled->at(fileName), modsEnabled->at(fileName), isHiddenMod, ModUtils::IsModLoaded(fileName)));
}

void ClearModToggles() {
	for (std::pair<std::string, UnityEngine::UI::Toggle*> togglePair : *modToggles) {
		GameObject* tmpToggle = togglePair.second->get_transform()->get_parent()->get_gameObject();
		if (tmpToggle != nullptr) GameObject::Destroy(tmpToggle);
	}

	modToggles->clear();
}

void PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<std::string, bool>* mods, bool isHiddenMods) {
	std::list<std::string> modsToHide = HiddenModConfigUtils::GetHiddenModsList();

	for (std::pair<std::string, bool> mod : *mods) {
		std::string toggleName = ModUtils::GetModName(mod.first);
		if (ModUtils::IsFileName(toggleName)) toggleName = ModUtils::GetLibName(toggleName);

		if (std::find(modsToHide.begin(), modsToHide.end(), ModUtils::GetLibName(mod.first)) != modsToHide.end()){
			if (!isHiddenMods) continue;
		} else {
			if (isHiddenMods) continue;
		}
		
		CreateModToggle(container, toggleName, mod.second, isHiddenMods);
	}
}

void PopulateModsEnabledMap() {
	modsEnabled->clear();

	for (std::string fileName : ModUtils::GetDirContents("/sdcard/Android/data/com.beatgames.beatsaber/files/mods/")) {
		if (!ModUtils::IsFileName(fileName)) continue;

		if (ModUtils::IsDisabled(fileName)) modsEnabled->emplace(fileName, false);
		else modsEnabled->emplace(fileName, true);
	}
}

void HotSwappableMods::ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (firstActivation) {
		mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
		
		UnityEngine::Transform* scrollViewParentTrans = mainContainer->get_transform()->get_parent()->get_parent()->get_parent();
		scrollViewParentTrans->GetComponent<UnityEngine::RectTransform*>()->set_anchorMin({0, 0.1f});

		// Dont wanna use the clear functions, as it will lead to nullptr derefrences. So we just manually clear them
		modToggles->clear();
		modsEnabled->clear();

		// Same idea with the text
		modText = nullptr;
		coreModText = nullptr;
		coreModDesc = nullptr;
	} else {
		if (modText != nullptr) 	{ GameObject::Destroy(modText->get_gameObject()); modText = nullptr; }
		if (coreModText != nullptr)	{ GameObject::Destroy(coreModText->get_gameObject()); coreModText = nullptr; }
		if (coreModDesc != nullptr)	{ GameObject::Destroy(coreModDesc->get_gameObject()); coreModDesc = nullptr; }

		ClearModToggles();
	}

	ModUtils::GetOddLibNames();
	ModUtils::UpdateAlwaysDisplayLibNames(getMainConfig().AlwaysShowFileNames.GetValue());

	PopulateModsEnabledMap();

	BackButton = GameObject::Find(il2cpp_utils::newcsstr("BackButton"))->GetComponent<HMUI::NoTransitionsButton*>();

	// Mod List

	modText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	modText->set_fontSize(10.0f);
	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	PopulateModToggles(mainContainer->get_transform(), modsEnabled, false);

	// Core Mods

	if (getMainConfig().ShowCoreMods.GetValue()) {
		coreModText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Core Mods", false);
		coreModText->set_fontSize(10.0f);
		coreModText->set_alignment(TMPro::TextAlignmentOptions::Top); // This actually positions it at the bottom. Dont ask me why
		coreModText->set_color({1.0f, 0.0f, 0.0f, 1.0f});

		coreModDesc = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Be very careful when messing with these!", false);
		coreModDesc->set_alignment(TMPro::TextAlignmentOptions::Center);
		coreModDesc->set_color({1.0f, 0.0f, 0.0f, 1.0f});

		PopulateModToggles(mainContainer->get_transform(), modsEnabled, true);
	}

	getLogger().info("Loaded UI!");

	if (!firstActivation) return;

	// Restart Button

	UnityEngine::UI::Button* restartButton = QuestUI::BeatSaberUI::CreateUIButton(get_transform(), "Restart", {"ApplyButton"}, [](){
		ModUtils::SetModsActive(modsToToggle);
        ModUtils::RestartBS();
	});

	restartButton->get_transform()->set_position({0, 0.2f, 4.35f});
}

void ClearModsToToggle() {
	modsToToggle->clear();
}