#include <string>
#include <typeinfo>

#include "main.hpp"
#include "Utils/BobbyUtils.hpp"

#include "beatsaber-hook/shared/utils/typedefs.h"

#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Component.hpp"

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"

bool BobbyUtils::IsNumber(const std::string& str)
{
	for (char const &c : str) {
		if (std::isdigit(c) == 0) return false;
	}
	return true;
}

std::string BobbyUtils::Il2cppStrToStr(Il2CppString* str) {
	return to_utf8(csstrtostr(str));
}

template<class T>
std::string BobbyUtils::ToString(T* obj) {
	if (obj->ToString()) {
		auto* csStr = obj->ToString();

		if (!std::is_same<decltype(csStr), Il2CppString*>()) { return "Null"; }

		return Il2cppStrToStr((Il2CppString*)csStr);
	}

	return "Null";
}

void BobbyUtils::LogComponents(UnityEngine::GameObject* go) {
	getLogger().info("Logging Componts for %s", Il2cppStrToStr(go->get_name()).c_str());

	Array<UnityEngine::Component*>* components = go->GetComponents(csTypeOf(UnityEngine::Component*));

	for(int i = 0; i < components->Length(); i++) {
		getLogger().info("- [%i] %s", i, ToString(components->GetValue(i)).c_str());
	}
}

void BobbyUtils::LogHierarchyRecurse(UnityEngine::Transform* trans, int parents, int level, UnityEngine::Transform* ogTrans, bool logComponents) {
	if (ogTrans == nullptr) ogTrans = trans;

	if (parents > 0) {
		UnityEngine::Transform* parent = trans->get_parent();
		if (parent != nullptr) LogHierarchyRecurse(parent, parents - 1, level, ogTrans, logComponents);
		else LogHierarchyRecurse(trans, 0, level, ogTrans, logComponents);
		return;
	}
	if (level == 0) getLogger().info("Logging %sHierarchy for %s:", logComponents ? "Component " : "", BobbyUtils::Il2cppStrToStr(trans->get_name()).c_str());

	std::string start;
	for (int i = 0; i < level; i++) {
		start += "\t";
	}
	start += "- ";

	for (int i = 0; i < trans->get_childCount(); i++) {

		if (logComponents) {
			Array<UnityEngine::Component*>* components = trans->GetChild(i)->GetComponents(csTypeOf(UnityEngine::Component*));

			for(int j = 0; j < components->Length(); j++) {
				getLogger().info("%s[%i:%i] %s%s", start.c_str(), i, j, ToString(components->GetValue(j)).c_str(), ogTrans == trans->GetChild(i) ? " (OBJECT BEING LOGGED)" : "");
			}
		}
		else {
			getLogger().info("%s[%i] %s%s", start.c_str(), i, BobbyUtils::Il2cppStrToStr(trans->GetChild(i)->get_name()).c_str(), ogTrans == trans->GetChild(i) ? " (OBJECT BEING LOGGED)" : "");
		}

		LogHierarchyRecurse(trans->GetChild(i), 0, level + 1, ogTrans, logComponents);
	}
}

void BobbyUtils::LogHierarchy(UnityEngine::Transform* trans, int parents) {
	LogHierarchyRecurse(trans, parents, 0, trans, false);
}

void BobbyUtils::LogComponentHierarchy(UnityEngine::GameObject* go, int parents) {
	LogHierarchyRecurse(go->get_transform(), parents, 0, go->get_transform(), true);
}