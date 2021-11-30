#include "ViewControllers/ModListViewController.hpp"

#include "Utils/BobbyUtils.hpp"
#include "Utils/ModUtils.hpp"

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

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, bool isActive) {
	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName](bool value){
		std::string fileName = ModUtils::GetFileName(toggleName);
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();

		if (!ModUtils::IsLibName(toggleName)) fileName = ModUtils::GetFileNameFromDisplayName(toggleName);

		if (value != modsEnabled->at(fileName)) {
			modsToToggle->emplace_front(fileName);
			textMesh->set_color({1.0f, 1.0f, 0.5f, 1.0f});
		}
		else { 
			modsToToggle->remove(fileName);
			textMesh->set_color({1.0f, 1.0f, 1.0f, 1.0f});
		}
	});

	modToggles->emplace(toggleName, newToggle);
}

void PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<std::string, bool>* mods) {
	for (std::pair<std::string, UnityEngine::UI::Toggle*> togglePair : *modToggles) {
		GameObject::Destroy(togglePair.second->get_transform()->get_parent()->get_gameObject());
	}
	modToggles->clear();

	for (std::pair<std::string, bool> mod : *mods) {
		std::string toggleName = ModUtils::GetModDisplayName(mod.first);
		if (ModUtils::IsFileName(toggleName)) toggleName = ModUtils::GetLibName(toggleName);

		CreateModToggle(container, toggleName, mod.second);
	}
}

void PopulateModsEnabledMap (std::unordered_map<std::string, bool>* map) {
	map->clear();

	for (std::string fileName : ModUtils::GetDirContents("/sdcard/Android/data/com.beatgames.beatsaber/files/mods/")) {
		if (!ModUtils::IsFileName(fileName)) continue;

		if (ModUtils::IsDisabled(fileName)) map->emplace(fileName, false);
		else map->emplace(fileName, true);
	}
}

void HotSwappableMods::ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	BackButton = GameObject::Find(il2cpp_utils::newcsstr("BackButton"))->GetComponent<HMUI::NoTransitionsButton*>();

	if (firstActivation) mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	else PopulateModToggles(mainContainer->get_transform(), modsEnabled);

	ModUtils::GetOddLibNames();
	PopulateModsEnabledMap(modsEnabled);

	if (!firstActivation) return;

	TMPro::TextMeshProUGUI* titleText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	titleText->set_fontSize(10.0f);
	titleText->set_alignment(TMPro::TextAlignmentOptions::Center);

	ModUtils::UpdateAlwaysDisplayLibNames(getMainConfig().AlwaysShowFileNames.GetValue());
	PopulateModToggles(mainContainer->get_transform(), modsEnabled);
}