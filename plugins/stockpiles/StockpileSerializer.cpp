#include "StockpileSerializer.h"

//  stockpiles plugin
#include "OrganicMatLookup.h"
#include "StockpileUtils.h"

//  dfhack
#include "MiscUtils.h"

//  df
#include "df/building_stockpilest.h"
#include "df/inorganic_raw.h"
#include "df/creature_raw.h"
#include "df/caste_raw.h"
#include "df/material.h"
#include "df/inorganic_raw.h"
#include "df/plant_raw.h"
#include "df/stockpile_group_set.h"
#include <df/itemdef_ammost.h>
#include <df/itemdef_weaponst.h>
#include <df/itemdef_ammost.h>
#include <df/itemdef_trapcompst.h>
#include <df/itemdef_armorst.h>
#include <df/itemdef_helmst.h>
#include <df/itemdef_shoesst.h>
#include <df/itemdef_glovesst.h>
#include <df/itemdef_pantsst.h>
#include <df/itemdef_shieldst.h>

// protobuf
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fstream>

using std::endl;
using namespace DFHack;
using namespace df::enums;
using namespace google::protobuf;
using namespace dfstockpiles;
using df::global::world;

//using std::placeholders::_1;



int NullBuffer::overflow ( int c )
{
    return c;
}

NullStream::NullStream() : std::ostream ( &m_sb ) {}

StockpileSerializer::StockpileSerializer ( df::building_stockpilest * stockpile )
    : mDebug ( false )
    , mOut ( 0 )
    , mNull()
    , mPile ( stockpile )
{

    // build other_mats indices
    furniture_setup_other_mats();
    bars_blocks_setup_other_mats();
    finished_goods_setup_other_mats();
    weapons_armor_setup_other_mats();
}

StockpileSerializer::~StockpileSerializer() {}

void StockpileSerializer::enable_debug ( std::ostream&out )
{
    mDebug = true;
    mOut = &out;
}

bool StockpileSerializer::serialize_to_ostream ( std::ostream* output )
{
    if ( output->fail( ) ) return false;
    mBuffer.Clear();
    write();
    {
        io::OstreamOutputStream zero_copy_output ( output );
        if ( !mBuffer.SerializeToZeroCopyStream ( &zero_copy_output ) ) return false;
    }
    return output->good();
}

bool StockpileSerializer::serialize_to_file ( const std::string24 & file )
{
    std::fstream output ( file.c_str(), std::ios::out | std::ios::binary |  std::ios::trunc );
    if ( output.fail()  )
    {
        debug() <<  "ERROR: failed to open file for writing: " <<  file <<  endl;
        return false;
    }
    return serialize_to_ostream ( &output );
}

bool StockpileSerializer::parse_from_istream ( std::istream* input )
{
    if ( input->fail( ) ) return false;
    mBuffer.Clear();
    io::IstreamInputStream zero_copy_input ( input );
    const bool res = mBuffer.ParseFromZeroCopyStream ( &zero_copy_input ) && input->eof();
    if ( res ) read();
    return res;
}


bool StockpileSerializer::unserialize_from_file ( const std::string24 & file )
{
    std::fstream input ( file.c_str(), std::ios::in | std::ios::binary );
    if ( input.fail()  )
    {
        debug() <<  "ERROR: failed to open file for reading: " <<  file <<  endl;
        return false;
    }
    return parse_from_istream ( &input );
}

std::ostream & StockpileSerializer::debug()
{
    if ( mDebug )
        return *mOut;
    return mNull;
}


void StockpileSerializer::write()
{
    //      debug() << "GROUP SET " << bitfield_to_string(mPile->settings.flags) << endl;
    write_general();
    if ( mPile->settings.flags.bits.animals )
        write_animals();
    if ( mPile->settings.flags.bits.food )
        write_food();
    if ( mPile->settings.flags.bits.furniture )
        write_furniture();
    if ( mPile->settings.flags.bits.refuse )
        write_refuse();
    if ( mPile->settings.flags.bits.stone )
        write_stone();
    if ( mPile->settings.flags.bits.ammo )
        write_ammo();
    if ( mPile->settings.flags.bits.coins )
        write_coins();
    if ( mPile->settings.flags.bits.bars_blocks )
        write_bars_blocks();
    if ( mPile->settings.flags.bits.gems )
        write_gems();
    if ( mPile->settings.flags.bits.finished_goods )
        write_finished_goods();
    if ( mPile->settings.flags.bits.leather )
        write_leather();
    if ( mPile->settings.flags.bits.cloth )
        write_cloth();
    if ( mPile->settings.flags.bits.wood )
        write_wood();
    if ( mPile->settings.flags.bits.weapons )
        write_weapons();
    if ( mPile->settings.flags.bits.armor )
        write_armor();
}

void StockpileSerializer::read ()
{
    debug() << endl << "==READ==" << endl;
    read_general();
    read_animals();
    read_food();
    read_furniture();
    read_refuse();
    read_stone();
    read_ammo();
    read_coins();
    read_bars_blocks();
    read_gems();
    read_finished_goods();
    read_leather();
    read_cloth();
    read_wood();
    read_weapons();
    read_armor();
}


void StockpileSerializer::serialize_list_organic_mat ( FuncWriteExport add_value, const std::vector12<char> * list, organic_mat_category::organic_mat_category cat )
{
    if ( !list )
    {
        debug()  << "serialize_list_organic_mat: list null" << endl;
    }
    for ( size_t i = 0; i < list->size(); ++i )
    {
        if ( list->at ( i ) )
        {
            std::string24 token = OrganicMatLookup::food_token_by_idx ( debug(), cat, i );
            if ( !token.empty() )
            {
                add_value ( token );
                debug() <<  " organic_material " << i << " is " <<  token <<  endl;
            }
            else
            {
                debug() << "food mat invalid :(" << endl;
            }
        }
    }
}

void StockpileSerializer::unserialize_list_organic_mat ( FuncReadImport get_value, size_t list_size, std::vector12<char> *pile_list,  organic_mat_category::organic_mat_category cat )
{
    pile_list->clear();
    pile_list->resize ( OrganicMatLookup::food_max_size ( cat ),  '\0' );
    for ( size_t i = 0; i < list_size; ++i )
    {
        std::string24 token = get_value ( i );
        int16_t idx = OrganicMatLookup::food_idx_by_token ( debug(), cat, token );
        debug() << "   organic_material " << idx << " is " << token << endl;
        if ( size_t(idx) >=  pile_list->size() )
        {
            debug() <<  "error organic mat index too large!   idx[" << idx <<  "] max_size[" << pile_list->size() <<  "]" <<   endl;
            continue;
        }
        pile_list->at ( idx ) = 1;
    }
}


void StockpileSerializer::serialize_list_item_type ( FuncItemAllowed is_allowed,  FuncWriteExport add_value,  const std::vector12<char> &list )
{
    using df::enums::item_type::item_type;
    //using type_traits = df::enum_traits<item_type>;
    typedef df::enum_traits<item_type> type_traits;
    debug() <<  "item_type size = " <<  list.size() <<  " size limit = " <<  type_traits::last_item_value <<  " typecasted:  " << ( size_t ) type_traits::last_item_value <<  endl;
    for ( size_t i = 0; i <= ( size_t ) type_traits::last_item_value; ++i )
    {
        if ( list.at ( i ) )
        {
            const item_type type = ( item_type ) ( ( df::enum_traits<item_type>::base_type ) i );
            std::string24 r_type ( type_traits::key_table[i+1] );
            if ( !is_allowed ( type ) ) continue;
            add_value ( r_type );
            debug() << "item_type key_table[" << i+1 << "] type[" << ( int16_t ) type <<  "] is " << r_type <<endl;
        }
    }
}


void StockpileSerializer::unserialize_list_item_type ( FuncItemAllowed is_allowed, FuncReadImport read_value,  int32_t list_size,  std::vector12<char> *pile_list )
{
    pile_list->clear();
    pile_list->resize ( 112,  '\0' );                   // TODO remove hardcoded list size value
    for ( size_t i = 0; i < pile_list->size(); ++i )
    {
        pile_list->at ( i ) = is_allowed ( ( item_type::item_type ) i )  ? 0 : 1;
    }
    using df::enums::item_type::item_type;
    df::enum_traits<item_type> type_traits;
    for ( int32_t i = 0; i < list_size; ++i )
    {
        const std::string24 token = read_value ( i );
        // subtract one because item_type starts at -1
        const df::enum_traits<item_type>::base_type idx = linear_index ( debug(), type_traits, token ) - 1;
        const item_type type = ( item_type ) idx;
        if ( !is_allowed ( type ) ) continue;
        debug() << "   item_type " << idx << " is " << token << endl;
        if ( size_t(idx) >=  pile_list->size() )
        {
            debug() <<  "error item_type index too large!   idx[" << idx <<  "] max_size[" << pile_list->size() <<  "]" <<   endl;
            continue;
        }
        pile_list->at ( idx ) =  1;
    }
}


void StockpileSerializer::serialize_list_material ( FuncMaterialAllowed is_allowed,  FuncWriteExport add_value,  const std::vector12<char> &list )
{
    MaterialInfo mi;
    for ( size_t i = 0; i < list.size(); ++i )
    {
        if ( list.at ( i ) )
        {
            mi.decode ( 0, i );
            if ( !is_allowed ( mi ) ) continue;
            debug() << "   material " << i << " is " << mi.getToken() << endl;
            add_value ( mi.getToken() );
        }
    }
}


void StockpileSerializer::unserialize_list_material ( FuncMaterialAllowed is_allowed, FuncReadImport read_value,  int32_t list_size,  std::vector12<char> *pile_list )
{
    //  we initialize all possible (allowed) values to 0,
    //  then all other not-allowed values to 1
    //  why? because that's how the memory is in DF before
    //  we muck with it.
    std::set8<int32_t> idx_set;
    pile_list->clear();
    pile_list->resize ( world->raws.inorganics.size(),  0 );
    for ( size_t i = 0; i < pile_list->size(); ++i )
    {
        MaterialInfo mi ( 0,  i );
        pile_list->at ( i ) = is_allowed ( mi )  ? 0 : 1;
    }
    for ( int i = 0; i < list_size; ++i )
    {
        const std::string24 token = read_value ( i );
        MaterialInfo mi;
        mi.find ( token );
        if ( !is_allowed ( mi ) ) continue;
        debug() << "   material " << mi.index << " is " << token << endl;
        if ( size_t(mi.index) >=  pile_list->size() )
        {
            debug() <<  "error material index too large!   idx[" << mi.index <<  "] max_size[" << pile_list->size() <<  "]" <<   endl;
            continue;
        }
        pile_list->at ( mi.index ) = 1;
    }
}


void StockpileSerializer::serialize_list_quality ( FuncWriteExport add_value, const bool ( &quality_list ) [7] )
{
    using df::enums::item_quality::item_quality;
    //using quality_traits = df::enum_traits<item_quality>;
    typedef df::enum_traits<item_quality> quality_traits;
    for ( size_t i = 0; i < 7; ++i )
    {
        if ( quality_list[i] )
        {
            const std::string24 f_type ( quality_traits::key_table[i] );
            add_value ( f_type );
            debug() << "  quality: " << i << " is " << f_type <<endl;
        }
    }
}


void StockpileSerializer::quality_clear ( bool ( &pile_list ) [7] )
{
    std::fill ( pile_list,  pile_list+7,  false );
}


void StockpileSerializer::unserialize_list_quality ( FuncReadImport read_value,  int32_t list_size, bool ( &pile_list ) [7] )
{
    quality_clear ( pile_list );
    if ( list_size > 0 && list_size <= 7 )
    {
        using df::enums::item_quality::item_quality;
        df::enum_traits<item_quality> quality_traits;
        for ( int i = 0; i < list_size; ++i )
        {
            const std::string24 quality = read_value ( i );
            df::enum_traits<item_quality>::base_type idx = linear_index ( debug(), quality_traits, quality );
            if ( idx < 0 )
            {
                debug() << " invalid quality token " << quality << endl;
                continue;
            }
            debug() << "   quality: " << idx << " is " << quality << endl;
            pile_list[idx] = true;
        }
    }
}


void StockpileSerializer::serialize_list_other_mats ( const std::map<int, std::string24> other_mats, FuncWriteExport add_value,  std::vector12<char> list )
{
    for ( size_t i = 0; i < list.size(); ++i )
    {
        if ( list.at ( i ) )
        {
            const std::string24 token = other_mats_index ( other_mats,  i );
            if ( token.empty() )
            {
                debug() << " invalid other material with index " << i << endl;
                continue;
            }
            add_value ( token );
            debug() << "  other mats " << i << " is " << token << endl;
        }
    }
}


void StockpileSerializer::unserialize_list_other_mats ( const std::map<int, std::string24> other_mats, FuncReadImport read_value,  int32_t list_size, std::vector12<char> *pile_list )
{
    pile_list->clear();
    pile_list->resize ( other_mats.size(),  '\0' );
    for ( int i = 0; i < list_size; ++i )
    {
        const std::string24 token = read_value ( i );
        size_t idx = other_mats_token ( other_mats, token );
        if ( idx < 0 )
        {
            debug() << "invalid other mat with token " << token;
            continue;
        }
        debug() << "  other_mats " << idx << " is " << token << endl;
        if ( idx >=  pile_list->size() )
        {
            debug() <<  "error other_mats index too large!   idx[" << idx <<  "] max_size[" << pile_list->size() <<  "]" <<   endl;
            continue;
        }
        pile_list->at ( idx ) = 1;
    }
}



