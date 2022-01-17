#include "UI/ViewControllers/ModListViewController.hpp"
#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"

#include "Utils/BobbyUtils.hpp"
#include "qmod-utils/shared/QModUtils.hpp"

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

std::unordered_map<QMod*, bool>* modsEnabled = new std::unordered_map<QMod*, bool>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::vector<QMod*>* modsToToggle = new std::vector<QMod*>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* noModsText;

UnityEngine::UI::Button* toggleButton;
UnityEngine::UI::Button* reloadButton;

std::string GetDisplayName(QMod* qmod) {
	if (getMainConfig().AlwaysShowFileNames.GetValue()) return qmod->FileName();
	else return qmod->Name();
}

bool HasLoadError(QMod* qmod) {
	for (std::string fileName : qmod->ModFiles()) {
		if (!QModUtils::IsModLibLoaded(fileName)) return true;
	}

	return false;
}

UnityEngine::Color GetTextColor(bool isInstalled, bool toggleValue, bool hasLoadError) {
	if (isInstalled != toggleValue) { // Is Value Different?
		return {1.0f, 1.0f, 0.0f, 1.0f}; // Mod Will Change after restart
	} else {
		if (!toggleValue) {  // Is Mod Disabled?
			return {0.55f, 0.55f, 0.55f, 1.0f};
		} else { // Is currently on
			if (!hasLoadError) return {1.0f, 1.0f, 1.0f, 1.0f};
			else return {1.0f, 0.0f, 0.0f, 1.0f};
		}
	}

	return {1.0f, 1.0f, 1.0f, 1.0f};
}

void GenerateModHoverHint(UnityEngine::GameObject* toggle, QMod* qmod) {
	if (!getMainConfig().ShowHoverHints.GetValue()) return;
	std::string hoverMessage = "";

	if (getMainConfig().ShowFileNameOnHoverHint.GetValue()) hoverMessage += string_format("File Name - %s", qmod->FileName().c_str());

	if (getMainConfig().ShowModIDOnHoverHint.GetValue()) 		hoverMessage += string_format("%sMod ID - %s", hoverMessage == "" ? "" : "\n", qmod->Id().c_str());
	if (getMainConfig().ShowModVersionOnHoverHint.GetValue()) 	hoverMessage += string_format("%sMod Version - %s", hoverMessage == "" ? "" : "\n", qmod->Version().c_str());

	if (qmod->IsInstalled()) {
		for (std::string fileName : qmod->ModFiles()) {
			if (!QModUtils::IsModLibLoaded(fileName) && getMainConfig().ShowModErrorsOnHoverHint.GetValue()) {
				hoverMessage += string_format("%sFailed To Load! Reason - %s", hoverMessage == "" ? "" : "\n", QModUtils::GetModError(fileName)->c_str());
			}
		}
	}

	if (hoverMessage != "") QuestUI::BeatSaberUI::AddHoverHint(toggle, std::string_view(hoverMessage));
}

void CreateModToggle(UnityEngine::Transform* container, QMod* qmod, bool isActive) {
	std::string toggleName = qmod->Name();

	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, qmod](bool value) {
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color(GetTextColor(qmod->IsInstalled(), value, HasLoadError(qmod)));

		if (value != modsEnabled->at(qmod)) modsToToggle->push_back(qmod);
		else modsToToggle->erase(std::remove(modsToToggle->begin(), modsToToggle->end(), qmod), modsToToggle->end());

		toggleButton->set_interactable(modsToToggle->size() != 0);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(qmod->IsInstalled(), modsEnabled->at(qmod), HasLoadError(qmod)));

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

int PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<QMod*, bool>* mods) {
	int togglesCreated = 0;

	for (std::pair<QMod*, bool> modPair : *mods) {
		if (modPair.first->Id() == "HotSwappableMods") continue;

		std::string toggleName = GetDisplayName(modPair.first);

		if (modPair.first->IsCoreMod()) {
			continue;
		}
		
		CreateModToggle(container, modPair.first, modPair.second);
		togglesCreated++;
	}

	return togglesCreated;
}

void PopulateModsEnabledMap() {
	modsEnabled->clear();

	for (std::pair<std::string, QMod*> modPair : *QMod::DownloadedQMods) {
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
			QModUtils::RestartGame();
		}
	);
	t.detach();
}

void HotSwappableMods::ModListViewController::ConfirmModal(bool isReloading) {
	std::unordered_map<std::string, QMod*>* installedModsMap = QModUtils::GetInstalledQMods();
	std::vector<QMod*>* installedMods = new std::vector<QMod*>();

	for (std::pair<std::string, QMod*> modPair : *installedModsMap) {
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

	if (!isReloading) {
		for (QMod* mod : *modsToToggle) {
			TMPro::TextMeshProUGUI* modText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), std::string_view("- " + mod->Name()), false);

			modText->set_alignment(TMPro::TextAlignmentOptions::Center);

			if (mod->IsInstalled()) {
				modText->set_color({1, 0, 0, 1});
				modText->get_transform()->SetAsLastSibling();
			} else {
				modText->set_color({0, 1, 0, 1});
				modText->get_transform()->SetSiblingIndex(2);
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

	// Question Mark Button

	UnityEngine::UI::Button* questionButton = QuestUI::BeatSaberUI::CreateUIButton(get_transform(), "?", "ApplyButton", [&](){
		HotSwappableMods::ModListFlowCoordinator* flowCoordinator = (HotSwappableMods::ModListFlowCoordinator*)(QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf());
		flowCoordinator->PresentViewController(flowCoordinator->InfoViewController, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false);
	});

	UnityEngine::RectTransform* questionRect = questionButton->GetComponent<UnityEngine::RectTransform*>();
	questionRect->set_anchorMax({0.9, 0.9});
	questionRect->set_anchorMin({0.9, 0.9});
	questionRect->set_anchoredPosition({0.5, 0.5});

	questionRect->set_sizeDelta({8, 8});

	questionRect->GetComponentInChildren<TMPro::TextMeshProUGUI*>()->set_alignment(TMPro::TextAlignmentOptions::Left);

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

	reloadButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Reload Mods", "ApplyButton", [&](){
		ConfirmModal(true);
	});

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