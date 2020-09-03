#pragma once

//  stockpiles plugin
#include "proto/stockpiles.pb.h"

//  dfhack
#include "modules/Materials.h"
#include "modules/Items.h"

//  df
#include "df/world.h"
#include "df/world_data.h"
#include "df/organic_mat_category.h"
#include "df/furniture_type.h"
#include "df/item_quality.h"
#include "df/item_type.h"

//  stl
#include <functional>

#include <ostream>
#include <istream>

namespace df {
struct building_stockpilest;
}


/**
 * Null buffer that acts like /dev/null for when debug is disabled
 */
class NullBuffer : public std::streambuf
{
public:
    int overflow ( int c );
};

class NullStream : public std::ostream
{
public:
    NullStream();
private:
    NullBuffer m_sb;
};


/**
 * Class for serializing the stockpile_settings structure into a Google protobuf
 */
class StockpileSerializer
{
public:
 /**
  * @param out for debugging
  * @param stockpile stockpile to read or write settings to
  */

    StockpileSerializer ( df::building_stockpilest * stockpile );

 ~StockpileSerializer();

 void enable_debug ( std::ostream &out );

 /**
  * Since we depend on protobuf-lite, not the full lib, we copy this function from
  * protobuf message.cc
  */
 bool serialize_to_ostream(std::ostream* output);

 /**
  * Will serialize stockpile settings to a file (overwrites existing files)
  * @return success/failure
  */
 bool serialize_to_file ( const std::string24 & file );

 /**
  * Again, copied from message.cc
  */
 bool parse_from_istream(std::istream* input);


 /**
  * Read stockpile settings from file
  */
 bool unserialize_from_file ( const std::string24 & file );

private:

 bool mDebug;
 std::ostream * mOut;
 NullStream mNull;
 df::building_stockpilest * mPile;
 dfstockpiles::StockpileSettings mBuffer;
 std::map<int, std::string24> mOtherMatsFurniture;
 std::map<int, std::string24> mOtherMatsFinishedGoods;
 std::map<int, std::string24> mOtherMatsBars;
 std::map<int, std::string24> mOtherMatsBlocks;
 std::map<int, std::string24> mOtherMatsWeaponsArmor;


 std::ostream & debug();

 /**
 read memory structures and serialize to protobuf
  */
 void write();

 //  parse serialized data into ui indices
 void read ();

 /**
  * Find an enum's value based off the std::string24 label.
  * @param traits the enum's trait struct
  * @param token the std::string24 value in key_table
  * @return the enum's value,  -1 if not found
  */
 template<typename E>
 static typename df::enum_traits<E>::base_type linear_index ( std::ostream & out, df::enum_traits<E> traits, const std::string24 &token )
 {
  df::enum_traits<E>::base_type j = traits.first_item_value;
  df::enum_traits<E>::base_type limit = traits.last_item_value;
  // sometimes enums start at -1, which is bad news for array indexing
  if ( j < 0 )
  {
   j += abs ( traits.first_item_value );
   limit += abs ( traits.first_item_value );
  }
  for ( ; j <= limit; ++j )
  {
   //             out << " linear_index("<< token <<") = table["<<j<<"/"<<limit<<"]: " <<traits.key_table[j] << endl;
   if ( token.compare ( traits.key_table[j] ) == 0 )
   return j;
  }
  return -1;
 }