void StockpileSerializer::serialize_list_itemdef ( FuncWriteExport add_value,  std::vector12<char> list,  std::vector12<df::itemdef *> items,  item_type::item_type type )
{
    for ( size_t i = 0; i < list.size(); ++i )
    {
        if ( list.at ( i ) )
        {
            const df::itemdef *a = items.at ( i );
            // skip procedurally generated items
            if ( a->base_flags.is_set ( df::itemdef_flags::GENERATED ) ) continue;
            ItemTypeInfo ii;
            if ( !ii.decode ( type,  i ) ) continue;
            add_value ( ii.getToken() );
            debug() <<  "  itemdef type" <<  i <<  " is " <<  ii.getToken() << endl;
        }
    }
}


void StockpileSerializer::unserialize_list_itemdef ( FuncReadImport read_value,  int32_t list_size, std::vector12<char> *pile_list, item_type::item_type type )
{
    pile_list->clear();
    pile_list->resize ( Items::getSubtypeCount ( type ),  '\0' );
    for ( int i = 0; i < list_size; ++i )
    {
        std::string24 token = read_value ( i );
        ItemTypeInfo ii;
        if ( !ii.find ( token ) ) continue;
        debug() <<  "  itemdef " <<  ii.subtype <<  " is " <<  token << endl;
        if ( size_t(ii.subtype) >=  pile_list->size() )
        {
            debug() <<  "error itemdef index too large!   idx[" << ii.subtype <<  "] max_size[" << pile_list->size() <<  "]" <<   endl;
            continue;
        }
        pile_list->at ( ii.subtype ) = 1;
    }
}


std::string24 StockpileSerializer::other_mats_index ( const std::map<int, std::string24> other_mats,  int idx )
{
    std::map<int, std::string24>::const_iterator it = other_mats.find ( idx );
    if ( it == other_mats.end() )
        return std::string24();
    return it->second;
}

int StockpileSerializer::other_mats_token ( const std::map<int, std::string24> other_mats,  const std::string24 & token )
{
    for (std::map<int, std::string24>::const_iterator it = other_mats.begin(); it != other_mats.end(); ++it )
    {
        if ( it->second == token )
            return it->first;
    }
    return -1;
}

void StockpileSerializer::write_general()
{
    mBuffer.set_max_bins ( mPile->max_bins );
    mBuffer.set_max_wheelbarrows ( mPile->max_wheelbarrows );
    mBuffer.set_max_barrels ( mPile->max_barrels );
    mBuffer.set_use_links_only ( mPile->use_links_only );
    mBuffer.set_unknown1 ( mPile->settings.unk1 );
    mBuffer.set_allow_inorganic ( mPile->settings.allow_inorganic );
    mBuffer.set_allow_organic ( mPile->settings.allow_organic );
    mBuffer.set_corpses ( mPile->settings.flags.bits.corpses );
}

void StockpileSerializer::read_general()
{
    if ( mBuffer.has_max_bins() )
        mPile->max_bins = mBuffer.max_bins();
    if ( mBuffer.has_max_wheelbarrows() )
        mPile->max_wheelbarrows = mBuffer.max_wheelbarrows();
    if ( mBuffer.has_max_barrels() )
        mPile->max_barrels = mBuffer.max_barrels();
    if ( mBuffer.has_use_links_only() )
        mPile->use_links_only = mBuffer.use_links_only();
    if ( mBuffer.has_unknown1() )
        mPile->settings.unk1 = mBuffer.unknown1();
    if ( mBuffer.has_allow_inorganic() )
        mPile->settings.allow_inorganic = mBuffer.allow_inorganic();
    if ( mBuffer.has_allow_organic() )
        mPile->settings.allow_organic = mBuffer.allow_organic();
    if ( mBuffer.has_corpses() )
        mPile->settings.flags.bits.corpses = mBuffer.corpses();
}

void StockpileSerializer::write_animals()
{
    StockpileSettings::AnimalsSet *animals= mBuffer.mutable_animals();
    animals->set_empty_cages ( mPile->settings.animals.empty_cages );
    animals->set_empty_traps ( mPile->settings.animals.empty_traps );
    for ( size_t i = 0; i < mPile->settings.animals.enabled.size(); ++i )
    {
        if ( mPile->settings.animals.enabled.at ( i ) == 1 )
        {
            df::creature_raw* r = find_creature ( i );
            debug() << "creature "<< r->creature_id << " " << i << endl;
            animals->add_enabled ( r->creature_id );
        }
    }
}

void StockpileSerializer::read_animals()
{
    if ( mBuffer.has_animals() )
    {
        mPile->settings.flags.bits.animals = 1;
        debug() << "animals:" << endl;
        mPile->settings.animals.empty_cages = mBuffer.animals().empty_cages();
        mPile->settings.animals.empty_traps = mBuffer.animals().empty_traps();

        mPile->settings.animals.enabled.clear();
        mPile->settings.animals.enabled.resize ( world->raws.creatures.all.size(), '\0' );
        debug() <<  " pile has " <<  mPile->settings.animals.enabled.size() <<  endl;
        for (int i = 0; i < mBuffer.animals().enabled_size(); ++i )
        {
            std::string24 id = mBuffer.animals().enabled ( i );
            int idx = find_creature ( id );
            debug() << id << " " << idx << endl;
            if ( idx < 0 ||  size_t(idx) >= mPile->settings.animals.enabled.size() )
            {
                debug() <<  "WARNING: animal index invalid: " <<  idx << endl;
                continue;
            }
            mPile->settings.animals.enabled.at ( idx ) = ( char ) 1;
        }
    }
    else
    {
        mPile->settings.animals.enabled.clear();
        mPile->settings.flags.bits.animals = 0;
        mPile->settings.animals.empty_cages = false;
        mPile->settings.animals.empty_traps = false;
    }
}

std::string24 StockpileSerializer::FuncReadImport::operator()(const size_t& idx) const
{
    switch (cat)
    {
        case ITEM_C_MEAT:               return buf->food().meat(idx);
        case ITEM_C_FISH:               return buf->food().fish(idx);
        case ITEM_C_UNPREP_FISH:        return buf->food().unprepared_fish(idx);
        case ITEM_C_EGGS:               return buf->food().egg(idx);
        case ITEM_C_PLANTS:             return buf->food().plants(idx);
        case ITEM_C_PLANTDRINK:         return buf->food().drink_plant(idx);
        case ITEM_C_CREATDRINK:         return buf->food().drink_animal(idx);
        case ITEM_C_PLANTCHEESE:        return buf->food().cheese_plant(idx);
        case ITEM_C_CREATCHEESE:        return buf->food().cheese_animal(idx);
        case ITEM_C_SEED:               return buf->food().seeds(idx);
        case ITEM_C_LEAF:               return buf->food().leaves(idx);
        case ITEM_C_PLANTPOWDER:        return buf->food().powder_plant(idx);
        case ITEM_C_CREATPOWDER:        return buf->food().powder_creature(idx);
        case ITEM_C_GLOB:               return buf->food().glob(idx);
        case ITEM_C_PLANTLIQUID:        return buf->food().liquid_plant(idx);
        case ITEM_C_CREATLIQUID:        return buf->food().liquid_animal(idx);
        case ITEM_C_MISCLIQUID:         return buf->food().liquid_misc(idx);
        case ITEM_C_PASTE:              return buf->food().glob_paste(idx);
        case ITEM_C_PRESSED:            return buf->food().glob_pressed(idx);
        case ITEM_T_LEATHER:            return buf->leather().mats(idx);
        case ITEM_T_FURNITURE:          return buf->furniture().mats(idx);
        case ITEM_T_FURNITURE_OTHER:    return buf->furniture().other_mats(idx);
        case ITEM_Q_FURNITURE_CORE:     return buf->furniture().quality_core(idx);
        case ITEM_Q_FURNITURE_TOTAL:    return buf->furniture().quality_total(idx);
        case ITEM_T_REFUSE:             return buf->refuse().type(idx);
        case ITEM_T_REFUSE_CORPSE:      return buf->refuse().corpses(idx);
        case ITEM_T_REFUSE_PART:        return buf->refuse().body_parts(idx);
        case ITEM_T_REFUSE_SKULL:       return buf->refuse().skulls(idx);
        case ITEM_T_REFUSE_BONE:        return buf->refuse().bones(idx);
        case ITEM_T_REFUSE_HAIR:        return buf->refuse().hair(idx);
        case ITEM_T_REFUSE_SHELL:       return buf->refuse().shells(idx);
        case ITEM_T_REFUSE_TEETH:       return buf->refuse().teeth(idx);
        case ITEM_T_REFUSE_HORN:        return buf->refuse().horns(idx);
        case ITEM_T_STONE:              return buf->stone().mats(idx);
        case ITEM_T_AMMO_TYPE:          return buf->ammo().type(idx);
        case ITEM_T_AMMO_MATS:          return buf->ammo().mats(idx);
        case ITEM_Q_AMMO_CORE:          return buf->ammo().quality_core(idx);
        case ITEM_Q_AMMO_TOTAL:         return buf->ammo().quality_total(idx);
        case ITEM_T_COIN_MATS:          return buf->coin().mats(idx);
        case ITEM_T_BAR_MATS:           return buf->barsblocks().bars_mats(idx);
        case ITEM_T_BAR_MATS_O:         return buf->barsblocks().bars_other_mats(idx);
        case ITEM_T_BLK_MATS:           return buf->barsblocks().blocks_mats(idx);
        case ITEM_T_BLK_MATS_O:         return buf->barsblocks().blocks_other_mats(idx);
        case ITEM_T_GEM_R_MATS:         return buf->gems().rough_mats(idx);
        case ITEM_T_GEM_C_MATS:         return buf->gems().cut_mats(idx);
        case ITEM_T_FGOOD_TYPE:         return buf->finished_goods().type(idx);
        case ITEM_T_FGOOD_MATS:         return buf->finished_goods().mats(idx);
        case ITEM_T_FGOOD_MATS_O:       return buf->finished_goods().other_mats(idx);
        case ITEM_Q_FGOOD_CORE:         return buf->finished_goods().quality_core(idx);
        case ITEM_Q_FGOOD_TOTAL:        return buf->finished_goods().quality_total(idx);
        case ITEM_T_CLOTH_T_SILK:       return buf->cloth().thread_silk(idx);
        case ITEM_T_CLOTH_T_PLANT:      return buf->cloth().thread_plant(idx);
        case ITEM_T_CLOTH_T_YARN:       return buf->cloth().thread_yarn(idx);
        case ITEM_T_CLOTH_T_METAL:      return buf->cloth().thread_metal(idx);
        case ITEM_T_CLOTH_SILK:         return buf->cloth().cloth_silk(idx);
        case ITEM_T_CLOTH_PLANT:        return buf->cloth().cloth_plant(idx);
        case ITEM_T_CLOTH_YARN:         return buf->cloth().cloth_yarn(idx);
        case ITEM_T_CLOTH_METAL:        return buf->cloth().cloth_metal(idx);
        case ITEM_T_WEAP_TYPE:          return buf->weapons().weapon_type(idx);
        case ITEM_T_WEAP_TYPE_T:        return buf->weapons().trapcomp_type(idx);
        case ITEM_T_WEAP_MATS:          return buf->weapons().mats(idx);
        case ITEM_T_WEAP_MATS_O:        return buf->weapons().other_mats(idx);
        case ITEM_Q_WEAP_CORE:          return buf->weapons().quality_core(idx);
        case ITEM_Q_WEAP_TOTAL:         return buf->weapons().quality_total(idx);
        case ITEM_T_ARMO_BODY:          return buf->armor().body(idx);
        case ITEM_T_ARMO_HELM:          return buf->armor().head(idx);
        case ITEM_T_ARMO_SHOES:         return buf->armor().feet(idx);
        case ITEM_T_ARMO_GLOVES:        return buf->armor().hands(idx);
        case ITEM_T_ARMO_PANTS:         return buf->armor().legs(idx);
        case ITEM_T_ARMO_SHIELD:        return buf->armor().shield(idx);
        case ITEM_T_ARMO_MATS:          return buf->armor().mats(idx);
        case ITEM_T_ARMO_MATS_O:        return buf->armor().other_mats(idx);
        case ITEM_Q_ARMO_CORE:          return buf->armor().quality_core(idx);
        case ITEM_Q_ARMO_TOTAL:         return buf->armor().quality_total(idx);
        default:                        return "";
    }
}
void StockpileSerializer::FuncWriteExport::operator()(const std::string24 &id)
{
    switch (cat)
    {
        case ITEM_C_MEAT:               buf->mutable_food()->add_meat(id);
        case ITEM_C_FISH:               buf->mutable_food()->add_fish(id);
        case ITEM_C_UNPREP_FISH:        buf->mutable_food()->add_unprepared_fish(id);
        case ITEM_C_EGGS:               buf->mutable_food()->add_egg(id);
        case ITEM_C_PLANTS:             buf->mutable_food()->add_plants(id);
        case ITEM_C_PLANTDRINK:         buf->mutable_food()->add_drink_plant(id);
        case ITEM_C_CREATDRINK:         buf->mutable_food()->add_drink_animal(id);
        case ITEM_C_PLANTCHEESE:        buf->mutable_food()->add_cheese_plant(id);
        case ITEM_C_CREATCHEESE:        buf->mutable_food()->add_cheese_animal(id);
        case ITEM_C_SEED:               buf->mutable_food()->add_seeds(id);
        case ITEM_C_LEAF:               buf->mutable_food()->add_leaves(id);
        case ITEM_C_PLANTPOWDER:        buf->mutable_food()->add_powder_plant(id);
        case ITEM_C_CREATPOWDER:        buf->mutable_food()->add_powder_creature(id);
        case ITEM_C_GLOB:               buf->mutable_food()->add_glob(id);
        case ITEM_C_PLANTLIQUID:        buf->mutable_food()->add_liquid_plant(id);
        case ITEM_C_CREATLIQUID:        buf->mutable_food()->add_liquid_animal(id);
        case ITEM_C_MISCLIQUID:         buf->mutable_food()->add_liquid_misc(id);
        case ITEM_C_PASTE:              buf->mutable_food()->add_glob_paste(id);
        case ITEM_C_PRESSED:            buf->mutable_food()->add_glob_pressed(id);
        case ITEM_T_LEATHER:            buf->mutable_leather()->add_mats(id);
        case ITEM_T_FURNITURE:          buf->mutable_furniture()->add_mats(id);
        case ITEM_T_FURNITURE_OTHER:    buf->mutable_furniture()->add_other_mats(id);
        case ITEM_Q_FURNITURE_CORE:     buf->mutable_furniture()->add_quality_core(id);
        case ITEM_Q_FURNITURE_TOTAL:    buf->mutable_furniture()->add_quality_total(id);
        case ITEM_T_REFUSE:             buf->mutable_refuse()->add_type(id);
        case ITEM_T_REFUSE_CORPSE:      buf->mutable_refuse()->add_corpses(id);
        case ITEM_T_REFUSE_PART:        buf->mutable_refuse()->add_body_parts(id);
        case ITEM_T_REFUSE_SKULL:       buf->mutable_refuse()->add_skulls(id);
        case ITEM_T_REFUSE_BONE:        buf->mutable_refuse()->add_bones(id);
        case ITEM_T_REFUSE_HAIR:        buf->mutable_refuse()->add_hair(id);
        case ITEM_T_REFUSE_SHELL:       buf->mutable_refuse()->add_shells(id);
        case ITEM_T_REFUSE_TEETH:       buf->mutable_refuse()->add_teeth(id);
        case ITEM_T_REFUSE_HORN:        buf->mutable_refuse()->add_horns(id);
        case ITEM_T_STONE:              buf->mutable_stone()->add_mats(id);
        case ITEM_T_AMMO_TYPE:          buf->mutable_ammo()->add_type(id);
        case ITEM_T_AMMO_MATS:          buf->mutable_ammo()->add_mats(id);
        case ITEM_Q_AMMO_CORE:          buf->mutable_ammo()->add_quality_core(id);
        case ITEM_Q_AMMO_TOTAL:         buf->mutable_ammo()->add_quality_total(id);
        case ITEM_T_COIN_MATS:          buf->mutable_coin()->add_mats(id);
        case ITEM_T_BAR_MATS:           buf->mutable_barsblocks()->add_bars_mats(id);
        case ITEM_T_BAR_MATS_O:         buf->mutable_barsblocks()->add_bars_other_mats(id);
        case ITEM_T_BLK_MATS:           buf->mutable_barsblocks()->add_blocks_mats(id);
        case ITEM_T_BLK_MATS_O:         buf->mutable_barsblocks()->add_blocks_other_mats(id);
        case ITEM_T_GEM_R_MATS:         buf->mutable_gems()->add_rough_mats(id);
        case ITEM_T_GEM_C_MATS:         buf->mutable_gems()->add_cut_mats(id);
        case ITEM_T_FGOOD_TYPE:         buf->mutable_finished_goods()->add_type(id);
        case ITEM_T_FGOOD_MATS:         buf->mutable_finished_goods()->add_mats(id);
        case ITEM_T_FGOOD_MATS_O:       buf->mutable_finished_goods()->add_other_mats(id);
        case ITEM_Q_FGOOD_CORE:         buf->mutable_finished_goods()->add_quality_core(id);
        case ITEM_Q_FGOOD_TOTAL:        buf->mutable_finished_goods()->add_quality_total(id);
        case ITEM_T_CLOTH_T_SILK:       buf->mutable_cloth()->add_thread_silk(id);
        case ITEM_T_CLOTH_T_PLANT:      buf->mutable_cloth()->add_thread_plant(id);
        case ITEM_T_CLOTH_T_YARN:       buf->mutable_cloth()->add_thread_yarn(id);
        case ITEM_T_CLOTH_T_METAL:      buf->mutable_cloth()->add_thread_metal(id);
        case ITEM_T_CLOTH_SILK:         buf->mutable_cloth()->add_cloth_silk(id);
        case ITEM_T_CLOTH_PLANT:        buf->mutable_cloth()->add_cloth_plant(id);
        case ITEM_T_CLOTH_YARN:         buf->mutable_cloth()->add_cloth_yarn(id);
        case ITEM_T_CLOTH_METAL:        buf->mutable_cloth()->add_cloth_metal(id);
        case ITEM_T_WEAP_TYPE:          buf->mutable_weapons()->add_weapon_type(id);
        case ITEM_T_WEAP_TYPE_T:        buf->mutable_weapons()->add_trapcomp_type(id);
        case ITEM_T_WEAP_MATS:          buf->mutable_weapons()->add_mats(id);
        case ITEM_T_WEAP_MATS_O:        buf->mutable_weapons()->add_other_mats(id);
        case ITEM_Q_WEAP_CORE:          buf->mutable_weapons()->add_quality_core(id);
        case ITEM_Q_WEAP_TOTAL:         buf->mutable_weapons()->add_quality_total(id);
        case ITEM_T_ARMO_BODY:          buf->mutable_armor()->add_body(id);
        case ITEM_T_ARMO_HELM:          buf->mutable_armor()->add_head(id);
        case ITEM_T_ARMO_SHOES:         buf->mutable_armor()->add_feet(id);
        case ITEM_T_ARMO_GLOVES:        buf->mutable_armor()->add_hands(id);
        case ITEM_T_ARMO_PANTS:         buf->mutable_armor()->add_legs(id);
        case ITEM_T_ARMO_SHIELD:        buf->mutable_armor()->add_shield(id);
        case ITEM_T_ARMO_MATS:          buf->mutable_armor()->add_mats(id);
        case ITEM_T_ARMO_MATS_O:        buf->mutable_armor()->add_other_mats(id);
        case ITEM_Q_ARMO_CORE:          buf->mutable_armor()->add_quality_core(id);
        case ITEM_Q_ARMO_TOTAL:         buf->mutable_armor()->add_quality_total(id);
        default:                        break;
    }
}

