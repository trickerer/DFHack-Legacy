#include <map>

#include <queue>

#include "Console.h"
#include "Core.h"
#include "DataDefs.h"
#include "Error.h"
#include "Export.h"
#include "LuaTools.h"
#include "LuaWrapper.h"
#include "PluginManager.h"
#include "VTableInterpose.h"
#include "modules/Gui.h"
#include "uicommon.h"

#include "df/building_tradedepotst.h"
#include "df/general_ref.h"
#include "df/general_ref_contained_in_itemst.h"
#include "df/viewscreen_dwarfmodest.h"
#include "df/viewscreen_justicest.h"
#include "df/viewscreen_layer_militaryst.h"
#include "df/viewscreen_locationsst.h"
#include "df/viewscreen_tradegoodsst.h"

using namespace DFHack;
using namespace df::enums;
using std::map;
using std::queue;



DFHACK_PLUGIN("confirm");
DFHACK_PLUGIN_IS_ENABLED(is_enabled);
REQUIRE_GLOBAL(gps);
REQUIRE_GLOBAL(ui);

typedef std::set8<df::interface_key> ikey_set;
command_result df_confirm (color_ostream &out, std::vector12<std::string24> & parameters);

struct conf_wrapper;
typedef map<std::string24, conf_wrapper*> ConfirmMap;
static ConfirmMap confirmations;
std::string24 active_id;
queue<std::string24> cmds;

template <typename VT, typename FT>
inline bool in_vector (std::vector12<VT> &vec, FT item)
{
    return std::find(vec.begin(), vec.end(), item) != vec.end();
}

std::string24 char_replace (std::string24 s, char a, char b)
{
    std::string24 res = s;
    size_t i = res.size();
    while (i--)
        if (res[i] == a)
            res[i] = b;
    return res;
}

bool set_conf_state (std::string24 name, bool state);

class confirmation_base {
public:
    enum cstate { INACTIVE, ACTIVE, SELECTED };
    virtual std::string24 get_id() = 0;
    virtual bool set_state(cstate) = 0;

    static bool set_state(std::string24 id, cstate state)
    {
        if (active && active->get_id() == id)
        {
            active->set_state(state);
            return true;
        }
        return false;
    }
protected:
    static confirmation_base *active;
};
confirmation_base *confirmation_base::active = NULL;

typedef std::set8<VMethodInterposeLinkBase*> HooksMap;
struct conf_wrapper {
private:
    bool enabled;
    HooksMap hooks;
public:
    conf_wrapper()
        :enabled(false)
    {}
    void add_hook(VMethodInterposeLinkBase *hook)
    {
        if (!hooks.count(hook))
            hooks.insert(hook);
    }
    bool apply (bool state)
    {
        if (state == enabled)
            return true;
        //for (auto hook : hooks)
        for (HooksMap::const_iterator cit = hooks.begin(); cit != hooks.end(); ++cit)
        {
            if (!(*cit)->apply(state))
                return false;
        }
        enabled = state;
        return true;
    }
    inline bool is_enabled() { return enabled; }
};

namespace trade {
    static bool goods_selected (const std::vector12<char> &selected)
    {
        //for (char c : selected)
        for (std::vector12<char>::const_iterator c = selected.begin(); c != selected.end(); ++c)
            if (*c)
                return true;
        return false;
    }
    inline bool trader_goods_selected (df::viewscreen_tradegoodsst *screen)
    {
        CHECK_NULL_POINTER(screen);
        return goods_selected(screen->trader_selected);
    }
    inline bool broker_goods_selected (df::viewscreen_tradegoodsst *screen)
    {
        CHECK_NULL_POINTER(screen);
        return goods_selected(screen->broker_selected);
    }

