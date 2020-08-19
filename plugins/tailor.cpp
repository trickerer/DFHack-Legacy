/*
 * Tailor plugin. Automatically manages keeping your dorfs clothed.
 * For best effect, place "tailor enable" in your dfhack.init configuration,
 * or set AUTOENABLE to true.
 */

#include "Core.h"
#include "DataDefs.h"
#include "PluginManager.h"

#include "df/creature_raw.h"
#include "df/global_objects.h"
#include "df/historical_entity.h"
#include "df/itemdef_armorst.h"
#include "df/itemdef_glovesst.h"
#include "df/itemdef_helmst.h"
#include "df/itemdef_pantsst.h"
#include "df/itemdef_shoesst.h"
#include "df/items_other_id.h"
#include "df/job.h"
#include "df/job_type.h"
#include "df/manager_order.h"
#include "df/ui.h"
#include "df/world.h"

#include "modules/Maps.h"
#include "modules/Units.h"
#include "modules/Translation.h"
#include "modules/World.h"

using namespace DFHack;
using namespace std;

using df::global::world;
using df::global::ui;

DFHACK_PLUGIN("tailor");
#define AUTOENABLE false
DFHACK_PLUGIN_IS_ENABLED(enabled);

REQUIRE_GLOBAL(world);
REQUIRE_GLOBAL(ui);

const char *tagline = "Allow the bookkeeper to queue jobs to keep dwarfs in adequate clothing.";
const char *usage = (
    "  tailor enable\n"
    "    Enable the plugin.\n"
    "  tailor disable\n"
    "    Disable the plugin.\n"
    "  tailor status\n"
    "    Display plugin status\n"
    "\n"
    "Whenever the bookkeeper updates stockpile records, this plugin will scan every unit in the fort,\n"
    "count up the number that are worn, and then order enough more made to replace all worn items.\n"
    "If there are enough replacement items in inventory to replace all worn items, the units wearing them\n"
    "will have the worn items confiscated (in the same manner as the _cleanowned_ plugin) so that they'll\n"
    "reeequip with replacement items.\n"
);

// ARMOR, SHOES, HELM, GLOVES, PANTS

// ah, if only STL had a bimap

//static map<df::job_type, df::item_type> jobTypeMap = {
//    { df::job_type::MakeArmor, df::item_type::ARMOR },
//    { df::job_type::MakePants, df::item_type::PANTS },
//    { df::job_type::MakeHelm, df::item_type::HELM },
//    { df::job_type::MakeGloves, df::item_type::GLOVES },
//    { df::job_type::MakeShoes, df::item_type::SHOES }
//};
//
//static map<df::item_type, df::job_type> itemTypeMap = {
//    { df::item_type::ARMOR, df::job_type::MakeArmor },
//    { df::item_type::PANTS, df::job_type::MakePants },
//    { df::item_type::HELM, df::job_type::MakeHelm},
//    { df::item_type::GLOVES, df::job_type::MakeGloves},
//    { df::item_type::SHOES, df::job_type::MakeShoes}
//};

std::pair<df::job_type, df::item_type> jobTypesArr[] = {
    std::make_pair( df::job_type::MakeArmor, df::item_type::ARMOR ),
    std::make_pair( df::job_type::MakePants, df::item_type::PANTS ),
    std::make_pair( df::job_type::MakeHelm, df::item_type::HELM ),
    std::make_pair( df::job_type::MakeGloves, df::item_type::GLOVES ),
    std::make_pair( df::job_type::MakeShoes, df::item_type::SHOES )
};
static map<df::job_type, df::item_type> jobTypeMap(jobTypesArr, jobTypesArr + sizeof(jobTypesArr)/sizeof(jobTypesArr[0]));

std::pair<df::item_type, df::job_type> itemTypesArr[] = {
    std::make_pair( df::item_type::ARMOR, df::job_type::MakeArmor ),
    std::make_pair( df::item_type::PANTS, df::job_type::MakePants ),
    std::make_pair( df::item_type::HELM, df::job_type::MakeHelm ),
    std::make_pair( df::item_type::GLOVES, df::job_type::MakeGloves ),
    std::make_pair( df::item_type::SHOES, df::job_type::MakeShoes )
};
static map<df::item_type, df::job_type> itemTypeMap(itemTypesArr, itemTypesArr + sizeof(itemTypesArr)/sizeof(itemTypesArr[0]));

