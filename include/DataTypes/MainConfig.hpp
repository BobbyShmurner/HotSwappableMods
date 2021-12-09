#pragma once

#include "config-utils/shared/config-utils.hpp"

#include <string>
#include <list>

DECLARE_CONFIG(MainConfig,
    CONFIG_VALUE(AlwaysShowFileNames, bool, "AlwaysShowFileNames", false);

    CONFIG_VALUE(ShowHoverHints, bool, "ShowHoverHints", true);
    CONFIG_VALUE(ShowFileNameOnHoverHint, bool, "ShowFileNameOnHoverHint", true);
    CONFIG_VALUE(ShowModIDOnHoverHint, bool, "ShowModIDOnHoverHint", true);
    CONFIG_VALUE(ShowModTypeOnHoverHint, bool, "ShowModTypeOnHoverHint", true);
    CONFIG_VALUE(ShowModErrorsOnHoverHint, bool, "ShowModErrorsOnHoverHint", true);

    CONFIG_VALUE(ShowAdvancedSettings, bool, "ShowAdvancedSettings", false);
    CONFIG_VALUE(ShowCoreMods, bool, "ShowCoreMods", false);
    CONFIG_VALUE(ShowLibs, bool, "ShowLibs", false);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(AlwaysShowFileNames);

        CONFIG_INIT_VALUE(ShowHoverHints);
        CONFIG_INIT_VALUE(ShowFileNameOnHoverHint);
        CONFIG_INIT_VALUE(ShowModIDOnHoverHint);
        CONFIG_INIT_VALUE(ShowModTypeOnHoverHint);
        CONFIG_INIT_VALUE(ShowModErrorsOnHoverHint);

        CONFIG_INIT_VALUE(ShowAdvancedSettings);
        CONFIG_INIT_VALUE(ShowCoreMods);
        CONFIG_INIT_VALUE(ShowLibs);
    )
)