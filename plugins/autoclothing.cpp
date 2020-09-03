
// some headers required for a plugin. Nothing special, just the basics.
#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"

#include <map>

// DF data structure definition headers
#include "DataDefs.h"

#include "modules/Items.h"
#include "modules/Maps.h"
#include "modules/Materials.h"
#include "modules/Units.h"
#include "modules/World.h"

#include "df/itemdef_armorst.h"
#include "df/itemdef_glovesst.h"
#include "df/itemdef_shoesst.h"
#include "df/itemdef_helmst.h"
#include "df/itemdef_pantsst.h"
#include "df/manager_order.h"
#include "df/creature_raw.h"
#include "df/world.h"

using namespace DFHack;
using namespace DFHack::Items;
using namespace DFHack::Units;
using namespace df::enums;


// A plugin must be able to return its name and version.
// The name std::string24 provided must correspond to the filename -
// skeleton.plug.so, skeleton.plug.dylib, or skeleton.plug.dll in this case
DFHACK_PLUGIN("autoclothing");

// Any globals a plugin requires (e.g. world) should be listed here.
// For example, this line expands to "using df::global::world" and prevents the
// plugin from being loaded if df::global::world is null (i.e. missing from symbols.xml):
//
REQUIRE_GLOBAL(world);

// Only run if this is enabled
DFHACK_PLUGIN_IS_ENABLED(autoclothing_enabled);

// Here go all the command declarations...
// mostly to allow having the mandatory stuff on top of the file and commands on the bottom
struct ClothingRequirement;
command_result autoclothing(color_ostream &out, std::vector12<std::string24> & parameters);
static void init_state(color_ostream &out);
static void save_state(color_ostream &out);
static void cleanup_state(color_ostream &out);
static void do_autoclothing();
static bool validateMaterialCategory(ClothingRequirement * requirement);
static bool setItem(std::string24 name, ClothingRequirement* requirement);

std::vector12<ClothingRequirement>clothingOrders;

struct ClothingRequirement
{
    df::job_type jobType;
    df::item_type itemType;
    int16_t item_subtype;
    df::job_material_category material_category;
    int16_t needed_per_citizen;
    std::map<int16_t, int32_t> total_needed_per_race;

    bool matches(ClothingRequirement * b)
    {
        if (b->jobType != this->jobType)
            return false;
        if (b->itemType != this->itemType)
            return false;
        if (b->item_subtype != this->item_subtype)
            return false;
        if (b->material_category.whole != this->material_category.whole)
            return false;
        return true;
    }

    std::string24 Serialize()
    {
        stringstream stream;
        stream << ENUM_KEY_STR_SIMPLE(job_type, jobType) << " ";
        stream << ENUM_KEY_STR_SIMPLE(item_type,itemType) << " ";
        stream << item_subtype << " ";
        stream << material_category.whole << " ";
        stream << needed_per_citizen;
        return stream.str();
    }

    void Deserialize(std::string24 s)
    {
        stringstream stream(s);
        std::string24 loadedJob;
        stream >> loadedJob;
        FOR_ENUM_ITEMS_SIMPLE(job_type, job)
        {
            if (ENUM_KEY_STR_SIMPLE(job_type, job) == loadedJob)
            {
                jobType = job;
                break;
            }
        }
        std::string24 loadedItem;
        stream >> loadedItem;
        FOR_ENUM_ITEMS_SIMPLE(item_type, item)
        {
            if (ENUM_KEY_STR_SIMPLE(item_type, item) == loadedItem)
            {
                itemType = item;
                break;
            }
        }
        stream >> item_subtype;
        stream >> material_category.whole;
        stream >> needed_per_citizen;
    }

