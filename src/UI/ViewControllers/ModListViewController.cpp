#include "UI/ViewControllers/ModListViewController.hpp"
#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"

#include "Utils/BobbyUtils.hpp"
#include "Utils/PtrLessThan.hpp"

#include "qmod-utils/shared/QModUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Sprite.hpp"
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
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/ViewController_AnimationType.hpp"

#include "TMPro/TextMeshProUGUI.hpp"

#include "GlobalNamespace/BoolSettingsController.hpp"

#include "Polyglot/LocalizedTextMeshProUGUI.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

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

using QMod = QModUtils::QMod;

bool ShouldRefreshList = true;

extern ModInfo modInfo;
extern std::vector<std::string> NoNoMods;

DEFINE_TYPE(HotSwappableMods, ModListViewController);

std::map<QMod*, bool, PtrLessThan<QMod>>* modsEnabled = new std::map<QMod*, bool, PtrLessThan<QMod>>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::vector<QMod*>* modsToToggle = new std::vector<QMod*>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* noModsText;

UnityEngine::UI::Button* toggleButton;

UnityEngine::Sprite* reloadSprite;
std::string reloadSpriteBase64 = "iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAFAmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4gPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iQWRvYmUgWE1QIENvcmUgNi4wLWMwMDIgNzkuMTY0NDg4LCAyMDIwLzA3LzEwLTIyOjA2OjUzICAgICAgICAiPiA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPiA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIiB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtbG5zOmRjPSJodHRwOi8vcHVybC5vcmcvZGMvZWxlbWVudHMvMS4xLyIgeG1sbnM6cGhvdG9zaG9wPSJodHRwOi8vbnMuYWRvYmUuY29tL3Bob3Rvc2hvcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIgeG1wOkNyZWF0b3JUb29sPSJBZG9iZSBQaG90b3Nob3AgMjIuMCAoV2luZG93cykiIHhtcDpDcmVhdGVEYXRlPSIyMDIyLTAxLTI2VDE4OjM3OjMyWiIgeG1wOk1vZGlmeURhdGU9IjIwMjItMDEtMjZUMjA6MDE6NTdaIiB4bXA6TWV0YWRhdGFEYXRlPSIyMDIyLTAxLTI2VDIwOjAxOjU3WiIgZGM6Zm9ybWF0PSJpbWFnZS9wbmciIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIgeG1wTU06SW5zdGFuY2VJRD0ieG1wLmlpZDozYjBjNWY5OS1mZWFiLTVhNGUtOTExYS03MmJkNDUw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";

std::string GetDisplayName(QMod* qmod) {
	if (getMainConfig().AlwaysShowFileNames.GetValue()) return qmod->FileName();
	else return qmod->Name();
}

UnityEngine::Color GetTextColor(QMod* mod, bool toggleValue) {
	if (mod->IsInstalled() != toggleValue) { // Is Value Different?
		return {1.0f, 1.0f, 0.0f, 1.0f}; // Mod Will Change after restart
	} else {
		if (!toggleValue) {  // Is Mod Disabled?
			return {0.55f, 0.55f, 0.55f, 1.0f};
		} else { // Is currently on
			if (QModUtils::ModHasError(mod)) return {1.0f, 0.0f, 0.0f, 1.0f};
			else return {1.0f, 1.0f, 1.0f, 1.0f};
		}
	}

	return {1.0f, 1.0f, 1.0f, 1.0f};
}

TMPro::TextMeshProUGUI* AddModalModListText(UnityEngine::Transform* modalTrans, std::string name, bool willBeDisabled) {
	TMPro::TextMeshProUGUI* modText = QuestUI::BeatSaberUI::CreateText(modalTrans, std::string_view("- " + name), false);

	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	if (willBeDisabled) {
		modText->set_color({1, 0, 0, 1});
		modText->get_transform()->SetAsLastSibling();
	} else {
		modText->set_color({0, 1, 0, 1});
		modText->get_transform()->SetSiblingIndex(2);
	}

	return modText;
}

