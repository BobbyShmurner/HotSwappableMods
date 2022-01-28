#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"

#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/ViewController_AnimationType.hpp"

extern bool ShouldRefreshList;

DEFINE_TYPE(HotSwappableMods, ModListFlowCoordinator);

void HotSwappableMods::ModListFlowCoordinator::Awake() {
    if (ShouldRefreshList) {
        if (ModListViewController) DismissViewController(ModListViewController, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
        ModListViewController = QuestUI::BeatSaberUI::CreateViewController<HotSwappableMods::ModListViewController*>();

        if (InfoViewController) DismissViewController(InfoViewController, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
        InfoViewController = QuestUI::BeatSaberUI::CreateViewController<HotSwappableMods::InfoViewController*>();
    }
}

void HotSwappableMods::ModListFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(firstActivation) {
        static Il2CppString* titleName = il2cpp_utils::newcsstr("Mod List");
        SetTitle(titleName, HMUI::ViewController::AnimationType::In);
        showBackButton = true;
        ProvideInitialViewControllers(ModListViewController, nullptr, nullptr, nullptr, nullptr);
    }
}

void HotSwappableMods::ModListFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController) {
    if (ModListViewController->IsRestarting) return;

    if (InfoViewController->get_isActivated()) {
        DismissViewController(InfoViewController, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    }
    else {
        this->parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    }
}