 //  read the token from the serailized list during import
 //typedef std::function<std::string24 ( const size_t& ) > FuncReadImport;
 //  add the token to the serialized list during export
 //typedef std::function<void ( const std::string24 & ) > FuncWriteExport;
 //  are item's of item_type allowed?
 //typedef std::function<bool ( df::enums::item_type::item_type ) > FuncItemAllowed;
 //  is this material allowed?
 //typedef std::function<bool ( const DFHack::MaterialInfo & ) > FuncMaterialAllowed;

enum IEDataType
{
    ITEM_C_MEAT = 0,
    ITEM_C_FISH,
    ITEM_C_UNPREP_FISH,
    ITEM_C_EGGS,
    ITEM_C_PLANTS,
    ITEM_C_PLANTDRINK,
    ITEM_C_CREATDRINK,
    ITEM_C_PLANTCHEESE,
    ITEM_C_CREATCHEESE,
    ITEM_C_SEED,
    ITEM_C_LEAF,
    ITEM_C_PLANTPOWDER,
    ITEM_C_CREATPOWDER,
    ITEM_C_GLOB,
    ITEM_C_PLANTLIQUID,
    ITEM_C_CREATLIQUID,
    ITEM_C_MISCLIQUID,
    ITEM_C_PASTE,
    ITEM_C_PRESSED,
    ITEM_T_LEATHER,
    ITEM_T_FURNITURE,
    ITEM_T_FURNITURE_OTHER,
    ITEM_Q_FURNITURE_CORE,
    ITEM_Q_FURNITURE_TOTAL,
    ITEM_T_REFUSE,
    ITEM_T_REFUSE_CORPSE,
    ITEM_T_REFUSE_PART,
    ITEM_T_REFUSE_SKULL,
    ITEM_T_REFUSE_BONE,
    ITEM_T_REFUSE_HAIR,
    ITEM_T_REFUSE_SHELL,
    ITEM_T_REFUSE_TEETH,
    ITEM_T_REFUSE_HORN,
    ITEM_T_STONE,
    ITEM_T_AMMO_TYPE,
    ITEM_T_AMMO_MATS,
    ITEM_Q_AMMO_CORE,
    ITEM_Q_AMMO_TOTAL,
    ITEM_T_COIN_MATS,
    ITEM_T_BAR_MATS,
    ITEM_T_BAR_MATS_O,
    ITEM_T_BLK_MATS,
    ITEM_T_BLK_MATS_O,
    ITEM_T_GEM_R_MATS,
    ITEM_T_GEM_C_MATS,
    ITEM_T_FGOOD_TYPE,
    ITEM_T_FGOOD_MATS,
    ITEM_T_FGOOD_MATS_O,
    ITEM_Q_FGOOD_CORE,
    ITEM_Q_FGOOD_TOTAL,
    ITEM_T_CLOTH_T_SILK,
    ITEM_T_CLOTH_T_PLANT,
    ITEM_T_CLOTH_T_YARN,
    ITEM_T_CLOTH_T_METAL,
    ITEM_T_CLOTH_SILK,
    ITEM_T_CLOTH_PLANT,
    ITEM_T_CLOTH_YARN,
    ITEM_T_CLOTH_METAL,
    ITEM_T_WEAP_TYPE,
    ITEM_T_WEAP_TYPE_T,
    ITEM_T_WEAP_MATS,
    ITEM_T_WEAP_MATS_O,
    ITEM_Q_WEAP_CORE,
    ITEM_Q_WEAP_TOTAL,
    ITEM_T_ARMO_BODY,
    ITEM_T_ARMO_HELM,
    ITEM_T_ARMO_SHOES,
    ITEM_T_ARMO_GLOVES,
    ITEM_T_ARMO_PANTS,
    ITEM_T_ARMO_SHIELD,
    ITEM_T_ARMO_MATS,
    ITEM_T_ARMO_MATS_O,
    ITEM_Q_ARMO_CORE,
    ITEM_Q_ARMO_TOTAL
};
struct FuncReadImport
{
public:
    FuncReadImport(dfstockpiles::StockpileSettings* _buf, IEDataType _cat) : buf(_buf), cat(_cat) {}
    FuncReadImport() : buf(NULL), cat(ITEM_C_MEAT) {} //validity checked in food_pair()

    std::string24 operator()(const size_t& idx) const;

private:
    dfstockpiles::StockpileSettings* buf;
    IEDataType cat;
};
struct FuncWriteExport
{
public:
    FuncWriteExport(dfstockpiles::StockpileSettings* _buf, IEDataType _cat) : buf(_buf), cat(_cat) {}
    FuncWriteExport() : buf(NULL), cat(ITEM_C_MEAT) {} //validity checked in food_pair()

    void operator()(const std::string24 &id);

private:
    dfstockpiles::StockpileSettings* buf;
    IEDataType cat;
};

enum ItemAllowType
{
    ITEM_ALLOW_REFUSE,
    ITEM_ALLOW_FGOODS
};
struct FuncItemAllowed
{
public:
    explicit FuncItemAllowed(StockpileSerializer* const _sr, ItemAllowType _cat) : sr(_sr), cat(_cat) {}