void GenerateModHoverHint(UnityEngine::GameObject* toggle, QMod* qmod) {
	if (!getMainConfig().ShowHoverHints.GetValue()) return;

	std::string hoverMessage = "";
	hoverMessage.reserve(100); // This is to avoid reallocating every time we push back. If there is a error there will most likely be another alloc

	if (getMainConfig().ShowFileNameOnHoverHint.GetValue()) hoverMessage += string_format("File Name - %s", qmod->FileName().c_str());

	if (getMainConfig().ShowModIDOnHoverHint.GetValue()) 		hoverMessage += string_format("%sMod ID - %s", hoverMessage == "" ? "" : "\n", qmod->Id().c_str());
	if (getMainConfig().ShowModVersionOnHoverHint.GetValue()) 	hoverMessage += string_format("%sMod Version - %s", hoverMessage == "" ? "" : "\n", qmod->Version().c_str());

	if (qmod->IsInstalled()) {
		if (getMainConfig().ShowModErrorsOnHoverHint.GetValue()) {
			std::optional<std::string> error = QModUtils::GetModError(qmod);

			if (error.has_value()) hoverMessage += string_format("%sFailed To Load! Reason - %s", hoverMessage == "" ? "" : "\n", error.value().c_str());
		}
	}

	if (hoverMessage != "") QuestUI::BeatSaberUI::AddHoverHint(toggle, hoverMessage);
}

void CreateModToggle(UnityEngine::Transform* container, std::string toggleName, QMod* qmod, bool isActive) {
	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, qmod](bool value) {
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color(GetTextColor(qmod, value));

		if (value != modsEnabled->at(qmod)) modsToToggle->push_back(qmod);
		else modsToToggle->erase(std::remove(modsToToggle->begin(), modsToToggle->end(), qmod), modsToToggle->end());

		toggleButton->set_interactable(modsToToggle->size() != 0);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(qmod, modsEnabled->at(qmod)));

	GenerateModHoverHint(newToggle->get_transform()->get_parent()->get_gameObject(), qmod);
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

int PopulateModToggles(UnityEngine::Transform* container, std::map<QMod*, bool, PtrLessThan<QMod>>* mods) {
	int togglesCreated = 0;

	std::vector<std::pair<QMod*, bool>> orderedModPairs;

	// First we want to order the toggles
	// They will be ordered in alphabetical order, but also sectioned into 3 categories
	//
	// 1. Mods that failed to load
	// 2. Enabled Mods
	// 3. Disabled Mods

	int errorCount = 0;
	int loadedCount = 0;

	for (auto modPairPtr = mods->rbegin(); modPairPtr != mods->rend(); modPairPtr++) {
		auto modPair = *modPairPtr;

		if (modPair.first->IsCoreMod()) continue;
		if (modPair.first->Id() == "HotSwappableMods") continue;

		if (QModUtils::ModHasError(modPair.first)) {
			orderedModPairs.insert(orderedModPairs.begin(), modPair);
			errorCount++;

			continue;
		}
		
		if (modPair.first->IsInstalled()) {
			orderedModPairs.insert(orderedModPairs.begin() + errorCount, modPair);
			loadedCount++;

			continue;
		}

		orderedModPairs.insert(orderedModPairs.begin() + errorCount + loadedCount, modPair);
	}

	for (std::pair<QMod*, bool> modPair : orderedModPairs) {
		std::string toggleName = GetDisplayName(modPair.first);
		
		CreateModToggle(container, toggleName, modPair.first, modPair.second);
		togglesCreated++;
	}

	return togglesCreated;
}

void PopulateModsEnabledMap() {
	modsEnabled->clear();

	for (std::pair<std::string, QMod*> modPair : *QMod::GetDownloadedQMods()) {
		getLogger().info("Found downloaded mod \"%s\" (Enabled: %s)", modPair.second->Name().c_str(), modPair.second->IsInstalled() ? "True" : "False");
		modsEnabled->emplace(modPair.second, modPair.second->IsInstalled());
	}
}

