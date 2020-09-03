#pragma once

//#include <array>


#include "df/world_region_type.h"

using namespace std;
//using std::array;
using std::ostringstream;



#define fileresult_file_name "./data/init/embark_assistant_fileresult.txt"

namespace embark_assist {
    namespace defs {
        //  Survey types
        //
        //enum class river_sizes : int8_t {
        //    None,
        //    Brook,
        //    Stream,
        //    Minor,
        //    Medium,
        //    Major
        //};

        //enum class aquifer_sizes : int8_t {
        //    NA,
        //    None,
        //    Light,
        //    None_Light,
        //    Heavy,
        //    None_Heavy,
        //    Light_Heavy,
        //    None_Light_Heavy
        //};

        //enum class tree_levels : int8_t {
        //    None,
        //    Very_Scarce,
        //    Scarce,
        //    Woodland,
        //    Heavily_Forested
        //};

        enum river_sizes : int8_t {
            RIVER_SIZE_NONE,
            RIVER_SIZE_BROOK,
            RIVER_SIZE_STREAM,
            RIVER_SIZE_MINOR,
            RIVER_SIZE_MEDIUM,
            RIVER_SIZE_MAJOR
        };

        enum aquifer_sizes : int8_t {
            AQUIFER_SIZE_NA,
            AQUIFER_SIZE_NONE,
            AQUIFER_SIZE_LIGHT,
            AQUIFER_SIZE_NONE_LIGHT,
            AQUIFER_SIZE_HEAVY,
            AQUIFER_SIZE_NONE_HEAVY,
            AQUIFER_SIZE_LIGHT_HEAVY,
            AQUIFER_SIZE_NONE_LIGHT_HEAVY
        };

        enum tree_levels : int8_t {
            TREE_LEVEL_NONE,
            TREE_LEVEL_VERY_SCARCE,
            TREE_LEVEL_SCARCE,
            TREE_LEVEL_WOODLAND,
            TREE_LEVEL_HEAVILY_FORESTED
        };

        struct mid_level_tile {
            mid_level_tile() {
                aquifer = AQUIFER_SIZE_NA;
                clay = false;
                sand = false;
                flux = false;
                coal = false;
                river_present = false;
                river_elevation = 100;

                soil_depth = 0;
                offset = 0;
                elevation = 0;
                adamantine_level = 0;
                magma_level = 0;
                biome_offset = 0;
                trees = TREE_LEVEL_NONE;
                savagery_level = 0;
                evilness_level = 0;
            }
            aquifer_sizes aquifer;
            bool clay;
            bool sand;
            bool flux;
            bool coal;
            int8_t soil_depth;
            int8_t offset;
            int16_t elevation;
            bool river_present;
            int16_t river_elevation;
            int8_t adamantine_level;  // -1 = none, 0 .. 3 = cavern 1 .. magma sea. Currently not used beyond present/absent.
            int8_t magma_level;  // -1 = none, 0 .. 3 = cavern 3 .. surface/volcano
            int8_t biome_offset;
            tree_levels trees;
            uint8_t savagery_level;  // 0 - 2
            uint8_t evilness_level;  // 0 - 2
            std::vector12<bool> metals;
            std::vector12<bool> economics;
            std::vector12<bool> minerals;
        };

        //typedef std::array<std::array<mid_level_tile, 16>, 16> mid_level_tiles;
        typedef std::vector12<std::vector12<mid_level_tile> > mid_level_tiles;