    static bool goods_all_selected(const std::vector12<char> &selected, const std::vector12<df::item*> &items)  \
    {
        for (size_t i = 0; i < selected.size(); ++i)
        {
            if (!selected[i])
            {
                // check to see if item is in a container
                // (if the container is not selected, it will be detected separately)
                bool in_container = false;
                //for (auto ref : items[i]->general_refs)
                for (std::vector12<df::general_ref*>::const_iterator ci = items[i]->general_refs.begin();
                    ci != items[i]->general_refs.end(); ++ci)
                {
                    if (virtual_cast<df::general_ref_contained_in_itemst>(*ci))
                    {
                        in_container = true;
                        break;
                    }
                }
                if (!in_container)
                    return false;
            }
        }
        return true;
    }
    inline bool trader_goods_all_selected(df::viewscreen_tradegoodsst *screen)
    {
        CHECK_NULL_POINTER(screen);
        return goods_all_selected(screen->trader_selected, screen->trader_items);
    }
    inline bool broker_goods_all_selected(df::viewscreen_tradegoodsst *screen)
    {
        CHECK_NULL_POINTER(screen);
        return goods_all_selected(screen->broker_selected, screen->broker_items);
    }
}

namespace conf_lua {
    static color_ostream_proxy *out;
    static lua_State *l_state;
    bool init (color_ostream &dfout)
    {
        out = new color_ostream_proxy(Core::getInstance().getConsole());
        l_state = Lua::Open(*out);
        return l_state;
    }
    void cleanup()
    {
        if (out)
        {
            delete out;
            out = NULL;
        }
        lua_close(l_state);
    }
    bool call (const char *func, int nargs = 0, int nres = 0)
    {
        if (!Lua::PushModulePublic(*out, l_state, "plugins.confirm", func))
            return false;
        if (nargs > 0)
            lua_insert(l_state, lua_gettop(l_state) - nargs);
        return Lua::SafeCall(*out, l_state, nargs, nres);
    }
    bool simple_call (const char *func)
    {
        Lua::StackUnwinder top(l_state);
        return call(func, 0, 0);
    }
    template <typename T>
    void push (T val)
    {
        Lua::Push(l_state, val);
    }
    namespace api {
        int get_ids (lua_State *L)
        {
            lua_newtable(L);
            //for (auto item : confirmations)
            for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
                Lua::TableInsert(L, ci->first, true);
            return 1;
        }
        int get_conf_data (lua_State *L)
        {
            lua_newtable(L);
            int i = 1;
            //for (auto item : confirmations)
            for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
            {
                Lua::Push(L, i++);
                lua_newtable(L);
                Lua::TableInsert(L, "id", ci->first);
                Lua::TableInsert(L, "enabled", ci->second->is_enabled());
                lua_settable(L, -3);
            }
            return 1;
        }
        int get_active_id (lua_State *L)
        {
            if (active_id.size())
                Lua::Push(L, active_id);
            else
                lua_pushnil(L);
            return 1;
        }
    }
}

#define CONF_LUA_FUNC(ns, name) {#name, df::wrap_function(ns::name, true)}
DFHACK_PLUGIN_LUA_FUNCTIONS {
    CONF_LUA_FUNC( , set_conf_state),
    CONF_LUA_FUNC(trade, broker_goods_selected),
    CONF_LUA_FUNC(trade, broker_goods_all_selected),
    CONF_LUA_FUNC(trade, trader_goods_selected),
    CONF_LUA_FUNC(trade, trader_goods_all_selected),
    DFHACK_LUA_END
};

#define CONF_LUA_CMD(name) {#name, conf_lua::api::name}
DFHACK_PLUGIN_LUA_COMMANDS {
    CONF_LUA_CMD(get_ids),
    CONF_LUA_CMD(get_conf_data),
    CONF_LUA_CMD(get_active_id),
    DFHACK_LUA_END
};

void show_options()
{
    cmds.push("gui/confirm-opts");
}

template <class T>
class confirmation : public confirmation_base {
public:
    typedef T screen_type;
    screen_type *screen;

