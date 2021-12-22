#include "UI/FlowCoordinators/ModListFlowCoordinator.hpp"

#include "HMUI/ViewController_AnimationDirection.hpp"
#include "HMUI/ViewController_AnimationType.hpp"

DEFINE_TYPE(HotSwappableMods, ModListFlowCoordinator);

void HotSwappableMods::ModListFlowCoordinator::Awake() {
    if (!ModListViewController) ModListViewController = QuestUI::BeatSaberUI::CreateViewController<HotSwappableMods::ModListViewController*>();
}

void HotSwappableMods::ModListFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        static Il2CppString* titleName = il2cpp_utils::newcsstr("Mod List");
        SetTitle(titleName, HMUI::ViewController::AnimationType::In);
        showBackButton = true;
        ProvideInitialViewControllers(ModListViewController, nullptr, nullptr, nullptr, nullptr);
    }
}

void HotSwappableMods::ModListFlowCoordinator::BackButtonWasPressed(HMUI::ViewController* topViewController){
    this->parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);

	ClearModsToToggle(); // This clears the mods to enable/disable list in the ModListViewController
}