        struct region_tile_datum {
            region_tile_datum() {
                surveyed = false;
                aquifer = AQUIFER_SIZE_NA;
                clay_count = 0;
                sand_count = 0;
                flux_count = 0;
                coal_count = 0;
                min_region_soil = 10;
                max_region_soil = 0;
                max_waterfall = 0;
                min_tree_level = TREE_LEVEL_HEAVILY_FORESTED;
                max_tree_level = TREE_LEVEL_NONE;

                river_size = RIVER_SIZE_NONE;
                biome_count = 0;
                blood_rain_possible = false;
                blood_rain_full = false;
                permanent_syndrome_rain_possible = false;
                permanent_syndrome_rain_full = false;
                temporary_syndrome_rain_possible = false;
                temporary_syndrome_rain_full = false;
                reanimating_possible = false;
                reanimating_full = false;
                thralling_possible = false;
                thralling_full = false;
                for (uint8 i = 0; i < 3; ++i)
                {
                    savagery_count[i] = 0;
                    evilness_count[i] = 0;
                }
                for (uint8 i = 0; i < 10; ++i)
                {
                    biome_index[i] = 0;
                    biome[i] = 0;
                    min_temperature[i] = 0;
                    max_temperature[i] = 0;
                    blood_rain[i] = false;
                    permanent_syndrome_rain[i] = false;
                    temporary_syndrome_rain[i] = false;
                    reanimating[i] = false;
                    thralling[i] = false;
                    permanent_syndrome_rain[i] = false;
                    permanent_syndrome_rain[i] = false;
                    permanent_syndrome_rain[i] = false;
                    permanent_syndrome_rain[i] = false;
                    necro_neighbors = 0;
                }
                for (uint8 i = 0; i < 16; ++i)
                {
                    north_corner_selection[i] = 0;
                    west_corner_selection[i] = 0;
                    north_row_biome_x[i] = 0;
                    west_column_biome_y[i] = 0;
                    for (uint8 j = 0; j < 16; ++j)
                        region_type[i][j] = df::world_region_type(0);
                }
            }
            bool surveyed;
            aquifer_sizes aquifer;
            uint16_t clay_count;
            uint16_t sand_count;
            uint16_t flux_count;
            uint16_t coal_count;
            uint8_t min_region_soil;
            uint8_t max_region_soil;
            uint8_t max_waterfall;
            river_sizes river_size;
            int16_t biome_index[10];  // Indexed through biome_offset; -1 = null, Index of region, [0] not used
            int16_t biome[10];        // Indexed through biome_offset; -1 = null, df::biome_type, [0] not used
            uint8_t biome_count;
            int16_t min_temperature[10];  // Indexed through biome_offset; -30000 = null, Urists - 10000, [0] not used
            int16_t max_temperature[10];  // Indexed through biome_offset; -30000 = null, Urists - 10000, [0] not used
            tree_levels min_tree_level;
            tree_levels max_tree_level;
            bool blood_rain[10];
            bool blood_rain_possible;
            bool blood_rain_full;
            bool permanent_syndrome_rain[10];
            bool permanent_syndrome_rain_possible;
            bool permanent_syndrome_rain_full;
            bool temporary_syndrome_rain[10];
            bool temporary_syndrome_rain_possible;
            bool temporary_syndrome_rain_full;
            bool reanimating[10];
            bool reanimating_possible;
            bool reanimating_full;
            bool thralling[10];
            bool thralling_possible;
            bool thralling_full;
            uint16_t savagery_count[3];
            uint16_t evilness_count[3];
            std::vector12<bool> metals;
            std::vector12<bool> economics;
            std::vector12<bool> minerals;
            std::vector12<int16_t> neighbors;  //  entity_raw indices
            uint8_t necro_neighbors;
            mid_level_tile north_row[16];
            mid_level_tile south_row[16];
            mid_level_tile west_column[16];
            mid_level_tile east_column[16];
            uint8_t north_corner_selection[16]; //  0 - 3. For some reason DF stores everything needed for incursion
            uint8_t west_corner_selection[16];  //  detection in 17:th row/colum data in the region details except
                                                //  this info, so we have to go to neighboring world tiles to fetch it.
            df::world_region_type region_type[16][16];  //  Required for incursion override detection. We could store only the
                                                //  edges, but storing it for every tile allows for a unified fetching
                                                //  logic.
            int8_t north_row_biome_x[16];    //  "biome_x" data cached for the northern row for access from the north.
            int8_t west_column_biome_y[16];  //  "biome_y" data cached for the western row for access from the west.
        };

