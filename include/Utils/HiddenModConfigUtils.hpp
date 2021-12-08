#pragma once

#include "main.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include <string>

namespace HiddenModConfigUtils {
	void SetModsToHide();

	std::list<std::string>* GetCoreMods();
	std::list<std::string> GetNoNoModsList();
}