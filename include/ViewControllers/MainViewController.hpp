#pragma once

#include "main.hpp"

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/register.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "HMUI/NoTransitionsButton.hpp"

DECLARE_CLASS_CODEGEN(HotSwappableMods, MainViewController, HMUI::ViewController,
	DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
)