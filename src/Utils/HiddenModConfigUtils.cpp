#include "Utils/HiddenModConfigUtils.hpp"
#include "main.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <list>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern ModInfo modInfo;

namespace HiddenModConfigUtils {
	std::list<std::string> ModsToHide = { {"libpinkcore"}, {"libsongdownloader"}, {"libsongloader"}, {"libplaylistmanager"} };
	std::list<std::string> NoNoMods = { {"libHotSwappableMods"}, {"libmain"}, {"libmodloader"} }; // These cant be disabled no matter what

	std::string ConfigDir = "/sdcard/Android/data/com.beatgames.beatsaber/files/HotSwappableMods/";
	std::string ConfigName = "modstohide.json";

	std::string ConfigLoc = ConfigDir + ConfigName;

	void SetModsToHide() {
		std::string configPath = ConfigLoc;

		struct stat st = {0};

		if (stat(ConfigDir.c_str(), &st) == -1) {
			mkdir(ConfigDir.c_str(), 0700);
		}

		std::ifstream configFile(configPath);
		std::stringstream configStream;
		configStream << configFile.rdbuf();

		rapidjson::Document configDoc;
		configDoc.Parse(configStream.str());

		if (!configDoc.IsObject()) {
			configDoc.SetObject();
		}

		if (configDoc.HasMember("ModsToHide")) {
			rapidjson::Value& configModsToHide = configDoc["ModsToHide"];

			if (configModsToHide.IsArray()) {
				ModsToHide.clear();
				for (rapidjson::SizeType i = 0; i < configModsToHide.Size(); i++) { // rapidjson uses SizeType instead of size_t.
					ModsToHide.emplace_front(std::string(configModsToHide[i].GetString()));
				}

				return;
			}
		}

		// Create the Array
		if (configDoc.HasMember("ModsToHide")) configDoc.RemoveMember("ModsToHide");

		rapidjson::Value ary (rapidjson::kArrayType);

		for (std::string modToHide : ModsToHide) {
			rapidjson::Value modVal;

			char buffer[50];
			int len = sprintf(buffer, "%s", modToHide.c_str()); // dynamically created string.
			modVal.SetString(buffer, len, configDoc.GetAllocator());
			memset(buffer, 0, sizeof(buffer));

			ary.PushBack(modVal, configDoc.GetAllocator());
		}

		configDoc.AddMember("ModsToHide", ary, configDoc.GetAllocator());

		// This was originally meant to be a seperate function, but the configDoc wasnt playing nice, so i just inlined it

		FILE* fp = fopen(ConfigLoc.c_str(), "w"); // Windows use "wb"
 
		char writeBuffer[65536];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		
		rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
		configDoc.Accept(writer);
		
		fclose(fp);
	}

	std::list<std::string> GetHiddenModsList() { return ModsToHide; }
	std::list<std::string> GetNoNoModsList() { return NoNoMods; }
}