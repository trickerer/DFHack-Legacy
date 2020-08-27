#pragma once
#include "Export.h"
#include <string>

#include "common.h"

namespace DFHack {
    namespace Once {
        DFHACK_EXPORT bool alreadyDone(std::string24&);
        DFHACK_EXPORT bool doOnce(std::string24);
    }
}

