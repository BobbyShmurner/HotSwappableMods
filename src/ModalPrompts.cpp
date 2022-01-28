#include "ModalPrompts.hpp"
#include "main.hpp"

#include "qmod-utils/shared/QModUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#include "custom-types/shared/coroutine.hpp"

#include "cpp-semver/shared/cpp-semver.hpp"

#include "GlobalNamespace/MainFlowCoordinator.hpp"

#include "UnityEngine/WaitForSecondsRealtime.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/RectOffset.hpp"

#include "HMUI/Screen.hpp"
#include "System/Collections/IEnumerator.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace UnityEngine::UI;

// Got the original code from NE: https://github.com/StackDoubleFlow/NoodleExtensions/blob/master/src/Hooks/OutdatedMods.cpp

UnityEngine::UI::HorizontalLayoutGroup* SetupBottomPannel(UnityEngine::Transform* modalTrans) {
	UnityEngine::UI::HorizontalLayoutGroup* bottomPannel = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(modalTrans);

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

	return bottomPannel;
}

custom_types::Helpers::Coroutine openDialogLater() {
    co_yield reinterpret_cast<System::Collections::IEnumerator *>(CRASH_UNLESS(WaitForSecondsRealtime::New_ctor(0.5f)));

    using namespace QuestUI;

	// Used Later in lambdas
	static UnityEngine::UI::Button* ConfirmButton = nullptr;
	static UnityEngine::UI::Button* CancelButton = nullptr;

    // Create canvas
    auto screen = BeatSaberUI::CreateCanvas();
	reinterpret_cast<UnityEngine::RectTransform *>(screen->get_transform())->set_sizeDelta({10000, 10000}); // Ur not clicking past this boi

    // Required for Modal
    screen->AddComponent<HMUI::Screen *>();
    auto canvasTransform = reinterpret_cast<UnityEngine::RectTransform *>(screen->get_transform());

    // Position in front of the user
    canvasTransform->set_localPosition({0, 1.5, 1.99f}); // The odd z value is just to ensure that it is infront of the NE modals if any show

    // Create modal
    auto modal = BeatSaberUI::CreateModal(screen->get_transform(), {80, 50}, nullptr, false);

	UnityEngine::UI::VerticalLayoutGroup* modalLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(modal->get_transform());

	modalLayout->get_rectTransform()->set_anchorMin({0, 0.1f});
	modalLayout->get_rectTransform()->set_anchorMax({1, 1});

	modalLayout->set_padding(UnityEngine::RectOffset::New_ctor(2, 2, 2, 2));
	modalLayout->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
	
	modalLayout->set_childControlHeight(true);
	modalLayout->set_childForceExpandHeight(false);
	modalLayout->set_childControlWidth(false);
	modalLayout->set_childForceExpandWidth(true);

	auto outdatedCoreMods = QModUtils::GetMissingCoreMods();

	if (outdatedCoreMods.size() != 0 && getMainConfig().PromptWhenCoreModsOutdated.GetValue()) {
		auto headerText = BeatSaberUI::CreateText(modalLayout->get_transform(), "There are updates available for the following core mods\n");

		headerText->set_alignment(TMPro::TextAlignmentOptions::Center);

		for (auto coreModInfo : outdatedCoreMods) {
			auto downloadedModOpt = QModUtils::QMod::GetDownloadedQMod(coreModInfo.first);

			std::string text = "- " + coreModInfo.first + " (Missing)";

			if (downloadedModOpt.has_value()) {
				auto downloadedMod = downloadedModOpt.value();
				text = string_format("- %s (%s) -> %s (%s)", downloadedMod->Name().c_str(), downloadedMod->Version().c_str(), downloadedMod->Name().c_str(), coreModInfo.second.version.c_str());
			}

			auto coreModText = BeatSaberUI::CreateText(modalLayout->get_transform(), text);

			coreModText->set_enableWordWrapping(true);
			coreModText->set_alignment(TMPro::TextAlignmentOptions::Center);
			if (!downloadedModOpt.has_value()) coreModText->set_color({1, 0, 0, 1}); // Make text red if core mod is missing
		}

		auto bottomPannel = SetupBottomPannel(modal->get_transform());
		
		CancelButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Ignore", {"CancelButton"}, [=](){
			Object::Destroy(screen);
		});

		ConfirmButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Update", {"ApplyButton"}, [=](){
			ConfirmButton->set_interactable(false);
			CancelButton->set_interactable(false);

			std::thread(QModUtils::InstallMissingCoreMods, true).detach();
		});
	} else {
		auto failedToLoad = QModUtils::GetFailedToLoadMods();
		if (failedToLoad.size() == 0 || !getMainConfig().PromptWhenModFailsToLoad.GetValue()) co_return;

		auto headerText = BeatSaberUI::CreateText(modalLayout->get_transform(), "There seems to be some mods that failed to load\n");
		headerText->set_alignment(TMPro::TextAlignmentOptions::Center);

		std::vector<QModUtils::QMod*>* modsToReload = new std::vector<QModUtils::QMod*>();

		for (auto modPair : failedToLoad) {
			auto downloadedMod = QModUtils::QMod::GetDownloadedQMod(modPair.first).value();

			auto modText = BeatSaberUI::CreateText(modalLayout->get_transform(), downloadedMod->Name());

			modText->set_enableWordWrapping(true);
			modText->set_alignment(TMPro::TextAlignmentOptions::Center);
			modText->set_color({1, 0, 0, 1});

			modsToReload->push_back(modPair.second);
		}

		auto reloadText = BeatSaberUI::CreateText(modalLayout->get_transform(), "\nReloading may possible fix these issues\nDo you want to reload?");
		reloadText->set_alignment(TMPro::TextAlignmentOptions::Center);

		auto bottomPannel = SetupBottomPannel(modal->get_transform());
		
		CancelButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Ignore", {"CancelButton"}, [=](){
			Object::Destroy(screen);
		});

		ConfirmButton = QuestUI::BeatSaberUI::CreateUIButton(bottomPannel->get_transform(), "Reload", {"ApplyButton"}, [=](){
			ConfirmButton->set_interactable(false);
			CancelButton->set_interactable(false);

			std::thread(
				[modsToReload] {
					QModUtils::ReloadMods(modsToReload, nullptr);
					JNIUtils::RestartApp();
				}
			).detach();
		});
	}

    modal->Show(true, true, nullptr);

    co_return;
}

MAKE_HOOK_MATCH(MainFlowCoordinator_DidActivate, &GlobalNamespace::MainFlowCoordinator::DidActivate, void, GlobalNamespace::MainFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    MainFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    static bool dialogOpened = false;
    if (!dialogOpened) {
        dialogOpened = true;
        self->StartCoroutine(
                reinterpret_cast<System::Collections::IEnumerator *>(custom_types::Helpers::CoroutineHelper::New(
                        openDialogLater())));
    }
}


void InstallModalPromptHook(Logger &logger) {
    if (QModUtils::GetMissingCoreMods().size() == 0 && QModUtils::GetModErrors().size() == 0) return;

    INSTALL_HOOK(logger, MainFlowCoordinator_DidActivate);
}