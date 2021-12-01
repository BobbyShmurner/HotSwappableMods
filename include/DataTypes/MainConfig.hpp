#pragma once

#include "config-utils/shared/config-utils.hpp"

#include <string>
#include <list>

DECLARE_CONFIG(MainConfig,
    CONFIG_VALUE(AlwaysShowFileNames, bool, "AlwaysShowFileNames", false);
    CONFIG_VALUE(ShowCoreMods, bool, "ShowCoreMods", false);
    CONFIG_VALUE(ShowLibs, bool, "ShowLibs", false);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(AlwaysShowFileNames);
        CONFIG_INIT_VALUE(ShowCoreMods);
        CONFIG_INIT_VALUE(ShowLibs);
    )
)