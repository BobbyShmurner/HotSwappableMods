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

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include <stdlib.h>

#include <sstream>
#include <string>
#include <vector>

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;
using namespace HMUI;
using namespace TMPro;

extern ModInfo modInfo;

DEFINE_TYPE(HotSwappableMods, MainViewController);

void PopulateModToggles(UnityEngine::Transform* container) {
	for (std::string fileName : DirUtils::GetDirContents("/sdcard/Android/data/com.beatgames.beatsaber/files/mods/")) {
		QuestUI::BeatSaberUI::CreateToggle(container, std::string_view(fileName), true);
	}
}

void HotSwappableMods::MainViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (!firstActivation) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Mod List", false);

	PopulateModToggles(mainContainer->get_transform());
}