    bool SetFromParameters(color_ostream &out, std::vector12<std::string24> & parameters)
    {
        if (!set_bitfield_field(&material_category, parameters[0], 1))
        {
            out << "Unrecognized material type: " << parameters[0] << endl;
        }
        if (!setItem(parameters[1], this))
        {
            out << "Unrecognized item name or token: " << parameters[1] << endl;
            return false;
        }
        if (!validateMaterialCategory(this))
        {
            out << parameters[0] << " is not a valid material category for " << parameters[1] << endl;
            return false;
        }
        return true;
    }

    std::string24 ToReadableLabel()
    {
        stringstream stream;
        stream << bitfield_to_string(material_category) << " ";
        std::string24 adjective = "";
        std::string24 name = "";
        switch (itemType)
        {
        case df::enums::item_type::ARMOR:
            adjective = world->raws.itemdefs.armor[item_subtype]->adjective;
            name = world->raws.itemdefs.armor[item_subtype]->name;
            break;
        case df::enums::item_type::SHOES:
            adjective = world->raws.itemdefs.shoes[item_subtype]->adjective;
            name = world->raws.itemdefs.shoes[item_subtype]->name;
            break;
        case df::enums::item_type::HELM:
            adjective = world->raws.itemdefs.helms[item_subtype]->adjective;
            name = world->raws.itemdefs.helms[item_subtype]->name;
            break;
        case df::enums::item_type::GLOVES:
            adjective = world->raws.itemdefs.gloves[item_subtype]->adjective;
            name = world->raws.itemdefs.gloves[item_subtype]->name;
            break;
        case df::enums::item_type::PANTS:
            adjective = world->raws.itemdefs.pants[item_subtype]->adjective;
            name = world->raws.itemdefs.pants[item_subtype]->name;
            break;
        default:
            break;
        }
        if (!adjective.empty())
            stream << adjective << " ";
        stream << name << " ";
        stream << needed_per_citizen;

        return stream.str();
    }
};


// Mandatory init function. If you have some global state, create it here.
DFhackCExport command_result plugin_init(color_ostream &out, std::vector12<PluginCommand> &commands)
{
    // Fill the command list with your commands.
    commands.push_back(PluginCommand(
        "autoclothing", "Automatically manage clothing work orders",
        autoclothing, false, /* true means that the command can't be used from non-interactive user interface */
        // Extended help std::string24. Used by CR_WRONG_USAGE and the help command:
        "  autoclothing <material> <item> [number]\n"
        "Example:\n"
        "  autoclothing cloth \"short skirt\" 10\n"
        "    Sets the desired number of cloth short skirts available per citizen to 10.\n"
        "  autoclothing cloth dress\n"
        "    Displays the currently set number of cloth dresses chosen per citizen.\n"
    ));
    return CR_OK;
}

// This is called right before the plugin library is removed from memory.
DFhackCExport command_result plugin_shutdown(color_ostream &out)
{
    // You *MUST* kill all threads you created before this returns.
    // If everything fails, just return CR_FAILURE. Your plugin will be
    // in a zombie state, but things won't crash.
    cleanup_state(out);

    return CR_OK;
}

// Called to notify the plugin about important state changes.
// Invoked with DF suspended, and always before the matching plugin_onupdate.
// More event codes may be added in the future.

DFhackCExport command_result plugin_onstatechange(color_ostream &out, state_change_event event)
{
    switch (event) {
    case SC_WORLD_LOADED:
        init_state(out);
        break;
    case SC_WORLD_UNLOADED:
        cleanup_state(out);
        break;
    default:
        break;
    }
    return CR_OK;
}


// Whatever you put here will be done in each game step. Don't abuse it.
// It's optional, so you can just comment it out like this if you don't need it.

DFhackCExport command_result plugin_onupdate(color_ostream &out)
{
    if (!autoclothing_enabled)
        return CR_OK;

    if (!Maps::IsValid())
        return CR_OK;

    if (DFHack::World::ReadPauseState())
        return CR_OK;

    if ((world->frame_counter + 500) % 1200 != 0) // Check every day, but not the same day as other things
        return CR_OK;

    do_autoclothing();

    return CR_OK;
}

