#include "UI/ViewControllers/InfoViewController.hpp"

#include "Utils/BobbyUtils.hpp"
#include "qmod-utils/shared/QModUtils.hpp"

#include "DataTypes/MainConfig.hpp"

#include "UnityEngine/RectOffset.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/TextAnchor.hpp"
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
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/ViewController_AnimationType.hpp"

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

DEFINE_TYPE(HotSwappableMods, InfoViewController);

#define CREATE_COLOR_INFO(colorName, colorR, colorG, colorB, colorA, colorDisplayName, info) \
UnityEngine::UI::HorizontalLayoutGroup* colorName##TextPannel = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(mainContainer->get_transform()); \
FormatPannel(colorName##TextPannel); \
\
TMPro::TextMeshProUGUI* colorName##Text = QuestUI::BeatSaberUI::CreateText(colorName##TextPannel->get_transform(), colorDisplayName, false); \
colorName##Text->set_color({colorR, colorG, colorB, colorA}); \
colorName##Text->set_alignment(TMPro::TextAlignmentOptions::Left); \
\
TMPro::TextMeshProUGUI* colorName##TextDesc = QuestUI::BeatSaberUI::CreateText(colorName##TextPannel->get_transform(), std::string("- ") + std::string(info), true); \
colorName##TextDesc->set_alignment(TMPro::TextAlignmentOptions::Left); \
colorName##TextDesc->set_lineSpacing(-40)

void FormatPannel(UnityEngine::UI::HorizontalLayoutGroup* pannel) {
	pannel->set_spacing(-50);
	pannel->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

	pannel->set_childForceExpandWidth(false);
	pannel->set_childForceExpandHeight(false);
	pannel->set_childControlWidth(false);
	pannel->set_childControlHeight(false);
}

void HotSwappableMods::InfoViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	QuestUI::BeatSaberUI::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf()->SetTitle(il2cpp_utils::newcsstr("Info"), HMUI::ViewController::AnimationType::In);

	if (!firstActivation) return;

	UnityEngine::GameObject* mainContainer = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
	TMPro::TextMeshProUGUI* titleText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Info", false);
	
	titleText->set_fontSize(10.0f);
	titleText->set_alignment(TMPro::TextAlignmentOptions::Center);

	TMPro::TextMeshProUGUI* reloadDescText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "The reload button allows you to reinstall each mod that is currently\nenabled. This can be useful if any of your mods break for whatever\nreason (Missing Images, Mods not Loading, etc.). So if you are ever\nfacing some odd issues with a mod or two, try reloading. There's no\nguarantee that it will fix the issue, but there's no harm in trying.", true);

	reloadDescText->set_alignment(TMPro::TextAlignmentOptions::Center);
	reloadDescText->set_lineSpacing(-40);
	reloadDescText->get_rectTransform()->set_sizeDelta({1, 30});

	TMPro::TextMeshProUGUI* colourDescText = QuestUI::BeatSaberUI::CreateText(mainContainer->get_transform(), "Each Mod may have different colored text, so to understand what\neach of them mean I've created this list which should explain what\neach color means", true);

	colourDescText->set_alignment(TMPro::TextAlignmentOptions::Center);
	colourDescText->set_lineSpacing(-40);

	// White Text

	CREATE_COLOR_INFO(white, 1, 1, 1, 1, "White", "Loaded Mod");

	// Gray Text

	CREATE_COLOR_INFO(gray, 0.55, 0.55, 0.55, 1, "Gray", "Unloaded Mod");

	// Yellow Text

	CREATE_COLOR_INFO(yellow, 1, 1, 0, 1, "Yellow", "Mod will be loaded/unloaded on reload");

	// Red Text

	CREATE_COLOR_INFO(orange, 1, 0, 0, 1, "Red", "Mod failed to load. You can check the Hover Hint\n  for more detail");
}