void OnReloadStart(QMod* qmod, TMPro::TextMeshProUGUI* loadingText) {
	QuestUI::MainThreadScheduler::Schedule(
		[qmod, loadingText] {
			loadingText->SetText(il2cpp_utils::newcsstr(string_format("Reloading %s...", qmod->Name().c_str()).c_str()));
		}
	);
}

void OnToggleStart(QMod* qmod, bool settingActive, TMPro::TextMeshProUGUI* loadingText) {
	QuestUI::MainThreadScheduler::Schedule(
		[qmod, settingActive, loadingText] {
			loadingText->SetText(il2cpp_utils::newcsstr(string_format("%s %s...", settingActive ? "Enabling" : "Disabling", qmod->Name().c_str()).c_str()));
		}
	);
}

void ToggleAndRestart(UnityEngine::Transform* trans, std::vector<QMod*>* mods, bool isReloading) {
	HMUI::ModalView* loadingModal = QuestUI::BeatSaberUI::CreateModal(trans, {80, 15}, nullptr, false);

	UnityEngine::UI::VerticalLayoutGroup* modalLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(loadingModal->get_transform());

	modalLayout->get_rectTransform()->set_anchorMin({0, 0.1f});
	modalLayout->get_rectTransform()->set_anchorMax({1, 1});

	modalLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
	modalLayout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);
	
	modalLayout->set_childControlHeight(true);
	modalLayout->set_childForceExpandHeight(false);
	modalLayout->set_childControlWidth(false);
	modalLayout->set_childForceExpandWidth(true);

	TMPro::TextMeshProUGUI* loadingText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), {"Loading..."}, false);
	loadingText->set_alignment(TMPro::TextAlignmentOptions::Center);
	loadingText->set_fontSize(5);
	
	loadingModal->Show(true, true, nullptr);

	std::thread t(
		[mods, loadingText, isReloading] {
			if (isReloading) {
				QModUtils::ReloadMods(mods, [loadingText](QMod* qmod){
					OnReloadStart(qmod, loadingText);
				});

			} else {
				QModUtils::ToggleMods(mods, [loadingText](QMod* qmod, bool settingActive){
					OnToggleStart(qmod, settingActive, loadingText);
				});
			}

			getLogger().info("Finished %s, Restarting Game!", isReloading ? "Reloading" : "Toggling");
			JNIUtils::RestartApp();
		}
	);
	t.detach();
}

