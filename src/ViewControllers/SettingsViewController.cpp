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

DEFINE_TYPE(HotSwappableMods, SettingsViewController);

void HotSwappableMods::SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if (!firstActivation) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());

	TMPro::TextMeshProUGUI* titleText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Settings", false);
	titleText->set_fontSize(10.0f);
	titleText->set_alignment(TMPro::TextAlignmentOptions::Center);

	QuestUI::BeatSaberUI::CreateToggle(mainContainer->get_transform(), "Always show file names", getMainConfig().AlwaysShowFileNames.GetValue(), [](bool value){
		ModUtils::UpdateAlwaysDisplayLibNames(value);
	});
}