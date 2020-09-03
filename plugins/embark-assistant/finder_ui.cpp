#include <stdio.h>
#include "Core.h"
#include "Console.h"

#include "modules/Gui.h"

#include "Types.h"

#include "MemAccess.h"
#include "df/biome_type.h"
#include "df/entity_raw.h"
#include "df/entity_raw_flags.h"
#include "df/inorganic_raw.h"
#include "df/material_flags.h"
#include "df/viewscreen_choose_start_sitest.h"
#include "df/world.h"
#include "df/world_data.h"
#include "df/world_raws.h"
#include "df/world_region_type.h"
#include "df/world_raws.h"

#include "embark-assistant.h"
#include "finder_ui.h"
#include "screen.h"

#include "../uicommon.h"

using df::global::world;

#define profile_file_name "./data/init/embark_assistant_profile.txt"

namespace embark_assist {
    namespace finder_ui {

        enum fields : int8_t {
            FIELDS_X_DIM,
            FIELDS_Y_DIM,
            FIELDS_SAVAGERY_CALM,
            FIELDS_SAVAGERY_MEDIUM,
            FIELDS_SAVAGERY_SAVAGE,
            FIELDS_GOOD,
            FIELDS_NEUTRAL,
            FIELDS_EVIL,
            FIELDS_AQUIFER,
            FIELDS_MIN_RIVER,
            FIELDS_MAX_RIVER,
            FIELDS_MIN_WATERFALL,
            FIELDS_FLAT,
            FIELDS_CLAY,
            FIELDS_SAND,
            FIELDS_FLUX,
            FIELDS_COAL,
            FIELDS_SOIL_MIN,
            FIELDS_SOIL_MIN_EVERYWHERE,
            FIELDS_SOIL_MAX,
            FIELDS_FREEZING,
            FIELDS_BLOOD_RAIN,
            FIELDS_SYNDROME_RAIN,
            FIELDS_REANIMATION,
            FIELDS_SPIRE_COUNT_MIN,
            FIELDS_SPIRE_COUNT_MAX,
            FIELDS_MAGMA_MIN,
            FIELDS_MAGMA_MAX,
            FIELDS_BIOME_COUNT_MIN,
            FIELDS_BIOME_COUNT_MAX,
            FIELDS_REGION_TYPE_1,
            FIELDS_REGION_TYPE_2,
            FIELDS_REGION_TYPE_3,
            FIELDS_BIOME_1,
            FIELDS_BIOME_2,
            FIELDS_BIOME_3,
            FIELDS_MIN_TREES,
            FIELDS_MAX_TREES,
            FIELDS_METAL_1,
            FIELDS_METAL_2,
            FIELDS_METAL_3,
            FIELDS_ECONOMIC_1,
            FIELDS_ECONOMIC_2,
            FIELDS_ECONOMIC_3,
            FIELDS_MINERAL_1,
            FIELDS_MINERAL_2,
            FIELDS_MINERAL_3,
            FIELDS_MIN_NECRO_NEIGHBORS,
            FIELDS_MAX_NECRO_NEIGHBORS,
            FIELDS_MIN_CIV_NEIGHBORS,
            FIELDS_MAX_CIV_NEIGHBORS,
            FIELDS_NEIGHBORS
        };
        fields first_fields = fields::FIELDS_X_DIM;
        fields last_fields = fields::FIELDS_NEIGHBORS;

        struct display_map_elements {
            display_map_elements(std::string24 _t, int16_t _k) : text(_t), key(_k) {}
            display_map_elements() : key(0) {}
            std::string24 text;
            int16_t key;
        };

        typedef std::vector12<display_map_elements> display_maps;
        typedef std::vector12<display_map_elements> name_lists;
        typedef std::list   <display_map_elements> sort_lists;

        struct ui_lists {
            ui_lists() {
                current_display_value = 0;
                current_value = 0;
                current_index = 0;
                focus = 0;
            }
            uint16_t current_display_value;  //  Not the value itself, but a reference to its index.
            int16_t current_value;           //  The integer representation of the value (if an enum).
            uint16_t current_index;          //  What's selected
            uint16_t focus;                  //  The value under the (possibly inactive) cursor
            display_maps list;               //  The strings to be displayed together with keys
                                             //  to allow location of the actual elements (e.g. a raws.inorganics mat_index
                                             //  or underlying enum value).
        };

        typedef std::vector12<ui_lists*> uis;

        const DFHack::Screen::Pen active_pen(' ', COLOR_YELLOW);
        const DFHack::Screen::Pen passive_pen(' ', COLOR_DARKGREY);
        const DFHack::Screen::Pen normal_pen(' ', COLOR_GREY);
        const DFHack::Screen::Pen white_pen(' ', COLOR_WHITE);
        const DFHack::Screen::Pen lr_pen(' ', COLOR_LIGHTRED);

        struct civ_entities {
            civ_entities(int16_t _id, std::string24 _d) : id(_id), description(_d) {}
            civ_entities() : id(0) {}
            int16_t id;
            std::string24 description;
        };

        //==========================================================================================================

        struct states {
            states() {
                find_callback = NULL;
                finder_list_focus = 0;
                finder_list_active = false;
                max_inorganic = 0;
            }
            embark_assist::defs::find_callbacks find_callback;
            uis ui;
            display_maps finder_list;  //  Don't need the element key, but it's easier to use the same type.
            uint16_t finder_list_focus;
            bool finder_list_active;
            uint16_t max_inorganic;
            std::vector12<civ_entities> civs;
        };

        static states *state = NULL;

        //==========================================================================================================

        bool compare(const display_map_elements& first, const display_map_elements& second) {
            uint16_t i = 0;
            while (i < first.text.length() && i < second.text.length()) {
                if (first.text[i] < second.text[i]) {
                    return true;
                }
                else if (first.text[i] > second.text[i]) {
                    return false;
                }
                ++i;
            }
            return first.text.length() < second.text.length();
        }

        //==========================================================================================================

        void append(sort_lists *sort_list, display_map_elements element) {
            sort_lists::iterator iterator;
            for (iterator = sort_list->begin(); iterator != sort_list->end(); ++iterator) {
                if (iterator->key == element.key) {
                    return;
                }
            }
            sort_list->push_back(element);
        }

        //==========================================================================================================