void HotSwappableMods::ModListViewController::ConfirmModal(bool isReloading) {
	std::unordered_map<std::string, QMod*> installedModsMap = QModUtils::GetInstalledMods();
	std::vector<QMod*>* installedMods = new std::vector<QMod*>();

	for (std::pair<std::string, QMod*> modPair : installedModsMap) {
		if (!modPair.second->IsCoreMod()) installedMods->push_back(modPair.second);
	}

	HMUI::ModalView* modal = nullptr;
	if (isReloading) modal = QuestUI::BeatSaberUI::CreateModal(get_transform(), {80, 32}, nullptr, true);
	else modal = QuestUI::BeatSaberUI::CreateModal(get_transform(), {80, 75}, nullptr, true);
	
	UnityEngine::UI::VerticalLayoutGroup* modalLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(modal->get_transform());

	modalLayout->get_rectTransform()->set_anchorMin({0, 0.1f});
	modalLayout->get_rectTransform()->set_anchorMax({1, 1});

	modalLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
	modalLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
	
	modalLayout->set_childControlHeight(true);
	modalLayout->set_childForceExpandHeight(false);
	modalLayout->set_childControlWidth(false);
	modalLayout->set_childForceExpandWidth(true);

	TMPro::TextMeshProUGUI* modalTitleText = nullptr;
	if (isReloading) modalTitleText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), {"Reload Mods"}, false);
	else modalTitleText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), {"Toggle Mods"}, false);

	modalTitleText->set_alignment(TMPro::TextAlignmentOptions::Center);
	modalTitleText->set_fontSize(8);

	TMPro::TextMeshProUGUI* descriptionText = nullptr;

	if (isReloading) descriptionText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), string_format("Are you sure you wanna reload %i mods?", (int)installedMods->size()).c_str(), false);
	else descriptionText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), "Are you sure you wanna toggle to following mods?\n", false);

	descriptionText->set_alignment(TMPro::TextAlignmentOptions::Center);

	// I dont like this code, but it works soooooooo ¯\_(ツ)_/¯

	if (!isReloading) {
		// This is just to make sure that the same mod isnt added to the list twice;
		std::unordered_map<std::string, TMPro::TextMeshProUGUI*> modTextList;

		for (QMod* mod : *modsToToggle) {
			if (modTextList.contains(mod->Id())) {
				if (modTextList[mod->Id()]->get_color() == UnityEngine::Color(1, 0, 0, 1)) {
					modTextList[mod->Id()]->set_color({0, 1, 0, 1});
					modTextList[mod->Id()]->get_transform()->SetSiblingIndex(2);
				}

				continue;
			}
			modTextList.emplace(mod->Id(), AddModalModListText(modalLayout->get_transform(), mod->Name(), mod->IsInstalled()));

			if (mod->IsInstalled()) {
				for (QMod* dependentMod : mod->FindModsDependingOn(true)) {
					if (modTextList.contains(dependentMod->Id())) continue;

					modTextList.emplace(dependentMod->Id(), AddModalModListText(modalLayout->get_transform(), dependentMod->Name(), true));
				}
			} else {
				for (QModUtils::Dependency dependency : mod->Dependencies()) {
					if (modTextList.contains(dependency.id)) {
						if (modTextList[dependency.id]->get_color() == UnityEngine::Color(1, 0, 0, 1)) {
							modTextList[dependency.id]->set_color({0, 1, 0, 1});
							modTextList[dependency.id]->get_transform()->SetSiblingIndex(2);
						}

						continue;
					}
				
					std::optional<QMod*> dependentMod = QMod::GetDownloadedQMod(dependency.id);

					if (dependentMod.has_value()) {
						if (!dependentMod.value()->IsInstalled()) modTextList.emplace(dependency.id, AddModalModListText(modalLayout->get_transform(), dependentMod.value()->Name(), false));
					} else {
						modTextList.emplace(dependency.id, AddModalModListText(modalLayout->get_transform(), dependency.id, false));
					}
				}
			}
		}
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
	
	UnityEngine::UI::Button* cancel = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Cancel", {"CancelButton"}, [=](){
		modal->Hide(true, nullptr);
	});

	UnityEngine::UI::Button* confirm = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Im Sure", {"ApplyButton"}, [=](){
		modal->Hide(true, nullptr);

		IsRestarting = true;

		if (isReloading) {
			ToggleAndRestart(get_transform(), installedMods, true);
		} else {
			ToggleAndRestart(get_transform(), modsToToggle, false);
		}
	});

	modal->Show(true, true, nullptr);
}

