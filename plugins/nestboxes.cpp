#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"

#include "DataDefs.h"
#include "df/world.h"
#include "df/ui.h"
#include "df/building_nest_boxst.h"
#include "df/building_type.h"
#include "df/buildings_other_id.h"
#include "df/global_objects.h"
#include "df/item.h"
#include "df/unit.h"
#include "df/building.h"
#include "df/items_other_id.h"
#include "df/creature_raw.h"
#include "modules/MapCache.h"
#include "modules/Items.h"




using std::endl;
using namespace DFHack;
using namespace df::enums;

using df::global::world;
using df::global::ui;

static command_result nestboxes(color_ostream &out, std::vector12<std::string24> & parameters);

DFHACK_PLUGIN("nestboxes");

DFHACK_PLUGIN_IS_ENABLED(enabled);

static void eggscan(color_ostream &out)
{
    CoreSuspender suspend;

    //for (df::building *build : world->buildings.other[df::buildings_other_id::NEST_BOX])
    std::vector12<df::building*> const& nboxes = world->buildings.other[df::buildings_other_id::NEST_BOX];
    for (std::vector12<df::building*>::const_iterator cit = nboxes.begin(); cit != nboxes.end(); ++cit)
    {
        df::building* build = *cit;
        df::building_type type = build->getType();
        if (df::enums::building_type::NestBox == type)
        {
            bool fertile = false;
            df::building_nest_boxst *nb = virtual_cast<df::building_nest_boxst>(build);
            if (nb->claimed_by != -1)
            {
                df::unit* u = df::unit::find(nb->claimed_by);
                if (u && u->pregnancy_timer > 0)
                    fertile = true;
            }
            for (size_t j = 1; j < nb->contained_items.size(); j++)
            {
                df::item* item = nb->contained_items[j]->item;
                if (item->flags.bits.forbid != fertile)
                {
                    item->flags.bits.forbid = fertile;
                    out << item->getStackSize() << " eggs " << (fertile ? "forbidden" : "unforbidden.") << endl;
                }
            }
        }
    }
}


DFhackCExport command_result plugin_init (color_ostream &out, std::vector12<PluginCommand> &commands)
{
    if (world && ui) {
        commands.push_back(
            PluginCommand("nestboxes", "Automatically scan for and forbid fertile eggs incubating in a nestbox.",
                nestboxes, false,
                "To enable: nestboxes enable\n"
                "To disable: nestboxes disable\n"
                "There is no other configuration.\n"
            )
        );
    }
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    return CR_OK;
}

DFhackCExport command_result plugin_onupdate(color_ostream &out)
{
    if (!enabled)
        return CR_OK;

    static unsigned cnt = 0;
    if ((++cnt % 5) != 0)
        return CR_OK;

    eggscan(out);

    return CR_OK;
}

DFhackCExport command_result plugin_enable(color_ostream &out, bool enable)
{
    enabled = enable;
    return CR_OK;
}

static command_result nestboxes(color_ostream &out, std::vector12<std::string24> & parameters)
{
    CoreSuspender suspend;

    if (parameters.size() == 1) {
        if (parameters[0] == "enable")
            enabled = true;
        else if (parameters[0] == "disable")
            enabled = false;
        else
            return CR_WRONG_USAGE;
    } else {
        out << "Plugin " << (enabled ? "enabled" : "disabled") << "." << endl;
    }
    return CR_OK;
}

