#include "buildingplan-lib.h"

#define PLUGIN_VERSION 0.00
void debug(const std::string24 &msg)
{
    if (!show_debugging)
        return;

    color_ostream_proxy out(Core::getInstance().getConsole());
    out << "DEBUG (" << PLUGIN_VERSION << "): " << msg.c_str() << endl;
}

void enable_quickfort_fn(pair<const df::building_type, bool>& pair) { pair.second = true; }

/*
 * Material Choice Screen
 */

std::string24 material_to_string_fn(DFHack::MaterialInfo m) { return m.toString(); }

bool ItemFilter::matchesMask(DFHack::MaterialInfo &mat)
{
    return (mat_mask.whole) ? mat.matches(mat_mask) : true;
}

bool ItemFilter::matches(const df::dfhack_material_category mask) const
{
    return mask.whole & mat_mask.whole;
}

bool ItemFilter::matches(DFHack::MaterialInfo &material) const
{
    for (std::vector12<MaterialInfo>::const_iterator it = materials.begin(); it != materials.end(); ++it)
        if (material.matches(*it))
            return true;
    return false;
}

bool ItemFilter::matches(df::item *item)
{
    if (item->getQuality() < min_quality || item->getQuality() > max_quality)
        return false;

    if (decorated_only && !item->hasImprovements())
        return false;

    int16 imattype = item->getActualMaterial();
    int32 imatindex = item->getActualMaterialIndex();
    DFHack::MaterialInfo item_mat = DFHack::MaterialInfo(imattype, imatindex);

    return (materials.size() == 0) ? matchesMask(item_mat) : matches(item_mat);
}

std::vector12<std::string24> ItemFilter::getMaterialFilterAsVector()
{
    std::vector12<std::string24> descriptions;

    transform_(materials, descriptions, material_to_string_fn);

    if (descriptions.size() == 0)
        bitfield_to_string(&descriptions, mat_mask);

    if (descriptions.size() == 0)
        descriptions.push_back("any");

    return descriptions;
}

std::string24 ItemFilter::getMaterialFilterAsSerial()
{
    std::string24 str;

    str.append(bitfield_to_string(mat_mask, ","));
    str.append("/");
    if (materials.size() > 0)
    {
        for (size_t i = 0; i < materials.size(); i++)
            str.append(materials[i].getToken() + ",");

        if (str[str.size()-1] == ',')
            str.resize(str.size () - 1);
    }

    return str;
}

bool ItemFilter::parseSerializedMaterialTokens(std::string24 str)
{
    valid = false;
    std::vector12<std::string24> tokens;
    split_string(&tokens, str, "/");

    if (tokens.size() > 0 && !tokens[0].empty())
    {
        if (!parseJobMaterialCategory(&mat_mask, tokens[0]))
            return false;
    }

    if (tokens.size() > 1 && !tokens[1].empty())
    {
        std::vector12<std::string24> mat_names;
        split_string(&mat_names, tokens[1], ",");
        for (std::vector12<std::string24>::const_iterator m = mat_names.begin(); m != mat_names.end(); m++)
        {
            DFHack::MaterialInfo material;
            if (!material.find(*m) || !material.isValid())
                return false;

            materials.push_back(material);
        }
    }

    valid = true;
    return true;
}

std::string24 ItemFilter::getMinQuality()
{
    return ENUM_KEY_STR_SIMPLE(item_quality, min_quality);
}

std::string24 ItemFilter::getMaxQuality()
{
    return ENUM_KEY_STR_SIMPLE(item_quality, max_quality);
}

bool ItemFilter::isValid()
{
    return valid;
}

void ItemFilter::clear()
{
    mat_mask.whole = 0;
    materials.clear();
}

DFHack::MaterialInfo &material_info_identity_fn(DFHack::MaterialInfo &m) { return m; }