    bool operator()(df::enums::item_type::item_type itype)
    {
        switch (cat)
        {
            case ITEM_ALLOW_REFUSE:     return sr->refuse_type_is_allowed(itype);
            case ITEM_ALLOW_FGOODS:     return sr->finished_goods_type_is_allowed(itype);
            default:                    return true;
        }
    }

private:
    StockpileSerializer* const sr;
    ItemAllowType cat;
};
friend struct FuncItemAllowed; //calling private functions here

enum ItemMaterialAllowType
{
    MATERIAL_ALLOW_FURNITURE,
    MATERIAL_ALLOW_STONE,
    MATERIAL_ALLOW_AMMO,
    MATERIAL_ALLOW_COINS,
    MATERIAL_ALLOW_BARS,
    MATERIAL_ALLOW_BLOCKS,
    MATERIAL_ALLOW_GEM_ROUGH,
    MATERIAL_ALLOW_GEM_CUT,
    MATERIAL_ALLOW_FGOODS,
    MATERIAL_ALLOW_WEAPON,
    MATERIAL_ALLOW_ARMOR
};
struct FuncMaterialAllowed
{
public:
    explicit FuncMaterialAllowed(StockpileSerializer* const _sr, ItemMaterialAllowType _cat) : sr(_sr), cat(_cat) {}

    bool operator()(const DFHack::MaterialInfo &mi)
    {
        switch (cat)
        {
            case MATERIAL_ALLOW_FURNITURE:      return sr->furniture_mat_is_allowed(mi);
            case MATERIAL_ALLOW_STONE:          return sr->stone_is_allowed(mi);
            case MATERIAL_ALLOW_AMMO:           return sr->ammo_mat_is_allowed(mi);
            case MATERIAL_ALLOW_COINS:          return sr->coins_mat_is_allowed(mi);
            case MATERIAL_ALLOW_BARS:           return sr->bars_mat_is_allowed(mi);
            case MATERIAL_ALLOW_BLOCKS:         return sr->blocks_mat_is_allowed(mi);
            case MATERIAL_ALLOW_GEM_ROUGH:      return sr->gem_mat_is_allowed(mi);
            case MATERIAL_ALLOW_GEM_CUT:        return sr->gem_cut_mat_is_allowed(mi);
            case MATERIAL_ALLOW_FGOODS:         return sr->finished_goods_mat_is_allowed(mi);
            case MATERIAL_ALLOW_WEAPON:         return sr->weapons_mat_is_allowed(mi);
            case MATERIAL_ALLOW_ARMOR:          return sr->armor_mat_is_allowed(mi);
            default:                            return true;
        }
    }

private:
    StockpileSerializer* const sr;
    ItemMaterialAllowType cat;
};
friend struct FuncMaterialAllowed; //calling private functions here

 // convenient struct for parsing food stockpile items
 struct food_pair
 {
  // exporting
  FuncWriteExport set_value;
  std::vector12<char> * stockpile_values;
  // importing
  FuncReadImport get_value;
  size_t serialized_count;
  bool valid;

  food_pair ( FuncWriteExport s, std::vector12<char>* sp_v, FuncReadImport g, size_t count )
 : set_value ( s )
 , stockpile_values ( sp_v )
 , get_value ( g )
 , serialized_count ( count )
 , valid ( true )
  {}
  food_pair(): valid( false ) {}
 };

 /**
  * There are many repeated (un)serialization cases throughout the stockpile_settings structure,
  * so the most common cases have been generalized into generic functions using lambdas.
  *
  * The basic process to serialize a stockpile_settings structure is:
  * 1. loop through the list
  * 2. for every element that is TRUE:
  * 3.   map the specific stockpile_settings index into a general material, creature, etc index
  * 4.   verify that type is allowed in the list (e.g.,  no stone in gems stockpiles)
  * 5.   add it to the protobuf using FuncWriteExport
  *
  * The unserialization process is the same in reverse.
  */
 void serialize_list_organic_mat ( FuncWriteExport add_value, const std::vector12<char> * list, df::enums::organic_mat_category::organic_mat_category cat );

 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_organic_mat ( FuncReadImport get_value, size_t list_size, std::vector12<char> *pile_list,  df::enums::organic_mat_category::organic_mat_category cat );


 /**
  * @see serialize_list_organic_mat
  */
 void serialize_list_item_type ( FuncItemAllowed is_allowed,  FuncWriteExport add_value,  const std::vector12<char> &list );


 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_item_type ( FuncItemAllowed is_allowed, FuncReadImport read_value,  int32_t list_size,  std::vector12<char> *pile_list );


 /**
  * @see serialize_list_organic_mat
  */
 void serialize_list_material ( FuncMaterialAllowed is_allowed,  FuncWriteExport add_value,  const std::vector12<char> &list );

 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_material ( FuncMaterialAllowed is_allowed, FuncReadImport read_value,  int32_t list_size,  std::vector12<char> *pile_list );