StockpileSerializer::food_pair StockpileSerializer::food_map ( organic_mat_category::organic_mat_category cat )
{
    using df::enums::organic_mat_category::organic_mat_category;
    //using namespace std::placeholders;

    typedef FuncWriteExport Wr;
    typedef FuncReadImport Re;
    switch ( cat )
    {
    case organic_mat_category::Meat:
        return food_pair(Wr(&mBuffer, ITEM_C_MEAT), &mPile->settings.food.meat, Re(&mBuffer, ITEM_C_MEAT), mBuffer.food().meat_size());
    case organic_mat_category::Fish:
        return food_pair ( Wr(&mBuffer, ITEM_C_FISH), &mPile->settings.food.fish, Re(&mBuffer, ITEM_C_FISH), mBuffer.food().fish_size() );
    case organic_mat_category::UnpreparedFish:
        return food_pair ( Wr(&mBuffer, ITEM_C_UNPREP_FISH), &mPile->settings.food.unprepared_fish, Re(&mBuffer, ITEM_C_UNPREP_FISH), mBuffer.food().unprepared_fish_size() );
    case organic_mat_category::Eggs:
        return food_pair ( Wr(&mBuffer, ITEM_C_EGGS), &mPile->settings.food.egg, Re(&mBuffer, ITEM_C_EGGS), mBuffer.food().egg_size() );
    case organic_mat_category::Plants:
        return food_pair ( Wr(&mBuffer, ITEM_C_PLANTS), &mPile->settings.food.plants, Re(&mBuffer, ITEM_C_PLANTS), mBuffer.food().plants_size() );
    case organic_mat_category::PlantDrink:
        return food_pair ( Wr(&mBuffer, ITEM_C_PLANTDRINK), &mPile->settings.food.drink_plant, Re(&mBuffer, ITEM_C_PLANTDRINK), mBuffer.food().drink_plant_size() );
    case organic_mat_category::CreatureDrink:
        return food_pair ( Wr(&mBuffer, ITEM_C_CREATDRINK), &mPile->settings.food.drink_animal, Re(&mBuffer, ITEM_C_CREATDRINK), mBuffer.food().drink_animal_size() );
    case organic_mat_category::PlantCheese:
        return food_pair ( Wr(&mBuffer, ITEM_C_PLANTCHEESE), &mPile->settings.food.cheese_plant, Re(&mBuffer, ITEM_C_PLANTCHEESE), mBuffer.food().cheese_plant_size() );
    case organic_mat_category::CreatureCheese:
        return food_pair ( Wr(&mBuffer, ITEM_C_CREATCHEESE), &mPile->settings.food.cheese_animal, Re(&mBuffer, ITEM_C_CREATCHEESE), mBuffer.food().cheese_animal_size() );
    case organic_mat_category::Seed:
        return food_pair ( Wr(&mBuffer, ITEM_C_SEED), &mPile->settings.food.seeds, Re(&mBuffer, ITEM_C_SEED), mBuffer.food().seeds_size() );
    case organic_mat_category::Leaf:
        return food_pair ( Wr(&mBuffer, ITEM_C_LEAF), &mPile->settings.food.leaves, Re(&mBuffer, ITEM_C_LEAF), mBuffer.food().leaves_size() );
    case organic_mat_category::PlantPowder:
        return food_pair ( Wr(&mBuffer, ITEM_C_PLANTPOWDER), &mPile->settings.food.powder_plant, Re(&mBuffer, ITEM_C_PLANTPOWDER), mBuffer.food().powder_plant_size() );
    case organic_mat_category::CreaturePowder:
        return food_pair ( Wr(&mBuffer, ITEM_C_CREATPOWDER), &mPile->settings.food.powder_creature, Re(&mBuffer, ITEM_C_CREATPOWDER), mBuffer.food().powder_creature_size() );
    case organic_mat_category::Glob:
        return food_pair ( Wr(&mBuffer, ITEM_C_GLOB), &mPile->settings.food.glob, Re(&mBuffer, ITEM_C_GLOB), mBuffer.food().glob_size() );
    case organic_mat_category::PlantLiquid:
        return food_pair ( Wr(&mBuffer, ITEM_C_PLANTLIQUID), &mPile->settings.food.liquid_plant, Re(&mBuffer, ITEM_C_PLANTLIQUID), mBuffer.food().liquid_plant_size() );
    case organic_mat_category::CreatureLiquid:
        return food_pair ( Wr(&mBuffer, ITEM_C_CREATLIQUID), &mPile->settings.food.liquid_animal, Re(&mBuffer, ITEM_C_CREATLIQUID), mBuffer.food().liquid_animal_size() );
    case organic_mat_category::MiscLiquid:
        return food_pair ( Wr(&mBuffer, ITEM_C_MISCLIQUID), &mPile->settings.food.liquid_misc, Re(&mBuffer, ITEM_C_MISCLIQUID), mBuffer.food().liquid_misc_size() );
    case organic_mat_category::Paste:
        return food_pair ( Wr(&mBuffer, ITEM_C_PASTE), &mPile->settings.food.glob_paste, Re(&mBuffer, ITEM_C_PASTE), mBuffer.food().glob_paste_size() );
    case organic_mat_category::Pressed:
        return food_pair ( Wr(&mBuffer, ITEM_C_PRESSED), &mPile->settings.food.glob_pressed, Re(&mBuffer, ITEM_C_PRESSED), mBuffer.food().glob_pressed_size() );
    //case organic_mat_category::Meat:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_meat ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().meat ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.meat, getter, mBuffer.food().meat_size() );
    //}
    //case organic_mat_category::Fish:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_fish ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().fish ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.fish, getter, mBuffer.food().fish_size() );
    //}
    //case organic_mat_category::UnpreparedFish:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_unprepared_fish ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().unprepared_fish ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.unprepared_fish, getter, mBuffer.food().unprepared_fish_size() );
    //}
    //case organic_mat_category::Eggs:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_egg ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().egg ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.egg, getter, mBuffer.food().egg_size() );
    //}
    //case organic_mat_category::Plants:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_plants ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().plants ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.plants, getter, mBuffer.food().plants_size() );
    //}
    //case organic_mat_category::PlantDrink:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_drink_plant ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().drink_plant ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.drink_plant, getter, mBuffer.food().drink_plant_size() );
    //}
    //case organic_mat_category::CreatureDrink:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_drink_animal ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().drink_animal ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.drink_animal, getter, mBuffer.food().drink_animal_size() );
    //}
    //case organic_mat_category::PlantCheese:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_cheese_plant ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().cheese_plant ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.cheese_plant, getter, mBuffer.food().cheese_plant_size() );
    //}
    //case organic_mat_category::CreatureCheese:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_cheese_animal ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().cheese_animal ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.cheese_animal, getter, mBuffer.food().cheese_animal_size() );
    //}
    //case organic_mat_category::Seed:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_seeds ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().seeds ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.seeds, getter, mBuffer.food().seeds_size() );
    //}
    //case organic_mat_category::Leaf:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_leaves ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().leaves ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.leaves, getter, mBuffer.food().leaves_size() );
    //}
    //case organic_mat_category::PlantPowder:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_powder_plant ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().powder_plant ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.powder_plant, getter, mBuffer.food().powder_plant_size() );
    //}
    //case organic_mat_category::CreaturePowder:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_powder_creature ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().powder_creature ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.powder_creature, getter, mBuffer.food().powder_creature_size() );
    //}
    //case organic_mat_category::Glob:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_glob ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().glob ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.glob, getter, mBuffer.food().glob_size() );
    //}
    //case organic_mat_category::PlantLiquid:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_liquid_plant ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().liquid_plant ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.liquid_plant, getter, mBuffer.food().liquid_plant_size() );
    //}
    //case organic_mat_category::CreatureLiquid:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_liquid_animal ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().liquid_animal ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.liquid_animal, getter, mBuffer.food().liquid_animal_size() );
    //}
    //case organic_mat_category::MiscLiquid:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_liquid_misc ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().liquid_misc ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.liquid_misc, getter, mBuffer.food().liquid_misc_size() );
    //}

    //case organic_mat_category::Paste:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_glob_paste ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().glob_paste ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.glob_paste, getter, mBuffer.food().glob_paste_size() );
    //}
    //case organic_mat_category::Pressed:
    //{
    //    FuncWriteExport setter = [=] ( const std::string24 &id )
    //    {
    //        mBuffer.mutable_food()->add_glob_pressed ( id );
    //    };
    //    FuncReadImport getter = [=] ( size_t idx ) -> std::string24 { return mBuffer.food().glob_pressed ( idx ); };
    //    return food_pair ( setter, &mPile->settings.food.glob_pressed, getter, mBuffer.food().glob_pressed_size() );
    //}
    case organic_mat_category::Leather:
    case organic_mat_category::Silk:
    case organic_mat_category::PlantFiber:
    case organic_mat_category::Bone:
    case organic_mat_category::Shell:
    case organic_mat_category::Wood:
    case organic_mat_category::Horn:
    case organic_mat_category::Pearl:
    case organic_mat_category::Tooth:
    case organic_mat_category::EdibleCheese:
    case organic_mat_category::AnyDrink:
    case organic_mat_category::EdiblePlant:
    case organic_mat_category::CookableLiquid:
    case organic_mat_category::CookablePowder:
    case organic_mat_category::CookableSeed:
    case organic_mat_category::CookableLeaf:
    case organic_mat_category::Yarn:
    case organic_mat_category::MetalThread:
    default:
        // not used in stockpile food menu
        break;
    }
    return food_pair();
}


