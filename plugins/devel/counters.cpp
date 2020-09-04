// Show creature counter values

#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include "modules/Gui.h"
#include "df/unit.h"
#include "df/unit_misc_trait.h"




using namespace DFHack;
using namespace df::enums;

command_result df_counters (color_ostream &out, std::vector12<std::string24> & parameters)
{
    CoreSuspender suspend;

    df::unit *unit = Gui::getSelectedUnit(out);
    if (!unit)
        return CR_WRONG_USAGE;
    std::vector12<df::unit_misc_trait*> &counters = unit->status.misc_traits;
    for (size_t i = 0; i < counters.size(); i++)
    {
        df::unit_misc_trait* counter = counters[i];
        out.print("%i (%s): %i\n", counter->id, ENUM_KEY_STR_SIMPLE(misc_trait_type, counter->id).c_str(), counter->value);
    }

    return CR_OK;
}

DFHACK_PLUGIN("counters");

DFhackCExport command_result plugin_init ( color_ostream &out, std::vector12<PluginCommand> &commands)
{
    commands.push_back(PluginCommand("counters",
                                     "Display counters for currently selected creature",
                                     df_counters));
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    return CR_OK;
}
