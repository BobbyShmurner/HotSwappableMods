#include "UI/ViewControllers/ModListViewController.hpp"
#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"

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

using QMod = ModloaderUtils::QMod;

bool ShouldRefreshList = true;

extern ModInfo modInfo;
extern std::vector<std::string> NoNoMods;

DEFINE_TYPE(HotSwappableMods, ModListViewController);

std::unordered_map<QMod*, bool>* modsEnabled = new std::unordered_map<QMod*, bool>();
std::unordered_map<std::string, UnityEngine::UI::Toggle*>* modToggles = new std::unordered_map<std::string, UnityEngine::UI::Toggle*>();

std::vector<QMod*>* modsToToggle = new std::vector<QMod*>();

UnityEngine::GameObject* mainContainer;

TMPro::TextMeshProUGUI* modText;
TMPro::TextMeshProUGUI* coreModsText;
TMPro::TextMeshProUGUI* dangerZoneText;
TMPro::TextMeshProUGUI* dangerZoneDesc;

TMPro::TextMeshProUGUI* noModsText;
TMPro::TextMeshProUGUI* noCoreModsText;

UnityEngine::UI::Button* restartButton;

std::string GetDisplayName(QMod* qmod) {
	if (getMainConfig().AlwaysShowFileNames.GetValue()) return qmod->FileName();
	else return qmod->Name();
}

UnityEngine::Color GetTextColor(bool isCurrentlyEnabled, bool toggleValue, bool isImportant, bool isModLoaded) {
	if (isCurrentlyEnabled != toggleValue) { // Is Value Different?
		if (isImportant) {
			if (!toggleValue) { // Is currently on, but will be disabled after restart
				return {1.0f, 0.0f, 0.0f, 1.0f};
			} else { // Is currently off, but will be enabled after restart
				return {0.0f, 1.0f, 0.0f, 1.0f};
			}
		}
		return {1.0f, 1.0f, 0.0f, 1.0f}; // Mod Will Change after restart
	} else {
		if (!toggleValue) {  // Is Mod Disabled?
			if (isImportant) return {1.0f, 0.0f, 0.0f, 1.0f}; // Is currently off, SHOULD be turned on tho
			else return {0.55f, 0.55f, 0.55f, 1.0f};
		} else { // Is currently on, lets leave it that way xD
			if (isModLoaded) return {1.0f, 1.0f, 1.0f, 1.0f};
			else return {1.0f, 0.5f, 0.0f, 1.0f};
		}
	}

	return {1.0f, 1.0f, 1.0f, 1.0f};
}

void GenerateModHoverHint(UnityEngine::GameObject* toggle, QMod* qmod) {
	if (!getMainConfig().ShowHoverHints.GetValue()) return;
	std::string hoverMessage = "";

	if (getMainConfig().ShowFileNameOnHoverHint.GetValue()) hoverMessage += string_format("File Name - %s", qmod->FileName().c_str());

	bool isCore = qmod->IsCoreMod();

	if (getMainConfig().ShowModIDOnHoverHint.GetValue()) 		hoverMessage += string_format("%sMod ID - %s", hoverMessage == "" ? "" : "\n", qmod->Id().c_str());
	if (getMainConfig().ShowModVersionOnHoverHint.GetValue()) 	hoverMessage += string_format("%sMod Version - %s", hoverMessage == "" ? "" : "\n", qmod->Version().c_str());

	if (qmod->Installed()) {
		for (std::string fileName : qmod->ModFiles()) {
			if (!ModloaderUtils::IsModLoaded(fileName) && getMainConfig().ShowModErrorsOnHoverHint.GetValue()) {
				hoverMessage += string_format("%sFailed To Load! Reason - %s", hoverMessage == "" ? "" : "\n", ModloaderUtils::GetModError(fileName)->c_str());
			}
		}
	}

	if (hoverMessage != "") QuestUI::BeatSaberUI::AddHoverHint(toggle, std::string_view(hoverMessage));
}

