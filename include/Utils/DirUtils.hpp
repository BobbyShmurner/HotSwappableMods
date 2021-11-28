#pragma once

#include <list>
#include <stdio.h>
#include <string>
#include <dirent.h>

namespace DirUtils {
	std::list<std::string> GetDirContents(const char* dirPath) {
		DIR* dir = opendir(dirPath);
		dirent* dp;
        std::list<std::string> files; 

		if (dir == nullptr) return files;

        while ((dp = readdir(dir)) != NULL) {
            if (strlen(dp->d_name) > 3) {
                files.emplace_front(std::string(dp->d_name));
            }
        }

        return files;
	}
}