void StockpileSerializer::write_food()
{
    StockpileSettings::FoodSet *food = mBuffer.mutable_food();
    debug() <<  " food: " <<  endl;
    food->set_prepared_meals ( mPile->settings.food.prepared_meals );

    using df::enums::organic_mat_category::organic_mat_category;
    //using traits = df::enum_traits<organic_mat_category>;
    typedef df::enum_traits<organic_mat_category> traits;
    for ( int32_t mat_category = traits::first_item_value; mat_category <traits::last_item_value; ++mat_category )
    {
        food_pair p = food_map ( ( organic_mat_category ) mat_category );
        if ( !p.valid ) continue;
        debug() <<  " food: " <<  traits::key_table[mat_category] <<  endl;
        serialize_list_organic_mat ( p.set_value, p.stockpile_values, ( organic_mat_category ) mat_category );
    }
}


void StockpileSerializer::read_food()
{
    using df::enums::organic_mat_category::organic_mat_category;
    //using traits = df::enum_traits<organic_mat_category>;
    typedef df::enum_traits<organic_mat_category> traits;
    if ( mBuffer.has_food() )
    {
        mPile->settings.flags.bits.food = 1;
        const StockpileSettings::FoodSet food = mBuffer.food();
        debug() << "food:" <<endl;

        if ( food.has_prepared_meals() )
            mPile->settings.food.prepared_meals = food.prepared_meals();
        else
            mPile->settings.food.prepared_meals = true;

        debug() <<  "  prepared_meals: " <<  mPile->settings.food.prepared_meals << endl;

        for ( int32_t mat_category = traits::first_item_value; mat_category <traits::last_item_value; ++mat_category )
        {
            food_pair p = food_map ( ( organic_mat_category ) mat_category );
            if ( !p.valid ) continue;
            unserialize_list_organic_mat ( p.get_value, p.serialized_count, p.stockpile_values, ( organic_mat_category ) mat_category );
        }
    }
    else
    {
        for ( int32_t mat_category = traits::first_item_value; mat_category <traits::last_item_value; ++mat_category )
        {
            food_pair p = food_map ( ( organic_mat_category ) mat_category );
            if ( !p.valid ) continue;
            p.stockpile_values->clear();
        }
        mPile->settings.flags.bits.food = 0;
        mPile->settings.food.prepared_meals = false;
    }
}

void StockpileSerializer::furniture_setup_other_mats()
{
    mOtherMatsFurniture.insert ( std::make_pair ( 0,"WOOD" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 1,"PLANT_CLOTH" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 2,"BONE" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 3,"TOOTH" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 4,"HORN" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 5,"PEARL" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 6,"SHELL" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 7,"LEATHER" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 8,"SILK" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 9,"AMBER" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 10,"CORAL" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 11,"GREEN_GLASS" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 12,"CLEAR_GLASS" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 13,"CRYSTAL_GLASS" ) );
    mOtherMatsFurniture.insert ( std::make_pair ( 14,"YARN" ) );
}

void StockpileSerializer::write_furniture()
{
    StockpileSettings::FurnitureSet *furniture= mBuffer.mutable_furniture();
    furniture->set_sand_bags ( mPile->settings.furniture.sand_bags );

    // FURNITURE type
    using df::enums::furniture_type::furniture_type;
    //using type_traits = df::enum_traits<furniture_type>;
    typedef df::enum_traits<furniture_type> type_traits;
    for ( size_t i = 0; i < mPile->settings.furniture.type.size(); ++i )
    {
        if ( mPile->settings.furniture.type.at ( i ) )
        {
            std::string24 f_type ( type_traits::key_table[i] );
            furniture->add_type ( f_type );
            debug() << "furniture_type " << i << " is " << f_type <<endl;
        }
    }
    // metal, stone/clay materials
    //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::furniture_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    furniture->add_mats ( token );
    //},  mPile->settings.furniture.mats );
    FuncMaterialAllowed filter(this, MATERIAL_ALLOW_FURNITURE);
    serialize_list_material(filter, FuncWriteExport(&mBuffer, ITEM_T_FURNITURE), mPile->settings.furniture.mats);

    // other mats
    //serialize_list_other_mats ( mOtherMatsFurniture, [=] ( const std::string24 &token )
    //{
    //    furniture->add_other_mats ( token );
    //}, mPile->settings.furniture.other_mats );
    serialize_list_other_mats(mOtherMatsFurniture, FuncWriteExport(&mBuffer, ITEM_T_FURNITURE_OTHER), mPile->settings.furniture.other_mats);

    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    furniture->add_quality_core ( token );
    //},  mPile->settings.furniture.quality_core );
    serialize_list_quality(FuncWriteExport(&mBuffer, ITEM_Q_FURNITURE_CORE), mPile->settings.furniture.quality_core);
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    furniture->add_quality_total ( token );
    //},  mPile->settings.furniture.quality_total );
    serialize_list_quality(FuncWriteExport(&mBuffer, ITEM_Q_FURNITURE_TOTAL), mPile->settings.furniture.quality_total);
}

bool StockpileSerializer::furniture_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material
           && ( mi.material->flags.is_set ( material_flags::IS_METAL )
                ||  mi.material->flags.is_set ( material_flags::IS_STONE ) );
}

void StockpileSerializer::read_furniture()
{
    if ( mBuffer.has_furniture() )
    {
        mPile->settings.flags.bits.furniture = 1;
        const StockpileSettings::FurnitureSet furniture = mBuffer.furniture();
        debug() << "furniture:" <<endl;

        if ( mBuffer.furniture().has_sand_bags() )
            mPile->settings.furniture.sand_bags = mBuffer.furniture().sand_bags();
        else
            mPile->settings.furniture.sand_bags = false;

        // type
        using df::enums::furniture_type::furniture_type;
        df::enum_traits<furniture_type> type_traits;
        mPile->settings.furniture.type.clear();
        mPile->settings.furniture.type.resize ( type_traits.last_item_value+1,  '\0' );
        if ( furniture.type_size() > 0 )
        {
            for ( int i = 0; i < furniture.type_size(); ++i )
            {
                const std::string24 type = furniture.type ( i );
                df::enum_traits<furniture_type>::base_type idx = linear_index ( debug(), type_traits, type );
                debug() << "   type " << idx << " is " << type << endl;
                if ( idx < 0 ||  size_t(idx) >=  mPile->settings.furniture.type.size() )
                {
                    debug() <<  "WARNING: furniture type index invalid " << type <<  ", idx=" << idx <<  endl;
                    continue;
                }
                mPile->settings.furniture.type.at ( idx ) = 1;
            }
        }

        //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::furniture_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return furniture.mats ( idx );
        //},  furniture.mats_size(),  &mPile->settings.furniture.mats );
        FuncMaterialAllowed filter(this, MATERIAL_ALLOW_FURNITURE);
        unserialize_list_material(filter, FuncReadImport(&mBuffer, ITEM_T_FURNITURE),
            furniture.mats_size(), &mPile->settings.furniture.mats);

        // other materials
        //unserialize_list_other_mats ( mOtherMatsFurniture,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return furniture.other_mats ( idx );
        //},  furniture.other_mats_size(),  &mPile->settings.furniture.other_mats );
        unserialize_list_other_mats(mOtherMatsFurniture, FuncReadImport(&mBuffer, ITEM_T_FURNITURE_OTHER),
            furniture.other_mats_size(), &mPile->settings.furniture.other_mats);

        // core quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return furniture.quality_core ( idx );
        //},  furniture.quality_core_size(),  mPile->settings.furniture.quality_core );
        unserialize_list_quality(FuncReadImport(&mBuffer, ITEM_Q_FURNITURE_CORE),
            furniture.quality_core_size(), mPile->settings.furniture.quality_core);

        // total quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return furniture.quality_total ( idx );
        //},  furniture.quality_total_size(),  mPile->settings.furniture.quality_total );
        unserialize_list_quality(FuncReadImport(&mBuffer, ITEM_Q_FURNITURE_TOTAL),
            furniture.quality_total_size(), mPile->settings.furniture.quality_total);

    }
    else
    {
        mPile->settings.flags.bits.furniture = 0;
        mPile->settings.furniture.type.clear();
        mPile->settings.furniture.other_mats.clear();
        mPile->settings.furniture.mats.clear();
        quality_clear ( mPile->settings.furniture.quality_core );
        quality_clear ( mPile->settings.furniture.quality_total );
    }
}

bool StockpileSerializer::refuse_creature_is_allowed ( const df::creature_raw *raw )
{
    if ( !raw ) return false;
    // wagon and generated creatures not allowed,  except angels
    const bool is_wagon = raw->creature_id == "EQUIPMENT_WAGON";
    const bool is_generated = raw->flags.is_set ( creature_raw_flags::GENERATED );
    const bool is_angel = is_generated && raw->creature_id.find ( "DIVINE_" ) != std::string24::npos;
    return !is_wagon && ! ( is_generated && !is_angel );
}

void StockpileSerializer::refuse_write_helper ( FuncWriteExport add_value, const std::vector12<char> & list )
{
    for ( size_t i = 0; i < list.size(); ++i )
    {
        if ( list.at ( i ) == 1 )
        {
            df::creature_raw* r = find_creature ( i );
            // skip forgotten beasts, titans, demons, and night creatures
            if ( !refuse_creature_is_allowed ( r ) ) continue;
            debug() << "creature "<< r->creature_id << " " << i << endl;
            add_value ( r->creature_id );
        }
    }
}

bool StockpileSerializer::refuse_type_is_allowed ( item_type::item_type type )
{
    if ( type ==  item_type::NONE
            || type ==  item_type::BAR
            || type ==  item_type::SMALLGEM
            || type ==  item_type::BLOCKS
            || type ==  item_type::ROUGH
            || type ==  item_type::BOULDER
            || type ==  item_type::CORPSE
            || type ==  item_type::CORPSEPIECE
            || type ==  item_type::ROCK
            || type ==  item_type::ORTHOPEDIC_CAST
       ) return false;
    return true;
}


void StockpileSerializer::write_refuse()
{
    StockpileSettings::RefuseSet *refuse = mBuffer.mutable_refuse();
    refuse->set_fresh_raw_hide ( mPile->settings.refuse.fresh_raw_hide );
    refuse->set_rotten_raw_hide ( mPile->settings.refuse.rotten_raw_hide );

    // type
    //FuncItemAllowed filter = std::bind ( &StockpileSerializer::refuse_type_is_allowed, this,  _1 );
    //serialize_list_item_type ( filter, [=] ( const std::string24 &token )
    //{
    //    refuse->add_type ( token );
    //},  mPile->settings.refuse.type );
    FuncItemAllowed filter(this, ITEM_ALLOW_REFUSE);
    serialize_list_item_type(filter, FuncWriteExport(&mBuffer, ITEM_T_REFUSE), mPile->settings.refuse.type);

    // corpses
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_corpses ( id );
    //}, mPile->settings.refuse.corpses );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_CORPSE), mPile->settings.refuse.corpses );
    // body_parts
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_body_parts ( id );
    //}, mPile->settings.refuse.body_parts );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_PART), mPile->settings.refuse.body_parts );
    // skulls
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_skulls ( id );
    //}, mPile->settings.refuse.skulls );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_SKULL), mPile->settings.refuse.skulls );
    // bones
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_bones ( id );
    //}, mPile->settings.refuse.bones );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_BONE), mPile->settings.refuse.bones );
    // hair
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_hair ( id );
    //}, mPile->settings.refuse.hair );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_HAIR), mPile->settings.refuse.hair );
    // shells
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_shells ( id );
    //}, mPile->settings.refuse.shells );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_SHELL), mPile->settings.refuse.shells );
    // teeth
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_teeth ( id );
    //}, mPile->settings.refuse.teeth );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_TEETH), mPile->settings.refuse.teeth );
    // horns
    //refuse_write_helper ( [=] ( const std::string24 &id )
    //{
    //    refuse->add_horns ( id );
    //}, mPile->settings.refuse.horns );
    refuse_write_helper (FuncWriteExport(&mBuffer, ITEM_T_REFUSE_HORN), mPile->settings.refuse.horns );
}