void CreateModToggle(UnityEngine::Transform* container, QMod* qmod, bool isActive, bool isImportant) {
	std::string toggleName = qmod->Name();

	UnityEngine::UI::Toggle* newToggle = QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(toggleName), isActive, [&, toggleName, isImportant, qmod](bool value){
		TMPro::TextMeshProUGUI* textMesh = modToggles->at(toggleName)->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
		textMesh->set_color(GetTextColor(modsEnabled->at(qmod), value, isImportant, qmod->Installed()));

		if (value != modsEnabled->at(qmod)) modsToToggle->push_back(qmod);
		else std::remove(modsToToggle->begin(), modsToToggle->end(), qmod);

		restartButton->set_interactable(modsToToggle->size() != 0);
	});

	modToggles->emplace(toggleName, newToggle);

	TMPro::TextMeshProUGUI* textMesh = newToggle->get_transform()->get_parent()->Find(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>("NameText"))->GetComponent<TMPro::TextMeshProUGUI*>();
	textMesh->set_color(GetTextColor(modsEnabled->at(qmod), modsEnabled->at(qmod), isImportant, qmod->Installed()));

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

int PopulateModToggles(UnityEngine::Transform* container, std::unordered_map<QMod*, bool>* mods, bool areCoreMods = false) {
	int togglesCreated = 0;

	for (std::pair<QMod*, bool> modPair : *mods) {
		// if (std::find(NoNoMods.begin(), NoNoMods.end(), ModloaderUtils::GetLibName(mod.first)) != NoNoMods.end()) continue;

		std::string toggleName = GetDisplayName(modPair.first);

		if (modPair.first->IsCoreMod()){
			if (!areCoreMods) continue;
		} else {
			if (areCoreMods) continue;
		}
		
		CreateModToggle(container, modPair.first, modPair.second, areCoreMods);
		togglesCreated++;
	}

	return togglesCreated;
}

void PopulateModsEnabledMap() {
	modsEnabled->clear();

	for (std::pair<std::string, QMod*> modPair : *QMod::DownloadedQMods) {
		getLogger().info("Found downloaded mod \"%s\" (Enabled: %s)", modPair.second->Name().c_str(), modPair.second->Installed() ? "True" : "False");
		modsEnabled->emplace(modPair.second, modPair.second->Installed());
	}
}

void OnToggleStart(QMod* qmod, bool settingActive, TMPro::TextMeshProUGUI* loadingText) {
	QuestUI::MainThreadScheduler::Schedule(
		[qmod, settingActive, loadingText] {
			loadingText->SetText(il2cpp_utils::newcsstr(string_format("%s %s...", settingActive ? "Enabling" : "Disabling", qmod->Name().c_str()).c_str()));
		}
	);
}

void ToggleAndRestart(UnityEngine::Transform* trans) {
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
		[loadingText] {
			ModloaderUtils::ToggleQMods(modsToToggle, [loadingText](QMod* qmod, bool settingActive){
				OnToggleStart(qmod, settingActive, loadingText);
			});

			getLogger().info("Finished Toggling, Restarting Game!");
			ModloaderUtils::RestartGame();
		}
	);
	t.detach();
}