 /**
  * @see serialize_list_organic_mat
  */
 void serialize_list_quality ( FuncWriteExport add_value, const bool ( &quality_list ) [7] );

 /**
  * Set all values in a bool[7] to false
  */
 void quality_clear ( bool ( &pile_list ) [7] );


 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_quality ( FuncReadImport read_value,  int32_t list_size, bool ( &pile_list ) [7] );


 /**
  * @see serialize_list_organic_mat
  */
 void serialize_list_other_mats ( const std::map<int, std::string24> other_mats, FuncWriteExport add_value,  std::vector12<char> list );

 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_other_mats ( const std::map<int, std::string24> other_mats, FuncReadImport read_value,  int32_t list_size, std::vector12<char> *pile_list );


 /**
  * @see serialize_list_organic_mat
  */
 void serialize_list_itemdef ( FuncWriteExport add_value,  std::vector12<char> list,  std::vector12<df::itemdef *> items,  df::enums::item_type::item_type type );


 /**
  * @see serialize_list_organic_mat
  */
 void unserialize_list_itemdef ( FuncReadImport read_value,  int32_t list_size, std::vector12<char> *pile_list, df::enums::item_type::item_type type );


 /**
  * Given a list of other_materials and an index,  return its corresponding token
  * @return empty std::string24 if not found
  * @see other_mats_token
  */
 std::string24 other_mats_index ( const std::map<int, std::string24> other_mats,  int idx );

 /**
  * Given a list of other_materials and a token,  return its corresponding index
  * @return -1 if not found
  * @see other_mats_index
  */
 int other_mats_token ( const std::map<int, std::string24> other_mats,  const std::string24 & token );

 void write_general();
 void read_general();


 void write_animals();
 void read_animals();


 food_pair food_map ( df::enums::organic_mat_category::organic_mat_category cat );


 void write_food();
 void read_food();

 void furniture_setup_other_mats();
 void write_furniture();
 bool furniture_mat_is_allowed ( const DFHack::MaterialInfo &mi );
 void read_furniture();

 bool refuse_creature_is_allowed ( const df::creature_raw *raw );


 //void refuse_write_helper ( std::function<void ( const std::string24 & ) > add_value, const std::vector12<char> & list );
 void refuse_write_helper ( FuncWriteExport add_value, const std::vector12<char> & list );


 bool refuse_type_is_allowed ( df::enums::item_type::item_type type );


 void write_refuse();
 //void refuse_read_helper ( std::function<std::string24 ( const size_t& ) > get_value, size_t list_size, std::vector12<char>* pile_list );
 void refuse_read_helper ( FuncReadImport get_value, size_t list_size, std::vector12<char>* pile_list );

 void read_refuse();

 bool stone_is_allowed ( const DFHack::MaterialInfo &mi );

 void write_stone();

 void read_stone();

 bool ammo_mat_is_allowed ( const DFHack::MaterialInfo &mi );

 void write_ammo();
 void read_ammo();
 bool coins_mat_is_allowed ( const DFHack::MaterialInfo &mi );

 void write_coins();

 void read_coins();

 void bars_blocks_setup_other_mats();

 bool bars_mat_is_allowed ( const DFHack::MaterialInfo &mi );

 bool blocks_mat_is_allowed ( const DFHack::MaterialInfo &mi );

 void write_bars_blocks();
 void read_bars_blocks();
 bool gem_mat_is_allowed ( const DFHack::MaterialInfo &mi );
 bool gem_cut_mat_is_allowed ( const DFHack::MaterialInfo &mi );
 bool gem_other_mat_is_allowed(DFHack::MaterialInfo &mi );

 void write_gems();

 void read_gems();

 bool finished_goods_type_is_allowed ( df::enums::item_type::item_type type );
 void finished_goods_setup_other_mats();
 bool finished_goods_mat_is_allowed ( const DFHack::MaterialInfo &mi );
 void write_finished_goods();
 void read_finished_goods();
 void write_leather();
 void read_leather();
 void write_cloth();
    void read_cloth();
    bool wood_mat_is_allowed ( const df::plant_raw * plant );
    void write_wood();
    void read_wood();
    bool weapons_mat_is_allowed ( const DFHack::MaterialInfo &mi );
    void write_weapons();
    void read_weapons();
    void weapons_armor_setup_other_mats();
    bool armor_mat_is_allowed ( const DFHack::MaterialInfo &mi );
    void write_armor();
    void read_armor();

};