void HotSwappableMods::ModListViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf()->SetTitle(il2cpp_utils::newcsstr("Mod List"), HMUI::ViewController::AnimationType::In);

	if (!ShouldRefreshList) return;
	ShouldRefreshList = false;

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
		noModsText = nullptr;

		// Create the reload sprite
		reloadSprite = BeatSaberUI::Base64ToSprite(reloadSpriteBase64);

	} else {
		if (modText != nullptr) 		{ GameObject::Destroy(modText->get_gameObject()); modText = nullptr; }
		if (noModsText != nullptr)		{ GameObject::Destroy(noModsText->get_gameObject()); noModsText = nullptr; }

		ClearModToggles();
		toggleButton->set_interactable(false);
	}

	PopulateModsEnabledMap();

	// Mod List

	modText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	modText->set_fontSize(10.0f);
	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	int modCount = PopulateModToggles(mainContainer->get_transform(), modsEnabled);

	if (modCount == 0) {
		noModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "No Mods Found!", false);
		noModsText->set_alignment(TMPro::TextAlignmentOptions::Center);
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

	QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Cancel", "CancelButton", [&](){
		QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf()->BackButtonWasPressed(this);
	});

	// Toggle Button

	toggleButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Toggle Mods", "ApplyButton", [&](){
		ConfirmModal(false);
	});
	toggleButton->set_interactable(false);

	// Reload Button

	UnityEngine::UI::Button* reloadButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "", "ApplyButton", [&](){
		ConfirmModal(true);
	});

	UnityEngine::RectTransform* reloadRect = reloadButton->GetComponent<UnityEngine::RectTransform*>();

	reloadRect->set_sizeDelta({8, 8});
	
	HMUI::ImageView* reloadImage = QuestUI::BeatSaberUI::CreateImage(reloadRect, reloadSprite, {0, 0}, {1, 1});
	reloadImage->set_color({1, 1, 1, 0.6f});

	// Info Button

	UnityEngine::UI::Button* infoButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "?", "ApplyButton", [&](){
		HotSwappableMods::ModListFlowCoordinator* flowCoordinator = (HotSwappableMods::ModListFlowCoordinator*)(QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf());
		flowCoordinator->PresentViewController(flowCoordinator->InfoViewController, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false);
	});

	UnityEngine::RectTransform* infoRect = infoButton->GetComponent<UnityEngine::RectTransform*>();

	infoRect->set_sizeDelta({8, 8});
	infoRect->GetComponentInChildren<TMPro::TextMeshProUGUI*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

	// -- In Game Downloading Test --

	// QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Install ALL QMods", "ApplyButton", [&](){
	// 	// QMod::InstallFromUrl("Qosmetics.qmod", "https://github.com/RedBrumbler/Qosmetics/releases/download/v1.3.3/Qosmetics.qmod");
	// 	// return;

	// 	rapidjson::Document document = QModUtils::WebUtils::GetJSONData("http://questboard.xyz/api/mods/").value();
	// 	const auto &mods = document["mods"].GetArray();

	// 	for (rapidjson::SizeType i = 0; i < mods.Size(); i++)
	// 	{
	// 		auto &mod = mods[i];
	// 		std::string name = mod["name"].GetString();

	// 		if (name == "PinkCore") continue;

	// 		getLogger().info("Found Mod \"%s\"!", name.c_str());

	// 		const auto &downloads = mod["downloads"].GetArray();
	// 		for (rapidjson::SizeType j = 0; j < downloads.Size(); j++) {
	// 			auto &download = downloads[j];
	// 			const auto &gameVersions = download["gameversion"].GetArray();

	// 			int correctVersionIndex = -1;
	// 			for (rapidjson::SizeType k = 0; k < gameVersions.Size(); k++) {
	// 				std::string gameVersion = gameVersions[k].GetString();

	// 				if (gameVersion == QModUtils::GetGameVersion()) {
	// 					getLogger().info("Got correct version for \"%s\"", name.c_str());
	// 					correctVersionIndex = k;
	// 					break;
	// 				}
	// 			}

	// 			if (correctVersionIndex != -1) {
	// 				std::string url = download["download"].GetString();

	// 				getLogger().info("Installing mod \"%s\" from url \"%s\"", name.c_str(), url.c_str());
	// 				QMod::InstallFromUrl(name + ".qmod", url);
	// 			}
	// 		}
	// 	}
	// });

	// QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Fuck Modlist.", "ApplyButton", [&](){
	// 	QMod* modList = QMod::GetDownloadedQMod("mod-list");
	// 	if (modList != nullptr) modList->Uninstall(false);
	// });
}

void ClearModsToToggle() {
	modsToToggle->clear();
}