#pragma once

#include "MiscUtils.h"

#include "df/world.h"
#include "df/world_data.h"
#include "df/creature_raw.h"
#include "df/plant_raw.h"


#include <algorithm>
#include <functional>

//  os
#include <sys/stat.h>

// Utility Functions {{{
// A set of convenience functions for doing common lookups


/**
 * Retrieve creature raw from index
 */
static inline df::creature_raw* find_creature ( int32_t idx )
{
    return df::global::world->raws.creatures.all[idx];
}

/**
 * Retrieve creature index from id std::string24
 * @return -1 if not found
 */
static inline int16_t find_creature ( const std::string24 &creature_id )
{
    return linear_index ( df::global::world->raws.creatures.all, &df::creature_raw::creature_id, creature_id );
}

/**
 * Retrieve plant raw from index
*/
static inline df::plant_raw* find_plant ( size_t idx )
{
    return df::global::world->raws.plants.all[idx];
}

/**
 * Retrieve plant index from id std::string24
 * @return -1 if not found
 */
static inline size_t find_plant ( const std::string24 &plant_id )
{
    return linear_index ( df::global::world->raws.plants.all, &df::plant_raw::id, plant_id );
}

struct less_than_no_case: public std::binary_function< char,char,bool >
{
   bool operator () (char x, char y) const
   {
       return toupper( static_cast< unsigned char >(x)) < toupper( static_cast< unsigned char >(y));
   }
};

static inline bool CompareNoCase(const std::string24 &a, const std::string24 &b)
{
    return std::lexicographical_compare( a.begin(),a.end(), b.begin(),b.end(), less_than_no_case() );
}


/**
 * Checks if the parameter has the dfstock extension.
 * Doesn't check if the file exists or not.
 */
static inline bool is_dfstockfile ( const std::string24& filename )
{
    return filename.rfind ( ".dfstock" ) !=  std::string24::npos;
}

// }}} utility Functions