        struct geo_datum {
            geo_datum() {
                soil_size = 0;
                top_soil_only = true;
                top_soil_aquifer_only = true;
                aquifer_absent = true;
                clay_absent = true;
                sand_absent = true;
                flux_absent = true;
                coal_absent = true;
            }
            uint8_t soil_size;
            bool top_soil_only;
            bool top_soil_aquifer_only;
            bool aquifer_absent;
            bool clay_absent;
            bool sand_absent;
            bool flux_absent;
            bool coal_absent;
            std::vector12<bool> possible_metals;
            std::vector12<bool> possible_economics;
            std::vector12<bool> possible_minerals;
        };

        typedef std::vector12<geo_datum> geo_data;

        struct sites {
            sites(uint8_t _x, uint8_t _y, char _t) : x(_x), y(_y), type(_y) {}
            sites() : x(0), y(0), type(0) {}
            uint8_t x;
            uint8_t y;
            char type;
        };

        struct site_infos {
            site_infos() {
                bool incursions_processed = false;
                aquifer = AQUIFER_SIZE_NA;
                min_soil = 0;
                max_soil = 0;
                flat = false;
                max_waterfall = 0;
                clay = false;
                sand = false;
                flux = false;
                coal = false;
                blood_rain = false;
                permanent_syndrome_rain = false;
                temporary_syndrome_rain = false;
                reanimating = false;
                thralling = false;
                necro_neighbors = 0;
            }
            bool incursions_processed;
            aquifer_sizes aquifer;
            uint8_t min_soil;
            uint8_t max_soil;
            bool flat;
            uint8_t max_waterfall;
            bool clay;
            bool sand;
            bool flux;
            bool coal;
            bool blood_rain;
            bool permanent_syndrome_rain;
            bool temporary_syndrome_rain;
            bool reanimating;
            bool thralling;
            std::vector12<uint16_t> metals;
            std::vector12<uint16_t> economics;
            std::vector12<uint16_t> minerals;
            //  Could add savagery, evilness, and biomes, but DF provides those easily.
            std::vector12<int16_t> neighbors;  //  entity_raw indices
            uint8_t necro_neighbors;
        };

        typedef std::vector12<sites> site_lists;

        typedef std::vector12<std::vector12<region_tile_datum> > world_tile_data;

        typedef bool mlt_matches[16][16];
        //  An embark region match is indicated by marking the top left corner
        //  tile as a match. Thus, the bottom and right side won't show matches
        //  unless the appropriate dimension has a width of 1.

        struct matches {
            matches() {
                preliminary_match = false;
                contains_match = false;
                for (uint8 i = 0; i < 16; ++i)
                    for (uint8 j = 0; j < 16; ++j)
                        mlt_match[i][j] = false;
            }
            bool preliminary_match;
            bool contains_match;
            mlt_matches mlt_match;
        };

        typedef std::vector12<std::vector12<matches> > match_results;

        //  matcher types
        //
        //enum class evil_savagery_values : int8_t {
        //    NA = -1,
        //    All,
        //    Present,
        //    Absent
        //};

        //enum class evil_savagery_ranges : int8_t {
        //    Low,
        //    Medium,
        //    High
        //};

        //enum class aquifer_ranges : int8_t {
        //    NA = -1,
        //    None,
        //    At_Most_Light,
        //    None_Plus_Light,
        //    None_Plus_At_Least_Light,
        //    Light,
        //    At_Least_Light,
        //    None_Plus_Heavy,
        //    At_Most_Light_Plus_Heavy,
        //    Light_Plus_Heavy,
        //    None_Light_Heavy,
        //    Heavy
        //};

