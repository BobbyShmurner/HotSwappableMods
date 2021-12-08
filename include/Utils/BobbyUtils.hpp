#pragma once

#include <string>
#include <typeinfo>

#include "beatsaber-hook/shared/utils/typedefs.h"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"

class BobbyUtils {
public:
	static bool IsNumber(const std::string& str);

	template<class T>
	static std::string ToString(T obj) {
		if (obj.ToString()) {
			auto* csStr = obj.ToString();

			if (!std::is_same<decltype(csStr), Il2CppString*>()) { return "Null"; }

			return Il2cppStrToStr((Il2CppString*)csStr);
		}

		return "Null";
	}

	static std::string Il2cppStrToStr(Il2CppString* str);

	static void LogComponents(UnityEngine::GameObject* go);

	static void LogHierarchy(UnityEngine::Transform* trans, int parents = 0);
	static void LogComponentHierarchy(UnityEngine::GameObject* go, int parents = 0);

private:
	static void LogHierarchyRecurse(UnityEngine::Transform* trans, int parents = 0, int level = 0, UnityEngine::Transform* ogTrans = nullptr, bool logComponents = false);
};