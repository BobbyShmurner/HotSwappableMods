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
#include "HMUI/HoverHint.hpp"
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

std::unordered_map<std::string, bool>* modsEnabled = new std::unordered_map<std::string, bool>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::list<std::string>* modsToToggle = new std::list<std::string>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* noModsText;
TMPro::TextMeshProUGUI* coreModText;
TMPro::TextMeshProUGUI* coreModDesc;

HMUI::NoTransitionsButton* BackButton;
UnityEngine::UI::Button* restartButton;
UnityEngine::UI::Button* cancelButton;

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

void GenerateModHoverHint(UnityEngine::GameObject* go, std::string fileName) {
	std::string hoverMessage = "";

	hoverMessage += string_format("File Name - %s", fileName.c_str());

	if (!ModUtils::IsDisabled(fileName)) {
		if (ModUtils::IsModLoaded(fileName)) hoverMessage += string_format("\nMod ID - %s", ModUtils::GetModID(fileName).c_str());
		else hoverMessage += string_format("\nFailed To Load! Reason - %s", ModUtils::GetModError(fileName)->c_str());
	}

	HMUI::HoverHint* hoverHint = QuestUI::BeatSaberUI::AddHoverHint(go, std::string_view(hoverMessage));
}

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, bool isActive, bool isHiddenMod) {
	std::string fileName = ModUtils::GetFileName(toggleName);

	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, isHiddenMod, fileName](bool value){
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color(GetTextColor(modsEnabled->at(fileName), value, isHiddenMod, ModUtils::IsModLoaded(fileName)));

		if (value != modsEnabled->at(fileName)) modsToToggle->emplace_front(fileName);
		else modsToToggle->remove(fileName);

		restartButton->set_interactable(modsToToggle->size() != 0);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(modsEnabled->at(fileName), modsEnabled->at(fileName), isHiddenMod, ModUtils::IsModLoaded(fileName)));
	textMesh->get_gameObject()->AddComponent(csTypeOf(HMUI::Touchable*));

	BobbyUtils::LogComponentHierarchy(newToggle->get_gameObject(), 1);
	BobbyUtils::LogComponents(textMesh->get_gameObject());
	GenerateModHoverHint(newToggle->get_transform()->get_parent()->get_gameObject(), fileName);
}

void ClearModToggles() {
	for (std::pair<std::string, UnityEngine::UI::Toggle*> togglePair : *modToggles) {
		GameObject* tmpToggle = togglePair.second->get_transform()->get_parent()->get_gameObject();
		if (tmpToggle != nullptr) GameObject::Destroy(tmpToggle);
	}

	modToggles->clear();
}

int PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<std::string, bool>* mods, bool isHiddenMods) {
	std::list<std::string> modsToHide = HiddenModConfigUtils::GetHiddenModsList();
	std::list<std::string> noNoMods = HiddenModConfigUtils::GetNoNoModsList();

	int togglesCreated = 0;

	for (std::pair<std::string, bool> mod : *mods) {
		if (std::find(noNoMods.begin(), noNoMods.end(), ModUtils::GetLibName(mod.first)) != noNoMods.end()) continue;

		std::string toggleName = ModUtils::GetModName(mod.first);
		if (ModUtils::IsFileName(toggleName)) toggleName = ModUtils::GetLibName(toggleName);

		if (std::find(modsToHide.begin(), modsToHide.end(), ModUtils::GetLibName(mod.first)) != modsToHide.end()){
			if (!isHiddenMods) continue;
		} else {
			if (isHiddenMods) continue;
		}
		
		CreateModToggle(container, toggleName, mod.second, isHiddenMods);
		togglesCreated++;
	}

	return togglesCreated;
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
		if (noModsText != nullptr)	{ GameObject::Destroy(noModsText->get_gameObject()); noModsText = nullptr; }

		ClearModToggles();
		restartButton->set_interactable(false);
	}

	ModUtils::GetOddLibNames();
	ModUtils::UpdateAlwaysDisplayLibNames(getMainConfig().AlwaysShowFileNames.GetValue());

	PopulateModsEnabledMap();

	BackButton = GameObject::Find(il2cpp_utils::newcsstr("BackButton"))->GetComponent<HMUI::NoTransitionsButton*>();

	// Mod List

	modText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	modText->set_fontSize(10.0f);
	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	int modCount = PopulateModToggles(mainContainer->get_transform(), modsEnabled, false);

	if (modCount == 0) {
		noModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "No Mods Found!", false);
		noModsText->set_alignment(TMPro::TextAlignmentOptions::Center);
	}

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

	if (!firstActivation) return;

	// Bottom Pannel

	UnityEngine::UI::HorizontalLayoutGroup* bottomPannel = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(get_transform());

	bottomPannel->get_transform()->set_position({0, 0.2f, 4.35f});

	bottomPannel->set_spacing(2);
	bottomPannel->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

	bottomPannel->set_childForceExpandWidth(false);
	bottomPannel->set_childForceExpandHeight(false);
	bottomPannel->set_childControlWidth(false);
	bottomPannel->set_childControlHeight(false);

	bottomPannel->dyn_m_TotalMinSize() = {70, 10};
	bottomPannel->dyn_m_TotalPreferredSize() = {70, 10};

	// Cancel Button

	cancelButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Cancel", {"CancelButton"}, [](){
		BackButton->get_onClick()->Invoke();
	});

	// Restart Button

	restartButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Reload Mods", {"ApplyButton"}, [](){
		ModUtils::SetModsActive(modsToToggle);
        ModUtils::RestartBS();
	});
	restartButton->set_interactable(false);
}

void ClearModsToToggle() {
	modsToToggle->clear();
}