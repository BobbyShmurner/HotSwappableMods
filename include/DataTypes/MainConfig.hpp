#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(MainConfig,
    CONFIG_VALUE(AlwaysShowFileNames, bool, "AlwaysShowFileNames", false);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(AlwaysShowFileNames);
    )
)