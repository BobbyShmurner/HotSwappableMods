#pragma once

#include <string>
#include <typeinfo>

#include "beatsaber-hook/shared/utils/typedefs.h"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"

namespace BobbyUtils {
	bool IsNumber(const std::string& str);

	template<class T>
	std::string ToString(T obj);
	std::string Il2cppStrToStr(Il2CppString* str);

	void LogHierarchy(UnityEngine::Transform* trans, int level = 0);
}