    bool set_state (cstate s) override
    {
        if (confirmation_base::active && confirmation_base::active != this)
        {
            // Stop this confirmation from appearing over another one
            return false;
        }

        state = s;
        if (s == INACTIVE) {
            active_id = "";
            confirmation_base::active = NULL;
        }
        else {
            active_id = get_id();
            confirmation_base::active = this;
        }
        return true;
    }
    bool feed (ikey_set *input) {
        if (state == INACTIVE)
        {
            //for (df::interface_key key : *input)
            for (ikey_set::const_iterator ci = input->begin(); ci != input->end(); ++ci)
            {
                df::interface_key key = *ci;
                if (intercept_key(key))
                {
                    if (set_state(ACTIVE))
                    {
                        last_key = key;
                        return true;
                    }
                }
            }
            return false;
        }
        else if (state == ACTIVE)
        {
            if (input->count(df::interface_key::LEAVESCREEN))
                set_state(INACTIVE);
            else if (input->count(df::interface_key::SELECT))
                set_state(SELECTED);
            else if (input->count(df::interface_key::CUSTOM_S))
                show_options();
            return true;
        }
        return false;
    }
    bool key_conflict (df::interface_key key)
    {
        if (key == df::interface_key::SELECT || key == df::interface_key::LEAVESCREEN)
            return false;
        return state == ACTIVE;
    }
    void render() {
        static std::vector12<std::string24> lines;
        Screen::Pen corner_ul = Screen::Pen((char)201, COLOR_GREY, COLOR_BLACK);
        Screen::Pen corner_ur = Screen::Pen((char)187, COLOR_GREY, COLOR_BLACK);
        Screen::Pen corner_dl = Screen::Pen((char)200, COLOR_GREY, COLOR_BLACK);
        Screen::Pen corner_dr = Screen::Pen((char)188, COLOR_GREY, COLOR_BLACK);
        Screen::Pen border_ud = Screen::Pen((char)205, COLOR_GREY, COLOR_BLACK);
        Screen::Pen border_lr = Screen::Pen((char)186, COLOR_GREY, COLOR_BLACK);
        if (state == ACTIVE)
        {
            split_string(&lines, get_message(), "\n");
            size_t max_length = 40;
            //for (std::string24 line : lines)
            for (std::vector12<std::string24>::const_iterator ci = lines.begin(); ci != lines.end(); ++ci)
                max_length = std::max<size_t>(max_length, (*ci).size());
            int width = max_length + 4;
            int height = lines.size() + 4;
            int x1 = (gps->dimx / 2) - (width / 2);
            int x2 = x1 + width - 1;
            int y1 = (gps->dimy / 2) - (height / 2);
            int y2 = y1 + height - 1;
            for (int x = x1; x <= x2; x++)
            {
                Screen::paintTile(border_ud, x, y1);
                Screen::paintTile(border_ud, x, y2);
            }
            for (int y = y1; y <= y2; y++)
            {
                Screen::paintTile(border_lr, x1, y);
                Screen::paintTile(border_lr, x2, y);
            }
            Screen::paintTile(corner_ul, x1, y1);
            Screen::paintTile(corner_ur, x2, y1);
            Screen::paintTile(corner_dl, x1, y2);
            Screen::paintTile(corner_dr, x2, y2);
            std::string24 title = " " + get_title() + " ";
            Screen::paintString(Screen::Pen(' ', COLOR_DARKGREY, COLOR_BLACK),
                x2 - 6, y1, "DFHack");
            Screen::paintString(Screen::Pen(' ', COLOR_BLACK, COLOR_GREY),
                (gps->dimx / 2) - (title.size() / 2), y1, title);
            int x = x1 + 2;
            int y = y2;
            OutputString(COLOR_LIGHTRED, x, y, Screen::getKeyDisplay(df::interface_key::LEAVESCREEN));
            OutputString(COLOR_WHITE, x, y, ": Cancel");
            x = (gps->dimx - (Screen::getKeyDisplay(df::interface_key::CUSTOM_S) + ": Settings").size()) / 2 + 1;
            OutputString(COLOR_LIGHTRED, x, y, Screen::getKeyDisplay(df::interface_key::CUSTOM_S));
            OutputString(COLOR_WHITE, x, y, ": Settings");
            x = x2 - 2 - 3 - Screen::getKeyDisplay(df::interface_key::SELECT).size();
            OutputString(COLOR_LIGHTRED, x, y, Screen::getKeyDisplay(df::interface_key::SELECT));
            OutputString(COLOR_WHITE, x, y, ": Ok");
            Screen::fillRect(Screen::Pen(' ', COLOR_BLACK, COLOR_BLACK), x1 + 1, y1 + 1, x2 - 1, y2 - 1);
            for (size_t i = 0; i < lines.size(); i++)
            {
                Screen::paintString(Screen::Pen(' ', get_color(), COLOR_BLACK), x1 + 2, y1 + 2 + i, lines[i]);
            }
        }
        else if (state == SELECTED)
        {
            ikey_set tmp;
            tmp.insert(last_key);
            screen->feed(&tmp);
            set_state(INACTIVE);
        }
    }
    virtual std::string24 get_id() override = 0;
    #define CONF_LUA_START using namespace conf_lua; Lua::StackUnwinder unwind(l_state); push(screen); push(get_id());
    bool intercept_key (df::interface_key key)
    {
        CONF_LUA_START;
        push(key);
        if (call("intercept_key", 3, 1))
            return lua_toboolean(l_state, -1);
        else
            return false;
    };
    std::string24 get_title()
    {
        CONF_LUA_START;
        if (call("get_title", 2, 1) && lua_isstring(l_state, -1))
            return lua_tostring(l_state, -1);
        else
            return "Confirm";
    }
    std::string24 get_message()
    {
        CONF_LUA_START;
        if (call("get_message", 2, 1) && lua_isstring(l_state, -1))
            return lua_tostring(l_state, -1);
        else
            return "<Message generation failed>";
    };
    UIColor get_color()
    {
        CONF_LUA_START;
        if (call("get_color", 2, 1) && lua_isnumber(l_state, -1))
            return lua_tointeger(l_state, -1) % 16;
        else
            return COLOR_YELLOW;
    }
    #undef CONF_LUA_START
protected:
    cstate state;
    df::interface_key last_key;
};