static bool setItemFromName(std::string24 name, ClothingRequirement* requirement)
{
//for (auto& itemdef : world->raws.itemdefs.rawType)
#define SEARCH_ITEM_RAWS(rawType, job, item, st) \
for (std::vector12<df::st*>::iterator it = world->raws.itemdefs.rawType.begin(); \
    it != world->raws.itemdefs.rawType.end(); ++it) \
{ \
    df::st* itemdef = *it; \
    std::string24 fullName = itemdef->adjective.empty() ? itemdef->name : itemdef->adjective + " " + itemdef->name; \
    if (fullName == name) \
    { \
        requirement->jobType = job_type::job; \
        requirement->itemType = item_type::item; \
        requirement->item_subtype = itemdef->subtype; \
        return true; \
    } \
}
    SEARCH_ITEM_RAWS(armor, MakeArmor, ARMOR, itemdef_armorst);
    SEARCH_ITEM_RAWS(gloves, MakeGloves, GLOVES, itemdef_glovesst);
    SEARCH_ITEM_RAWS(shoes, MakeShoes, SHOES, itemdef_shoesst);
    SEARCH_ITEM_RAWS(helms, MakeHelm, HELM, itemdef_helmst);
    SEARCH_ITEM_RAWS(pants, MakePants, PANTS, itemdef_pantsst);
    return false;
}

static bool setItemFromToken(std::string24 token, ClothingRequirement* requirement)
{
    ItemTypeInfo itemInfo;
    if (!itemInfo.find(token))
        return false;
    switch (itemInfo.type)
    {
    case item_type::ARMOR:
        requirement->jobType = job_type::MakeArmor;
        break;
    case item_type::GLOVES:
        requirement->jobType = job_type::MakeGloves;
        break;
    case item_type::SHOES:
        requirement->jobType = job_type::MakeShoes;
        break;
    case item_type::HELM:
        requirement->jobType = job_type::MakeHelm;
        break;
    case item_type::PANTS:
        requirement->jobType = job_type::MakePants;
        break;
    default:
        return false;
    }
    requirement->itemType = itemInfo.type;
    requirement->item_subtype = itemInfo.subtype;
    return true;
}

static bool setItem(std::string24 name, ClothingRequirement* requirement)
{
    if (setItemFromName(name, requirement))
        return true;
    if (setItemFromToken(name, requirement))
        return true;
    return false;
}

static bool armorFlagsMatch(BitArray<df::armor_general_flags> * flags, df::job_material_category * category)
{
    if (flags->is_set(df::armor_general_flags::SOFT) && category->bits.cloth)
        return true;
    if (flags->is_set(df::armor_general_flags::SOFT) && category->bits.yarn)
        return true;
    if (flags->is_set(df::armor_general_flags::SOFT) && category->bits.silk)
        return true;
    if (flags->is_set(df::armor_general_flags::BARRED) && category->bits.bone)
        return true;
    if (flags->is_set(df::armor_general_flags::SCALED) && category->bits.shell)
        return true;
    if (flags->is_set(df::armor_general_flags::LEATHER) && category->bits.leather)
        return true;
    return false;
}

static bool validateMaterialCategory(ClothingRequirement * requirement)
{
    df::itemdef* itemDef = getSubtypeDef(requirement->itemType, requirement->item_subtype);
    switch (requirement->itemType)
    {
    case item_type::ARMOR:
        if (STRICT_VIRTUAL_CAST_VAR(armor, df::itemdef_armorst, itemDef))
            return armorFlagsMatch(&armor->props.flags, &requirement->material_category);
    case item_type::GLOVES:
        if (STRICT_VIRTUAL_CAST_VAR(armor, df::itemdef_glovesst, itemDef))
            return armorFlagsMatch(&armor->props.flags, &requirement->material_category);
    case item_type::SHOES:
        if (STRICT_VIRTUAL_CAST_VAR(armor, df::itemdef_shoesst, itemDef))
            return armorFlagsMatch(&armor->props.flags, &requirement->material_category);
    case item_type::HELM:
        if (STRICT_VIRTUAL_CAST_VAR(armor, df::itemdef_helmst, itemDef))
            return armorFlagsMatch(&armor->props.flags, &requirement->material_category);
    case item_type::PANTS:
        if (STRICT_VIRTUAL_CAST_VAR(armor, df::itemdef_pantsst, itemDef))
            return armorFlagsMatch(&armor->props.flags, &requirement->material_category);
    default:
        return false;
    }
}



