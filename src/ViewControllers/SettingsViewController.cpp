#include "ViewControllers/SettingsViewController.hpp"

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
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/Events/UnityAction.hpp"
#include "UnityEngine/Events/UnityAction_1.hpp"
#include "HMUI/ScrollView.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/Touchable.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/NoTransitionsButton.hpp"
#include "HMUI/TextPageScrollView.hpp"
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

DEFINE_TYPE(HotSwappableMods, SettingsViewController);

std::list<UnityEngine::GameObject*>* advancedSettings = new std::list<UnityEngine::GameObject*>();

void UpdateAdvandcedSettings(bool enabled) {
	for (UnityEngine::GameObject* setting : *advancedSettings) {
		setting->SetActive(enabled);
	}
}

void HotSwappableMods::SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (!firstActivation) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	TMPro::TextMeshProUGUI* titleText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Settings", false);
	
	titleText->set_fontSize(10.0f);
	titleText->set_alignment(TMPro::TextAlignmentOptions::Center);

	QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Always show file names", getMainConfig().AlwaysShowFileNames.GetValue(), [](bool value){
		getMainConfig().AlwaysShowFileNames.SetValue(value);
	});

	// Advanced Settings

	QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Advanced Settings", getMainConfig().ShowAdvancedSettings.GetValue(), [&](bool value){
		UpdateAdvandcedSettings(value);
		getMainConfig().ShowAdvancedSettings.SetValue(value);
	});

	TMPro::TextMeshProUGUI* warningTitle = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "WARNING!", false);
	advancedSettings->emplace_front(warningTitle->get_gameObject());

	warningTitle->set_fontSize(10.0f);
	warningTitle->set_alignment(TMPro::TextAlignmentOptions::Top); // This actually positions it at the bottom. Dont ask me why
	warningTitle->set_color({1.0f, 0.0f, 0.0f, 1.0f});

	TMPro::TextMeshProUGUI* warningDesc = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Dont mess with these settings unless you know what your doing.\nIf you break your game because of these settings, its on you - not us!\nYou Have Been Warned!!!", false, {0, 0}, {1, 20});
	advancedSettings->emplace_front(warningDesc->get_gameObject());

	warningDesc->set_alignment(TMPro::TextAlignmentOptions::Center);
	warningDesc->set_color({1.0f, 0.0f, 0.0f, 1.0f});

	UnityEngine::UI::Toggle* showCoreMods = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Core Mods", getMainConfig().ShowCoreMods.GetValue(), [](bool value){
		getMainConfig().ShowCoreMods.SetValue(value);
	});
	advancedSettings->emplace_front(showCoreMods->get_transform()->get_parent()->get_gameObject());


	UnityEngine::UI::Toggle* showLibs = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Libraries", getMainConfig().ShowLibs.GetValue(), [](bool value){
		getMainConfig().ShowLibs.SetValue(value);
	});
	advancedSettings->emplace_front(showLibs->get_transform()->get_parent()->get_gameObject());

	UpdateAdvandcedSettings(getMainConfig().ShowAdvancedSettings.GetValue());
}