template<typename T>
int conf_register(confirmation<T> *c, const std::vector12<VMethodInterposeLinkBase*> &hooks)
{
    conf_wrapper *w = new conf_wrapper();
    confirmations[c->get_id()] = w;
    //for (auto hook : hooks)
    for (std::vector12<VMethodInterposeLinkBase*>::const_iterator ci = hooks.begin(); ci != hooks.end(); ++ci)
        w->add_hook(*ci);
    return 0;
}

#define IMPLEMENT_CONFIRMATION_HOOKS(cls, prio) \
static cls cls##_instance; \
struct cls##_hooks : cls::screen_type { \
    typedef cls::screen_type interpose_base; \
    DEFINE_VMETHOD_INTERPOSE(void, feed, (ikey_set *input)) \
    { \
        cls##_instance.screen = this; \
        if (!cls##_instance.feed(input)) \
            INTERPOSE_NEXT(feed)(input); \
    } \
    DEFINE_VMETHOD_INTERPOSE(void, render, ()) \
    { \
        cls##_instance.screen = this; \
        INTERPOSE_NEXT(render)(); \
        cls##_instance.render(); \
    } \
    DEFINE_VMETHOD_INTERPOSE(bool, key_conflict, (df::interface_key key)) \
    { \
        return cls##_instance.key_conflict(key) || INTERPOSE_NEXT(key_conflict)(key); \
    } \
}; \
IMPLEMENT_VMETHOD_INTERPOSE_PRIO(cls##_hooks, feed, prio); \
IMPLEMENT_VMETHOD_INTERPOSE_PRIO(cls##_hooks, render, prio); \
IMPLEMENT_VMETHOD_INTERPOSE_PRIO(cls##_hooks, key_conflict, prio); \
static VMethodInterposeLinkBase* cls##_link_bases[] = { \
    &INTERPOSE_HOOK(cls##_hooks, feed), \
    &INTERPOSE_HOOK(cls##_hooks, render), \
    &INTERPOSE_HOOK(cls##_hooks, key_conflict), \
}; \
static int conf_register_##cls = conf_register(&cls##_instance, \
    std::vector12<VMethodInterposeLinkBase*>(cls##_link_bases, \
    cls##_link_bases + sizeof(cls##_link_bases)/sizeof(cls##_link_bases[0])));
//static int conf_register_##cls = conf_register(&cls##_instance, {\
//    &INTERPOSE_HOOK(cls##_hooks, feed), \
//    &INTERPOSE_HOOK(cls##_hooks, render), \
//    &INTERPOSE_HOOK(cls##_hooks, key_conflict), \
//});

#define DEFINE_CONFIRMATION(cls, screen) \
    class confirmation_##cls : public confirmation<df::screen> { \
        virtual std::string24 get_id() { static std::string24 id = char_replace(#cls, '_', '-'); return id; } \
    }; \
    IMPLEMENT_CONFIRMATION_HOOKS(confirmation_##cls, 0);

/* This section defines stubs for all confirmation dialogs, with methods
    implemented in plugins/lua/confirm.lua.
    IDs (used in the "confirm enable/disable" command, by Lua, and in the docs)
    are obtained by replacing '_' with '-' in the first argument to DEFINE_CONFIRMATION
*/
DEFINE_CONFIRMATION(trade,              viewscreen_tradegoodsst);
DEFINE_CONFIRMATION(trade_cancel,       viewscreen_tradegoodsst);
DEFINE_CONFIRMATION(trade_seize,        viewscreen_tradegoodsst);
DEFINE_CONFIRMATION(trade_offer,        viewscreen_tradegoodsst);
DEFINE_CONFIRMATION(trade_select_all,   viewscreen_tradegoodsst);
DEFINE_CONFIRMATION(haul_delete,        viewscreen_dwarfmodest);
DEFINE_CONFIRMATION(depot_remove,       viewscreen_dwarfmodest);
DEFINE_CONFIRMATION(squad_disband,      viewscreen_layer_militaryst);
DEFINE_CONFIRMATION(uniform_delete,     viewscreen_layer_militaryst);
DEFINE_CONFIRMATION(note_delete,        viewscreen_dwarfmodest);
DEFINE_CONFIRMATION(route_delete,       viewscreen_dwarfmodest);
DEFINE_CONFIRMATION(location_retire,    viewscreen_locationsst);
DEFINE_CONFIRMATION(convict,            viewscreen_justicest);

DFhackCExport command_result plugin_init (color_ostream &out, std::vector12<PluginCommand> &commands)
{
    if (!conf_lua::init(out))
        return CR_FAILURE;
    commands.push_back(PluginCommand(
        "confirm",
        "Confirmation dialogs",
        df_confirm,
        false, //allow non-interactive use

        "  confirmation enable|disable option|all ...\n"
        "  confirmation help|status\n"
    ));
    return CR_OK;
}

DFhackCExport command_result plugin_enable (color_ostream &out, bool enable)
{
    if (is_enabled != enable)
    {
        //for (auto c : confirmations)
        for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
        {
            if (!ci->second->apply(enable))
                return CR_FAILURE;
        }
        is_enabled = enable;
    }
    if (is_enabled)
    {
        conf_lua::simple_call("check");
    }
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    if (plugin_enable(out, false) != CR_OK)
        return CR_FAILURE;
    conf_lua::cleanup();

    //for (auto item : confirmations)
    for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
    {
        delete ci->second;
    }
    confirmations.clear();

    return CR_OK;
}

DFhackCExport command_result plugin_onupdate (color_ostream &out)
{
    while (!cmds.empty())
    {
        Core::getInstance().runCommand(out, cmds.front());
        cmds.pop();
    }
    return CR_OK;
}

bool set_conf_state (std::string24 name, bool state)
{
    bool found = false;
    //for (auto it : confirmations)
    for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
    {
        if (ci->first == name)
        {
            found = true;
            ci->second->apply(state);
        }
    }

    if (state == false)
    {
        // dismiss the confirmation too
        confirmation_base::set_state(name, confirmation_base::INACTIVE);
    }

    return found;
}

void enable_conf (color_ostream &out, std::string24 name, bool state)
{
    if (!set_conf_state(name, state))
        out.printerr("Unrecognized option: %s\n", name.c_str());
}

command_result df_confirm (color_ostream &out, std::vector12<std::string24> & parameters)
{
    CoreSuspender suspend;
    bool state = true;
    if (parameters.empty() || in_vector(parameters, "help") || in_vector(parameters, "status"))
    {
        out << "Available options: \n";
        //for (auto it : confirmations)
        for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
            out.print("  %20s: %s\n", ci->first.c_str(), ci->second->is_enabled() ? "enabled" : "disabled");
        return CR_OK;
    }
    //for (std::string24 param : parameters)
    for (std::vector12<std::string24>::const_iterator cit = parameters.begin(); cit != parameters.end(); ++cit)
    {
        std::string24 param = *cit;
        if (param == "enable")
            state = true;
        else if (param == "disable")
            state = false;
        else if (param == "all")
        {
            //for (auto it : confirmations)
            for (ConfirmMap::const_iterator ci = confirmations.begin(); ci != confirmations.end(); ++ci)
                ci->second->apply(state);
        }
        else
            enable_conf(out, param, state);
    }
    return CR_OK;
}