void StockpileSerializer::refuse_read_helper ( FuncReadImport get_value, size_t list_size, std::vector12<char>* pile_list )
{
    pile_list->clear();
    pile_list->resize ( world->raws.creatures.all.size(),  '\0' );
    if ( list_size > 0 )
    {
        for ( size_t i = 0; i < list_size; ++i )
        {
            const std::string24 creature_id = get_value ( i );
            const int idx = find_creature ( creature_id );
            const df::creature_raw* creature = find_creature ( idx );
            if ( idx < 0 ||  !refuse_creature_is_allowed ( creature ) ||  size_t(idx) >=  pile_list->size() )
            {
                debug() << "WARNING invalid refuse creature " << creature_id << ",  idx=" <<  idx <<  endl;
                continue;
            }
            debug() << "      creature " << idx << " is " << creature_id << endl;
            pile_list->at ( idx ) = 1;
        }
    }
}



void StockpileSerializer::read_refuse()
{
    if ( mBuffer.has_refuse() )
    {
        mPile->settings.flags.bits.refuse = 1;
        const StockpileSettings::RefuseSet refuse = mBuffer.refuse();
        debug() << "refuse: " <<endl;
        debug() <<  "  fresh hide " <<  refuse.fresh_raw_hide() << endl;
        debug() <<  "  rotten hide " << refuse.rotten_raw_hide() << endl;
        mPile->settings.refuse.fresh_raw_hide =  refuse.fresh_raw_hide();
        mPile->settings.refuse.rotten_raw_hide =  refuse.rotten_raw_hide();

        // type
        //FuncItemAllowed filter = std::bind ( &StockpileSerializer::refuse_type_is_allowed, this,  _1 );
        //unserialize_list_item_type ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.type ( idx );
        //},  refuse.type_size(),  &mPile->settings.refuse.type );
        FuncItemAllowed filter(this, ITEM_ALLOW_REFUSE);
        unserialize_list_item_type (filter, FuncReadImport(&mBuffer, ITEM_T_REFUSE),
            refuse.type_size(), &mPile->settings.refuse.type );

        // corpses
        debug() << "  corpses" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.corpses ( idx );
        //}, refuse.corpses_size(), &mPile->settings.refuse.corpses );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_CORPSE),
            refuse.corpses_size(), &mPile->settings.refuse.corpses );
        // body_parts
        debug() << "  body_parts" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.body_parts ( idx );
        //}, refuse.body_parts_size(),  &mPile->settings.refuse.body_parts );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_PART),
            refuse.body_parts_size(), &mPile->settings.refuse.body_parts );
        // skulls
        debug() << "  skulls" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.skulls ( idx );
        //}, refuse.skulls_size(),  &mPile->settings.refuse.skulls );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_SKULL),
            refuse.skulls_size(), &mPile->settings.refuse.skulls );
        // bones
        debug() << "  bones" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.bones ( idx );
        //}, refuse.bones_size(),  &mPile->settings.refuse.bones );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_BONE),
            refuse.bones_size(), &mPile->settings.refuse.bones );
        // hair
        debug() << "  hair" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.hair ( idx );
        //}, refuse.hair_size(),  &mPile->settings.refuse.hair );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_HAIR),
            refuse.hair_size(), &mPile->settings.refuse.hair );
        // shells
        debug() << "  shells" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.shells ( idx );
        //}, refuse.shells_size(),  &mPile->settings.refuse.shells );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_SHELL),
            refuse.shells_size(), &mPile->settings.refuse.shells );
        // teeth
        debug() << "  teeth" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.teeth ( idx );
        //}, refuse.teeth_size(),  &mPile->settings.refuse.teeth );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_TEETH),
            refuse.teeth_size(), &mPile->settings.refuse.teeth );
        // horns
        debug() << "  horns" << endl;
        //refuse_read_helper ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return refuse.horns ( idx );
        //}, refuse.horns_size(),  &mPile->settings.refuse.horns );
        refuse_read_helper (FuncReadImport(&mBuffer, ITEM_T_REFUSE_HORN),
            refuse.horns_size(), &mPile->settings.refuse.horns );
    }
    else
    {
        mPile->settings.flags.bits.refuse = 0;
        mPile->settings.refuse.type.clear();
        mPile->settings.refuse.corpses.clear();
        mPile->settings.refuse.body_parts.clear();
        mPile->settings.refuse.skulls.clear();
        mPile->settings.refuse.bones.clear();
        mPile->settings.refuse.hair.clear();
        mPile->settings.refuse.shells.clear();
        mPile->settings.refuse.teeth.clear();
        mPile->settings.refuse.horns.clear();
        mPile->settings.refuse.fresh_raw_hide = false;
        mPile->settings.refuse.rotten_raw_hide = false;
    }
}

bool StockpileSerializer::stone_is_allowed ( const MaterialInfo &mi )
{
    if ( !mi.isValid() ) return false;
    const bool is_allowed_soil = mi.inorganic->flags.is_set ( inorganic_flags::SOIL ) && !mi.inorganic->flags.is_set ( inorganic_flags::AQUIFER );
    const bool is_allowed_stone = mi.material->flags.is_set ( material_flags::IS_STONE ) && !mi.material->flags.is_set ( material_flags::NO_STONE_STOCKPILE );
    return is_allowed_soil ||  is_allowed_stone;
}

void StockpileSerializer::write_stone()
{
    StockpileSettings::StoneSet *stone= mBuffer.mutable_stone();

    //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::stone_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    stone->add_mats ( token );
    //},  mPile->settings.stone.mats );
    FuncMaterialAllowed filter(this, MATERIAL_ALLOW_STONE);
    serialize_list_material(filter, FuncWriteExport(&mBuffer, ITEM_T_STONE), mPile->settings.stone.mats);
}

void StockpileSerializer::read_stone()
{
    if ( mBuffer.has_stone() )
    {
        mPile->settings.flags.bits.stone = 1;
        const StockpileSettings::StoneSet stone = mBuffer.stone();
        debug() << "stone: " <<endl;

        //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::stone_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return stone.mats ( idx );
        //},  stone.mats_size(),  &mPile->settings.stone.mats );
        FuncMaterialAllowed filter(this, MATERIAL_ALLOW_STONE);
        unserialize_list_material(filter, FuncReadImport(&mBuffer, ITEM_T_STONE),
            stone.mats_size(), &mPile->settings.stone.mats);
    }
    else
    {
        mPile->settings.flags.bits.stone = 0;
        mPile->settings.stone.mats.clear();
    }
}

bool StockpileSerializer::ammo_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && mi.material->flags.is_set ( material_flags::IS_METAL );
}

void StockpileSerializer::write_ammo()
{
    StockpileSettings::AmmoSet *ammo = mBuffer.mutable_ammo();

    //  ammo type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    ammo->add_type ( token );
    //},  mPile->settings.ammo.type,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.ammo.begin(),world->raws.itemdefs.ammo.end() ),
    //item_type::AMMO );
    serialize_list_itemdef(FuncWriteExport(&mBuffer, ITEM_T_AMMO_TYPE), mPile->settings.ammo.type,
        std::vector12<df::itemdef*>(world->raws.itemdefs.ammo.begin(),world->raws.itemdefs.ammo.end()),
        item_type::AMMO);

    // metal
    //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::ammo_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    ammo->add_mats ( token );
    //},  mPile->settings.ammo.mats );
    FuncMaterialAllowed filter(this, MATERIAL_ALLOW_AMMO);
    serialize_list_material(filter, FuncWriteExport(&mBuffer, ITEM_T_AMMO_MATS), mPile->settings.ammo.mats);

    // other mats - only wood and bone
    if ( mPile->settings.ammo.other_mats.size() > 2 )
    {
        debug() << "WARNING: ammo other materials > 2! " <<  mPile->settings.ammo.other_mats.size() <<  endl;
    }

    for ( size_t i = 0; i < std::min ( size_t ( 2 ), mPile->settings.ammo.other_mats.size() ); ++i )
    {
        if ( !mPile->settings.ammo.other_mats.at ( i ) )
            continue;
        const std::string24 token = i ==  0  ?  "WOOD"  :  "BONE";
        ammo->add_other_mats ( token );
        debug() << "  other mats " << i << " is " << token << endl;
    }

    // quality core
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    ammo->add_quality_core ( token );
    //},  mPile->settings.ammo.quality_core );
    serialize_list_quality(FuncWriteExport(&mBuffer, ITEM_Q_AMMO_CORE), mPile->settings.ammo.quality_core);

    // quality total
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    ammo->add_quality_total ( token );
    //},  mPile->settings.ammo.quality_total );
    serialize_list_quality(FuncWriteExport(&mBuffer, ITEM_Q_AMMO_TOTAL), mPile->settings.ammo.quality_total);
}

void StockpileSerializer::read_ammo()
{
    if ( mBuffer.has_ammo() )
    {
        mPile->settings.flags.bits.ammo = 1;
        const StockpileSettings::AmmoSet ammo = mBuffer.ammo();
        debug() << "ammo: " <<endl;

        //  ammo type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return ammo.type ( idx );
        //},  ammo.type_size(),  &mPile->settings.ammo.type,  item_type::AMMO );
        unserialize_list_itemdef(FuncReadImport(&mBuffer, ITEM_T_AMMO_TYPE),
            ammo.type_size(), &mPile->settings.ammo.type, item_type::AMMO);

        //  materials metals
        //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::ammo_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return ammo.mats ( idx );
        //},  ammo.mats_size(),  &mPile->settings.ammo.mats );
        FuncMaterialAllowed filter(this, MATERIAL_ALLOW_AMMO);
        unserialize_list_material(filter, FuncReadImport(&mBuffer, ITEM_T_AMMO_MATS),
            ammo.mats_size(), &mPile->settings.ammo.mats);

        //  others
        mPile->settings.ammo.other_mats.clear();
        mPile->settings.ammo.other_mats.resize ( 2,  '\0' );
        if ( ammo.other_mats_size() > 0 )
        {
            // TODO remove hardcoded value
            for ( int i = 0; i < ammo.other_mats_size(); ++i )
            {
                const std::string24 token = ammo.other_mats ( i );
                const int32_t idx = token ==  "WOOD"  ?  0 :  token ==  "BONE"  ? 1 : -1;
                debug() << "   other mats " << idx << " is " << token << endl;
                if ( idx !=  -1 )
                    mPile->settings.ammo.other_mats.at ( idx ) = 1;
            }
        }

        // core quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return ammo.quality_core ( idx );
        //},  ammo.quality_core_size(),  mPile->settings.ammo.quality_core );
        unserialize_list_quality(FuncReadImport(&mBuffer, ITEM_Q_AMMO_CORE),
            ammo.quality_core_size(), mPile->settings.ammo.quality_core);

        // total quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return ammo.quality_total ( idx );
        //},  ammo.quality_total_size(),  mPile->settings.ammo.quality_total );
        unserialize_list_quality(FuncReadImport(&mBuffer, ITEM_Q_AMMO_TOTAL),
            ammo.quality_total_size(), mPile->settings.ammo.quality_total);
    }
    else
    {
        mPile->settings.flags.bits.ammo = 0;
        mPile->settings.ammo.type.clear();
        mPile->settings.ammo.mats.clear();
        mPile->settings.ammo.other_mats.clear();
        quality_clear ( mPile->settings.ammo.quality_core );
        quality_clear ( mPile->settings.ammo.quality_total );
    }
}

bool StockpileSerializer::coins_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid();
}

void StockpileSerializer::write_coins()
{
    StockpileSettings::CoinSet *coins = mBuffer.mutable_coin();
    //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::coins_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    coins->add_mats ( token );
    //},  mPile->settings.coins.mats );
    FuncMaterialAllowed filter(this, MATERIAL_ALLOW_COINS);
    serialize_list_material ( filter, FuncWriteExport(&mBuffer, ITEM_T_COIN_MATS), mPile->settings.coins.mats );
}

void StockpileSerializer::read_coins()
{
    if ( mBuffer.has_coin() )
    {
        mPile->settings.flags.bits.coins = 1;
        const StockpileSettings::CoinSet coins = mBuffer.coin();
        debug() << "coins: " <<endl;

        //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::coins_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return coins.mats ( idx );
        //},  coins.mats_size(),  &mPile->settings.coins.mats );
        FuncMaterialAllowed filter(this, MATERIAL_ALLOW_COINS);
        unserialize_list_material ( filter, FuncReadImport(&mBuffer, ITEM_T_COIN_MATS),
            coins.mats_size(), &mPile->settings.coins.mats );
    }
    else
    {
        mPile->settings.flags.bits.coins = 0;
        mPile->settings.coins.mats.clear();
    }
}

void StockpileSerializer::bars_blocks_setup_other_mats()
{
    mOtherMatsBars.insert ( std::make_pair ( 0,"COAL" ) );
    mOtherMatsBars.insert ( std::make_pair ( 1,"POTASH" ) );
    mOtherMatsBars.insert ( std::make_pair ( 2,"ASH" ) );
    mOtherMatsBars.insert ( std::make_pair ( 3,"PEARLASH" ) );
    mOtherMatsBars.insert ( std::make_pair ( 4,"SOAP" ) );

    mOtherMatsBlocks.insert ( std::make_pair ( 0,"GREEN_GLASS" ) );
    mOtherMatsBlocks.insert ( std::make_pair ( 1,"CLEAR_GLASS" ) );
    mOtherMatsBlocks.insert ( std::make_pair ( 2,"CRYSTAL_GLASS" ) );
    mOtherMatsBlocks.insert ( std::make_pair ( 3,"WOOD" ) );
}

