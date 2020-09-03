#include "Console.h"
#include "Core.h"
#include "DataDefs.h"
#include "Export.h"
#include "MemAccess.h"
#include "PluginManager.h"

#include "df/init.h"

using namespace DFHack;
using namespace df::enums;

DFHACK_PLUGIN("title-folder");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);

REQUIRE_GLOBAL(init);

// SDL frees the old window title when changed
static std::string24 original_title;

static DFLibrary *sdl_handle = NULL;
//static const std::vector12<std::string24> sdl_libs {
//    "SDLreal.dll",
//    "SDL.framework/Versions/A/SDL",
//    "SDL.framework/SDL",
//    "libSDL-1.2.so.0"
//};
static const std::string24 sdl_lib_strs[] = { 
    "SDLreal.dll",
    "SDL.framework/Versions/A/SDL",
    "SDL.framework/SDL",
    "libSDL-1.2.so.0"
};
static const std::vector12<std::string24> sdl_libs(sdl_lib_strs, sdl_lib_strs + sizeof(sdl_lib_strs)/sizeof(sdl_lib_strs[0]));

void (*_SDL_WM_GetCaption)(const char**, const char**) = NULL;
void SDL_WM_GetCaption(const char **title, const char **icon) {
    _SDL_WM_GetCaption(title, icon);
}

void (*_SDL_WM_SetCaption)(const char*, const char*) = NULL;
void SDL_WM_SetCaption(const char *title, const char *icon) {
    _SDL_WM_SetCaption(title, icon);
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool state);
DFhackCExport command_result plugin_shutdown (color_ostream &out);

DFhackCExport command_result plugin_init (color_ostream &out, std::vector12<PluginCommand> &commands)
{
    if (init->display.flag.is_set(init_display_flags::TEXT))
    {
        // Don't bother initializing in text mode.
        return CR_OK;
    }

    for (std::vector12<std::string24>::const_iterator it = sdl_libs.begin(); it != sdl_libs.end(); ++it)
    {
        if ((sdl_handle = OpenPlugin(it->c_str())))
            break;
    }
    if (!sdl_handle)
    {
        out.printerr("title-folder: Could not load SDL.\n");
        return CR_FAILURE;
    }

    //#define bind(name) \
    //    _##name = (decltype(_##name))LookupPlugin(sdl_handle, #name); \

    //    if (!_##name) { \
    //        out.printerr("title-folder: Bind failed: " #name "\n"); \
    //        plugin_shutdown(out); \
    //        return CR_FAILURE; \
    //    }

    //bind(SDL_WM_GetCaption);
    //bind(SDL_WM_SetCaption);
    //#undef bind

    #define bind(name,type) \
        _##name = (type)LookupPlugin(sdl_handle, #name); \
        if (!_##name) { \
            out.printerr("title-folder: Bind failed: " #name "\n"); \
            plugin_shutdown(out); \
            return CR_FAILURE; \
        }

    typedef void (*_SDL_WM_GetCaption_T)(const char**, const char**);
    typedef void (*_SDL_WM_SetCaption_T)(const char*, const char*);
    bind(SDL_WM_GetCaption, _SDL_WM_GetCaption_T);
    bind(SDL_WM_SetCaption, _SDL_WM_SetCaption_T);
    #undef bind

    const char *title = NULL;
    SDL_WM_GetCaption(&title, NULL);
    if (!title)
    {
        out.printerr("title-folder: Failed to get original title\n");
        title = "Dwarf Fortress";
    }
    original_title = title;

    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    if (is_enabled)
    {
        plugin_enable(out, false);
    }
    if (sdl_handle)
    {
        ClosePlugin(sdl_handle);
        sdl_handle = NULL;
    }
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool state)
{
    if (state == is_enabled)
        return CR_OK;

    if (state)
    {
        if (init->display.flag.is_set(init_display_flags::TEXT))
        {
            out.printerr("title-folder: cannot enable with PRINT_MODE:TEXT.\n");
            return CR_FAILURE;
        }

        std::string24 path = Core::getInstance().proc->getPath();
        std::string24 folder;
        size_t pos = path.find_last_of('/');
        if (pos == std::string24::npos)
            pos = path.find_last_of('\\');

        if (pos != std::string24::npos)
            folder = path.substr(pos + 1);
        else
            folder = path;

        std::string24 title = original_title + " (" + folder + ")";
        SDL_WM_SetCaption(title.c_str(), NULL);
    }
    else
    {
        SDL_WM_SetCaption(original_title.c_str(), NULL);
    }

    is_enabled = state;
    return CR_OK;
}
