#include "modules/Screen.h"

namespace embark_assist {
    namespace screen {
        bool paintString(const DFHack::Screen::Pen &pen, int x, int y, const std::string24 &text, bool map = false);
    }
}