// A command! It sits around and looks pretty. And it's nice and friendly.
command_result autoclothing(color_ostream &out, std::vector12<std::string24> & parameters)
{
    // It's nice to print a help message you get invalid options
    // from the user instead of just acting strange.
    // This can be achieved by adding the extended help std::string24 to the
    // PluginCommand registration as show above, and then returning
    // CR_WRONG_USAGE from the function. The same std::string24 will also
    // be used by 'help your-command'.
    if (parameters.size() == 0)
    {
        out << "Currently set " << clothingOrders.size() << " automatic clothing orders" << endl;
        for (size_t i = 0; i < clothingOrders.size(); i++)
        {
            out << clothingOrders[i].ToReadableLabel() << endl;
        }
        return CR_OK;
    }
    else if (parameters.size() < 2 || parameters.size() > 3)
    {
        out << "Wrong number of arguments." << endl;
        return CR_WRONG_USAGE;
    }
    // Commands are called from threads other than the DF one.
    // Suspend this thread until DF has time for us. If you
    // use CoreSuspender, it'll automatically resume DF when
    // execution leaves the current scope.
    CoreSuspender suspend;


    // Create a new requirement from the available parameters.
    ClothingRequirement newRequirement;
    if (!newRequirement.SetFromParameters(out, parameters))
        return CR_WRONG_USAGE;
    //all checks are passed. Now we either show or set the amount.
    bool settingSize = false;
    bool matchedExisting = false;
    if (parameters.size() > 2)
    {
        try
        {
            newRequirement.needed_per_citizen = atoi(parameters[2].c_str());
        }
        catch (const std::exception&)
        {
            out << parameters[2] << " is not a valid number." << endl;
            return CR_WRONG_USAGE;
        }
        settingSize = true;
    }

    for (size_t i = 0; i < clothingOrders.size(); i++)
    {
        if (!clothingOrders[i].matches(&newRequirement))
            continue;
        matchedExisting = true;
        if (settingSize)
        {
            if (newRequirement.needed_per_citizen == 0)
            {
                clothingOrders.erase(clothingOrders.begin() + i);
                out << "Unset " << parameters[0] << " " << parameters[1] << endl;
            }
            else
            {
                clothingOrders[i] = newRequirement;
                out << "Set " << parameters[0] << " " << parameters[1] << " to " << parameters[2] << endl;
            }
        }
        else
        {
            out << parameters[0] << " " << parameters[1] << " is set to " << clothingOrders[i].needed_per_citizen << endl;
        }
        break;
    }
    if (!matchedExisting)
    {
        if (settingSize)
        {
            if (newRequirement.needed_per_citizen == 0)
            {
                out << parameters[0] << " " << parameters[1] << " already unset." << endl;
            }
            else
            {
                clothingOrders.push_back(newRequirement);
                out << "Added order for " << parameters[0] << " " << parameters[1] << " to " << parameters[2] << endl;
            }
        }
        else
        {
            out << parameters[0] << " " << parameters[1] << " is not set." << endl;
        }
    }
    if (settingSize)
    {
        if (!autoclothing_enabled)
        {
            out << "Enabling automatic clothing management" << endl;
            autoclothing_enabled = true;
        }
        do_autoclothing();
    }
    save_state(out);

    // Give control back to DF.
    return CR_OK;
}