bool StockpileSerializer::bars_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && mi.material->flags.is_set ( material_flags::IS_METAL );
}

bool StockpileSerializer::blocks_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material
           && ( mi.material->flags.is_set ( material_flags::IS_METAL )
                ||  mi.material->flags.is_set ( material_flags::IS_STONE ) );
}


void StockpileSerializer::write_bars_blocks()
{
    StockpileSettings::BarsBlocksSet *bars_blocks = mBuffer.mutable_barsblocks();
    MaterialInfo mi;
    //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::bars_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    bars_blocks->add_bars_mats ( token );
    //},  mPile->settings.bars_blocks.bars_mats );
    FuncMaterialAllowed filter(this, MATERIAL_ALLOW_BARS);
    serialize_list_material ( filter, FuncWriteExport(&mBuffer, ITEM_T_BAR_MATS),  mPile->settings.bars_blocks.bars_mats );

    //  blocks mats
    //filter = std::bind ( &StockpileSerializer::blocks_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter, [=] ( const std::string24 &token )
    //{
    //    bars_blocks->add_blocks_mats ( token );
    //},  mPile->settings.bars_blocks.blocks_mats );
    FuncMaterialAllowed filter2(this, MATERIAL_ALLOW_BLOCKS);
    serialize_list_material ( filter2, FuncWriteExport(&mBuffer, ITEM_T_BLK_MATS),  mPile->settings.bars_blocks.blocks_mats );

    //  bars other mats
    //serialize_list_other_mats ( mOtherMatsBars, [=] ( const std::string24 &token )
    //{
    //    bars_blocks->add_bars_other_mats ( token );
    //}, mPile->settings.bars_blocks.bars_other_mats );
    serialize_list_other_mats ( mOtherMatsBars, FuncWriteExport(&mBuffer, ITEM_T_BAR_MATS_O), mPile->settings.bars_blocks.bars_other_mats );

    //  blocks other mats
    //serialize_list_other_mats ( mOtherMatsBlocks, [=] ( const std::string24 &token )
    //{
    //    bars_blocks->add_blocks_other_mats ( token );
    //}, mPile->settings.bars_blocks.blocks_other_mats );
    serialize_list_other_mats ( mOtherMatsBlocks, FuncWriteExport(&mBuffer, ITEM_T_BLK_MATS_O), mPile->settings.bars_blocks.blocks_other_mats );
}

void StockpileSerializer::read_bars_blocks()
{
    if ( mBuffer.has_barsblocks() )
    {
        mPile->settings.flags.bits.bars_blocks = 1;
        const StockpileSettings::BarsBlocksSet bars_blocks = mBuffer.barsblocks();
        debug() << "bars_blocks: " <<endl;
        // bars
        //FuncMaterialAllowed filter = std::bind ( &StockpileSerializer::bars_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return bars_blocks.bars_mats ( idx );
        //},  bars_blocks.bars_mats_size(),  &mPile->settings.bars_blocks.bars_mats );
        FuncMaterialAllowed filter(this, MATERIAL_ALLOW_BARS);
        unserialize_list_material ( filter, FuncReadImport(&mBuffer, ITEM_T_BAR_MATS),
            bars_blocks.bars_mats_size(), &mPile->settings.bars_blocks.bars_mats );

        //  blocks
        //filter = std::bind ( &StockpileSerializer::blocks_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return bars_blocks.blocks_mats ( idx );
        //},  bars_blocks.blocks_mats_size(),  &mPile->settings.bars_blocks.blocks_mats );
        FuncMaterialAllowed filter2(this, MATERIAL_ALLOW_BLOCKS);
        unserialize_list_material ( filter2, FuncReadImport(&mBuffer, ITEM_T_BLK_MATS),
            bars_blocks.blocks_mats_size(), &mPile->settings.bars_blocks.blocks_mats );
        //  bars other mats
        //unserialize_list_other_mats ( mOtherMatsBars,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return bars_blocks.bars_other_mats ( idx );
        //},  bars_blocks.bars_other_mats_size(),  &mPile->settings.bars_blocks.bars_other_mats );
        unserialize_list_other_mats ( mOtherMatsBars, FuncReadImport(&mBuffer, ITEM_T_BAR_MATS_O),
            bars_blocks.bars_other_mats_size(), &mPile->settings.bars_blocks.bars_other_mats );


        //  blocks other mats
        //unserialize_list_other_mats ( mOtherMatsBlocks,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return bars_blocks.blocks_other_mats ( idx );
        //},  bars_blocks.blocks_other_mats_size(),  &mPile->settings.bars_blocks.blocks_other_mats );
        unserialize_list_other_mats ( mOtherMatsBlocks, FuncReadImport(&mBuffer, ITEM_T_BLK_MATS_O),
            bars_blocks.blocks_other_mats_size(), &mPile->settings.bars_blocks.blocks_other_mats );

    }
    else
    {
        mPile->settings.flags.bits.bars_blocks = 0;
        mPile->settings.bars_blocks.bars_other_mats.clear();
        mPile->settings.bars_blocks.bars_mats.clear();
        mPile->settings.bars_blocks.blocks_other_mats.clear();
        mPile->settings.bars_blocks.blocks_mats.clear();
    }
}

bool StockpileSerializer::gem_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && mi.material->flags.is_set ( material_flags::IS_GEM );
}
bool StockpileSerializer::gem_cut_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && ( mi.material->flags.is_set ( material_flags::IS_GEM ) || mi.material->flags.is_set ( material_flags::IS_STONE ) ) ;
}
bool StockpileSerializer::gem_other_mat_is_allowed ( MaterialInfo &mi )
{
    return mi.isValid() && ( mi.getToken() ==  "GLASS_GREEN" ||  mi.getToken() == "GLASS_CLEAR" || mi.getToken() ==  "GLASS_CRYSTAL" );
}

void StockpileSerializer::write_gems()
{
    StockpileSettings::GemsSet *gems = mBuffer.mutable_gems();
    MaterialInfo mi;
    // rough mats
    //FuncMaterialAllowed filter_rough = std::bind ( &StockpileSerializer::gem_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter_rough, [=] ( const std::string24 &token )
    //{
    //    gems->add_rough_mats ( token );
    //},  mPile->settings.gems.rough_mats );
    FuncMaterialAllowed filter_rough(this, MATERIAL_ALLOW_GEM_ROUGH);
    serialize_list_material ( filter_rough, FuncWriteExport(&mBuffer, ITEM_T_GEM_R_MATS), mPile->settings.gems.rough_mats );
    // cut mats
    //FuncMaterialAllowed filter_cut = std::bind ( &StockpileSerializer::gem_cut_mat_is_allowed, this,  _1 );
    //serialize_list_material ( filter_cut, [=] ( const std::string24 &token )
    //{
    //    gems->add_cut_mats ( token );
    //},  mPile->settings.gems.cut_mats );
    FuncMaterialAllowed filter_cut(this, MATERIAL_ALLOW_GEM_CUT);
    serialize_list_material ( filter_cut, FuncWriteExport(&mBuffer, ITEM_T_GEM_C_MATS), mPile->settings.gems.cut_mats );
    //  rough other
    for ( size_t i = 0; i < mPile->settings.gems.rough_other_mats.size(); ++i )
    {
        if ( mPile->settings.gems.rough_other_mats.at ( i ) )
        {
            mi.decode ( i, -1 );
            if ( !gem_other_mat_is_allowed ( mi ) ) continue;
            debug() << "   gem rough_other mat" << i << " is " << mi.getToken() << endl;
            gems->add_rough_other_mats ( mi.getToken() );
        }
    }
    //  cut other
    for ( size_t i = 0; i < mPile->settings.gems.cut_other_mats.size(); ++i )
    {
        if ( mPile->settings.gems.cut_other_mats.at ( i ) )
        {
            mi.decode ( i, -1 );
            if ( !mi.isValid() ) mi.decode ( 0, i );
            if ( !gem_other_mat_is_allowed ( mi ) ) continue;
            debug() << "   gem cut_other mat" << i << " is " << mi.getToken() << endl;
            gems->add_cut_other_mats ( mi.getToken() );
        }
    }
}

void StockpileSerializer::read_gems()
{
    if ( mBuffer.has_gems() )
    {
        mPile->settings.flags.bits.gems = 1;
        const StockpileSettings::GemsSet gems = mBuffer.gems();
        debug() << "gems: " <<endl;
        // rough
        //FuncMaterialAllowed filter_rough = std::bind ( &StockpileSerializer::gem_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter_rough, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return gems.rough_mats ( idx );
        //},  gems.rough_mats_size(),  &mPile->settings.gems.rough_mats );
        FuncMaterialAllowed filter_rough(this, MATERIAL_ALLOW_GEM_ROUGH);
        unserialize_list_material(filter_rough, FuncReadImport(&mBuffer, ITEM_T_GEM_R_MATS),
            gems.rough_mats_size(), &mPile->settings.gems.rough_mats);

        // cut
        //FuncMaterialAllowed filter_cut = std::bind ( &StockpileSerializer::gem_cut_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( filter_cut, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return gems.cut_mats ( idx );
        //},  gems.cut_mats_size(), &mPile->settings.gems.cut_mats );
        FuncMaterialAllowed filter_cut(this, MATERIAL_ALLOW_GEM_CUT);
        unserialize_list_material(filter_cut, FuncReadImport(&mBuffer, ITEM_T_GEM_C_MATS),
            gems.cut_mats_size(), &mPile->settings.gems.cut_mats);

        //const size_t builtin_size = std::extent<decltype ( world->raws.mat_table.builtin ) >::value;
        static const size_t builtin_size = sizeof(world->raws.mat_table.builtin)/sizeof(world->raws.mat_table.builtin[0]);
        // rough other
        mPile->settings.gems.rough_other_mats.clear();
        mPile->settings.gems.rough_other_mats.resize ( builtin_size, '\0' );
        for ( int i = 0; i < gems.rough_other_mats_size(); ++i )
        {
            const std::string24 token = gems.rough_other_mats ( i );
            MaterialInfo mi;
            mi.find ( token );
            if ( !mi.isValid() ||  size_t(mi.type) >=  builtin_size )
            {
                debug() <<  "WARNING: invalid gem mat " <<  token <<  ". idx=" <<  mi.type << endl;
                continue;
            }
            debug() << "   rough_other mats " << mi.type << " is " << token << endl;
            mPile->settings.gems.rough_other_mats.at ( mi.type ) = 1;
        }

        // cut other
        mPile->settings.gems.cut_other_mats.clear();
        mPile->settings.gems.cut_other_mats.resize ( builtin_size, '\0' );
        for ( int i = 0; i < gems.cut_other_mats_size(); ++i )
        {
            const std::string24 token = gems.cut_other_mats ( i );
            MaterialInfo mi;
            mi.find ( token );
            if ( !mi.isValid() ||  size_t(mi.type) >=  builtin_size )
            {
                debug() <<  "WARNING: invalid gem mat " <<  token <<  ". idx=" <<  mi.type << endl;
                continue;
            }
            debug() << "   cut_other mats " << mi.type << " is " << token << endl;
            mPile->settings.gems.cut_other_mats.at ( mi.type ) = 1;
        }
    }
    else
    {
        mPile->settings.flags.bits.gems = 0;
        mPile->settings.gems.cut_other_mats.clear();
        mPile->settings.gems.cut_mats.clear();
        mPile->settings.gems.rough_other_mats.clear();
        mPile->settings.gems.rough_mats.clear();
    }
}

bool StockpileSerializer::finished_goods_type_is_allowed ( item_type::item_type type )
{
    switch ( type )
    {
    case item_type::CHAIN:
    case item_type::FLASK:
    case item_type::GOBLET:
    case item_type::INSTRUMENT:
    case item_type::TOY:
    case item_type::ARMOR:
    case item_type::SHOES:
    case item_type::HELM:
    case item_type::GLOVES:
    case item_type::FIGURINE:
    case item_type::AMULET:
    case item_type::SCEPTER:
    case item_type::CROWN:
    case item_type::RING:
    case item_type::EARRING:
    case item_type::BRACELET:
    case item_type::GEM:
    case item_type::TOTEM:
    case item_type::PANTS:
    case item_type::BACKPACK:
    case item_type::QUIVER:
    case item_type::SPLINT:
    case item_type::CRUTCH:
    case item_type::TOOL:
    case item_type::BOOK:
        return true;
    default:
        return false;
    }

}

void StockpileSerializer::finished_goods_setup_other_mats()
{
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 0,"WOOD" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 1,"PLANT_CLOTH" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 2,"BONE" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 3,"TOOTH" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 4,"HORN" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 5,"PEARL" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 6,"SHELL" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 7,"LEATHER" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 8,"SILK" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 9,"AMBER" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 10,"CORAL" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 11,"GREEN_GLASS" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 12,"CLEAR_GLASS" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 13,"CRYSTAL_GLASS" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 14,"YARN" ) );
    mOtherMatsFinishedGoods.insert ( std::make_pair ( 15,"WAX" ) );
}

bool StockpileSerializer::finished_goods_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid()
           && mi.material
           && ( mi.material->flags.is_set ( material_flags::IS_GEM )
                || mi.material->flags.is_set ( material_flags::IS_METAL )
                || mi.material->flags.is_set ( material_flags::IS_STONE ) ) ;
}

