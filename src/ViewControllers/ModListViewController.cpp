#include "ViewControllers/ModListViewController.hpp"

#include "Utils/BobbyUtils.hpp"
#include "modloader-utils/shared/ModloaderUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/UI/Image.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "UnityEngine/UI/Toggle_ToggleEvent.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "HMUI/ScrollView.hpp"
#include "HMUI/HoverHint.hpp"
#include "HMUI/HoverHintController.hpp"
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
extern std::list<std::string> NoNoMods;

DEFINE_TYPE(HotSwappableMods, ModListViewController);

std::unordered_map<std::string, bool>* modsEnabled = new std::unordered_map<std::string, bool>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::list<std::string>* modsToToggle = new std::list<std::string>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* noModsText;
TMPro::TextMeshProUGUI* dangerZoneText;
TMPro::TextMeshProUGUI* dangerZoneDesc;
TMPro::TextMeshProUGUI* coreModsText;
TMPro::TextMeshProUGUI* libsText;


HMUI::NoTransitionsButton* BackButton;
UnityEngine::UI::Button* restartButton;
UnityEngine::UI::Button* cancelButton;

std::string GetDisplayName(std::string name) {
	if (getMainConfig().AlwaysShowFileNames.GetValue()) return ModloaderUtils::GetLibName(name);
	else return ModloaderUtils::GetModID(name);
}

UnityEngine::Color GetTextColor(bool isCurrentlyEnabled, bool toggleValue, bool isImportant, bool isModLoaded) {
	if (isCurrentlyEnabled != toggleValue) {
			if (isImportant) {
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
				if (isImportant) return {1.0f, 0.0f, 0.0f, 1.0f}; // Is currently off, SHOULD be turned on tho
				else return {1.0f, 1.0f, 1.0f, 1.0f};
			} else { // Is currently on, lets leave it that way xD
				if (isModLoaded) return {1.0f, 1.0f, 1.0f, 1.0f};
				else return {1.0f, 0.5f, 0.0f, 1.0f};
			}
		}

	return {1.0f, 1.0f, 1.0f, 1.0f};
}

void GenerateModHoverHint(UnityEngine::GameObject* toggle, UnityEngine::GameObject* text, std::string fileName) {
	if (!getMainConfig().ShowHoverHints.GetValue()) return;
	std::string hoverMessage = "";

	if (getMainConfig().ShowFileNameOnHoverHint.GetValue()) hoverMessage += string_format("File Name - %s", fileName.c_str());

	bool isCore = ModloaderUtils::IsCoreMod(fileName);
	bool isLibrary = ModloaderUtils::IsModALibrary(fileName);

	if (!ModloaderUtils::IsDisabled(fileName)) {
		if (ModloaderUtils::IsModLoaded(fileName)) {
			if (!isLibrary) {
				if (getMainConfig().ShowModIDOnHoverHint.GetValue()) 		hoverMessage += string_format("%sMod ID - %s", hoverMessage == "" ? "" : "\n", ModloaderUtils::GetModID(fileName).c_str());
				if (getMainConfig().ShowModVersionOnHoverHint.GetValue()) 	hoverMessage += string_format("%sMod Version - %s", hoverMessage == "" ? "" : "\n", ModloaderUtils::GetModVersion(fileName).c_str());
			}
		}
		else if (getMainConfig().ShowModErrorsOnHoverHint.GetValue()) hoverMessage += string_format("%sFailed To Load! Reason - %s", hoverMessage == "" ? "" : "\n", ModloaderUtils::GetModError(fileName)->c_str());
	}

	if (getMainConfig().ShowModTypeOnHoverHint.GetValue()) { 
		if ( isCore && !isLibrary)	hoverMessage += string_format("%sMod Type - Core Mod", hoverMessage == "" ? "" : "\n");
		if (!isCore &&  isLibrary)	hoverMessage += string_format("%sMod Type - Library", hoverMessage == "" ? "" : "\n");
		if ( isCore &&  isLibrary)	hoverMessage += string_format("%sMod Type - Core Mod, Library", hoverMessage == "" ? "" : "\n");
		if (!isCore && !isLibrary)	hoverMessage += string_format("%sMod Type - Mod", hoverMessage == "" ? "" : "\n");
	}

	if (hoverMessage != "") QuestUI::BeatSaberUI::AddHoverHint(toggle, std::string_view(hoverMessage));
}

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, bool isActive, bool isImportant) {
	std::string fileName = ModloaderUtils::GetFileName(toggleName);

	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, isImportant, fileName](bool value){
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color(GetTextColor(modsEnabled->at(fileName), value, isImportant, ModloaderUtils::IsModLoaded(fileName)));

		if (value != modsEnabled->at(fileName)) modsToToggle->emplace_front(fileName);
		else modsToToggle->remove(fileName);

		restartButton->set_interactable(modsToToggle->size() != 0);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(modsEnabled->at(fileName), modsEnabled->at(fileName), isImportant, ModloaderUtils::IsModLoaded(fileName)));

	GenerateModHoverHint(newToggle->get_transform()->get_parent()->get_gameObject(), textMesh->get_gameObject(), fileName);
}