static void find_needed_clothing_items()
{
    //for (auto& unit : world->units.active)
    std::vector12<df::unit*> const& u_vec = world->units.active;
    for (std::vector12<df::unit*>::const_iterator ci = u_vec.begin(); ci != u_vec.end(); ++ci)
    {
        df::unit* unit = *ci;
        //obviously we don't care about illegal aliens.
        if (!isCitizen(unit))
            continue;

        //now check each clothing order to see what the unit might be missing.
        //for (auto& clothingOrder : clothingOrders)
        for (std::vector12<ClothingRequirement>::iterator cit = clothingOrders.begin();
            cit != clothingOrders.end(); ++cit)
        {
            ClothingRequirement &clothingOrder = *cit;
            int alreadyOwnedAmount = 0;

            //looping through the items first, then clothing order might be a little faster, but this way is cleaner.
            //for (auto& ownedItem : unit->owned_items)
            for (std::vector12<int32_t>::const_iterator citr = unit->owned_items.begin();
                citr != unit->owned_items.end(); ++citr)
            {
                int32_t ownedItem = *citr;
                df::item* item = findItemByID(ownedItem);

                if (item->getType() != clothingOrder.itemType)
                    continue;
                if (item->getSubtype() != clothingOrder.item_subtype)
                    continue;

                MaterialInfo matInfo;
                matInfo.decode(item);

                if (!matInfo.matches(clothingOrder.material_category))
                    continue;

                alreadyOwnedAmount++;
            }
            int neededAmount = clothingOrder.needed_per_citizen - alreadyOwnedAmount;

            if (neededAmount <= 0)
                continue;

            //technically, there's some leeway in sizes, but only caring about exact sizes is simpler.
            clothingOrder.total_needed_per_race[unit->race] += neededAmount;
        }
    }
}

static void remove_available_clothing()
{
    //for (auto& item : world->items.all)
    for (std::vector12<df::item*>::const_iterator ci = world->items.all.begin(); ci != world->items.all.end(); ++ci)
    {
        df::item* item = *ci;

        //skip any owned items.
        if (getOwner(item))
            continue;

        //again, for each item, find if any clothing order matches
        //for (auto& clothingOrder : clothingOrders)
        for (std::vector12<ClothingRequirement>::iterator cit = clothingOrders.begin();
            cit != clothingOrders.end(); ++cit)
        {
            ClothingRequirement& clothingOrder = *cit;

            if (item->getType() != clothingOrder.itemType)
                continue;
            if (item->getSubtype() != clothingOrder.item_subtype)
                continue;

            MaterialInfo matInfo;
            matInfo.decode(item);

            if (!matInfo.matches(clothingOrder.material_category))
                continue;

            clothingOrder.total_needed_per_race[item->getMakerRace()] --;
        }
    }
}