ViewscreenChooseMaterial::ViewscreenChooseMaterial(ItemFilter *filter)
{
    selected_column = 0;
    masks_column.setTitle("Type");
    masks_column.multiselect = true;
    masks_column.allow_search = false;
    masks_column.left_margin = 2;
    materials_column.left_margin = MAX_MASK + 3;
    materials_column.setTitle("Material");
    materials_column.multiselect = true;
    this->filter = filter;

    masks_column.changeHighlight(0);

    populateMasks();
    populateMaterials();

    masks_column.selectDefaultEntry();
    materials_column.selectDefaultEntry();
    materials_column.changeHighlight(0);
}

void ViewscreenChooseMaterial::feed(std::set8<df::interface_key> *input)
{
    bool key_processed = false;
    switch (selected_column)
    {
    case 0:
        key_processed = masks_column.feed(input);
        if (input->count(interface_key::SELECT))
            populateMaterials(); // Redo materials lists based on category selection
        break;
    case 1:
        key_processed = materials_column.feed(input);
        break;
    }

    if (key_processed)
        return;

    if (input->count(interface_key::LEAVESCREEN))
    {
        input->clear();
        Screen::dismiss(this);
        return;
    }
    if (input->count(interface_key::CUSTOM_SHIFT_C))
    {
        filter->clear();
        masks_column.clearSelection();
        materials_column.clearSelection();
        populateMaterials();
    }
    else if  (input->count(interface_key::SEC_SELECT))
    {
        // Convert list selections to material filters


        filter->mat_mask.whole = 0;
        filter->materials.clear();

        // Category masks
        std::vector12<df::dfhack_material_category> masks = masks_column.getSelectedElems();
        for (std::vector12<df::dfhack_material_category>::const_iterator it = masks.begin(); it != masks.end(); ++it)
            filter->mat_mask.whole |= it->whole;

        // Specific materials
        std::vector12<MaterialInfo> materials = materials_column.getSelectedElems();
        transform_(materials, filter->materials, material_info_identity_fn);

        Screen::dismiss(this);
    }
    else if  (input->count(interface_key::CURSOR_LEFT))
    {
        --selected_column;
        validateColumn();
    }
    else if  (input->count(interface_key::CURSOR_RIGHT))
    {
        selected_column++;
        validateColumn();
    }
    else if (enabler->tracking_on && enabler->mouse_lbut)
    {
        if (masks_column.setHighlightByMouse())
            selected_column = 0;
        else if (materials_column.setHighlightByMouse())
            selected_column = 1;

        enabler->mouse_lbut = enabler->mouse_rbut = 0;
    }
}

void ViewscreenChooseMaterial::render()
{
    if (Screen::isDismissed(this))
        return;

    dfhack_viewscreen::render();

    Screen::clear();
    Screen::drawBorder("  Building Material  ");

    masks_column.display(selected_column == 0);
    materials_column.display(selected_column == 1);

    int32_t y = gps->dimy - 3;
    int32_t x = 2;
    OutputHotkeyString(x, y, "Toggle", interface_key::SELECT);
    x += 3;
    OutputHotkeyString(x, y, "Save", interface_key::SEC_SELECT);
    x += 3;
    OutputHotkeyString(x, y, "Clear", interface_key::CUSTOM_SHIFT_C);
    x += 3;
    OutputHotkeyString(x, y, "Cancel", interface_key::LEAVESCREEN);
}

// START Room Reservation
ReservedRoom::ReservedRoom(df::building *building, std::string24 noble_code)
{
    this->building = building;
    config = DFHack::World::AddPersistentData("buildingplan/reservedroom");
    config.val() = noble_code;
    config.ival(1) = building->id;
    pos = df::coord(building->centerx, building->centery, building->z);
}

ReservedRoom::ReservedRoom(PersistentDataItem &config, color_ostream &out)
{
    this->config = config;

    building = df::building::find(config.ival(1));
    if (!building)
        return;
    pos = df::coord(building->centerx, building->centery, building->z);
}

