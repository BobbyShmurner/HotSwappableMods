#include "UI/ViewControllers/SettingsViewController.hpp"

#include "Utils/BobbyUtils.hpp"
#include "qmod-utils/shared/QModUtils.hpp"

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
#include "HMUI/InputFieldViewStaticAnimations.hpp"
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
extern bool ShouldRefreshList;

DEFINE_TYPE(HotSwappableMods, SettingsViewController);

std::list<UnityEngine::GameObject*>* hoverHintSettings = new std::list<UnityEngine::GameObject*>();

bool ShouldClearSettingsList = true;
UnityEngine::GameObject* SeperatorTemplate = nullptr;

void UpdateHoverHintSettings(bool enabled) {
	for (UnityEngine::GameObject* setting : *hoverHintSettings) {
		setting->SetActive(enabled);
	}
}

// Fern has insane skill issue
void CreateSeperatorTemplate(UnityEngine::Transform* parent) {
	UnityEngine::Transform* inputTrans = QuestUI::BeatSaberUI::CreateStringSetting(parent, "", "")->get_transform();
	UnityEngine::GameObject* seperator;

	for (int i = 0; i < inputTrans->get_childCount();) {
		UnityEngine::GameObject* obj = inputTrans->GetChild(i)->get_gameObject();

		if (BobbyUtils::Il2cppStrToStr(obj->get_name()) == "Line") {
			seperator = obj;

			seperator->GetComponent<UnityEngine::RectTransform*>()->set_anchorMin({0, 0.9f});
			seperator->GetComponent<UnityEngine::RectTransform*>()->set_anchorMax({1, 0.9f});

			i++;
		} else {
			obj->get_transform()->set_parent(nullptr);
			UnityEngine::GameObject::Destroy(obj);
		}
	}

	UnityEngine::Object::Destroy(inputTrans->GetComponent<HMUI::InputFieldView*>());
	UnityEngine::Object::Destroy(inputTrans->GetComponent<HMUI::InputFieldViewStaticAnimations*>());
	UnityEngine::Object::Destroy(inputTrans->GetComponent<HMUI::Touchable*>());

	inputTrans->get_gameObject()->SetActive(false);
	SeperatorTemplate = inputTrans->get_gameObject();
}

UnityEngine::GameObject* CreateSeperator(UnityEngine::Transform* parent) {
	if (SeperatorTemplate == nullptr) CreateSeperatorTemplate(parent);

	UnityEngine::GameObject* seperator = UnityEngine::GameObject::Instantiate(SeperatorTemplate, parent);
	seperator->SetActive(true);

	return seperator;
}