        void save_profile() {
            color_ostream_proxy out(Core::getInstance().getConsole());

            FILE* outfile = fopen(profile_file_name, "w");
            fields i = first_fields;
            size_t civ = 0;

            while (true) {
                for (size_t k = 0; k < state->ui[static_cast<int8_t>(i)]->list.size(); k++) {
                    if (state->ui[static_cast<int8_t>(i) + civ]->current_value == state->ui[static_cast<int8_t>(i) + civ]->list[k].key) {
                        fprintf(outfile, "[%s:%s]\n", state->finder_list[static_cast<int8_t>(i) + civ].text.c_str(), state->ui[static_cast<int8_t>(i) + civ]->list[k].text.c_str());
                        break;
                    }
                }
//                fprintf(outfile, "[%s:%i]\n", state->finder_list[static_cast<int8_t>(i)].text.c_str(), state->ui[static_cast<int8_t>(i)]->current_value);
                if (i == last_fields) {
                    civ++;

                    if (civ == state->civs.size()) {
                        break;  // done
                    }
                }
                else {
                    i = static_cast <fields>(static_cast<int8_t>(i) + 1);
                }
            }

            fclose(outfile);
        }

        //==========================================================================================================

        void load_profile() {
            color_ostream_proxy out(Core::getInstance().getConsole());
            FILE* infile = fopen(profile_file_name, "r");
            size_t civ = 0;

            if (!infile) {
                out.printerr("No profile file found at %s\n", profile_file_name);
                return;
            }

            fields i = first_fields;
            char line[80];
            int count = 80;
            bool found;

            while (true) {
                if (!fgets(line, count, infile) || line[0] != '[') {
                    out.printerr("Failed to find token start '[' at line %i\n", static_cast<int8_t>(i));
                    fclose(infile);
                    return;
                }

                for (int k = 1; k < count; k++) {
                    if (line[k] == ':') {
                        for (int l = 1; l < k; l++) {
                            if (state->finder_list[static_cast<int8_t>(i) + civ].text.c_str()[l - 1] != line[l]) {
                                out.printerr("Token mismatch of %s vs %s\n", line, state->finder_list[static_cast<int8_t>(i) + civ].text.c_str());
                                fclose(infile);
                                return;
                            }
                        }

                        found = false;

                        for (size_t l = 0; l < state->ui[static_cast<int8_t>(i) + civ]->list.size(); l++) {
                            for (int m = k + 1; m < count; m++) {
                                if (state->ui[static_cast<int8_t>(i) + civ]->list[l].text.c_str()[m - (k + 1)] != line[m]) {
                                    if (state->ui[static_cast<int8_t>(i) + civ]->list[l].text.c_str()[m - (k + 1)] == '\0' &&
                                        line[m] == ']') {
                                        found = true;
                                    }
                                    break;
                                }
                            }
                            if (found) {
                                break;
                            }
                        }

                        if (!found) {
                            out.printerr("Value extraction failure from %s\n", line);
                            fclose(infile);
                            return;
                        }

                        break;
                    }
                }

                if (!found) {
                    out.printerr("Value delimiter not found in %s\n", line);
                    fclose(infile);
                    return;
                }

                if (i == last_fields) {
                    civ++;

                    if (civ == state->civs.size()) {
                        break;  // done
                    }
                }
                else {
                    i = static_cast <fields>(static_cast<int8_t>(i) + 1);
                }
            }

            fclose(infile);

            //  Checking done. Now do the work.

            infile = fopen(profile_file_name, "r");
            i = first_fields;
            civ = 0;

            while (true) {
                if (!fgets(line, count, infile))
                {
                    break;
                }

                for (int k = 1; k < count; k++) {
                    if (line[k] == ':') {

                        found = false;

                        for (size_t l = 0; l < state->ui[static_cast<int8_t>(i) + civ]->list.size(); l++) {
                            for (int m = k + 1; m < count; m++) {
                                if (state->ui[static_cast<int8_t>(i) + civ]->list[l].text.c_str()[m - (k + 1)] != line[m]) {
                                    if (state->ui[static_cast<int8_t>(i) + civ]->list[l].text.c_str()[m - (k + 1)] == '\0' &&
                                        line[m] == ']') {
                                        state->ui[static_cast<int8_t>(i) + civ]->current_value = state->ui[static_cast<int8_t>(i) + civ]->list[l].key;
                                        state->ui[static_cast<int8_t>(i) + civ]->current_display_value = l;
                                        found = true;
                                    }

                                    break;
                                }
                            }
                            if (found) {
                                break;
                            }
                        }

                        break;
                    }
                }

                if (i == last_fields) {
                    civ++;

                    if (civ == state->civs.size()) {
                        break;  // done
                    }
                }
                else {
                    i = static_cast <fields>(static_cast<int8_t>(i) + 1);
                }
            }

            fclose(infile);
        }

        //==========================================================================================================