void StockpileSerializer::write_finished_goods()
{
    StockpileSettings::FinishedGoodsSet *finished_goods = mBuffer.mutable_finished_goods();

    // type
    //FuncItemAllowed filter = std::bind ( &StockpileSerializer::finished_goods_type_is_allowed, this,  _1 );
    //serialize_list_item_type ( filter, [=] ( const std::string24 &token )
    //{
    //    finished_goods->add_type ( token );
    //},  mPile->settings.finished_goods.type );
    FuncItemAllowed filter(this, ITEM_ALLOW_FGOODS);
    serialize_list_item_type ( filter, FuncWriteExport(&mBuffer, ITEM_T_FGOOD_TYPE),
        mPile->settings.finished_goods.type );

    // materials
    //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::finished_goods_mat_is_allowed, this,  _1 );
    //serialize_list_material ( mat_filter, [=] ( const std::string24 &token )
    //{
    //    finished_goods->add_mats ( token );
    //},  mPile->settings.finished_goods.mats );
    FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_FGOODS);
    serialize_list_material ( mat_filter, FuncWriteExport(&mBuffer, ITEM_T_FGOOD_MATS),
        mPile->settings.finished_goods.mats );

    // other mats
    //serialize_list_other_mats ( mOtherMatsFinishedGoods, [=] ( const std::string24 &token )
    //{
    //    finished_goods->add_other_mats ( token );
    //}, mPile->settings.finished_goods.other_mats );
    serialize_list_other_mats ( mOtherMatsFinishedGoods, FuncWriteExport(&mBuffer, ITEM_T_FGOOD_MATS_O),
        mPile->settings.finished_goods.other_mats );

    // quality core
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    finished_goods->add_quality_core ( token );
    //},  mPile->settings.finished_goods.quality_core );
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_FGOOD_CORE),
        mPile->settings.finished_goods.quality_core );

    // quality total
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    finished_goods->add_quality_total ( token );
    //},  mPile->settings.finished_goods.quality_total );
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_FGOOD_TOTAL),
        mPile->settings.finished_goods.quality_total );
}

void StockpileSerializer::read_finished_goods()
{
    if ( mBuffer.has_finished_goods() )
    {
        mPile->settings.flags.bits.finished_goods = 1;
        const StockpileSettings::FinishedGoodsSet finished_goods = mBuffer.finished_goods();
        debug() << "finished_goods: " <<endl;

        // type
        //FuncItemAllowed filter = std::bind ( &StockpileSerializer::finished_goods_type_is_allowed, this,  _1 );
        //unserialize_list_item_type ( filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return finished_goods.type ( idx );
        //},  finished_goods.type_size(),  &mPile->settings.finished_goods.type );
        FuncItemAllowed filter(this, ITEM_ALLOW_FGOODS);
        unserialize_list_item_type ( filter, FuncReadImport(&mBuffer, ITEM_T_FGOOD_TYPE),
            finished_goods.type_size(), &mPile->settings.finished_goods.type );

        // materials
        //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::finished_goods_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( mat_filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return finished_goods.mats ( idx );
        //},  finished_goods.mats_size(),  &mPile->settings.finished_goods.mats );
        FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_FGOODS);
        unserialize_list_material ( mat_filter, FuncReadImport(&mBuffer, ITEM_T_FGOOD_MATS),
            finished_goods.mats_size(), &mPile->settings.finished_goods.mats );

        // other mats
        //unserialize_list_other_mats ( mOtherMatsFinishedGoods,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return finished_goods.other_mats ( idx );
        //},  finished_goods.other_mats_size(),  &mPile->settings.finished_goods.other_mats );
        unserialize_list_other_mats ( mOtherMatsFinishedGoods, FuncReadImport(&mBuffer, ITEM_T_FGOOD_MATS_O),
            finished_goods.other_mats_size(), &mPile->settings.finished_goods.other_mats );

        // core quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return finished_goods.quality_core ( idx );
        //},  finished_goods.quality_core_size(),  mPile->settings.finished_goods.quality_core );
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_FGOOD_CORE),
            finished_goods.quality_core_size(), mPile->settings.finished_goods.quality_core );

        // total quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return finished_goods.quality_total ( idx );
        //},  finished_goods.quality_total_size(),  mPile->settings.finished_goods.quality_total );
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_FGOOD_TOTAL),
            finished_goods.quality_total_size(), mPile->settings.finished_goods.quality_total );

    }
    else
    {
        mPile->settings.flags.bits.finished_goods = 0;
        mPile->settings.finished_goods.type.clear();
        mPile->settings.finished_goods.other_mats.clear();
        mPile->settings.finished_goods.mats.clear();
        quality_clear ( mPile->settings.finished_goods.quality_core );
        quality_clear ( mPile->settings.finished_goods.quality_total );
    }
}

void StockpileSerializer::write_leather()
{
    StockpileSettings::LeatherSet *leather = mBuffer.mutable_leather();

    //FuncWriteExport setter = [=] ( const std::string24 &id )
    //{
    //    leather->add_mats ( id );
    //};
    //serialize_list_organic_mat ( setter, &mPile->settings.leather.mats, organic_mat_category::Leather );
    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_LEATHER),
        &mPile->settings.leather.mats, organic_mat_category::Leather );
}
void StockpileSerializer::read_leather()
{
    if ( mBuffer.has_leather() )
    {
        mPile->settings.flags.bits.leather = 1;
        const StockpileSettings::LeatherSet leather = mBuffer.leather();
        debug() << "leather: " <<endl;

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return leather.mats ( idx );
        //}, leather.mats_size(), &mPile->settings.leather.mats, organic_mat_category::Leather );
        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_LEATHER),
            leather.mats_size(), &mPile->settings.leather.mats, organic_mat_category::Leather );
    }
    else
    {
        mPile->settings.flags.bits.leather = 0;
        mPile->settings.leather.mats.clear();
    }
}

void StockpileSerializer::write_cloth()
{
    StockpileSettings::ClothSet * cloth = mBuffer.mutable_cloth();

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_thread_silk ( token );
    //}, &mPile->settings.cloth.thread_silk, organic_mat_category::Silk );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_thread_plant ( token );
    //}, &mPile->settings.cloth.thread_plant,  organic_mat_category::PlantFiber );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_thread_yarn ( token );
    //}, &mPile->settings.cloth.thread_yarn, organic_mat_category::Yarn );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_thread_metal ( token );
    //}, &mPile->settings.cloth.thread_metal, organic_mat_category::MetalThread );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_cloth_silk ( token );
    //}, &mPile->settings.cloth.cloth_silk, organic_mat_category::Silk );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_cloth_plant ( token );
    //}, &mPile->settings.cloth.cloth_plant,  organic_mat_category::PlantFiber );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_cloth_yarn ( token );
    //}, &mPile->settings.cloth.cloth_yarn, organic_mat_category::Yarn );

    //serialize_list_organic_mat ( [=] ( const std::string24 &token )
    //{
    //    cloth->add_cloth_metal ( token );
    //}, &mPile->settings.cloth.cloth_metal, organic_mat_category::MetalThread );
    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_T_SILK),
        &mPile->settings.cloth.thread_silk, organic_mat_category::Silk );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_T_PLANT),
        &mPile->settings.cloth.thread_plant,  organic_mat_category::PlantFiber );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_T_YARN),
        &mPile->settings.cloth.thread_yarn, organic_mat_category::Yarn );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_T_METAL),
        &mPile->settings.cloth.thread_metal, organic_mat_category::MetalThread );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_SILK),
        &mPile->settings.cloth.cloth_silk, organic_mat_category::Silk );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_PLANT),
        &mPile->settings.cloth.cloth_plant,  organic_mat_category::PlantFiber );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_YARN),
        &mPile->settings.cloth.cloth_yarn, organic_mat_category::Yarn );

    serialize_list_organic_mat (FuncWriteExport(&mBuffer, ITEM_T_CLOTH_METAL),
        &mPile->settings.cloth.cloth_metal, organic_mat_category::MetalThread );

}
void StockpileSerializer::read_cloth()
{
    if ( mBuffer.has_cloth() )
    {
        mPile->settings.flags.bits.cloth = 1;
        const StockpileSettings::ClothSet cloth = mBuffer.cloth();
        debug() << "cloth: " <<endl;

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.thread_silk ( idx );
        //}, cloth.thread_silk_size(), &mPile->settings.cloth.thread_silk, organic_mat_category::Silk );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.thread_plant ( idx );
        //}, cloth.thread_plant_size(), &mPile->settings.cloth.thread_plant, organic_mat_category::PlantFiber );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.thread_yarn ( idx );
        //}, cloth.thread_yarn_size(),  &mPile->settings.cloth.thread_yarn, organic_mat_category::Yarn );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.thread_metal ( idx );
        //}, cloth.thread_metal_size(),  &mPile->settings.cloth.thread_metal, organic_mat_category::MetalThread );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.cloth_silk ( idx );
        //}, cloth.cloth_silk_size(),  &mPile->settings.cloth.cloth_silk, organic_mat_category::Silk );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.cloth_plant ( idx );
        //}, cloth.cloth_plant_size(),  &mPile->settings.cloth.cloth_plant, organic_mat_category::PlantFiber );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.cloth_yarn ( idx );
        //}, cloth.cloth_yarn_size(),  &mPile->settings.cloth.cloth_yarn, organic_mat_category::Yarn );

        //unserialize_list_organic_mat ( [=] ( size_t idx ) -> std::string24
        //{
        //    return cloth.cloth_metal ( idx );
        //}, cloth.cloth_metal_size(),  &mPile->settings.cloth.cloth_metal, organic_mat_category::MetalThread );
        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_T_SILK),
            cloth.thread_silk_size(), &mPile->settings.cloth.thread_silk, organic_mat_category::Silk );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_T_PLANT),
            cloth.thread_plant_size(), &mPile->settings.cloth.thread_plant, organic_mat_category::PlantFiber );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_T_YARN),
            cloth.thread_yarn_size(),  &mPile->settings.cloth.thread_yarn, organic_mat_category::Yarn );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_T_METAL),
            cloth.thread_metal_size(),  &mPile->settings.cloth.thread_metal, organic_mat_category::MetalThread );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_SILK),
            cloth.cloth_silk_size(),  &mPile->settings.cloth.cloth_silk, organic_mat_category::Silk );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_PLANT),
            cloth.cloth_plant_size(),  &mPile->settings.cloth.cloth_plant, organic_mat_category::PlantFiber );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_YARN),
            cloth.cloth_yarn_size(),  &mPile->settings.cloth.cloth_yarn, organic_mat_category::Yarn );

        unserialize_list_organic_mat (FuncReadImport(&mBuffer, ITEM_T_CLOTH_METAL),
            cloth.cloth_metal_size(),  &mPile->settings.cloth.cloth_metal, organic_mat_category::MetalThread );
    }
    else
    {
        mPile->settings.cloth.thread_metal.clear();
        mPile->settings.cloth.thread_plant.clear();
        mPile->settings.cloth.thread_silk.clear();
        mPile->settings.cloth.thread_yarn.clear();
        mPile->settings.cloth.cloth_metal.clear();
        mPile->settings.cloth.cloth_plant.clear();
        mPile->settings.cloth.cloth_silk.clear();
        mPile->settings.cloth.cloth_yarn.clear();
        mPile->settings.flags.bits.cloth = 0;
    }
}

bool StockpileSerializer::wood_mat_is_allowed ( const df::plant_raw * plant )
{
    return plant && plant->flags.is_set ( plant_raw_flags::TREE );
}

void StockpileSerializer::write_wood()
{
    StockpileSettings::WoodSet * wood = mBuffer.mutable_wood();
    for ( size_t i = 0; i < mPile->settings.wood.mats.size(); ++i )
    {
        if ( mPile->settings.wood.mats.at ( i ) )
        {
            const df::plant_raw * plant = find_plant ( i );
            if ( !wood_mat_is_allowed ( plant ) ) continue;
            wood->add_mats ( plant->id );
            debug() <<  "  plant " <<  i <<  " is " <<  plant->id <<  endl;
        }
    }
}
void StockpileSerializer::read_wood()
{
    if ( mBuffer.has_wood() )
    {
        mPile->settings.flags.bits.wood = 1;
        const StockpileSettings::WoodSet wood = mBuffer.wood();
        debug() << "wood: " <<endl;

        mPile->settings.wood.mats.clear();
        mPile->settings.wood.mats.resize ( world->raws.plants.all.size(), '\0' );
        for ( int i = 0; i <  wood.mats_size(); ++i )
        {
            const std::string24 token = wood.mats ( i );
            const size_t idx = find_plant ( token );
            if ( idx < 0 ||  idx >= mPile->settings.wood.mats.size() )
            {
                debug() <<  "WARNING wood mat index invalid " <<  token <<  ",  idx=" << idx <<  endl;
                continue;
            }
            debug() <<  "   plant " <<  idx <<  " is " <<  token <<  endl;
            mPile->settings.wood.mats.at ( idx ) = 1;
        }
    }
    else
    {
        mPile->settings.flags.bits.wood = 0;
        mPile->settings.wood.mats.clear();
    }
}

bool StockpileSerializer::weapons_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && (
               mi.material->flags.is_set ( material_flags::IS_METAL ) ||
               mi.material->flags.is_set ( material_flags::IS_STONE ) );

}