        //enum class river_ranges : int8_t {
        //    NA = -1,
        //    None,
        //    Brook,
        //    Stream,
        //    Minor,
        //    Medium,
        //    Major
        //};

//  For possible future use. That's the level of data actually collected.
//        enum class adamantine_ranges : int8_t {
//            NA = -1,
//            Cavern_1,
//            Cavern_2,
//            Cavern_3,
//            Magma_Sea
//        };

        //enum class magma_ranges : int8_t {
        //    NA = -1,
        //    Cavern_3,
        //    Cavern_2,
        //    Cavern_1,
        //    Volcano
        //};

        //enum class yes_no_ranges : int8_t {
        //    NA = -1,
        //    Yes,
        //    No
        //};

        //enum class all_present_ranges : int8_t {
        //    All,
        //    Present
        //};
        //enum class present_absent_ranges : int8_t {
        //    NA = -1,
        //    Present,
        //    Absent
        //};

        //enum class soil_ranges : int8_t {
        //    NA = -1,
        //    None,
        //    Very_Shallow,
        //    Shallow,
        //    Deep,
        //    Very_Deep
        //};

        //enum class syndrome_rain_ranges : int8_t {
        //    NA = -1,
        //    Any,
        //    Permanent,
        //    Temporary,
        //    Not_Permanent,
        //    None
        //};

        //enum class reanimation_ranges : int8_t {
        //    NA = -1,
        //    Both,
        //    Any,
        //    Thralling,
        //    Reanimation,
        //    Not_Thralling,
        //    None
        //};

        //enum class freezing_ranges : int8_t {
        //    NA = -1,
        //    Permanent,
        //    At_Least_Partial,
        //    Partial,
        //    At_Most_Partial,
        //    Never
        //};

        //enum class tree_ranges : int8_t {
        //    NA = -1,
        //    None,
        //    Very_Scarce,  // DF dislays this with a different color but still the "scarce" text
        //    Scarce,
        //    Woodland,
        //    Heavily_Forested
        //};

        enum evil_savagery_values : int8_t {
            EVIL_SAVAGERY_NA = -1,
            EVIL_SAVAGERY_ALL,
            EVIL_SAVAGERY_PRESENT,
            EVIL_SAVAGERY_ABSENT
        };

        enum evil_savagery_ranges : int8_t {
            EVIL_SAVAGERY_LOW,
            EVIL_SAVAGERY_MEDIUM,
            EVIL_SAVAGERY_HIGH
        };

        enum aquifer_ranges : int8_t {
            AQUIFER_NA = -1,
            AQUIFER_NONE,
            AQUIFER_AT_MOST_LIGHT,
            AQUIFER_NONE_PLUS_LIGHT,
            AQUIFER_NONE_PLUS_AT_LEAST_LIGHT,
            AQUIFER_LIGHT,
            AQUIFER_AT_LEAST_LIGHT,
            AQUIFER_NONE_PLUS_HEAVY,
            AQUIFER_AT_MOST_LIGHT_PLUS_HEAVY,
            AQUIFER_LIGHT_PLUS_HEAVY,
            AQUIFER_NONE_LIGHT_HEAVY,
            AQUIFER_HEAVY
        };

        enum river_ranges : int8_t {
            RIVER_NA = -1,
            RIVER_NONE,
            RIVER_BROOK,
            RIVER_STREAM,
            RIVER_MINOR,
            RIVER_MEDIUM,
            RIVER_MAJOR
        };

  //For possible future use. That's the level of data actually collected.
  //      enum class adamantine_ranges : int8_t {
  //          NA = -1,
  //          Cavern_1,
  //          Cavern_2,
  //          Cavern_3,
  //          Magma_Sea
  //      };

        enum magma_ranges : int8_t {
            MAGMA_NA = -1,
            MAGMA_CAVERN_3,
            MAGMA_CAVERN_2,
            MAGMA_CAVERN_1,
            MAGMA_VOLCANO
        };