        void ui_setup(embark_assist::defs::find_callbacks find_callback, uint16_t max_inorganic) {
//            color_ostream_proxy out(Core::getInstance().getConsole());
            if (!embark_assist::finder_ui::state) {
                state = new(states);
                state->finder_list_focus = 0;
                state->finder_list_active = true;
                state->find_callback = find_callback;
                state->max_inorganic = max_inorganic;
            }

            fields i = first_fields;
            ui_lists *element;
            int16_t controllable_civs = 0;
            int16_t max_civs;

            for (int16_t i = 0; i < (int16_t)world->raws.entities.size(); i++) {
                if (world->raws.entities[i]->flags.is_set(df::entity_raw_flags::CIV_CONTROLLABLE)) controllable_civs++;
            }

            for (int16_t i = 0; i < (int16_t)world->raws.entities.size(); i++) {
                if (!world->raws.entities[i]->flags.is_set(df::entity_raw_flags::LAYER_LINKED) &&  // Animal people
                    !world->raws.entities[i]->flags.is_set(df::entity_raw_flags::GENERATED) &&     // Vault guardians
                    (controllable_civs > 1 ||                                                              //  Suppress the playable civ when only 1
                        !world->raws.entities[i]->flags.is_set(df::entity_raw_flags::CIV_CONTROLLABLE))) { //  Too much work to change dynamically for modded worlds.
                    if (world->raws.entities[i]->translation == "") {
                        state->civs.push_back(civ_entities(i, world->raws.entities[i]->code));                       //  Kobolds don't have a translation...
                    }
                    else {
                        state->civs.push_back(civ_entities(i, world->raws.entities[i]->translation));
                    }
                }
            }

            max_civs = state->civs.size();

            if (controllable_civs > 1) max_civs = max_civs - 1;

            while (true) {
                element = new ui_lists;
                element->current_display_value = 0;
                element->current_index = 0;
                element->focus = 0;

                switch (i) {
                case fields::FIELDS_X_DIM:
                    for (int16_t k = 1; k <= 16; k++) {
                        element->list.push_back(display_map_elements(int_to_string(k), k));
                    }

                    break;

                case fields::FIELDS_Y_DIM:
                    for (int16_t k = 1; k <= 16; k++) {
                        element->list.push_back(display_map_elements(int_to_string(k), k));
                    }

                    break;

                case fields::FIELDS_SAVAGERY_CALM:
                case fields::FIELDS_SAVAGERY_MEDIUM:
                case fields::FIELDS_SAVAGERY_SAVAGE:
                case fields::FIELDS_GOOD:
                case fields::FIELDS_NEUTRAL:
                case fields::FIELDS_EVIL:
                {
                    embark_assist::defs::evil_savagery_values k = embark_assist::defs::evil_savagery_values::EVIL_SAVAGERY_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::EVIL_SAVAGERY_NA:
                            element->list.push_back(display_map_elements("N/A", static_cast<int8_t>(k)));
                            break;

                        case embark_assist::defs::evil_savagery_values::EVIL_SAVAGERY_ALL:
                            element->list.push_back(display_map_elements("All", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::evil_savagery_values::EVIL_SAVAGERY_PRESENT:
                            element->list.push_back(display_map_elements("Present", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::evil_savagery_values::EVIL_SAVAGERY_ABSENT:
                            element->list.push_back(display_map_elements("Absent", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::evil_savagery_values::EVIL_SAVAGERY_ABSENT) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::evil_savagery_values>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_AQUIFER:
                {
                    embark_assist::defs::aquifer_ranges k = embark_assist::defs::aquifer_ranges::AQUIFER_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::aquifer_ranges::AQUIFER_NA:
                            element->list.push_back(display_map_elements("N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_NONE:
                            element->list.push_back(display_map_elements("None", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_AT_MOST_LIGHT:
                            element->list.push_back(display_map_elements( "<= Light", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_NONE_PLUS_LIGHT:
                            element->list.push_back(display_map_elements( "None + Light", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_NONE_PLUS_AT_LEAST_LIGHT:
                            element->list.push_back(display_map_elements( "None + >= Light", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_LIGHT:
                            element->list.push_back(display_map_elements( "Light", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_AT_LEAST_LIGHT:
                            element->list.push_back(display_map_elements( ">= Light", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_NONE_PLUS_HEAVY:
                            element->list.push_back(display_map_elements( "None + Heavy", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_AT_MOST_LIGHT_PLUS_HEAVY:
                            element->list.push_back(display_map_elements( "<= Light + Heavy", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_LIGHT_PLUS_HEAVY:
                            element->list.push_back(display_map_elements( "Light + Heavy", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_NONE_LIGHT_HEAVY:
                            element->list.push_back(display_map_elements( "None + Light + Heavy", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::aquifer_ranges::AQUIFER_HEAVY:
                            element->list.push_back(display_map_elements( "Heavy", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::aquifer_ranges::AQUIFER_HEAVY) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::aquifer_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_MIN_RIVER:
                case fields::FIELDS_MAX_RIVER:
                {
                    embark_assist::defs::river_ranges k = embark_assist::defs::river_ranges::RIVER_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::river_ranges::RIVER_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_NONE:
                            element->list.push_back(display_map_elements( "None", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_BROOK:
                            element->list.push_back(display_map_elements( "Brook", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_STREAM:
                            element->list.push_back(display_map_elements( "Stream", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_MINOR:
                            element->list.push_back(display_map_elements( "Minor", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_MEDIUM:
                            element->list.push_back(display_map_elements( "Medium", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::river_ranges::RIVER_MAJOR:
                            element->list.push_back(display_map_elements( "Major", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::river_ranges::RIVER_MAJOR) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::river_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_MIN_WATERFALL:
                    for (int16_t k = -1; k <= 9; k++) {
                        if (k == -1) {
                            element->list.push_back(display_map_elements( "N/A", k ));
                        }
                        else if (k == 0) {
                            element->list.push_back(display_map_elements( "Absent", k ));
                        }
                        else {
                            element->list.push_back(display_map_elements( int_to_string(k), k ));
                        }
                    }

                break;

                case fields::FIELDS_BLOOD_RAIN:
                case fields::FIELDS_FLAT:
                {
                    embark_assist::defs::yes_no_ranges k = embark_assist::defs::yes_no_ranges::YES_NO_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::yes_no_ranges::YES_NO_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::yes_no_ranges::YES_NO_YES:
                            element->list.push_back(display_map_elements( "Yes", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::yes_no_ranges::YES_NO_NO:
                            element->list.push_back(display_map_elements( "No", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::yes_no_ranges::YES_NO_NO) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::yes_no_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_SOIL_MIN_EVERYWHERE:
                {
                    embark_assist::defs::all_present_ranges k = embark_assist::defs::all_present_ranges::ALL_PRESENT_ALL;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::all_present_ranges::ALL_PRESENT_ALL:
                            element->list.push_back(display_map_elements( "All", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::all_present_ranges::ALL_PRESENT_PRESENT:
                            element->list.push_back(display_map_elements( "Present", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::all_present_ranges::ALL_PRESENT_PRESENT) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::all_present_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_CLAY:
                case fields::FIELDS_SAND:
                case fields::FIELDS_FLUX:
                case fields::FIELDS_COAL:
                {
                    embark_assist::defs::present_absent_ranges k = embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_PRESENT:
                            element->list.push_back(display_map_elements( "Present", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_ABSENT:
                            element->list.push_back(display_map_elements( "Absent", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_ABSENT) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::present_absent_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_SOIL_MIN:
                case fields::FIELDS_SOIL_MAX:
                {
                    embark_assist::defs::soil_ranges k = embark_assist::defs::soil_ranges::SOIL_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::soil_ranges::SOIL_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::soil_ranges::SOIL_NONE:
                            element->list.push_back(display_map_elements( "None", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::soil_ranges::SOIL_VERY_SHALLOW:
                            element->list.push_back(display_map_elements( "Very Shallow", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::soil_ranges::SOIL_SHALLOW:
                            element->list.push_back(display_map_elements( "Shallow", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::soil_ranges::SOIL_DEEP:
                            element->list.push_back(display_map_elements( "Deep", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::soil_ranges::SOIL_VERY_DEEP:
                            element->list.push_back(display_map_elements( "Very Deep", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::soil_ranges::SOIL_VERY_DEEP) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::soil_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_FREEZING:
                {
                    embark_assist::defs::freezing_ranges k = embark_assist::defs::freezing_ranges::FREEZING_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::freezing_ranges::FREEZING_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::freezing_ranges::FREEZING_PERMANENT:
                            element->list.push_back(display_map_elements( "Permanent", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::freezing_ranges::FREEZING_AT_LEAST_PARTIAL:
                            element->list.push_back(display_map_elements( "At Least Partially Frozen", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::freezing_ranges::FREEZING_PARTIAL:
                            element->list.push_back(display_map_elements( "Partially Frozen", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::freezing_ranges::FREEZING_AT_MOST_PARTIAL:
                            element->list.push_back(display_map_elements( "At Most Partially Frozen", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::freezing_ranges::FREEZING_NEVER:
                            element->list.push_back(display_map_elements( "Never Frozen", static_cast<int8_t>(k) ));
                            break;

                        }

                        if (k == embark_assist::defs::freezing_ranges::FREEZING_NEVER ||
                               (world->world_data->world_height != 17 &&  //  Can't handle temperature in non standard height worlds.
                                world->world_data->world_height != 33 &&
                                world->world_data->world_height != 65 &&
                                world->world_data->world_height != 129 &&
                                world->world_data->world_height != 257)) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::freezing_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_SYNDROME_RAIN:
                {
                    embark_assist::defs::syndrome_rain_ranges k = embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_ANY:
                            element->list.push_back(display_map_elements( "Any Syndrome", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_PERMANENT:
                            element->list.push_back(display_map_elements( "Permanent Syndrome", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_TEMPORARY:
                            element->list.push_back(display_map_elements( "Temporary Syndrome", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_NOT_PERMANENT:
                            element->list.push_back(display_map_elements( "Not Permanent Syndrome", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_NONE:
                            element->list.push_back(display_map_elements( "No Syndrome", static_cast<int8_t>(k) ));
                            break;

                        }

                        if (k == embark_assist::defs::syndrome_rain_ranges::SYNDROME_RAIN_NONE) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::syndrome_rain_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_REANIMATION:
                {
                    embark_assist::defs::reanimation_ranges k = embark_assist::defs::reanimation_ranges::REANIMATION_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::reanimation_ranges::REANIMATION_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_BOTH:
                            element->list.push_back(display_map_elements( "Reanimation & Thralling", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_ANY:
                            element->list.push_back(display_map_elements( "Reanimation or Thralling", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_THRALLING:
                            element->list.push_back(display_map_elements( "Thralling", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_REANIMATION:
                            element->list.push_back(display_map_elements( "Reanimation", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_NOT_THRALLING:
                            element->list.push_back(display_map_elements( "Not Thralling", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::reanimation_ranges::REANIMATION_NONE:
                            element->list.push_back(display_map_elements( "None", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::reanimation_ranges::REANIMATION_NONE) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::reanimation_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_SPIRE_COUNT_MIN:
                case fields::FIELDS_SPIRE_COUNT_MAX:
                    for (int16_t k = -1; k <= 9; k++) {
                        if (k == -1) {
                            element->list.push_back(display_map_elements( "N/A", k ));
                        }
                        else {
                            element->list.push_back(display_map_elements( int_to_string(k), k ));
                        }
                    }

                    break;

                case fields::FIELDS_MAGMA_MIN:
                case fields::FIELDS_MAGMA_MAX:
                {
                    embark_assist::defs::magma_ranges k = embark_assist::defs::magma_ranges::MAGMA_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::magma_ranges::MAGMA_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::magma_ranges::MAGMA_CAVERN_3:
                            element->list.push_back(display_map_elements( "Third Cavern", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::magma_ranges::MAGMA_CAVERN_2:
                            element->list.push_back(display_map_elements( "Second Cavern", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::magma_ranges::MAGMA_CAVERN_1:
                            element->list.push_back(display_map_elements( "First Cavern", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::magma_ranges::MAGMA_VOLCANO:
                            element->list.push_back(display_map_elements( "Volcano", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::magma_ranges::MAGMA_VOLCANO) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::magma_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_BIOME_COUNT_MIN:
                case fields::FIELDS_BIOME_COUNT_MAX:
                    for (int16_t k = 0; k < 10; k++) {
                        if (k == 0) {
                            element->list.push_back(display_map_elements( "N/A", -1 ));
                        }
                        else {
                            element->list.push_back(display_map_elements( int_to_string(k), k ));
                        }
                    }

                    break;

                case fields::FIELDS_REGION_TYPE_1:
                case fields::FIELDS_REGION_TYPE_2:
                case fields::FIELDS_REGION_TYPE_3:
                {
                    std::list<display_map_elements> name;
                    std::list<display_map_elements>::iterator iterator;

                    FOR_ENUM_ITEMS_SIMPLE(world_region_type, iter) {
                        name.push_back(display_map_elements( ENUM_KEY_STR_SIMPLE(world_region_type, iter),  static_cast<int16_t>(iter) ));
                    }
                    name.sort(compare);

                    element->list.push_back(display_map_elements( "N/A", -1 ));

                    for (iterator = name.begin(); iterator != name.end(); ++iterator) {
                        element->list.push_back(display_map_elements( iterator->text, iterator->key ));
                    }

                    name.clear();
                }
                break;

                case fields::FIELDS_BIOME_1:
                case fields::FIELDS_BIOME_2:
                case fields::FIELDS_BIOME_3:
                {
                    sort_lists name;
                    sort_lists::iterator iterator;

                    FOR_ENUM_ITEMS_SIMPLE(biome_type, iter) {
                        std::string24 s = ENUM_KEY_STR_SIMPLE(biome_type, iter);

                        if (s.substr(0, 4) != "POOL" &&
                            s.substr(0, 5) != "RIVER" &&
                            s.substr(0, 3) != "SUB") {
                            name.push_back(display_map_elements( s, static_cast<int16_t>(iter) ));
                        }
                    }
                    name.sort(compare);

                    element->list.push_back(display_map_elements( "N/A", -1 ));

                    for (iterator = name.begin(); iterator != name.end(); ++iterator) {
                        element->list.push_back(display_map_elements( iterator->text,  iterator->key ));
                    }

                    name.clear();
                }
                break;

                case fields::FIELDS_MIN_TREES:
                case fields::FIELDS_MAX_TREES:
                {
                    embark_assist::defs::tree_ranges k = embark_assist::defs::tree_ranges::TREE_NA;
                    while (true) {
                        switch (k) {
                        case embark_assist::defs::tree_ranges::TREE_NA:
                            element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::tree_ranges::TREE_NONE:
                            element->list.push_back(display_map_elements( "None", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::tree_ranges::TREE_VERY_SCARCE:
                            element->list.push_back(display_map_elements( "Very Scarce", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::tree_ranges::TREE_SCARCE:
                            element->list.push_back(display_map_elements( "Scarce", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::tree_ranges::TREE_WOODLAND:
                            element->list.push_back(display_map_elements( "Woodland", static_cast<int8_t>(k) ));
                            break;

                        case embark_assist::defs::tree_ranges::TREE_HEAVILY_FORESTED:
                            element->list.push_back(display_map_elements( "Heavily Forested", static_cast<int8_t>(k) ));
                            break;
                        }

                        if (k == embark_assist::defs::tree_ranges::TREE_HEAVILY_FORESTED) {
                            break;
                        }

                        k = static_cast <embark_assist::defs::tree_ranges>(static_cast<int8_t>(k) + 1);
                    }
                }

                break;

                case fields::FIELDS_METAL_1:
                case fields::FIELDS_METAL_2:
                case fields::FIELDS_METAL_3:
                {
                    sort_lists name;
                    sort_lists::iterator iterator;

                    for (uint16_t k = 0; k < embark_assist::finder_ui::state->max_inorganic; k++) {
                        for (uint16_t l = 0; l < world->raws.inorganics[k]->metal_ore.mat_index.size(); l++) {
                            append(&name,
                                display_map_elements(
                                world->raws.inorganics[world->raws.inorganics[k]->metal_ore.mat_index[l]]->id,
                                world->raws.inorganics[k]->metal_ore.mat_index[l] ));
                        }
                    }

                    name.sort(compare);

                    element->list.push_back(display_map_elements( "N/A", -1 ));

                    for (iterator = name.begin(); iterator != name.end(); ++iterator) {
                        element->list.push_back(display_map_elements( iterator->text,  iterator->key ));
                    }

                    name.clear();
                }
                break;

                case fields::FIELDS_ECONOMIC_1:
                case fields::FIELDS_ECONOMIC_2:
                case fields::FIELDS_ECONOMIC_3:
                {
                    sort_lists name;
                    sort_lists::iterator iterator;

                    for (int16_t k = 0; k < embark_assist::finder_ui::state->max_inorganic; k++) {
                        if (world->raws.inorganics[k]->economic_uses.size() != 0 &&
                            !world->raws.inorganics[k]->material.flags.is_set(df::material_flags::IS_METAL)) {
                            append(&name, display_map_elements( world->raws.inorganics[k]->id, k ));
                        }
                    }

                    name.sort(compare);

                    element->list.push_back(display_map_elements( "N/A", -1 ));

                    for (iterator = name.begin(); iterator != name.end(); ++iterator) {
                        element->list.push_back(display_map_elements( iterator->text,  iterator->key ));
                    }

                    name.clear();
                }
                break;

                case fields::FIELDS_MINERAL_1:
                case fields::FIELDS_MINERAL_2:
                case fields::FIELDS_MINERAL_3:
                {
                    sort_lists name;
                    sort_lists::iterator iterator;

                    for (int16_t k = 0; k < embark_assist::finder_ui::state->max_inorganic; k++) {
                        if (world->raws.inorganics[k]->environment.location.size() != 0 ||
                            world->raws.inorganics[k]->environment_spec.mat_index.size() != 0 ||
                            world->raws.inorganics[k]->flags.is_set(df::inorganic_flags::SEDIMENTARY) ||
                            world->raws.inorganics[k]->flags.is_set(df::inorganic_flags::IGNEOUS_EXTRUSIVE) ||
                            world->raws.inorganics[k]->flags.is_set(df::inorganic_flags::IGNEOUS_INTRUSIVE) ||
                            world->raws.inorganics[k]->flags.is_set(df::inorganic_flags::METAMORPHIC) ||
                            world->raws.inorganics[k]->flags.is_set(df::inorganic_flags::SOIL)) {
                            append(&name, display_map_elements( world->raws.inorganics[k]->id, k ));
                        }
                    }

                    name.sort(compare);

                    element->list.push_back(display_map_elements( "N/A", -1 ));

                    for (iterator = name.begin(); iterator != name.end(); ++iterator) {
                        element->list.push_back(display_map_elements( iterator->text,  iterator->key ));
                    }

                    name.clear();
                }
                break;

                case fields::FIELDS_MIN_NECRO_NEIGHBORS:
                case fields::FIELDS_MAX_NECRO_NEIGHBORS:
                    for (int16_t k = -1; k <= 16; k++) {
                        if (k == -1) {
                            element->list.push_back(display_map_elements( "N/A", k ));
                        }
                        else {
                            element->list.push_back(display_map_elements( int_to_string(k), k ));
                        }
                    }

                    break;

                case fields::FIELDS_MIN_CIV_NEIGHBORS:
                case fields::FIELDS_MAX_CIV_NEIGHBORS:
                    for (int16_t k = -1; k <= max_civs; k++) {
                        if (k == -1) {
                            element->list.push_back(display_map_elements( "N/A", k ));
                        }
                        else {
                            element->list.push_back(display_map_elements( int_to_string(k), k ));
                        }
                    }

                    break;

                case fields::FIELDS_NEIGHBORS:
                    for (size_t l = 0; l < state->civs.size(); l++) {
                        embark_assist::defs::present_absent_ranges k = embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_NA;
                        while (true) {
                            switch (k) {
                            case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_NA:
                                element->list.push_back(display_map_elements( "N/A", static_cast<int8_t>(k) ));
                                break;

                            case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_PRESENT:
                                element->list.push_back(display_map_elements( "Present", static_cast<int8_t>(k) ));
                                break;

                            case embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_ABSENT:
                                element->list.push_back(display_map_elements( "Absent", static_cast<int8_t>(k) ));
                                break;
                            }

                            if (k == embark_assist::defs::present_absent_ranges::PRESENT_ABSENT_ABSENT) {
                                break;
                            }

                            k = static_cast <embark_assist::defs::present_absent_ranges>(static_cast<int8_t>(k) + 1);
                        }

                        if (l < state->civs.size() - 1) {
                            element->current_value = element->list[0].key;
                            state->ui.push_back(element);
                            element = new ui_lists;
                            element->current_display_value = 0;
                            element->current_index = 0;
                            element->focus = 0;
                        }
                    }

                    break;

                }

                element->current_value = element->list[0].key;
                state->ui.push_back(element);

                switch (i) {
                case fields::FIELDS_X_DIM:
                    state->finder_list.push_back(display_map_elements( "X Dimension", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_Y_DIM:
                    state->finder_list.push_back(display_map_elements( "Y Dimension", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SAVAGERY_CALM:
                    state->finder_list.push_back(display_map_elements( "Low Savagery", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SAVAGERY_MEDIUM:
                    state->finder_list.push_back(display_map_elements( "Medium Savagery", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SAVAGERY_SAVAGE:
                    state->finder_list.push_back(display_map_elements( "High Savagery", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_GOOD:
                    state->finder_list.push_back(display_map_elements( "Good", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_NEUTRAL:
                    state->finder_list.push_back(display_map_elements( "Neutral", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_EVIL:
                    state->finder_list.push_back(display_map_elements( "Evil", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_AQUIFER:
                    state->finder_list.push_back(display_map_elements( "Aquifer", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MIN_RIVER:
                    state->finder_list.push_back(display_map_elements( "Min River", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAX_RIVER:
                    state->finder_list.push_back(display_map_elements( "Max River", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MIN_WATERFALL:
                    state->finder_list.push_back(display_map_elements( "Min Waterfall Drop", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_FLAT:
                    state->finder_list.push_back(display_map_elements( "Flat", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SOIL_MIN_EVERYWHERE:
                    state->finder_list.push_back(display_map_elements( "Min Soil Everywhere", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_FREEZING:
                    state->finder_list.push_back(display_map_elements( "Freezing", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BLOOD_RAIN:
                    state->finder_list.push_back(display_map_elements( "Blood Rain", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SYNDROME_RAIN:
                    state->finder_list.push_back(display_map_elements( "Syndrome Rain", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_REANIMATION:
                    state->finder_list.push_back(display_map_elements( "Reanimation", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_CLAY:
                    state->finder_list.push_back(display_map_elements( "Clay", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SAND:
                    state->finder_list.push_back(display_map_elements( "Sand", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_FLUX:
                    state->finder_list.push_back(display_map_elements( "Flux", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_COAL:
                    state->finder_list.push_back(display_map_elements( "Coal", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SOIL_MIN:
                    state->finder_list.push_back(display_map_elements( "Min Soil", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SOIL_MAX:
                    state->finder_list.push_back(display_map_elements( "Max Soil", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SPIRE_COUNT_MIN:
                    state->finder_list.push_back(display_map_elements( "Min Adamantine", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_SPIRE_COUNT_MAX:
                    state->finder_list.push_back(display_map_elements( "Max Adamantine", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAGMA_MIN:
                    state->finder_list.push_back(display_map_elements( "Min Magma", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAGMA_MAX:
                    state->finder_list.push_back(display_map_elements( "Max Magma", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BIOME_COUNT_MIN:
                    state->finder_list.push_back(display_map_elements( "Min Biome Count", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BIOME_COUNT_MAX:
                    state->finder_list.push_back(display_map_elements( "Max Biome Count", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_REGION_TYPE_1:
                    state->finder_list.push_back(display_map_elements( "Region Type 1", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_REGION_TYPE_2:
                    state->finder_list.push_back(display_map_elements( "Region Type 2", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_REGION_TYPE_3:
                    state->finder_list.push_back(display_map_elements( "Region Type 3", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BIOME_1:
                    state->finder_list.push_back(display_map_elements( "Biome 1", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BIOME_2:
                    state->finder_list.push_back(display_map_elements( "Biome 2", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_BIOME_3:
                    state->finder_list.push_back(display_map_elements( "Biome 3", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MIN_TREES:
                    state->finder_list.push_back(display_map_elements( "Min Trees", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAX_TREES:
                    state->finder_list.push_back(display_map_elements( "Max Trees", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_METAL_1:
                    state->finder_list.push_back(display_map_elements( "Metal 1", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_METAL_2:
                    state->finder_list.push_back(display_map_elements( "Metal 2", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_METAL_3:
                    state->finder_list.push_back(display_map_elements( "Metal 3", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_ECONOMIC_1:
                    state->finder_list.push_back(display_map_elements( "Economic 1", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_ECONOMIC_2:
                    state->finder_list.push_back(display_map_elements( "Economic 2", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_ECONOMIC_3:
                    state->finder_list.push_back(display_map_elements( "Economic 3", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MINERAL_1:
                    state->finder_list.push_back(display_map_elements( "Mineral 1", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MINERAL_2:
                    state->finder_list.push_back(display_map_elements( "Mineral 2", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MINERAL_3:
                    state->finder_list.push_back(display_map_elements( "Mineral 3", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MIN_NECRO_NEIGHBORS:
                    state->finder_list.push_back(display_map_elements( "Min Necro Tower", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAX_NECRO_NEIGHBORS:
                    state->finder_list.push_back(display_map_elements( "Max Necro Tower", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MIN_CIV_NEIGHBORS:
                    state->finder_list.push_back(display_map_elements( "Min Near Civs", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_MAX_CIV_NEIGHBORS:
                    state->finder_list.push_back(display_map_elements( "Max Near Civs", static_cast<int8_t>(i) ));
                    break;

                case fields::FIELDS_NEIGHBORS:
                    for (uint8_t k = 0; k < state->civs.size(); k++) {
                        state->finder_list.push_back(display_map_elements( state->civs[k].description, (int16_t)(static_cast<int8_t>(i) + k) ));
                    }
                    break;
                }

                if (i == last_fields) {
                    break;  // done
                }

                i = static_cast <fields>(static_cast<int8_t>(i) + 1);
            }

            //  Default embark area size to that of the current selection. The "size" calculation is actually one
            //  off to compensate for the list starting with 1 at index 0.
            //
            df::viewscreen_choose_start_sitest* screen = Gui::getViewscreenByType<df::viewscreen_choose_start_sitest>(0);
            int16_t x = screen->location.region_pos.x;
            int16_t y = screen->location.region_pos.y;
            state->ui[static_cast<int8_t>(fields::FIELDS_X_DIM)]->current_display_value =
                Gui::getViewscreenByType<df::viewscreen_choose_start_sitest>(0)->location.embark_pos_max.x -
                Gui::getViewscreenByType<df::viewscreen_choose_start_sitest>(0)->location.embark_pos_min.x;
            state->ui[static_cast<int8_t>(fields::FIELDS_X_DIM)]->current_index =
                state->ui[static_cast<int8_t>(fields::FIELDS_X_DIM)]->current_display_value;
            state->ui[static_cast<int8_t>(fields::FIELDS_X_DIM)]->current_value =
                state->ui[static_cast<int8_t>(fields::FIELDS_X_DIM)]->current_display_value + 1;

            state->ui[static_cast<int8_t>(fields::FIELDS_Y_DIM)]->current_display_value =
                Gui::getViewscreenByType<df::viewscreen_choose_start_sitest>(0)->location.embark_pos_max.y -
                Gui::getViewscreenByType<df::viewscreen_choose_start_sitest>(0)->location.embark_pos_min.y;
            state->ui[static_cast<int8_t>(fields::FIELDS_Y_DIM)]->current_index =
                state->ui[static_cast<int8_t>(fields::FIELDS_Y_DIM)]->current_display_value;
            state->ui[static_cast<int8_t>(fields::FIELDS_Y_DIM)]->current_value =
                state->ui[static_cast<int8_t>(fields::FIELDS_Y_DIM)]->current_display_value + 1;
        }

        //==========================================================================================================

        void find() {
//            color_ostream_proxy out(Core::getInstance().getConsole());
            embark_assist::defs::finders finder;
            fields i = first_fields;

            while (true) {
                switch (i) {
                case fields::FIELDS_X_DIM:
                    finder.x_dim = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_Y_DIM:
                    finder.y_dim = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_SAVAGERY_CALM:
                    finder.savagery[0] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SAVAGERY_MEDIUM:
                    finder.savagery[1] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;
                case fields::FIELDS_SAVAGERY_SAVAGE:
                    finder.savagery[2] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_GOOD:
                    finder.evilness[0] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_NEUTRAL:
                    finder.evilness[1] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_EVIL:
                    finder.evilness[2] =
                        static_cast<embark_assist::defs::evil_savagery_values>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_AQUIFER:
                    finder.aquifer =
                        static_cast<embark_assist::defs::aquifer_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_MIN_RIVER:
                    finder.min_river =
                        static_cast<embark_assist::defs::river_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_MAX_RIVER:
                    finder.max_river =
                        static_cast<embark_assist::defs::river_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_MIN_WATERFALL:
                    finder.min_waterfall = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_FLAT:
                    finder.flat =
                        static_cast<embark_assist::defs::yes_no_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SOIL_MIN_EVERYWHERE:
                    finder.soil_min_everywhere =
                        static_cast<embark_assist::defs::all_present_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_FREEZING:
                    finder.freezing =
                        static_cast<embark_assist::defs::freezing_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_BLOOD_RAIN:
                    finder.blood_rain =
                        static_cast<embark_assist::defs::yes_no_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SYNDROME_RAIN:
                    finder.syndrome_rain =
                        static_cast<embark_assist::defs::syndrome_rain_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_REANIMATION:
                    finder.reanimation =
                        static_cast<embark_assist::defs::reanimation_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_CLAY:
                    finder.clay =
                        static_cast<embark_assist::defs::present_absent_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SAND:
                    finder.sand =
                        static_cast<embark_assist::defs::present_absent_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_FLUX:
                    finder.flux =
                        static_cast<embark_assist::defs::present_absent_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_COAL:
                    finder.coal =
                        static_cast<embark_assist::defs::present_absent_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SOIL_MIN:
                    finder.soil_min =
                        static_cast<embark_assist::defs::soil_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SOIL_MAX:
                    finder.soil_max =
                        static_cast<embark_assist::defs::soil_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_SPIRE_COUNT_MIN:
                    finder.spire_count_min = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_SPIRE_COUNT_MAX:
                    finder.spire_count_max = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MAGMA_MIN:
                    finder.magma_min =
                        static_cast<embark_assist::defs::magma_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_MAGMA_MAX:
                    finder.magma_max =
                        static_cast<embark_assist::defs::magma_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_BIOME_COUNT_MIN:
                    finder.biome_count_min = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_BIOME_COUNT_MAX:
                    finder.biome_count_max = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_REGION_TYPE_1:
                    finder.region_type_1 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_REGION_TYPE_2:
                    finder.region_type_2 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_REGION_TYPE_3:
                    finder.region_type_3 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_BIOME_1:
                    finder.biome_1 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_BIOME_2:
                    finder.biome_2 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_BIOME_3:
                    finder.biome_3 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MIN_TREES:
                    finder.min_trees = static_cast<embark_assist::defs::tree_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_MAX_TREES:
                    finder.max_trees = static_cast<embark_assist::defs::tree_ranges>(state->ui[static_cast<uint8_t>(i)]->current_value);
                    break;

                case fields::FIELDS_METAL_1:
                    finder.metal_1 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_METAL_2:
                    finder.metal_2 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_METAL_3:
                    finder.metal_3 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_ECONOMIC_1:
                    finder.economic_1 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_ECONOMIC_2:
                    finder.economic_2 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_ECONOMIC_3:
                    finder.economic_3 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MINERAL_1:
                    finder.mineral_1 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MINERAL_2:
                    finder.mineral_2 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MINERAL_3:
                    finder.mineral_3 = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MIN_NECRO_NEIGHBORS:
                    finder.min_necro_neighbors = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MAX_NECRO_NEIGHBORS:
                    finder.max_necro_neighbors = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MIN_CIV_NEIGHBORS:
                    finder.min_civ_neighbors = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_MAX_CIV_NEIGHBORS:
                    finder.max_civ_neighbors = state->ui[static_cast<uint8_t>(i)]->current_value;
                    break;

                case fields::FIELDS_NEIGHBORS:
                    for (size_t k = 0; k < state->civs.size(); k++) {
                        finder.neighbors.push_back(embark_assist::defs::neighbor( state->civs[k].id, static_cast<embark_assist::defs::present_absent_ranges>(state->ui[static_cast<uint8_t>(i) + k]->current_value) ));
                    }
                    break;
                }

                if (i == last_fields) {
                    break;  // done
                }

                i = static_cast <fields>(static_cast<int8_t>(i) + 1);
            }

            state->find_callback(finder);
        }

        //==========================================================================================================

        class ViewscreenFindUi : public dfhack_viewscreen
        {
        public:
            ViewscreenFindUi();

            void feed(std::set8<df::interface_key> *input);

            void render();

            std::string24 getFocusString() { return "Finder UI"; }

        private:
        };

        //===============================================================================

        void ViewscreenFindUi::feed(std::set8<df::interface_key> *input) {
            if (input->count(df::interface_key::LEAVESCREEN))
            {
                input->clear();
                Screen::dismiss(this);
                return;

            } else if (input->count(df::interface_key::STANDARDSCROLL_LEFT) ||
                input->count(df::interface_key::STANDARDSCROLL_RIGHT)) {
                    state->finder_list_active = !state->finder_list_active;

            } else if (input->count(df::interface_key::STANDARDSCROLL_UP)) {
                if (state->finder_list_active) {
                    if (state->finder_list_focus > 0) {
                        state->finder_list_focus--;
                    }
                    else {
                        state->finder_list_focus = static_cast<uint16_t>(last_fields) + state->civs.size() - 1;
                    }
                }
                else {
                    if (state->ui[state->finder_list_focus]->current_index > 0) {
                        state->ui[state->finder_list_focus]->current_index--;
                    } else {
                        state->ui[state->finder_list_focus]->current_index = static_cast<uint16_t>(state->ui[state->finder_list_focus]->list.size()) - 1;
                    }
                }

            } else if (input->count(df::interface_key::STANDARDSCROLL_DOWN)) {
                if (state->finder_list_active) {
                    if (state->finder_list_focus < static_cast<uint16_t>(last_fields) + state->civs.size() - 1) {
                        state->finder_list_focus++;
                    } else {
                        state->finder_list_focus = 0;
                    }
                }
                else {
                    if (state->ui[state->finder_list_focus]->current_index < state->ui[state->finder_list_focus]->list.size() - 1) {
                        state->ui[state->finder_list_focus]->current_index++;
                    } else {
                        state->ui[state->finder_list_focus]->current_index = 0;
                    }
                }

            } else if (input->count(df::interface_key::SELECT)) {
                if (!state->finder_list_active) {
                    state->ui[state->finder_list_focus]->current_display_value = state->ui[state->finder_list_focus]->current_index;
                    state->ui[state->finder_list_focus]->current_value = state->ui[state->finder_list_focus]->list[state->ui[state->finder_list_focus]->current_index].key;
                    state->finder_list_active = true;
                }

            } else if (input->count(df::interface_key::CUSTOM_F)) {
                input->clear();
                Screen::dismiss(this);
                find();
                return;

            } else if (input->count(df::interface_key::CUSTOM_S)) {  //  Save
                save_profile();

            } else if (input->count(df::interface_key::CUSTOM_L)) {  //  Load
                load_profile();
            }
        }

        //===============================================================================

        void ViewscreenFindUi::render() {
//            color_ostream_proxy out(Core::getInstance().getConsole());
            df::coord2d screen_size = DFHack::Screen::getWindowSize();
            uint16_t top_row = 2;
            uint16_t list_column = 53;
            uint16_t offset = 0;

            Screen::clear();
            Screen::drawBorder("  Embark Assistant Site Finder  ");

            embark_assist::screen::paintString(lr_pen, 1, 1, DFHack::Screen::getKeyDisplay(df::interface_key::STANDARDSCROLL_LEFT).c_str());
            embark_assist::screen::paintString(white_pen, 2, 1, "/");
            embark_assist::screen::paintString(lr_pen, 3, 1, DFHack::Screen::getKeyDisplay(df::interface_key::STANDARDSCROLL_RIGHT).c_str());
            embark_assist::screen::paintString(white_pen, 4, 1, ": \x1b/\x1a");
            embark_assist::screen::paintString(lr_pen, 10, 1, DFHack::Screen::getKeyDisplay(df::interface_key::STANDARDSCROLL_UP).c_str());
            embark_assist::screen::paintString(white_pen, 11, 1, "/");
            embark_assist::screen::paintString(lr_pen, 12, 1, DFHack::Screen::getKeyDisplay(df::interface_key::STANDARDSCROLL_DOWN).c_str());
            embark_assist::screen::paintString(white_pen, 13, 1, ": \x18/\x19");
            embark_assist::screen::paintString(lr_pen, 19, 1, DFHack::Screen::getKeyDisplay(df::interface_key::SELECT).c_str());
            embark_assist::screen::paintString(white_pen, 24, 1, ": Select");
            embark_assist::screen::paintString(lr_pen, 33, 1, DFHack::Screen::getKeyDisplay(df::interface_key::CUSTOM_F).c_str());
            embark_assist::screen::paintString(white_pen, 34, 1, ": Find");
            embark_assist::screen::paintString(lr_pen, 41, 1, DFHack::Screen::getKeyDisplay(df::interface_key::LEAVESCREEN).c_str());
            embark_assist::screen::paintString(white_pen, 44, 1, ": Abort");
            embark_assist::screen::paintString(lr_pen, 52, 1, DFHack::Screen::getKeyDisplay(df::interface_key::CUSTOM_S).c_str());
            embark_assist::screen::paintString(white_pen, 53, 1, ": Save");
            embark_assist::screen::paintString(lr_pen, 60, 1, DFHack::Screen::getKeyDisplay(df::interface_key::CUSTOM_L).c_str());
            embark_assist::screen::paintString(white_pen, 61, 1, ": Load");

            //  Implement scrolling lists if they don't fit on the screen.
            if (int32_t(state->finder_list.size()) > screen_size.y - top_row - 1) {
                offset = (screen_size.y - top_row - 1) / 2;
                if (state->finder_list_focus < offset) {
                    offset = 0;
                }
                else {
                    offset = state->finder_list_focus - offset;
                }

                if (int32_t(state->finder_list.size() - offset) < screen_size.y - top_row - 1) {
                    offset = static_cast<uint16_t>(state->finder_list.size()) - (screen_size.y - top_row - 1);
                }
            }

            for (uint16_t i = offset; i < state->finder_list.size(); i++) {
                if (i == state->finder_list_focus) {
                    if (state->finder_list_active) {
                        embark_assist::screen::paintString(active_pen, 1, top_row + i - offset, state->finder_list[i].text);
                    }
                    else {
                        embark_assist::screen::paintString(passive_pen, 1, top_row + i - offset, state->finder_list[i].text);
                    }

                    embark_assist::screen::paintString(active_pen,
                        21,
                        top_row + i - offset,
                        state->ui[i]->list[state->ui[i]->current_display_value].text);
                }
                else {
                    embark_assist::screen::paintString(normal_pen, 1, top_row + i - offset, state->finder_list[i].text);

                    embark_assist::screen::paintString(white_pen,
                        21,
                        top_row + i - offset,
                        state->ui[i]->list[state->ui[i]->current_display_value].text);
                }
            }

            //  Implement scrolling lists if they don't fit on the screen.
            offset = 0;

            if (int32_t(state->ui[state->finder_list_focus]->list.size()) > screen_size.y - top_row - 1) {
                offset = (screen_size.y - top_row - 1) / 2;
                if (state->ui[state->finder_list_focus]->current_index < offset) {
                    offset = 0;
                }
                else {
                    offset = state->ui[state->finder_list_focus]->current_index - offset;
                }

                if (int32_t(state->ui[state->finder_list_focus]->list.size() - offset) < screen_size.y - top_row - 1) {
                    offset = static_cast<uint16_t>(state->ui[state->finder_list_focus]->list.size()) - (screen_size.y - top_row - 1);
                }
            }

            for (uint16_t i = offset; i < state->ui[state->finder_list_focus]->list.size(); i++) {
                if (i == state->ui[state->finder_list_focus]->current_index) {
                    if (!state->finder_list_active) {  // Negated expression to get the display lines in the same order as above.
                        embark_assist::screen::paintString(active_pen, list_column, top_row + i - offset, state->ui[state->finder_list_focus]->list[i].text);
                    }
                    else {
                        embark_assist::screen::paintString(passive_pen, list_column, top_row + i - offset, state->ui[state->finder_list_focus]->list[i].text);
                    }
                }
                else {
                    embark_assist::screen::paintString(normal_pen, list_column, top_row + i - offset, state->ui[state->finder_list_focus]->list[i].text);
                }
            }

            dfhack_viewscreen::render();
        }

        //===============================================================================

        ViewscreenFindUi::ViewscreenFindUi() {
        }
    }
}

//===============================================================================
//  Exported operations
//===============================================================================

void embark_assist::finder_ui::init(DFHack::Plugin *plugin_self, embark_assist::defs::find_callbacks find_callback, uint16_t max_inorganic, bool fileresult) {
    if (!embark_assist::finder_ui::state) {  //  First call. Have to do the setup
        embark_assist::finder_ui::ui_setup(find_callback, max_inorganic);
    }
    if (!fileresult)
    {
        //Screen::show(dts::make_unique<ViewscreenFindUi>(), plugin_self);
        Screen::show(new ViewscreenFindUi(), NULL, plugin_self);
    }
    else
    {
        load_profile();
        find();
    }
}

//===============================================================================

void embark_assist::finder_ui::activate() {
}

//===============================================================================

void embark_assist::finder_ui::shutdown() {
    if (embark_assist::finder_ui::state) {
        for (uint16_t i = 0; i < embark_assist::finder_ui::state->ui.size(); i++) {
            delete embark_assist::finder_ui::state->ui[i];
        }

        delete embark_assist::finder_ui::state;
        embark_assist::finder_ui::state = NULL;
    }
}