//tuple substitute
struct JobKey
{
public:
    JobKey() : type(df::job_type::NONE), subtype(0), size(0) {}
    explicit JobKey(df::job_type _type, int _subtype, int _size) : type(_type), subtype(_subtype), size(_size) {}

    df::job_type type;
    int subtype;
    int size;
};
struct JobKeyHash
{
   size_t operator()(const JobKey& key) const
   {
       return key.subtype + (key.type*244)^0xBADFEEDA;
   }
	bool operator()(const JobKey& key1, const JobKey& key2) const
	{
	    return &key1 == &key2;
	}
	enum
	{
	    bucket_size = 4,
	    min_buckets = 8
   };
};

void do_scan(color_ostream& out)
{
    typedef map<pair<df::item_type, int>, int> OrdersMap;
    OrdersMap available; // key is item type & size
    OrdersMap needed;    // same
    OrdersMap queued;    // same

    map<int, int> sizes; // this maps body size to races

    //map<tuple<df::job_type, int, int>, int> orders;  // key is item type, item subtype, size
    map<JobKey, int, JobKeyHash> orders;

    df::item_flags bad_flags;
    bad_flags.whole = 0;

#define F(x) bad_flags.bits.x = true;
    F(dump); F(forbid); F(garbage_collect);
    F(hostile); F(on_fire); F(rotten); F(trader);
    F(in_building); F(construction); F(owned);
#undef F

    available.empty();
    needed.empty();
    queued.empty();
    orders.empty();

    int silk = 0, yarn = 0, cloth = 0, leather = 0;

    // scan for useable clothing

    //for (auto i : world->items.other[df::items_other_id::ANY_GENERIC37]) // GENERIC37 is "clothing"
    std::vector<df::item*> const& ovec = world->items.other[df::items_other_id::ANY_GENERIC37];
    for (std::vector<df::item*>::const_iterator ci = ovec.begin(); ci != ovec.end(); ++ci)
    {
        df::item* i = *ci;
        if (i->flags.whole & bad_flags.whole)
            continue;
        if (i->flags.bits.owned)
            continue;
        if (i->getWear() >= 1)
            continue;
        df::item_type t = i->getType();
        int size = world->raws.creatures.all[i->getMakerRace()]->adultsize;

        available[make_pair(t, size)] += 1;
    }

    // scan for clothing raw materials

    //for (auto i : world->items.other[df::items_other_id::CLOTH])
    std::vector<df::item*> const& cvec = world->items.other[df::items_other_id::CLOTH];
    for (std::vector<df::item*>::const_iterator ci = cvec.begin(); ci != cvec.end(); ++ci)
    {
        df::item* i = *ci;
        if (i->flags.whole & bad_flags.whole)
            continue;
        if (!i->hasImprovements()) // only count dyed
            continue;
        MaterialInfo mat(i);
        int ss = i->getStackSize();

        if (mat.material)
        {
            if (mat.material->flags.is_set(df::material_flags::SILK))
                silk += ss;
            else if (mat.material->flags.is_set(df::material_flags::THREAD_PLANT))
                cloth += ss;
            else if (mat.material->flags.is_set(df::material_flags::YARN))
                yarn += ss;
        }
    }

    //for (auto i : world->items.other[df::items_other_id::SKIN_TANNED])
    std::vector<df::item*> const& stvec = world->items.other[df::items_other_id::SKIN_TANNED];
    for (std::vector<df::item*>::const_iterator ci = stvec.begin(); ci != stvec.end(); ++ci)
    {
        if ((*ci)->flags.whole & bad_flags.whole)
            continue;
        leather += (*ci)->getStackSize();
    }

    out.print("available: silk %d yarn %d cloth %d leather %d\n", silk, yarn, cloth, leather);

    // scan for units who need replacement clothing

    //for (auto u : world->units.active)
    std::vector<df::unit*> const& aunits = world->units.active;
    for (std::vector<df::unit*>::const_iterator ci = aunits.begin(); ci != aunits.end(); ++ci)
    {
        df::unit* u = *ci;
        if (!Units::isOwnCiv(u) ||
            !Units::isOwnGroup(u) ||
            !Units::isActive(u) ||
            Units::isBaby(u))
            continue; // skip units we don't control

        set <df::item_type> wearing;
        wearing.empty();

        deque<df::item*> worn;
        worn.empty();

        //for (auto inv : u->inventory)
        for (std::vector<df::unit_inventory_item*>::const_iterator cit = u->inventory.begin(); cit != u->inventory.end(); ++cit)
        {
            df::unit_inventory_item* inv = *cit;
            if (inv->mode != df::unit_inventory_item::Worn)
                continue;
            if (inv->item->getWear() > 0)
                worn.push_back(inv->item);
            else
                wearing.insert(inv->item->getType());
        }

        int size = world->raws.creatures.all[u->race]->adultsize;
        sizes[size] = u->race;

        //for (auto ty : set<df::item_type>{ df::item_type::ARMOR, df::item_type::PANTS, df::item_type::SHOES })
        static const df::item_type apstypes[3] = { df::item_type::ARMOR, df::item_type::PANTS, df::item_type::SHOES };
        for (uint8 i = 0; i < 3; ++i)
        {
            df::item_type ty = apstypes[i];
            if (wearing.count(ty) == 0)
                needed[make_pair(ty, size)] += 1;
        }

        //for (auto w : worn)
        for (std::deque<df::item*>::const_iterator cit = worn.begin(); cit != worn.end(); ++cit)
        {
            df::item* w = *cit;
            df::item_type ty = w->getType();
            map<df::item_type, df::job_type>::const_iterator oo = itemTypeMap.find(ty);
            if (oo == itemTypeMap.end())
                continue;
            df::job_type o = oo->second;

            int size = world->raws.creatures.all[w->getMakerRace()]->adultsize;
            std::string description;
            w->getItemDescription(&description, 0);

            if (available[make_pair(ty, size)] > 0)
            {
                if (w->flags.bits.owned)
                {
                    bool confiscated = Items::setOwner(w, NULL);

                    out.print(
                        "%s %s from %s.\n",
                        (confiscated ? "Confiscated" : "Could not confiscate"),
                        description.c_str(),
                        Translation::TranslateName(&u->name, false).c_str()
                    );
                }

                if (wearing.count(ty) == 0)
                    available[make_pair(ty, size)] -= 1;

                if (w->getWear() > 1)
                    w->flags.bits.dump = true;
            }
            else
            {
//                out.print("%s worn by %s needs replacement\n",
//                    description.c_str(),
//                    Translation::TranslateName(&u->name, false).c_str()
//                );
                orders[JobKey(o, w->getSubtype(), size)] += 1;
            }
        }
    }

    df::historical_entity* entity = world->entities.all[ui->civ_id];

    //for (auto a : needed)
    for (OrdersMap::const_iterator ci = needed.begin(); ci != needed.end(); ++ci)
    {
        pair<pair<df::item_type, int>, int> const& a = *ci;
        df::item_type ty = a.first.first;
        int size = a.first.second;
        int count = a.second;

        int sub = 0;
        vector<int16_t> v;

        switch (ty) {
        case df::item_type::ARMOR:  v = entity->resources.armor_type; break;
        case df::item_type::GLOVES: v = entity->resources.gloves_type; break;
        case df::item_type::HELM:   v = entity->resources.helm_type; break;
        case df::item_type::PANTS:  v = entity->resources.pants_type; break;
        case df::item_type::SHOES:  v = entity->resources.shoes_type; break;
        default: break;
        }

        //for (auto vv : v)
        for (vector<int16_t>::const_iterator cit = v.begin(); cit != v.end(); ++cit)
        {
            uint16 vv = *cit;
            bool isClothing = false;
            switch (ty) {
            case df::item_type::ARMOR:  isClothing = world->raws.itemdefs.armor[vv] ->armorlevel == 0; break;
            case df::item_type::GLOVES: isClothing = world->raws.itemdefs.gloves[vv]->armorlevel == 0; break;
            case df::item_type::HELM:   isClothing = world->raws.itemdefs.helms[vv] ->armorlevel == 0; break;
            case df::item_type::PANTS:  isClothing = world->raws.itemdefs.pants[vv] ->armorlevel == 0; break;
            case df::item_type::SHOES:  isClothing = world->raws.itemdefs.shoes[vv] ->armorlevel == 0; break;
            default: break;
            }
            if (isClothing)
            {
                sub = vv;
                break;
            }
        }

        orders[JobKey(itemTypeMap[ty], sub, size)] += count;
    }

    // scan orders

    //for (auto o : world->manager_orders)
    for (std::vector<df::manager_order*>::const_iterator ci = world->manager_orders.begin(); ci != world->manager_orders.end(); ++ci)
    {
        df::manager_order const* o = *ci;
        map<df::job_type, df::item_type>::const_iterator f = jobTypeMap.find(o->job_type);
        if (f == jobTypeMap.end())
            continue;

        uint16 sub = o->item_subtype;
        int race = o->hist_figure_id;
        if (race == -1)
            continue; // -1 means that the race of the worker will determine the size made; we must ignore these jobs

        int size = world->raws.creatures.all[race]->adultsize;

        orders[JobKey(o->job_type, sub, size)] -= o->amount_left;
    }

    // place orders

    //for (auto o : orders)
    for (std::map<JobKey,int,JobKeyHash>::const_iterator ci = orders.begin(); ci != orders.end(); ++ci)
    {
        std::pair<JobKey, int> const& o = *ci;
        //df::job_type ty;
        //int sub;
        //int size;
        //tie(ty, sub, size) = o.first;
        df::job_type ty = o.first.type;
        int sub = o.first.subtype;
        int size = o.first.size;

        int count = o.second;

        if (count > 0)
        {
            vector<int16_t> v;
            BitArray<df::armor_general_flags>* fl;
            string name_s, name_p;

            switch (ty) {
            case df::job_type::MakeArmor:
                name_s = world->raws.itemdefs.armor[sub]->name;
                name_p = world->raws.itemdefs.armor[sub]->name_plural;
                v = entity->resources.armor_type;
                fl = &world->raws.itemdefs.armor[sub]->props.flags;
                break;
            case df::job_type::MakeGloves:
                name_s = world->raws.itemdefs.gloves[sub]->name;
                name_p = world->raws.itemdefs.gloves[sub]->name_plural;
                v = entity->resources.gloves_type;
                fl = &world->raws.itemdefs.gloves[sub]->props.flags;
                break;
            case df::job_type::MakeHelm:
                name_s = world->raws.itemdefs.helms[sub]->name;
                name_p = world->raws.itemdefs.helms[sub]->name_plural;
                v = entity->resources.helm_type;
                fl = &world->raws.itemdefs.helms[sub]->props.flags;
                break;
            case df::job_type::MakePants:
                name_s = world->raws.itemdefs.pants[sub]->name;
                name_p = world->raws.itemdefs.pants[sub]->name_plural;
                v = entity->resources.pants_type;
                fl = &world->raws.itemdefs.pants[sub]->props.flags;
                break;
            case df::job_type::MakeShoes:
                name_s = world->raws.itemdefs.shoes[sub]->name;
                name_p = world->raws.itemdefs.shoes[sub]->name_plural;
                v = entity->resources.shoes_type;
                fl = &world->raws.itemdefs.shoes[sub]->props.flags;
                break;
            default:
                break;
            }

            bool can_make = false;
            //for (auto vv : v)
            for (vector<int16_t>::const_iterator cit = v.begin(); cit != v.end(); ++cit)
            {
                if ((*cit) == sub)
                {
                    can_make = true;
                    break;
                }
            }

            if (!can_make)
            {
                out.print("Cannot make %s, skipped\n", name_p.c_str());
                continue; // this civilization does not know how to make this item, so sorry
            }

            switch (ty) {
            case df::item_type::ARMOR:  break;
            case df::item_type::GLOVES: break;
            case df::item_type::HELM:   break;
            case df::item_type::PANTS:  break;
            case df::item_type::SHOES:  break;
            default: break;
            }

            df::job_material_category mat;

            if (silk > count + 10 && fl->is_set(df::armor_general_flags::SOFT)) {
                mat.whole = df::job_material_category::mask_silk;
                silk -= count;
            }
            else if (cloth > count + 10 && fl->is_set(df::armor_general_flags::SOFT)) {
                mat.whole = df::job_material_category::mask_cloth;
                cloth -= count;
            }
            else if (yarn > count + 10 && fl->is_set(df::armor_general_flags::SOFT)) {
                mat.whole = df::job_material_category::mask_yarn;
                yarn -= count;
            }
            else if (leather > count + 10 && fl->is_set(df::armor_general_flags::LEATHER)) {
                mat.whole = df::job_material_category::mask_leather;
                leather -= count;
            }
            else // not enough appropriate material available
                continue;

            df::manager_order* order = new df::manager_order();
            order->job_type = ty;
            order->item_type = df::item_type::NONE;
            order->item_subtype = sub;
            order->mat_type = -1;
            order->mat_index = -1;
            order->amount_left = count;
            order->amount_total = count;
            order->status.bits.validated = false;
            order->status.bits.active = false;
            order->id = world->manager_order_next_id++;
            order->hist_figure_id = sizes[size];
            order->material_category = mat;

            world->manager_orders.push_back(order);

            out.print("Added order #%d for %d %s %s (sized for %s)\n",
                order->id,
                count,
                bitfield_to_string(order->material_category).c_str(),
                (count > 1) ? name_p.c_str() : name_s.c_str(),
                world->raws.creatures.all[order->hist_figure_id]->name[1].c_str()
            );
        }
    }
}