void ClearModToggles() {
	for (std::pair<std::string, UnityEngine::UI::Toggle*> togglePair : *modToggles) {
		GameObject* tmpToggle = togglePair.second->get_transform()->get_parent()->get_gameObject();
		if (tmpToggle != nullptr) GameObject::Destroy(tmpToggle);
	}

	modToggles->clear();
	modsToToggle->clear();
	modsEnabled->clear();
}

int PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<std::string, bool>* mods, bool areCoreMods = false, bool areLibs = false) {
	int togglesCreated = 0;

	for (std::pair<std::string, bool> mod : *mods) {
		if (std::find(NoNoMods.begin(), NoNoMods.end(), ModloaderUtils::GetLibName(mod.first)) != NoNoMods.end()) continue;

		std::string toggleName = GetDisplayName(mod.first);
		if (ModloaderUtils::IsFileName(toggleName)) toggleName = ModloaderUtils::GetLibName(toggleName); // GetModID will return the file name is the mod isnt loaded, so just remove the file extension by getting the lib

		if (ModloaderUtils::IsCoreMod(toggleName)){
			if (!areCoreMods) continue;
		} else {
			if (areCoreMods) continue;

			if (ModloaderUtils::IsModALibrary(toggleName)){
				if (!areLibs) continue;
			} else {
				if (areLibs) continue;
			}
		}
		
		CreateModToggle(container, toggleName, mod.second, areCoreMods || areLibs);
		togglesCreated++;
	}

	return togglesCreated;
}

void PopulateModsEnabledMap() {
	modsEnabled->clear();

	std::list<std::string> modsFolder = ModloaderUtils::GetDirContents(ModloaderUtils::GetModsFolder());
	std::list<std::string> libsFolder = ModloaderUtils::GetDirContents(ModloaderUtils::GetLibsFolder());

	std::list<std::string> files = modsFolder;
	files.merge(libsFolder);

	for (std::string fileName : files) {
		if (!ModloaderUtils::IsFileName(fileName)) continue;

		if (ModloaderUtils::IsDisabled(fileName)) modsEnabled->emplace(fileName, false);
		else modsEnabled->emplace(fileName, true);
	}
}

bool CoreModModal(UnityEngine::Transform* trans) {
	std::list<std::string> coreModsDisabled = std::list<std::string>();

	for (std::string mod : *modsToToggle) {
		if ((ModloaderUtils::IsCoreMod(mod) || ModloaderUtils::IsModALibrary(mod)) && modsEnabled->at(mod)) {
			coreModsDisabled.emplace_front(mod);
		}
	}

	if (coreModsDisabled.size() == 0) return false;

	HMUI::ModalView* modal = QuestUI::BeatSaberUI::CreateModal(trans, {80, 75}, nullptr, true);
	
	UnityEngine::UI::VerticalLayoutGroup* modalLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(modal->get_transform());

	modalLayout->get_rectTransform()->set_anchorMin({0, 0.1f});
	modalLayout->get_rectTransform()->set_anchorMax({1, 1});

	modalLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
	modalLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
	
	modalLayout->set_childControlHeight(true);
	modalLayout->set_childForceExpandHeight(false);
	modalLayout->set_childControlWidth(false);
	modalLayout->set_childForceExpandWidth(true);

	TMPro::TextMeshProUGUI* modalWarnText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), {"Warning!"}, false);
	modalWarnText->set_alignment(TMPro::TextAlignmentOptions::Center);
	modalWarnText->set_color({1, 0, 0, 1});
	modalWarnText->set_fontSize(8);

	QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), {"Are you sure you wanna dissable to following mods?\n"}, false)->set_alignment(TMPro::TextAlignmentOptions::Center);

	for (std::string mod : coreModsDisabled) {
		TMPro::TextMeshProUGUI* modText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), std::string_view("- " + GetDisplayName(mod)), false);

		modText->set_alignment(TMPro::TextAlignmentOptions::Center);
		modText->set_color({1, 0, 0, 1});
	}

	UnityEngine::UI::HorizontalLayoutGroup* bottomPannel = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(modal->get_transform());

	bottomPannel->get_rectTransform()->set_pivot({0.5f, 0});
	bottomPannel->get_rectTransform()->set_anchoredPosition({0, 0});

	bottomPannel->get_rectTransform()->set_anchorMin({0, 0});
	bottomPannel->get_rectTransform()->set_anchorMax({1, 0});

	bottomPannel->set_spacing(2);
	bottomPannel->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

	bottomPannel->set_childForceExpandWidth(false);
	bottomPannel->set_childForceExpandHeight(false);
	bottomPannel->set_childControlWidth(false);
	bottomPannel->set_childControlHeight(false);

	bottomPannel->dyn_m_TotalMinSize() = {60, 5};
	bottomPannel->dyn_m_TotalPreferredSize() = {60, 5};
	
	UnityEngine::UI::Button* cancel = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Cancel", {"CancelButton"}, [&](){
		mainContainer->get_transform()->get_parent()->get_parent()->get_parent()->get_parent()->GetComponentInChildren<HMUI::ModalView*>()->Hide(true, nullptr);
	});

	UnityEngine::UI::Button* confirm = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Im Sure", {"ApplyButton"}, [&](){
		ModloaderUtils::ToggleMods(modsToToggle);
		ModloaderUtils::RestartGame();
	});

	modal->Show(true, true, nullptr);

	return true;
}

