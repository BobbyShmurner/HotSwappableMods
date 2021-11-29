#include "ViewControllers/MainViewController.hpp"

#include "Utils/DirUtils.hpp"

#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/RectTransform.hpp"
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

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include <stdlib.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;
using namespace HMUI;
using namespace TMPro;

extern ModInfo modInfo;

DEFINE_TYPE(HotSwappableMods, MainViewController);

HMUI::NoTransitionsButton* BackButton;

std::unordered_map<std::string, bool>* modsEnabled = new std::unordered_map<std::string, bool>();
std::list<std::string>* modsToToggle = new std::list<std::string>();

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, bool isActive) {
	QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName](bool value){
		if (value != modsEnabled->at(toggleName)) modsToToggle->emplace_front(toggleName);
		else modsToToggle->remove(toggleName);
	});
}

void PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<std::string, bool>* mods) {
	for (std::pair<std::string, bool> mod : *mods) {
		CreateModToggle(container, mod.first, mod.second);
	}
}

void PopulateModsEnabledMap (std::unordered_map<std::string, bool>* map) {
	for (std::string fileName : DirUtils::GetDirContents("/sdcard/Android/data/com.beatgames.beatsaber/files/mods/")) {
		if (fileName.length() > 9 && !strcmp(fileName.substr(fileName.size() - 9).c_str(), ".disabled")) {
			map->emplace(fileName.substr(0, fileName.size() - 9), false);
		} else if (fileName.length() > 3 && !strcmp(fileName.substr(fileName.size() - 3).c_str(), ".so")) {
			map->emplace(fileName.substr(0, fileName.size() - 3), true);
		}
	}
}

void HotSwappableMods::MainViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	BackButton = GameObject::Find(il2cpp_utils::newcsstr("BackButton"))->GetComponent<HMUI::NoTransitionsButton*>();

	if (!firstActivation) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	PopulateModsEnabledMap(modsEnabled);

	QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);

	PopulateModToggles(mainContainer->get_transform(), modsEnabled);
}