#define DELTA_TICKS 600

DFhackCExport command_result plugin_onupdate(color_ostream &out)
{
    if (!enabled)
        return CR_OK;

    if (!Maps::IsValid())
        return CR_OK;

    if (DFHack::World::ReadPauseState())
        return CR_OK;

    if (world->frame_counter % DELTA_TICKS != 0)
        return CR_OK;

    bool found = false;

    for (df::job_list_link* link = &world->jobs.list; link != NULL; link = link->next)
    {
        if (link->item == NULL) continue;
        if (link->item->job_type == df::enums::job_type::UpdateStockpileRecords)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        do_scan(out);
    }

    return CR_OK;
}

static command_result tailor_cmd(color_ostream &out, vector <string> & parameters) {
    bool desired = enabled;
    if (parameters.size() == 1)
    {
        if (parameters[0] == "enable" || parameters[0] == "on" || parameters[0] == "1")
        {
            desired = true;
        }
        else if (parameters[0] == "disable" || parameters[0] == "off" || parameters[0] == "0")
        {
            desired = false;
        }
        else if (parameters[0] == "usage" || parameters[0] == "help" || parameters[0] == "?")
        {
            out.print("%s: %s\nUsage:\n%s", plugin_name, tagline, usage);
            return CR_OK;
        }
        else if (parameters[0] == "test")
        {
            do_scan(out);
            return CR_OK;
        }
        else if (parameters[0] != "status")
        {
            return CR_WRONG_USAGE;
        }
    }
    else
        return CR_WRONG_USAGE;

    out.print("Tailor is %s %s.\n", (desired == enabled)? "currently": "now", desired? "enabled": "disabled");
    enabled = desired;

    return CR_OK;
}


DFhackCExport command_result plugin_onstatechange(color_ostream &out, state_change_event event)
{
    return CR_OK;
}

DFhackCExport command_result plugin_enable(color_ostream& out, bool enable)
{
    enabled = enable;
    return CR_OK;
}

DFhackCExport command_result plugin_init(color_ostream &out, std::vector <PluginCommand> &commands)
{
    if (AUTOENABLE) {
        enabled = true;
    }

    commands.push_back(PluginCommand(plugin_name, tagline, tailor_cmd, false, usage));
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown(color_ostream &out) {
    return plugin_enable(out, false);
}