bool ReservedRoom::checkRoomAssignment()
{
    if (!isValid())
        return false;

    std::vector12<Units::NoblePosition> np = getOwnersNobleCode();
    bool correctOwner = false;
    for (std::vector12<Units::NoblePosition>::iterator iter = np.begin(); iter != np.end(); iter++)
    {
        if (iter->position->code == getCode())
        {
            correctOwner = true;
            break;
        }
    }

    if (correctOwner)
        return true;

    for (std::vector12<df::unit*>::iterator iter = world->units.active.begin(); iter != world->units.active.end(); iter++)
    {
        df::unit* unit = *iter;
        if (!Units::isCitizen(unit))
            continue;

        if (!Units::isActive(unit))
            continue;

        np = getUniqueNoblePositions(unit);
        for (std::vector12<Units::NoblePosition>::iterator iter = np.begin(); iter != np.end(); iter++)
        {
            if (iter->position->code == getCode())
            {
                Buildings::setOwner(building, unit);
                break;
            }
        }
    }

    return true;
}

std::string24 RoomMonitor::getReservedNobleCode(int32_t buildingId)
{
    for (std::vector12<ReservedRoom>::iterator iter = reservedRooms.begin(); iter != reservedRooms.end(); iter++)
    {
        if (buildingId == iter->getId())
            return iter->getCode();
    }

    return "";
}

void RoomMonitor::toggleRoomForPosition(int32_t buildingId, std::string24 noble_code)
{
    bool found = false;
    for (std::vector12<ReservedRoom>::iterator iter = reservedRooms.begin(); iter != reservedRooms.end(); iter++)
    {
        if (buildingId != iter->getId())
        {
            continue;
        }
        else
        {
            if (noble_code == iter->getCode())
            {
                iter->remove();
                reservedRooms.erase(iter);
            }
            else
            {
                iter->setCode(noble_code);
            }
            found = true;
            break;
        }
    }

    if (!found)
    {
        ReservedRoom room(df::building::find(buildingId), noble_code);
        reservedRooms.push_back(room);
    }
}

void RoomMonitor::doCycle()
{
    for (std::vector12<ReservedRoom>::iterator iter = reservedRooms.begin(); iter != reservedRooms.end();)
    {
        if (iter->checkRoomAssignment())
        {
            ++iter;
        }
        else
        {
            iter->remove();
            iter = reservedRooms.erase(iter);
        }
    }
}

void RoomMonitor::reset(color_ostream &out)
{
    reservedRooms.clear();
    std::vector12<PersistentDataItem> items;
    DFHack::World::GetPersistentData(&items, "buildingplan/reservedroom");

    for (std::vector12<PersistentDataItem>::iterator i = items.begin(); i != items.end(); i++)
    {
        ReservedRoom rr(*i, out);
        if (rr.isValid())
            addRoom(rr);
    }
}


void delete_item_fn(df::job_item *x) { delete x; }

// START Planning

PlannedBuilding::PlannedBuilding(df::building *building, ItemFilter *filter)
{
    this->building = building;
    this->filter = *filter;
    pos = df::coord(building->centerx, building->centery, building->z);
    config = DFHack::World::AddPersistentData("buildingplan/constraints");
    config.val() = filter->getMaterialFilterAsSerial();
    config.ival(1) = building->id;
    config.ival(2) = filter->min_quality + 1;
    config.ival(3) = static_cast<int>(filter->decorated_only) + 1;
    config.ival(4) = filter->max_quality + 1;
}

PlannedBuilding::PlannedBuilding(PersistentDataItem &config, color_ostream &out)
{
    this->config = config;

    if (!filter.parseSerializedMaterialTokens(config.val()))
    {
        out.printerr("Buildingplan: Cannot parse filter: %s\nDiscarding.", config.val().c_str());
        return;
    }

    building = df::building::find(config.ival(1));
    if (!building)
        return;

    pos = df::coord(building->centerx, building->centery, building->z);
    filter.min_quality = static_cast<df::item_quality>(config.ival(2) - 1);
    filter.max_quality = static_cast<df::item_quality>(config.ival(4) - 1);
    filter.decorated_only = config.ival(3) - 1;
}