        enum yes_no_ranges : int8_t {
            YES_NO_NA = -1,
            YES_NO_YES,
            YES_NO_NO
        };

        enum all_present_ranges : int8_t {
            ALL_PRESENT_ALL,
            ALL_PRESENT_PRESENT
        };
        enum present_absent_ranges : int8_t {
            PRESENT_ABSENT_NA = -1,
            PRESENT_ABSENT_PRESENT,
            PRESENT_ABSENT_ABSENT
        };

        enum soil_ranges : int8_t {
            SOIL_NA = -1,
            SOIL_NONE,
            SOIL_VERY_SHALLOW,
            SOIL_SHALLOW,
            SOIL_DEEP,
            SOIL_VERY_DEEP
        };

        enum syndrome_rain_ranges : int8_t {
            SYNDROME_RAIN_NA = -1,
            SYNDROME_RAIN_ANY,
            SYNDROME_RAIN_PERMANENT,
            SYNDROME_RAIN_TEMPORARY,
            SYNDROME_RAIN_NOT_PERMANENT,
            SYNDROME_RAIN_NONE
        };

        enum reanimation_ranges : int8_t {
            REANIMATION_NA = -1,
            REANIMATION_BOTH,
            REANIMATION_ANY,
            REANIMATION_THRALLING,
            REANIMATION_REANIMATION,
            REANIMATION_NOT_THRALLING,
            REANIMATION_NONE
        };

        enum freezing_ranges : int8_t {
            FREEZING_NA = -1,
            FREEZING_PERMANENT,
            FREEZING_AT_LEAST_PARTIAL,
            FREEZING_PARTIAL,
            FREEZING_AT_MOST_PARTIAL,
            FREEZING_NEVER
        };

        enum tree_ranges : int8_t {
            TREE_NA = -1,
            TREE_NONE,
            TREE_VERY_SCARCE,
            TREE_SCARCE,
            TREE_WOODLAND,
            TREE_HEAVILY_FORESTED
        };

        struct neighbor {
            neighbor(int16_t id, present_absent_ranges pr) : entity_raw(id), present(pr) {}
            neighbor() : entity_raw(0), present(PRESENT_ABSENT_NA) {}
            int16_t entity_raw;  //  entity_raw
            present_absent_ranges present;
        };