void HotSwappableMods::ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (firstActivation) {
		mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
		
		UnityEngine::Transform* scrollViewParentTrans = mainContainer->get_transform()->get_parent()->get_parent()->get_parent();
		scrollViewParentTrans->GetComponent<UnityEngine::RectTransform*>()->set_anchorMin({0, 0.1f});

		// Dont wanna use the clear functions, as it will lead to nullptr derefrences. So we just manually clear them
		modToggles->clear();
		modsToToggle->clear();
		modsEnabled->clear();

		// Same idea with the text
		modText = nullptr;
		dangerZoneText = nullptr;
		dangerZoneDesc = nullptr;
		coreModsText = nullptr;
		libsText = nullptr;
	} else {
		if (modText != nullptr) 		{ GameObject::Destroy(modText->get_gameObject()); modText = nullptr; }
		if (noModsText != nullptr)		{ GameObject::Destroy(noModsText->get_gameObject()); noModsText = nullptr; }
		if (dangerZoneText != nullptr)	{ GameObject::Destroy(dangerZoneText->get_gameObject()); dangerZoneText = nullptr; }
		if (dangerZoneDesc != nullptr)	{ GameObject::Destroy(dangerZoneDesc->get_gameObject()); dangerZoneDesc = nullptr; }
		if (coreModsText != nullptr)	{ GameObject::Destroy(coreModsText->get_gameObject()); coreModsText = nullptr; }
		if (libsText != nullptr)		{ GameObject::Destroy(libsText->get_gameObject()); libsText = nullptr; }

		ClearModToggles();
		restartButton->set_interactable(false);
	}

	ModloaderUtils::GetOddLibNames();

	PopulateModsEnabledMap();

	BackButton = GameObject::Find(il2cpp_utils::newcsstr("BackButton"))->GetComponent<HMUI::NoTransitionsButton*>();

	// Mod List

	modText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	modText->set_fontSize(10.0f);
	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	int modCount = PopulateModToggles(mainContainer->get_transform(), modsEnabled, false, false);

	if (modCount == 0) {
		noModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "No Mods Found!", false);
		noModsText->set_alignment(TMPro::TextAlignmentOptions::Center);
	}

	// Danger Zone

	if (getMainConfig().ShowCoreMods.GetValue() || getMainConfig().ShowLibs.GetValue()) {
		dangerZoneText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "The Danger Zone!", false);
		dangerZoneText->set_fontSize(12.0f);
		dangerZoneText->set_alignment(TMPro::TextAlignmentOptions::Top); // This actually positions it at the bottom. Dont ask me why
		dangerZoneText->set_color({1.0f, 0.0f, 0.0f, 1.0f});

		dangerZoneDesc = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Be very careful when messing with these!", false);
		dangerZoneDesc->set_alignment(TMPro::TextAlignmentOptions::Center);
		dangerZoneDesc->set_color({1.0f, 0.0f, 0.0f, 1.0f});
	}

	if (getMainConfig().ShowCoreMods.GetValue()) {
		coreModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Core Mods", false);
		coreModsText->set_fontSize(10.0f);
		coreModsText->set_alignment(TMPro::TextAlignmentOptions::Top);
		coreModsText->set_color({1.0f, 0.0f, 0.0f, 1.0f});

		PopulateModToggles(mainContainer->get_transform(), modsEnabled, true, false);
	}

	if (getMainConfig().ShowLibs.GetValue()) {
		libsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Libraries", false);
		libsText->set_fontSize(10.0f);
		libsText->set_alignment(TMPro::TextAlignmentOptions::Top);
		libsText->set_color({1.0f, 0.0f, 0.0f, 1.0f});

		PopulateModToggles(mainContainer->get_transform(), modsEnabled, false, true);
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
	restartButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Reload Mods", {"ApplyButton"}, [&](){
		if (CoreModModal(get_transform())) return;
		
		ModloaderUtils::ToggleMods(modsToToggle);
		ModloaderUtils::RestartGame();
	});
	restartButton->set_interactable(false);
}

void ClearModsToToggle() {
	modsToToggle->clear();
}