void StockpileSerializer::write_weapons()
{
    StockpileSettings::WeaponsSet * weapons = mBuffer.mutable_weapons();

    weapons->set_unusable ( mPile->settings.weapons.unusable );
    weapons->set_usable ( mPile->settings.weapons.usable );

    // weapon type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    weapons->add_weapon_type ( token );
    //},  mPile->settings.weapons.weapon_type,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.weapons.begin(),world->raws.itemdefs.weapons.end() ),
    //item_type::WEAPON );
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_WEAP_TYPE), mPile->settings.weapons.weapon_type,
        std::vector12<df::itemdef*> ( world->raws.itemdefs.weapons.begin(),world->raws.itemdefs.weapons.end() ),
        item_type::WEAPON );

    // trapcomp type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    weapons->add_trapcomp_type ( token );
    //},  mPile->settings.weapons.trapcomp_type,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.trapcomps.begin(),world->raws.itemdefs.trapcomps.end() ),
    //item_type::TRAPCOMP );
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_WEAP_TYPE_T), mPile->settings.weapons.trapcomp_type,
        std::vector12<df::itemdef*> ( world->raws.itemdefs.trapcomps.begin(),world->raws.itemdefs.trapcomps.end() ),
        item_type::TRAPCOMP );

    // materials
    //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::weapons_mat_is_allowed, this,  _1 );
    //serialize_list_material ( mat_filter, [=] ( const std::string24 &token )
    //{
    //    weapons->add_mats ( token );
    //},  mPile->settings.weapons.mats );
    FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_WEAPON);
    serialize_list_material (mat_filter, FuncWriteExport(&mBuffer, ITEM_T_WEAP_MATS), mPile->settings.weapons.mats );

    // other mats
    //serialize_list_other_mats ( mOtherMatsWeaponsArmor, [=] ( const std::string24 &token )
    //{
    //    weapons->add_other_mats ( token );
    //}, mPile->settings.weapons.other_mats );
    serialize_list_other_mats (mOtherMatsWeaponsArmor, FuncWriteExport(&mBuffer, ITEM_T_WEAP_MATS_O),
        mPile->settings.weapons.other_mats );

    // quality core
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    weapons->add_quality_core ( token );
    //},  mPile->settings.weapons.quality_core );
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_WEAP_CORE),  mPile->settings.weapons.quality_core );

    // quality total
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    weapons->add_quality_total ( token );
    //},  mPile->settings.weapons.quality_total );
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_WEAP_TOTAL),  mPile->settings.weapons.quality_total );
}

void StockpileSerializer::read_weapons()
{
    if ( mBuffer.has_weapons() )
    {
        mPile->settings.flags.bits.weapons = 1;
        const StockpileSettings::WeaponsSet weapons = mBuffer.weapons();
        debug() << "weapons: " <<endl;

        bool unusable = weapons.unusable();
        bool usable = weapons.usable();
        debug() <<  "unusable " <<  unusable <<  endl;
        debug() <<  "usable " <<  usable <<  endl;
        mPile->settings.weapons.unusable = unusable;
        mPile->settings.weapons.usable = usable;

        // weapon type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.weapon_type ( idx );
        //},  weapons.weapon_type_size(),  &mPile->settings.weapons.weapon_type, item_type::WEAPON );
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_WEAP_TYPE),
            weapons.weapon_type_size(), &mPile->settings.weapons.weapon_type, item_type::WEAPON );

        // trapcomp type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.trapcomp_type ( idx );
        //},  weapons.trapcomp_type_size(),  &mPile->settings.weapons.trapcomp_type, item_type::TRAPCOMP );
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_WEAP_TYPE_T),
            weapons.trapcomp_type_size(),  &mPile->settings.weapons.trapcomp_type, item_type::TRAPCOMP );

        // materials
        //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::weapons_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( mat_filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.mats ( idx );
        //},  weapons.mats_size(),  &mPile->settings.weapons.mats );
        FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_WEAPON);
        unserialize_list_material (mat_filter, FuncReadImport(&mBuffer, ITEM_T_WEAP_MATS),
            weapons.mats_size(),  &mPile->settings.weapons.mats );

        // other mats
        //unserialize_list_other_mats ( mOtherMatsWeaponsArmor,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.other_mats ( idx );
        //},  weapons.other_mats_size(),  &mPile->settings.weapons.other_mats );
        unserialize_list_other_mats (mOtherMatsWeaponsArmor, FuncReadImport(&mBuffer, ITEM_T_WEAP_MATS_O),
            weapons.other_mats_size(),  &mPile->settings.weapons.other_mats );


        // core quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.quality_core ( idx );
        //},  weapons.quality_core_size(), mPile->settings.weapons.quality_core );
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_WEAP_CORE),
            weapons.quality_core_size(), mPile->settings.weapons.quality_core );
        // total quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return weapons.quality_total ( idx );
        //},  weapons.quality_total_size(), mPile->settings.weapons.quality_total );
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_WEAP_TOTAL),
            weapons.quality_total_size(), mPile->settings.weapons.quality_total );
    }
    else
    {
        mPile->settings.flags.bits.weapons = 0;
        mPile->settings.weapons.weapon_type.clear();
        mPile->settings.weapons.trapcomp_type.clear();
        mPile->settings.weapons.other_mats.clear();
        mPile->settings.weapons.mats.clear();
        quality_clear ( mPile->settings.weapons.quality_core );
        quality_clear ( mPile->settings.weapons.quality_total );
    }

}

void StockpileSerializer::weapons_armor_setup_other_mats()
{
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 0,"WOOD" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 1,"PLANT_CLOTH" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 2,"BONE" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 3,"SHELL" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 4,"LEATHER" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 5,"SILK" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 6,"GREEN_GLASS" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 7,"CLEAR_GLASS" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 8,"CRYSTAL_GLASS" ) );
    mOtherMatsWeaponsArmor.insert ( std::make_pair ( 9,"YARN" ) );
}

bool StockpileSerializer::armor_mat_is_allowed ( const MaterialInfo &mi )
{
    return mi.isValid() && mi.material && mi.material->flags.is_set ( material_flags::IS_METAL );
}

void StockpileSerializer::write_armor()
{
    StockpileSettings::ArmorSet * armor = mBuffer.mutable_armor();

    armor->set_unusable ( mPile->settings.armor.unusable );
    armor->set_usable ( mPile->settings.armor.usable );

    //// armor type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_body ( token );
    //},  mPile->settings.armor.body,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.armor.begin(),world->raws.itemdefs.armor.end() ),
    //item_type::ARMOR );

    //// helm type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_head ( token );
    //},  mPile->settings.armor.head,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.helms.begin(),world->raws.itemdefs.helms.end() ),
    //item_type::HELM );

    //// shoes type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_feet ( token );
    //},  mPile->settings.armor.feet,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.shoes.begin(),world->raws.itemdefs.shoes.end() ),
    //item_type::SHOES );

    //// gloves type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_hands ( token );
    //},  mPile->settings.armor.hands,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.gloves.begin(),world->raws.itemdefs.gloves.end() ),
    //item_type::GLOVES );

    //// pant type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_legs ( token );
    //},  mPile->settings.armor.legs,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.pants.begin(),world->raws.itemdefs.pants.end() ),
    //item_type::PANTS );

    //// shield type
    //serialize_list_itemdef ( [=] ( const std::string24 &token )
    //{
    //    armor->add_shield ( token );
    //},  mPile->settings.armor.shield,
    //std::vector12<df::itemdef*> ( world->raws.itemdefs.shields.begin(),world->raws.itemdefs.shields.end() ),
    //item_type::SHIELD );

    //// materials
    //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::armor_mat_is_allowed, this,  _1 );
    //serialize_list_material ( mat_filter, [=] ( const std::string24 &token )
    //{
    //    armor->add_mats ( token );
    //},  mPile->settings.armor.mats );

    //// other mats
    //serialize_list_other_mats ( mOtherMatsWeaponsArmor, [=] ( const std::string24 &token )
    //{
    //    armor->add_other_mats ( token );
    //}, mPile->settings.armor.other_mats );

    //// quality core
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    armor->add_quality_core ( token );
    //},  mPile->settings.armor.quality_core );

    //// quality total
    //serialize_list_quality ( [=] ( const std::string24 &token )
    //{
    //    armor->add_quality_total ( token );
    //},  mPile->settings.armor.quality_total );
    // armor type

    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_BODY),  mPile->settings.armor.body,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.armor.begin(),world->raws.itemdefs.armor.end() ),
    item_type::ARMOR );
    // helm type
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_HELM),  mPile->settings.armor.head,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.helms.begin(),world->raws.itemdefs.helms.end() ),
    item_type::HELM );
    // shoes type
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_SHOES),  mPile->settings.armor.feet,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.shoes.begin(),world->raws.itemdefs.shoes.end() ),
    item_type::SHOES );
    // gloves type
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_GLOVES),  mPile->settings.armor.hands,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.gloves.begin(),world->raws.itemdefs.gloves.end() ),
    item_type::GLOVES );
    // pant type
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_PANTS),  mPile->settings.armor.legs,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.pants.begin(),world->raws.itemdefs.pants.end() ),
    item_type::PANTS );
    // shield type
    serialize_list_itemdef (FuncWriteExport(&mBuffer, ITEM_T_ARMO_SHIELD),  mPile->settings.armor.shield,
    std::vector12<df::itemdef*> ( world->raws.itemdefs.shields.begin(),world->raws.itemdefs.shields.end() ),
    item_type::SHIELD );

    // materials
    FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_ARMOR);
    serialize_list_material ( mat_filter, FuncWriteExport(&mBuffer, ITEM_T_ARMO_MATS),  mPile->settings.armor.mats );
    // other mats
    serialize_list_other_mats (mOtherMatsWeaponsArmor, FuncWriteExport(&mBuffer, ITEM_T_ARMO_MATS_O), mPile->settings.armor.other_mats );
    // quality core
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_ARMO_CORE),  mPile->settings.armor.quality_core );
    // quality total
    serialize_list_quality (FuncWriteExport(&mBuffer, ITEM_Q_ARMO_TOTAL),  mPile->settings.armor.quality_total );
}

void StockpileSerializer::read_armor()
{
    if ( mBuffer.has_armor() )
    {
        mPile->settings.flags.bits.armor = 1;
        const StockpileSettings::ArmorSet armor = mBuffer.armor();
        debug() << "armor: " <<endl;

        bool unusable = armor.unusable();
        bool usable = armor.usable();
        debug() <<  "unusable " <<  unusable <<  endl;
        debug() <<  "usable " <<  usable <<  endl;
        mPile->settings.armor.unusable = unusable;
        mPile->settings.armor.usable = usable;

        //// body type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.body ( idx );
        //},  armor.body_size(),  &mPile->settings.armor.body,  item_type::ARMOR );

        //// head type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.head ( idx );
        //},  armor.head_size(), &mPile->settings.armor.head, item_type::HELM );

        //// feet type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.feet ( idx );
        //},  armor.feet_size(), &mPile->settings.armor.feet, item_type::SHOES );

        //// hands type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.hands ( idx );
        //},  armor.hands_size(),  &mPile->settings.armor.hands,  item_type::GLOVES );

        //// legs type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.legs ( idx );
        //},  armor.legs_size(),  &mPile->settings.armor.legs,  item_type::PANTS );

        //// shield type
        //unserialize_list_itemdef ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.shield ( idx );
        //},  armor.shield_size(),  &mPile->settings.armor.shield, item_type::SHIELD );

        //// materials
        //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::armor_mat_is_allowed, this,  _1 );
        //unserialize_list_material ( mat_filter, [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.mats ( idx );
        //},  armor.mats_size(),  &mPile->settings.armor.mats );

        //// other mats
        //unserialize_list_other_mats ( mOtherMatsWeaponsArmor,  [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.other_mats ( idx );
        //},  armor.other_mats_size(),  &mPile->settings.armor.other_mats );

        //// core quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.quality_core ( idx );
        //},  armor.quality_core_size(), mPile->settings.armor.quality_core );
        //// total quality
        //unserialize_list_quality ( [=] ( const size_t & idx ) -> const std::string24&
        //{
        //    return armor.quality_total ( idx );
        //},  armor.quality_total_size(),  mPile->settings.armor.quality_total );

        // body type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_BODY),
            armor.body_size(),  &mPile->settings.armor.body,  item_type::ARMOR );
        // head type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_HELM),
            armor.head_size(), &mPile->settings.armor.head, item_type::HELM );
        // feet type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_SHOES),
            armor.feet_size(), &mPile->settings.armor.feet, item_type::SHOES );
        // hands type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_GLOVES),
            armor.hands_size(),  &mPile->settings.armor.hands,  item_type::GLOVES );
        // legs type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_PANTS),
            armor.legs_size(),  &mPile->settings.armor.legs,  item_type::PANTS );
        // shield type
        unserialize_list_itemdef (FuncReadImport(&mBuffer, ITEM_T_ARMO_SHIELD),
            armor.shield_size(),  &mPile->settings.armor.shield, item_type::SHIELD );

        // materials
        //FuncMaterialAllowed mat_filter = std::bind ( &StockpileSerializer::armor_mat_is_allowed, this,  _1 );
        FuncMaterialAllowed mat_filter(this, MATERIAL_ALLOW_ARMOR);
        unserialize_list_material ( mat_filter, FuncReadImport(&mBuffer, ITEM_T_ARMO_MATS),
            armor.mats_size(),  &mPile->settings.armor.mats );
        // other mats
        unserialize_list_other_mats ( mOtherMatsWeaponsArmor, FuncReadImport(&mBuffer, ITEM_T_ARMO_MATS_O),
            armor.other_mats_size(),  &mPile->settings.armor.other_mats );
        // core quality
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_ARMO_CORE),
            armor.quality_core_size(), mPile->settings.armor.quality_core );
        // total quality
        unserialize_list_quality (FuncReadImport(&mBuffer, ITEM_Q_ARMO_TOTAL),
            armor.quality_total_size(),  mPile->settings.armor.quality_total );
    }
    else
    {
        mPile->settings.flags.bits.armor = 0;
        mPile->settings.armor.body.clear();
        mPile->settings.armor.head.clear();
        mPile->settings.armor.feet.clear();
        mPile->settings.armor.hands.clear();
        mPile->settings.armor.legs.clear();
        mPile->settings.armor.shield.clear();
        mPile->settings.armor.other_mats.clear();
        mPile->settings.armor.mats.clear();
        quality_clear ( mPile->settings.armor.quality_core );
        quality_clear ( mPile->settings.armor.quality_total );
    }
}