        struct finders {
            finders() {
                x_dim = 0;
                y_dim = 0;
                aquifer = AQUIFER_NA;
                min_river = RIVER_NA;
                max_river = RIVER_NA;
                min_waterfall = 0;
                flat = YES_NO_NA;
                clay = PRESENT_ABSENT_NA;
                sand = PRESENT_ABSENT_NA;
                flux = PRESENT_ABSENT_NA;
                coal = PRESENT_ABSENT_NA;
                soil_min = SOIL_NA;
                soil_max = SOIL_NA;
                soil_min_everywhere = ALL_PRESENT_ALL;
                freezing = FREEZING_NA;
                blood_rain = YES_NO_NA;
                syndrome_rain = SYNDROME_RAIN_NA;
                reanimation = REANIMATION_NA;
                spire_count_min = 0;
                spire_count_max = 0;
                magma_min = MAGMA_NA;
                magma_max = MAGMA_NA;
                biome_count_min = 0;
                biome_count_max = 0;
                region_type_1 = 0;
                region_type_2 = 0;
                region_type_3 = 0;
                biome_1 = 0;
                biome_2 = 0;
                biome_3 = 0;
                min_trees = TREE_NA;
                max_trees = TREE_NA;
                metal_1 = 0;
                metal_2 = 0;
                metal_3 = 0;
                economic_1 = 0;
                economic_2 = 0;
                economic_3 = 0;
                mineral_1 = 0;
                mineral_2 = 0;
                mineral_3 = 0;
                min_necro_neighbors = 0;
                max_necro_neighbors = 0;
                min_civ_neighbors = 0;
                max_civ_neighbors = 0;
                for (uint8 i = 0; i < EVIL_SAVAGERY_HIGH + 1; ++i)
                {
                    savagery[i] = EVIL_SAVAGERY_NA;
                    evilness[i] = EVIL_SAVAGERY_NA;
                }
            }
            uint16_t x_dim;
            uint16_t y_dim;
            evil_savagery_values savagery[EVIL_SAVAGERY_HIGH + 1];
            evil_savagery_values evilness[EVIL_SAVAGERY_HIGH + 1];
            aquifer_ranges aquifer;
            river_ranges min_river;
            river_ranges max_river;
            int8_t min_waterfall; // N/A(-1), Absent, 1-9
            yes_no_ranges flat;
            present_absent_ranges clay;
            present_absent_ranges sand;
            present_absent_ranges flux;
            present_absent_ranges coal;
            soil_ranges soil_min;
            all_present_ranges soil_min_everywhere;
            soil_ranges soil_max;
            freezing_ranges freezing;
            yes_no_ranges blood_rain;  //  Will probably blow up with the magic release arcs...
            syndrome_rain_ranges syndrome_rain;
            reanimation_ranges reanimation;
            int8_t spire_count_min; // N/A(-1), 0-9
            int8_t spire_count_max; // N/A(-1), 0-9
            magma_ranges magma_min;
            magma_ranges magma_max;
            int8_t biome_count_min; // N/A(-1), 1-9
            int8_t biome_count_max; // N/A(-1), 1-9
            int8_t region_type_1;   // N/A(-1), df::world_region_type
            int8_t region_type_2;   // N/A(-1), df::world_region_type
            int8_t region_type_3;   // N/A(-1), df::world_region_type
            int8_t biome_1;         // N/A(-1), df::biome_type
            int8_t biome_2;         // N/A(-1), df::biome_type
            int8_t biome_3;         // N/A(-1), df::biome_type
            tree_ranges min_trees;
            tree_ranges max_trees;
            int16_t metal_1;        // N/A(-1), 0-max_inorganic;
            int16_t metal_2;        // N/A(-1), 0-max_inorganic;
            int16_t metal_3;        // N/A(-1), 0-max_inorganic;
            int16_t economic_1;     // N/A(-1), 0-max_inorganic;
            int16_t economic_2;     // N/A(-1), 0-max_inorganic;
            int16_t economic_3;     // N/A(-1), 0-max_inorganic;
            int16_t mineral_1;      // N/A(-1), 0-max_inorganic;
            int16_t mineral_2;      // N/A(-1), 0-max_inorganic;
            int16_t mineral_3;      // N/A(-1), 0-max_inorganic;
            int8_t min_necro_neighbors; // N/A(-1), 0 - 9, where 9 = 9+
            int8_t max_necro_neighbors; // N/A(-1), 0 - 9, where 9 = 9+
            int8_t min_civ_neighbors; // N/A(-1), 0 - 9, where 9 = 9+
            int8_t max_civ_neighbors; // N/A(-1), 0 - 9, where 9 = 9+
            std::vector12<neighbor> neighbors;
        };

        struct match_iterators {
            match_iterators() {
                active = false;
                x = 0;
                y = 0;
                i = 0;
                k = 0;
                x_right = false;
                y_down = false;
                inhibit_x_turn = false;
                inhibit_y_turn = false;
                target_location_x = 0;
                target_location_y = 0;
                count = 0;
            }
            bool active;
            uint16_t x;  //  x position of focus when iteration started so we can return it.
            uint16_t y;  //  y
            uint16_t i;
            uint16_t k;
            bool x_right;
            bool y_down;
            bool inhibit_x_turn;
            bool inhibit_y_turn;
            uint16_t target_location_x;
            uint16_t target_location_y;
            uint16_t count;
            finders finder;
        };

        typedef void(*find_callbacks) (embark_assist::defs::finders finder);
    }
}