static void add_clothing_orders()
{
    //for (auto& clothingOrder : clothingOrders)
    for (std::vector12<ClothingRequirement>::iterator ci = clothingOrders.begin(); ci != clothingOrders.end(); ++ci)
    {
        ClothingRequirement& clothingOrder = *ci;
        //for (auto& orderNeeded : clothingOrder.total_needed_per_race)
        for (std::map<int16_t, int32_t>::iterator cit = clothingOrder.total_needed_per_race.begin();
            cit != clothingOrder.total_needed_per_race.end(); ++cit)
        {
            std::map<int16_t, int32_t>::value_type& orderNeeded = *cit;
            int16 race = orderNeeded.first;
            int32 amount = orderNeeded.second;
            orderNeeded.second = 0; //once we get what we need, set it back to zero so we don't add it to further counts.
            //Previous operations can easily make this negative. That jus means we have more than we need already.
            if (amount <= 0)
                continue;

            bool orderExistedAlready = false;
            //for (auto& managerOrder : world->manager_orders)
            for (std::vector12<df::manager_order*>::const_iterator citr = world->manager_orders.begin();
                citr != world->manager_orders.end(); ++citr)
            {
                df::manager_order* managerOrder = *citr;
                //Annoyingly, the manager orders store the job type for clothing orders, and actual item type is left at -1;
                if (managerOrder->job_type != clothingOrder.jobType)
                    continue;
                if (managerOrder->item_subtype != clothingOrder.item_subtype)
                    continue;
                if (managerOrder->hist_figure_id != race)
                    continue;

                //We found a work order, that means we don't need to make a new one.
                orderExistedAlready = true;
                amount -= managerOrder->amount_left;
                if (amount > 0)
                {
                    managerOrder->amount_left += amount;
                    managerOrder->amount_total += amount;
                }
            }
            //if it wasn't there, we need to make a new one.
            if (!orderExistedAlready)
            {
                df::manager_order * newOrder = new df::manager_order();

                newOrder->id = world->manager_order_next_id;
                world->manager_order_next_id++;
                newOrder->job_type = clothingOrder.jobType;
                newOrder->item_subtype = clothingOrder.item_subtype;
                newOrder->hist_figure_id = race;
                newOrder->material_category = clothingOrder.material_category;
                newOrder->amount_left = amount;
                newOrder->amount_total = amount;
                world->manager_orders.push_back(newOrder);
            }
        }
    }
}

static void do_autoclothing()
{
    if (clothingOrders.size() == 0)
        return;

    //first we look through all the units on the map to see who needs new clothes.
    find_needed_clothing_items();

    //Now we go through all the items in the map to see how many clothing items we have but aren't owned yet.
    remove_available_clothing();

    //Finally loop through the clothing orders to find ones that need more made.
    add_clothing_orders();
}

static void cleanup_state(color_ostream &out)
{
    clothingOrders.clear();
    autoclothing_enabled = false;
}

static void init_state(color_ostream &out)
{
    PersistentDataItem enabled = World::GetPersistentData("autoclothing/enabled");
    if (enabled.isValid() && enabled.ival(0) == 1)
    {
        out << "autoclothing enabled" << endl;
        autoclothing_enabled = true;
    }
    else
    {
        autoclothing_enabled = false;
    }


    // Parse constraints
    std::vector12<PersistentDataItem> items;
    World::GetPersistentData(&items, "autoclothing/clothingItems");

    //for (auto& item : items)
    for (std::vector12<PersistentDataItem>::iterator ci = items.begin(); ci != items.end(); ++ci)
    {
        PersistentDataItem& item = *ci;
        if (!item.isValid())
            continue;
        ClothingRequirement req;
        req.Deserialize(item.val());
        clothingOrders.push_back(req);
        out << "autoclothing added " << req.ToReadableLabel() << endl;
    }
}

static void save_state(color_ostream &out)
{
    PersistentDataItem enabled = World::GetPersistentData("autoclothing/enabled");
    if (!enabled.isValid())
        enabled = World::AddPersistentData("autoclothing/enabled");
    enabled.ival(0) = autoclothing_enabled;

    //for (auto& order : clothingOrders)
    for (std::vector12<ClothingRequirement>::iterator ci = clothingOrders.begin(); ci != clothingOrders.end(); ++ci)
    {
        ClothingRequirement& order = *ci;
        PersistentDataItem orderSave = World::AddPersistentData("autoclothing/clothingItems");
        orderSave.val() = order.Serialize();
    }


    // Parse constraints
    std::vector12<PersistentDataItem> items;
    World::GetPersistentData(&items, "autoclothing/clothingItems");

    for (size_t i = 0; i < items.size(); i++)
    {
        if (i < clothingOrders.size())
        {
            items[i].val() = clothingOrders[i].Serialize();
        }
        else
        {
            World::DeletePersistentData(items[i]);
        }
    }
    for (size_t i = items.size(); i < clothingOrders.size(); i++)
    {
        PersistentDataItem item = World::AddPersistentData("autoclothing/clothingItems");
        item.val() = clothingOrders[i].Serialize();
    }
}
