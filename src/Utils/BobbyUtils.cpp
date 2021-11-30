#include <string>
#include <typeinfo>

#include "main.hpp"

#include "beatsaber-hook/shared/utils/typedefs.h"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"

namespace BobbyUtils {
	bool IsNumber(const std::string& str)
	{
		for (char const &c : str) {
			if (std::isdigit(c) == 0) return false;
		}
		return true;
	}

	std::string Il2cppStrToStr(Il2CppString* str) {
		return to_utf8(csstrtostr(str));
	}

	template<class T>
	std::string ToString(T obj) {
		if (obj.ToString()) {
			auto* csStr = obj.ToString();

			if (!std::is_same<decltype(csStr), Il2CppString*>()) { return "Null"; }

			return Il2cppStrToStr((Il2CppString*)csStr);
		}

		return "Null";
	}

	void LogHierarchy(UnityEngine::Transform* trans, int level = 0) {
		if (level == 0) getLogger().info("Logging Hierarchy for %s:", BobbyUtils::Il2cppStrToStr(trans->get_name()).c_str());

		std::string start;
		for (int i = 0; i < level; i++) {
			start += "\t";
		}
		start += "- ";

		for (int i = 0; i < trans->get_childCount(); i++) {
			getLogger().info("%s[%i] %s", start.c_str(), i, BobbyUtils::Il2cppStrToStr(trans->GetChild(i)->get_name()).c_str());
			LogHierarchy(trans->GetChild(i), level + 1);
		}
	}
}