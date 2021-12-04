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

	void LogComponents(UnityEngine::GameObject* go);

	void LogHierarchy(UnityEngine::Transform* trans, int parents = 0);
	void LogComponentHierarchy(UnityEngine::GameObject* go, int parents = 0);

	void LogHierarchyRecurse(UnityEngine::Transform* trans, int parents = 0, int level = 0, UnityEngine::Transform* ogTrans = nullptr, bool logComponents = false);
}