void HotSwappableMods::SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (ShouldClearSettingsList) {
		hoverHintSettings->clear();
	}

	UpdateHoverHintSettings(getMainConfig().ShowHoverHints.GetValue());

	if (!firstActivation && !ShouldClearSettingsList) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	TMPro::TextMeshProUGUI* titleText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Settings", false);
	
	titleText->set_fontSize(10.0f);
	titleText->set_alignment(TMPro::TextAlignmentOptions::Center);

	UnityEngine::UI::Toggle* alwaysShowFileNames = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Always show file names", getMainConfig().AlwaysShowFileNames.GetValue(), [](bool value){
		getMainConfig().AlwaysShowFileNames.SetValue(value);
		ShouldRefreshList = true;
	});
	QuestUI::BeatSaberUI::AddHoverHint(alwaysShowFileNames->get_gameObject(), "When enabled, File names will always be displayed, instead of Mod IDs");



	UnityEngine::UI::Toggle* promptWhenCoreModsOutdated = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Prompt When Core Mods Are Outdated", getMainConfig().PromptWhenCoreModsOutdated.GetValue(), [](bool value){
		getMainConfig().PromptWhenCoreModsOutdated.SetValue(value);
	});
	QuestUI::BeatSaberUI::AddHoverHint(promptWhenCoreModsOutdated->get_gameObject(), "When enabled, you will be prompted to update any outdated core mods.\n\nIt is recommended that you keep this setting on");



	UnityEngine::UI::Toggle* promptWhenModFailsToLoad = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Prompt When A Mod Fails To Load", getMainConfig().PromptWhenModFailsToLoad.GetValue(), [](bool value){
		getMainConfig().PromptWhenModFailsToLoad.SetValue(value);
	});
	QuestUI::BeatSaberUI::AddHoverHint(promptWhenModFailsToLoad->get_gameObject(), "When enabled, you will be prompted to reload any mods that failed to load, which will fix the issue 99%% of of the time");


	// -- Hover Hint Settings --

	CreateSeperator(mainContainer->get_transform());

	// Show Hover Hints

	UnityEngine::UI::Toggle* showHoverHints = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Hover Hints", getMainConfig().ShowHoverHints.GetValue(), [](bool value){
		UpdateHoverHintSettings(value);
		getMainConfig().ShowHoverHints.SetValue(value);
		ShouldRefreshList = true;
	});
	QuestUI::BeatSaberUI::AddHoverHint(showHoverHints->get_gameObject(), "When enabled, Hover Hints like these will be displayed when hovering over mod toggles");

	// Show File Names On Hover Hints

	UnityEngine::UI::Toggle* showFileNameOnHoverHint = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show File Names On Hover Hints", getMainConfig().ShowFileNameOnHoverHint.GetValue(), [](bool value){
		getMainConfig().ShowFileNameOnHoverHint.SetValue(value);
		ShouldRefreshList = true;
	});
	hoverHintSettings->push_back(showFileNameOnHoverHint->get_transform()->get_parent()->get_gameObject());
	QuestUI::BeatSaberUI::AddHoverHint(showFileNameOnHoverHint->get_gameObject(), "When enabled, a mod's File Name will be displayed on its hover hint");

	// Show Mod IDs On Hover Hints

	UnityEngine::UI::Toggle* showModIDOnHoverHint = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Mod IDs On Hover Hints", getMainConfig().ShowModIDOnHoverHint.GetValue(), [](bool value){
		getMainConfig().ShowModIDOnHoverHint.SetValue(value);
		ShouldRefreshList = true;
	});
	hoverHintSettings->push_back(showModIDOnHoverHint->get_transform()->get_parent()->get_gameObject());
	QuestUI::BeatSaberUI::AddHoverHint(showModIDOnHoverHint->get_gameObject(), "When enabled, a mod's Mod ID will be displayed on its hover hint");

	// Show Mod Versions On Hover Hints

	UnityEngine::UI::Toggle* showModVersionOnHoverHint = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Mod Versions On Hover Hints", getMainConfig().ShowModVersionOnHoverHint.GetValue(), [](bool value){
		getMainConfig().ShowModVersionOnHoverHint.SetValue(value);
		ShouldRefreshList = true;
	});
	hoverHintSettings->push_back(showModVersionOnHoverHint->get_transform()->get_parent()->get_gameObject());
	QuestUI::BeatSaberUI::AddHoverHint(showModVersionOnHoverHint->get_gameObject(), "When enabled, a mod's version will be displayed on its hover hint");

	// Show Mod Errors On Hover Hints

	UnityEngine::UI::Toggle* showModErrorsOnHoverHint = QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Show Mod Errors On Hover Hints", getMainConfig().ShowModErrorsOnHoverHint.GetValue(), [](bool value){
		getMainConfig().ShowModErrorsOnHoverHint.SetValue(value);
		ShouldRefreshList = true;
	});
	hoverHintSettings->push_back(showModErrorsOnHoverHint->get_transform()->get_parent()->get_gameObject());
	QuestUI::BeatSaberUI::AddHoverHint(showModErrorsOnHoverHint->get_gameObject(), "When enabled, an error will be displayed on the hover hints of mods that failed to load that explains why the mod failed to load");

	ShouldClearSettingsList = false;
}