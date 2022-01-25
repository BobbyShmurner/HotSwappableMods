#pragma once

#include "config-utils/shared/config-utils.hpp"

#include <string>
#include <list>

DECLARE_CONFIG(MainConfig,
    CONFIG_VALUE(AlwaysShowFileNames, bool, "AlwaysShowFileNames", false);
    CONFIG_VALUE(PromptWhenCoreModsOutdated, bool, "PromptWhenCoreModsOutdated", true);
    CONFIG_VALUE(PromptWhenModFailsToLoad, bool, "PromptWhenModFailsToLoad", true);

    CONFIG_VALUE(ShowHoverHints, bool, "ShowHoverHints", true);
    CONFIG_VALUE(ShowFileNameOnHoverHint, bool, "ShowFileNameOnHoverHint", true);
    CONFIG_VALUE(ShowModIDOnHoverHint, bool, "ShowModIDOnHoverHint", true);
    CONFIG_VALUE(ShowModVersionOnHoverHint, bool, "ShowModVersionOnHoverHint", true);
    CONFIG_VALUE(ShowModErrorsOnHoverHint, bool, "ShowModErrorsOnHoverHint", true);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(AlwaysShowFileNames);
        CONFIG_INIT_VALUE(PromptWhenCoreModsOutdated);
        CONFIG_INIT_VALUE(PromptWhenModFailsToLoad);

        CONFIG_INIT_VALUE(ShowHoverHints);
        CONFIG_INIT_VALUE(ShowFileNameOnHoverHint);
        CONFIG_INIT_VALUE(ShowModIDOnHoverHint);
        CONFIG_INIT_VALUE(ShowModVersionOnHoverHint);
        CONFIG_INIT_VALUE(ShowModErrorsOnHoverHint);
    )
)