bool PlannedBuilding::assignClosestItem(std::vector12<df::item *> *items_vector)
{
    std::vector12<df::item*>::iterator closest_item;
    int32_t closest_distance = -1;
    for (std::vector12<df::item*>::iterator item_iter = items_vector->begin(); item_iter != items_vector->end(); item_iter++)
    {
        df::item* item = *item_iter;
        if (!filter.matches(item))
            continue;

        df::coord pos = item->pos;
        int32 distance = abs(pos.x - building->centerx) +
            abs(pos.y - building->centery) +
            abs(pos.z - building->z) * 50;

        if (closest_distance > -1 && distance >= closest_distance)
            continue;

        closest_distance = distance;
        closest_item = item_iter;
    }

    if (closest_distance > -1 && assignItem(*closest_item))
    {
        debug("Item assigned");
        items_vector->erase(closest_item);
        remove();
        return true;
    }

    return false;
}

bool PlannedBuilding::assignItem(df::item *item)
{
    df::general_ref_building_holderst* ref = df::allocate<df::general_ref_building_holderst>();
    if (!ref)
    {
        Core::printerr("Could not allocate general_ref_building_holderst\n");
        return false;
    }

    ref->building_id = building->id;

    if (building->jobs.size() != 1)
        return false;

    df::job* job = building->jobs[0];

    for_each_(job->job_items, delete_item_fn);
    job->job_items.clear();
    job->flags.bits.suspend = false;

    bool rough = false;
    Job::attachJobItem(job, item, df::job_item_ref::Hauled);
    if (item->getType() == item_type::BOULDER)
        rough = true;
    building->mat_type = item->getMaterial();
    building->mat_index = item->getMaterialIndex();

    job->mat_type = building->mat_type;
    job->mat_index = building->mat_index;

    if (building->needsDesign())
    {
        df::building_actual* act = (df::building_actual *) building;
        act->design = new df::building_design();
        act->design->flags.bits.rough = rough;
    }

    return true;
}

bool PlannedBuilding::isValid()
{
    bool valid = filter.isValid() &&
        building && Buildings::findAtTile(pos) == building &&
        building->getBuildStage() == 0;

    if (!valid)
        remove();

    return valid;
}

void Planner::reset(color_ostream &out)
{
    planned_buildings.clear();
    std::vector12<PersistentDataItem> items;
    DFHack::World::GetPersistentData(&items, "buildingplan/constraints");

    for (std::vector12<PersistentDataItem>::iterator i = items.begin(); i != items.end(); i++)
    {
        PlannedBuilding pb(*i, out);
        if (pb.isValid())
            planned_buildings.push_back(pb);
    }
}

void Planner::initialize()
{
#define add_building_type(btype, itype) \
    item_for_building_type[df::building_type::btype] = df::item_type::itype; \
    default_item_filters[df::building_type::btype] =  ItemFilter(); \
    available_item_vectors[df::item_type::itype] = std::vector12<df::item *>(); \
    is_relevant_item_type[df::item_type::itype] = true; \
    if (planmode_enabled.find(df::building_type::btype) == planmode_enabled.end()) \
        planmode_enabled[df::building_type::btype] = false

    FOR_ENUM_ITEMS_SIMPLE(item_type, it)
        is_relevant_item_type[it] = false;

    add_building_type(Armorstand, ARMORSTAND);
    add_building_type(Bed, BED);
    add_building_type(Chair, CHAIR);
    add_building_type(Coffin, COFFIN);
    add_building_type(Door, DOOR);
    add_building_type(Floodgate, FLOODGATE);
    add_building_type(Hatch, HATCH_COVER);
    add_building_type(GrateWall, GRATE);
    add_building_type(GrateFloor, GRATE);
    add_building_type(BarsVertical, BAR);
    add_building_type(BarsFloor, BAR);
    add_building_type(Cabinet, CABINET);
    add_building_type(Box, BOX);
    // skip kennels, farm plot
    add_building_type(Weaponrack, WEAPONRACK);
    add_building_type(Statue, STATUE);
    add_building_type(Slab, SLAB);
    add_building_type(Table, TABLE);
    // skip roads ... furnaces
    add_building_type(WindowGlass, WINDOW);
    // skip gem window ... support
    add_building_type(AnimalTrap, ANIMALTRAP);
    add_building_type(Chain, CHAIN);
    add_building_type(Cage, CAGE);
    // skip archery target
    add_building_type(TractionBench, TRACTION_BENCH);
    // skip nest box, hive (tools)

#undef add_building_type
}