bool CoreModModal(UnityEngine::Transform* trans) {
	std::vector<QMod*> coreModsDisabled = std::vector<QMod*>();

	for (QMod* mod : *modsToToggle) {
		if (mod->IsCoreMod() && mod->Installed()) { // This may seem counterintuitive, but were checking looping through the list of mods that will be toggeled, so if a mod is currently installed, it should be added to the list as it will be disabled on reload
			coreModsDisabled.push_back(mod);
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

	for (QMod* mod : coreModsDisabled) {
		TMPro::TextMeshProUGUI* modText = QuestUI::BeatSaberUI::CreateText(modalLayout->get_transform(), std::string_view("- " + mod->Name()), false);

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
		modal->Hide(true, nullptr);

		ToggleAndRestart(trans);
	});

	modal->Show(true, true, nullptr);

	return true;
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
		noCoreModsText = nullptr;
		dangerZoneText = nullptr;
		dangerZoneDesc = nullptr;
		coreModsText = nullptr;
	} else {
		if (modText != nullptr) 		{ GameObject::Destroy(modText->get_gameObject()); modText = nullptr; }
		if (noModsText != nullptr)		{ GameObject::Destroy(noModsText->get_gameObject()); noModsText = nullptr; }
		if (noCoreModsText != nullptr)	{ GameObject::Destroy(noCoreModsText->get_gameObject()); noCoreModsText = nullptr; }
		if (dangerZoneText != nullptr)	{ GameObject::Destroy(dangerZoneText->get_gameObject()); dangerZoneText = nullptr; }
		if (dangerZoneDesc != nullptr)	{ GameObject::Destroy(dangerZoneDesc->get_gameObject()); dangerZoneDesc = nullptr; }
		if (coreModsText != nullptr)	{ GameObject::Destroy(coreModsText->get_gameObject()); coreModsText = nullptr; }

		ClearModToggles();
		restartButton->set_interactable(false);
	}

	PopulateModsEnabledMap();

	// Mod List

	modText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);
	modText->set_fontSize(10.0f);
	modText->set_alignment(TMPro::TextAlignmentOptions::Center);

	int modCount = PopulateModToggles(mainContainer->get_transform(), modsEnabled, false);

	if (modCount == 0) {
		noModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "No Mods Found!", false);
		noModsText->set_alignment(TMPro::TextAlignmentOptions::Center);
	}

	// Danger Zone

	if (getMainConfig().ShowCoreMods.GetValue()) {
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

		int coreModCount = PopulateModToggles(mainContainer->get_transform(), modsEnabled, true);

		if (coreModCount == 0) {
			noCoreModsText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "No Core Mods Found!", false);
			noCoreModsText->set_alignment(TMPro::TextAlignmentOptions::Center);
		}
	}

	if (!firstActivation) return;

	// Question Mark Button

	UnityEngine::UI::Button* questionButton = QuestUI::BeatSaberUI::CreateUIButton(get_transform(), "?", "ApplyButton", [&](){
		HotSwappableMods::ModListFlowCoordinator* flowCoordinator = (HotSwappableMods::ModListFlowCoordinator*)(QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf());
		flowCoordinator->PresentViewController(flowCoordinator->InfoViewController, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, false);
	});

	UnityEngine::RectTransform* questionRect = questionButton->GetComponent<UnityEngine::RectTransform*>();
	questionRect->set_anchorMax({0.94, 0.1});
	questionRect->set_anchorMin({0.94, 0.1});
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

	// Restart Button
	restartButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Reload Mods", "ApplyButton", [&](){
		if (CoreModModal(get_transform())) return;
		
		ToggleAndRestart(get_transform());
	});
	restartButton->set_interactable(false);

	QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Install ALL QMods", "ApplyButton", [&](){
		// QMod::InstallFromUrl("Qosmetics.qmod", "https://github.com/RedBrumbler/Qosmetics/releases/download/v1.3.3/Qosmetics.qmod");
		// return;

		rapidjson::Document document = ModloaderUtils::WebUtils::GetJSONData("http://questboard.xyz/api/mods/");
		const auto &mods = document["mods"].GetArray();

		for (rapidjson::SizeType i = 0; i < mods.Size(); i++)
		{
			auto &mod = mods[i];
			std::string name = mod["name"].GetString();

			if (name == "PinkCore") continue;

			getLogger().info("Found Mod \"%s\"!", name.c_str());

			const auto &downloads = mod["downloads"].GetArray();
			for (rapidjson::SizeType j = 0; j < downloads.Size(); j++) {
				auto &download = downloads[j];
				const auto &gameVersions = download["gameversion"].GetArray();

				int correctVersionIndex = -1;
				for (rapidjson::SizeType k = 0; k < gameVersions.Size(); k++) {
					std::string gameVersion = gameVersions[k].GetString();

					if (gameVersion == ModloaderUtils::GetGameVersion()) {
						getLogger().info("Got correct version for \"%s\"", name.c_str());
						correctVersionIndex = k;
						break;
					}
				}

				if (correctVersionIndex != -1) {
					std::string url = download["download"].GetString();

					getLogger().info("Installing mod \"%s\" from url \"%s\"", name.c_str(), url.c_str());
					QMod::InstallFromUrl(name + ".qmod", url);
				}
			}
		}
	});

	QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Fuck Modlist.", "ApplyButton", [&](){
		QMod* modList = QMod::GetDownloadedQMod("mod-list");
		if (modList != nullptr) modList->Uninstall(false);
	});
}

void ClearModsToToggle() {
	modsToToggle->clear();
}