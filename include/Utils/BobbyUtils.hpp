#pragma once

#include <string>
#include <typeinfo>
#include <concepts>

#include "beatsaber-hook/shared/utils/typedefs.h"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"

// Thanks to Mr Sc2badam for this wonderful code
template<class T>
concept stringable = requires (T t) {
	{t.ToString()} -> std::same_as<Il2CppString*>;
};

class BobbyUtils {
public:
	static bool IsNumber(const std::string& str);

	// Once again, a huge thanks to Sir Scad Mc Bad for showing me how to do code correctly

	// Only allows you to call ToString<T> on types that actually HAVE
	// a ToString method that returns a const char* (in this case)
	template<class T>
	requires ((std::is_reference_v<T> && ((std::is_pointer_v<std::remove_reference_t<T>> && stringable<std::remove_pointer_t<std::remove_reference_t<T>>>) || stringable<std::remove_reference_t<T>>)) || (std::is_pointer_v<T> && stringable<std::remove_pointer_t<T>>) || stringable<T>)
	static std::string ToString(T&& arg) {
		if constexpr (std::is_reference_v<T> || std::is_pointer_v<T>) {
			return Il2cppStrToStr(arg->ToString());
		} else {
			return Il2cppStrToStr(arg.ToString());
		}
	}

	static std::string Il2cppStrToStr(Il2CppString* str);

	static void LogComponents(UnityEngine::GameObject* go);

	static void LogHierarchy(UnityEngine::Transform* trans, int parents = 0);
	static void LogComponentHierarchy(UnityEngine::Transform* trans, int parents = 0);

private:
	static void LogHierarchyRecurse(UnityEngine::Transform* trans, int parents = 0, int level = 0, UnityEngine::Transform* ogTrans = nullptr, bool logComponents = false);
};