void Planner::doCycle()
{
    debug("Running Cycle");
    if (planned_buildings.size() == 0)
        return;

    debug("Planned count: " + int_to_string(planned_buildings.size()));

    gather_available_items();
    for (std::vector12<PlannedBuilding>::iterator building_iter = planned_buildings.begin(); building_iter != planned_buildings.end();)
    {
        if (building_iter->isValid())
        {
            if (show_debugging)
                debug(std::string24("Trying to allocate ") + enum_item_key_str_simple(building_iter->getType()));

            df::item_type required_item_type = item_for_building_type[building_iter->getType()];
            std::vector12<df::item*> *items_vector = &available_item_vectors[required_item_type];
            if (items_vector->size() == 0 || !building_iter->assignClosestItem(items_vector))
            {
                debug("Unable to allocate an item");
                ++building_iter;
                continue;
            }
        }
        debug("Removing building plan");
        building_iter = planned_buildings.erase(building_iter);
    }
}

bool Planner::allocatePlannedBuilding(df::building_type type)
{
    coord32_t cursor;
    if (!DFHack::Gui::getCursorCoords(cursor.x, cursor.y, cursor.z))
        return false;

    df::building* newinst = Buildings::allocInstance(cursor.get_coord16(), type);
    if (!newinst)
        return false;

    df::job_item *filter = new df::job_item();
    filter->item_type = item_type::NONE;
    filter->mat_index = 0;
    filter->flags2.bits.building_material = true;
    std::vector12<df::job_item*> filters;
    filters.push_back(filter);

    if (!Buildings::constructWithFilters(newinst, filters))
    {
        delete newinst;
        return false;
    }

    for (std::vector12<df::job*>::iterator iter = newinst->jobs.begin(); iter != newinst->jobs.end(); iter++)
    {
        (*iter)->flags.bits.suspend = true;
    }

    if (type == building_type::Door)
    {
        df::building_doorst* door = virtual_cast<df::building_doorst>(newinst);
        if (door)
            door->door_flags.bits.pet_passable = true;
    }

    addPlannedBuilding(newinst);

    return true;
}

PlannedBuilding *Planner::getSelectedPlannedBuilding()
{
    for (std::vector12<PlannedBuilding>::iterator building_iter = planned_buildings.begin(); building_iter != planned_buildings.end(); building_iter++)
    {
        if (building_iter->isCurrentlySelectedBuilding())
        {
            return &(*building_iter);
        }
    }

    return NULL;
}

void Planner::adjustMinQuality(df::building_type type, int amount)
{
    df::item_quality* min_quality = &getDefaultItemFilterForType(type)->min_quality;
    *min_quality = static_cast<df::item_quality>(*min_quality + amount);

    boundsCheckItemQuality(min_quality);
    df::item_quality* max_quality = &getDefaultItemFilterForType(type)->max_quality;
    if (*min_quality > *max_quality)
        (*max_quality) = *min_quality;

}

void Planner::adjustMaxQuality(df::building_type type, int amount)
{
    df::item_quality* max_quality = &getDefaultItemFilterForType(type)->max_quality;
    *max_quality = static_cast<df::item_quality>(*max_quality + amount);

    boundsCheckItemQuality(max_quality);
    df::item_quality* min_quality = &getDefaultItemFilterForType(type)->min_quality;
    if (*max_quality < *min_quality)
        (*min_quality) = *max_quality;
}

void Planner::boundsCheckItemQuality(item_quality::item_quality *quality)
{
    *quality = static_cast<df::item_quality>(*quality);
    if (*quality > item_quality::Artifact)
        (*quality) = item_quality::Artifact;
    if (*quality < item_quality::Ordinary)
        (*quality) = item_quality::Ordinary;
}

map<df::building_type, bool> planmode_enabled, saved_planmodes;

bool show_debugging = false;
bool show_help = false;

Planner planner;

RoomMonitor roomMonitor;
