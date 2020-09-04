/*
https://github.com/peterix/dfhack
Copyright (c) 2009-2012 Petr Mrázek (peterix@gmail.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#pragma once

#include "Pragma.h"
#include "Export.h"
#include "Hooks.h"
#include <vector>
#include <stack>
#include <map>
#include <memory>
#include "common.h"
#include "Console.h"
#include "modules/Graphic.h"

//#include <atomic>
//#include <condition_variable>
//#include <mutex>
//#include <thread>
#include "tinythread.h"

#include "RemoteClient.h"

#define DFH_MOD_SHIFT 1
#define DFH_MOD_CTRL 2
#define DFH_MOD_ALT 4

struct WINDOW;

namespace df
{
    struct viewscreen;
}

namespace DFHack
{
    class Process;
    class Module;
    class Materials;
    class Notes;
    struct VersionInfo;
    class VersionInfoFactory;
    class PluginManager;
    class Core;
    class ServerMain;
    class CoreSuspender;

    namespace Lua { namespace Core {
        DFHACK_EXPORT void Reset(color_ostream &out, const char *where);
    } }
    namespace Windows
    {
        class df_window;
    }

    namespace Screen
    {
        struct Hide;
    }

    enum state_change_event
    {
        SC_UNKNOWN = -1,
        SC_WORLD_LOADED = 0,
        SC_WORLD_UNLOADED = 1,
        SC_MAP_LOADED = 2,
        SC_MAP_UNLOADED = 3,
        SC_VIEWSCREEN_CHANGED = 4,
        SC_CORE_INITIALIZED = 5,
        SC_BEGIN_UNLOAD = 6,
        SC_PAUSED = 7,
        SC_UNPAUSED = 8
    };

    class DFHACK_EXPORT StateChangeScript
    {
    public:
        state_change_event event;
        std::string24 path;
        bool save_specific;
        StateChangeScript(state_change_event event, std::string24 path, bool save_specific = false)
            :event(event), path(path), save_specific(save_specific)
        { }
        bool operator==(const StateChangeScript& other)
        {
            return event == other.event && path == other.path && save_specific == other.save_specific;
        }
    };

    // Core is a singleton. Why? Because it is closely tied to SDL calls. It tracks the global state of DF.
    // There should never be more than one instance
    // Better than tracking some weird variables all over the place.
    class DFHACK_EXPORT Core
    {
#ifdef _DARWIN
        friend int  ::DFH_SDL_NumJoysticks(void);
        friend void ::DFH_SDL_Quit(void);
        friend int  ::DFH_SDL_PollEvent(SDL::Event *);
        friend int  ::DFH_SDL_Init(uint32_t flags);
        friend int  ::DFH_wgetch(WINDOW * w);
#else
        friend int  ::SDL_NumJoysticks(void);
        friend void ::SDL_Quit(void);
        friend int  ::SDL_PollEvent(SDL::Event *);
        friend int  ::SDL_Init(uint32_t flags);
        friend int  ::wgetch(WINDOW * w);
#endif
        friend int  ::egg_init(void);
        friend int  ::egg_shutdown(void);
        friend int  ::egg_tick(void);
        friend int  ::egg_prerender(void);
        friend int  ::egg_sdl_event(SDL::Event* event);
        friend int  ::egg_curses_event(int orig_return);
    public:
        /// Get the single Core instance or make one.
        static Core& getInstance()
        {
            static Core instance;
            return instance;
        }
        /// check if the activity lock is owned by this thread
        bool isSuspended(void);
        /// Is everything OK?
        bool isValid(void) { return !errorstate; }

        /// get the materials module
        Materials * getMaterials();
        /// get the notes module
        Notes * getNotes();
        /// get the graphic module
        Graphic * getGraphic();
        /// sets the current hotkey command
        bool setHotkeyCmd( std::string24 cmd );
        /// removes the hotkey command and gives it to the caller thread
        std::string24 getHotkeyCmd( bool &keep_going );

        /// adds a named pointer (for later or between plugins)
        void RegisterData(void *data, std::string24 key);
        /// returns a named pointer.
        void *GetData(std::string24 key);

        command_result runCommand(color_ostream &out, const std::string24 &command, std::vector12 <std::string24> &parameters);
        command_result runCommand(color_ostream &out, const std::string24 &command);
        bool loadScriptFile(color_ostream &out, std::string24 fname, bool silent = false);

        bool addScriptPath(std::string24 path, bool search_before = false);
        bool removeScriptPath(std::string24 path);
        std::string24 findScript(std::string24 name);
        void getScriptPaths(std::vector12<std::string24> *dest);

        bool ClearKeyBindings(std::string24 keyspec);
        bool AddKeyBinding(std::string24 keyspec, std::string24 cmdline);
        std::vector12<std::string24> ListKeyBindings(std::string24 keyspec);
        int8_t getModstate() { return modstate; }

        bool AddAlias(const std::string24 &name, const std::vector12<std::string24> &command, bool replace = false);
        bool RemoveAlias(const std::string24 &name);
        bool IsAlias(const std::string24 &name);
        bool RunAlias(color_ostream &out, const std::string24 &name,
            const std::vector12<std::string24> &parameters, command_result &result);
        std::map<std::string24, std::vector12<std::string24> > ListAliases();
        std::string24 GetAliasCommand(const std::string24 &name, const std::string24 &default_ = "");

        std::string24 getHackPath();

        bool isWorldLoaded() { return (last_world_data_ptr != NULL); }
        bool isMapLoaded() { return (last_local_map_ptr != NULL && last_world_data_ptr != NULL); }

        static df::viewscreen *getTopViewscreen() { return getInstance().top_viewscreen; }

        DFHack::Console &getConsole() { return con; }

        DFHack::Process* proc;
        DFHack::VersionInfo* vinfo;
        DFHack::Windows::df_window * screen_window;

        static void print(const char *format, ...) Wformat(printf,1,2);
        static void printerr(const char *format, ...) Wformat(printf,1,2);

        PluginManager *getPluginManager() { return plug_mgr; }

        static void cheap_tokenise(std::string24 const& input, std::vector12<std::string24> &output);

    private:
        DFHack::Console con;

        Core();
        ~Core();

        struct Private
        {
            Private::Private() : last_autosave_request(false), was_load_save(false), iothread(NULL), hotkeythread(NULL) {}

            tthread::thread* iothread;
            tthread::thread* hotkeythread;

            bool last_autosave_request;
            bool was_load_save;
        };
        Private* d;

        bool Init();
        int Update (void);
        int TileUpdate (void);
        int Shutdown (void);
        int DFH_SDL_Event(SDL::Event* event);
        bool ncurses_wgetch(int in, int & out);

        void doUpdate(color_ostream &out, bool first_update);
        void onUpdate(color_ostream &out);
        void onStateChange(color_ostream &out, state_change_event event);
        void handleLoadAndUnloadScripts(color_ostream &out, state_change_event event);
        void doSaveData(color_ostream &out);
        void doLoadData(color_ostream &out);

        Core(Core const&);              // Don't Implement
        void operator=(Core const&);    // Don't implement

        // report error to user while failing
        void fatal (std::string24 output);

        // 1 = fatal failure
        bool errorstate;
        // regulate access to DF
        struct Cond;

        // FIXME: shouldn't be kept around like this
        //std::unique_ptr<DFHack::VersionInfoFactory> vif;
        // Module storage
        struct
        {
            Materials * pMaterials;
            Notes * pNotes;
            Graphic * pGraphic;
        } s_mods;
        std::vector12<Module*> allModules;
        DFHack::PluginManager * plug_mgr;

        std::vector12<std::string24> script_paths[2];
        tthread::mutex script_path_mutex;

        // hotkey-related stuff
        struct KeyBinding {
            int modifiers;
            std::vector12<std::string24> command;
            std::string24 cmdline;
            std::string24 focus;
        };
        int8_t modstate;

        std::map<int, std::vector12<KeyBinding> > key_bindings;
        std::map<int, bool> hotkey_states;
        std::string24 hotkey_cmd;
        enum hotkey_set_t {
            NO,
            SET,
            SHUTDOWN,
        };
        hotkey_set_t hotkey_set;
        tthread::mutex HotkeyMutex;
        tthread::condition_variable HotkeyCond;

        std::map<std::string24, std::vector12<std::string24> > aliases;
        tthread::recursive_mutex alias_mutex;

        bool SelectHotkey(int key, int modifiers);

        // for state change tracking
        void *last_world_data_ptr;
        // for state change tracking
        void *last_local_map_ptr;
        friend struct Screen::Hide;
        df::viewscreen *top_viewscreen;
        bool last_pause_state;
        // Very important!
        tthread::atomic<bool> started;
        // Additional state change scripts
        std::vector12<StateChangeScript> state_change_scripts;

        tthread::mutex misc_data_mutex;
        std::map<std::string24,void*> misc_data_map;

        /*!
         * \defgroup core_suspend CoreSuspender state handling serialization to
         * DF memory.
         * \sa DFHack::CoreSuspender
         * \{
         */
        tthread::recursive_mutex CoreSuspendMutex;
        tthread::condition_variable CoreWakeup;
        tthread::atomic<size_t> toolCount;
        tthread::atomic<tthread::thread::id> ownerThread;
        //! \}

        friend class CoreService;
        friend class ServerConnection;
        friend class CoreSuspender;
        friend class CoreSuspenderBase;
        friend struct CoreSuspendClaimMain;
        //friend struct CoreSuspendReleaseMain;
    };

    typedef tthread::defer_lock_guard<tthread::recursive_mutex> lock_type;
    class CoreSuspenderBase  : protected lock_type {
    protected:
        tthread::thread::id tid;

        explicit CoreSuspenderBase(Core* core) : lock_type(core->CoreSuspendMutex), tid() {}

    public:
        void lock()
        {
            lock_type::lock();
            tid = Core::getInstance().ownerThread.exchange(tthread::this_thread::get_id(), tthread::memory_order_acquire);
        }

        void unlock()
        {
            //Core& core = Core::getInstance();
            ///* Restore core owner to previous value */
            //core.ownerThread.store(tid, tthread::memory_order_release);
            //if (tid == tthread::thread::id())
            //    Lua::Core::Reset(core.getConsole(), "suspend");
            release();
            lock_type::unlock(); //unlock mutex explicitly only if called explicitly
        }

        void release()
        {
            Core& core = Core::getInstance();
            core.ownerThread.store(tid, tthread::memory_order_release);
            if (tid == tthread::thread::id())
                Lua::Core::Reset(core.getConsole(), "suspend");
        }

        bool owns_lock() const
        {
            return lock_type::is_locked();
        }

        int get_recursion() const
        {
            return lock_type::get_recursion();
        }

        ~CoreSuspenderBase()
        {
            //this is done in parent
            //if (owns_lock())
            //    unlock();
        }
        friend class MainThread;
    };

    /*!
     * CoreSuspender allows serialization to DF data with std::unique_lock like
     * interface. It includes handling for recursive CoreSuspender calls and
     * notification to main thread after all queue tools have been handled.
     *
     * State transitions are:
     * - Startup setups Core::SuspendMutex to unlocked states
     * - Core::Init locks Core::SuspendMutex until the thread exits or that thread
     *   calls Core::Shutdown or Core::~Core.
     * - Other thread request core suspend by atomic incrementation of Core::toolCount
     *   and then locking Core::CoreSuspendMutex. After locking CoreSuspendMutex
     *   success callers exchange their std::thread::id to Core::ownerThread.
     * - Core::Update() makes sure that queued tools are run when it calls
     *   Core::CoreWakup::wait. The wait keeps Core::CoreSuspendMutex unlocked
     *   and waits until Core::toolCount is reduced back to zero.
     * - CoreSuspender::~CoreSuspender() first stores the previous Core::ownerThread
     *   back. In case of recursive call Core::ownerThread equals tid. If tis is
     *   zero then we are releasing the recursive_mutex which means suspend
     *   context is over. It is time to reset lua.
     *   The last step is to decrement Core::toolCount and wakeup main thread if
     *   no more tools are queued trying to acquire the
     *   Core::CoreSuspenderMutex.
     */
    class CoreSuspender : public CoreSuspenderBase {
    public:
        CoreSuspender() : CoreSuspenderBase(&Core::getInstance()) { lock(); }

        void lock()
        {
            Core& core = Core::getInstance();
            core.toolCount.fetch_add(1, tthread::memory_order_relaxed);
            CoreSuspenderBase::lock();
        }

        //should not be used
        /*
        void unlock()
        {
            Core& core = Core::getInstance();
            CoreSuspenderBase::unlock();
        */
            /* Notify core to continue when all queued tools have completed
             * 0 = None wants to own the core
             * 1+ = There are tools waiting core access
             * fetch_add returns old value before subtraction
             */
        /*
            if (core.toolCount.fetch_add(-1, tthread::memory_order_relaxed) == 1)
                core.CoreWakeup.notify_one();
        }
        */

        ~CoreSuspender()
        {
            //this is done in parent
            //if (owns_lock())
            //    unlock();
            //if (int a = get_recursion())
            //    std::cerr << "\n~CoreSusp(): cur recursion '" << a << "'";
            Core& core = Core::getInstance();
            release();
            //it is safe to notify CoreWakeup waiter before mutex is unlocked
            //since waiting thread will not be able to relock temporarily
            //unlocked (and locked by this thread) mutex until we unlock it
            if (core.toolCount.fetch_add(-1, tthread::memory_order_relaxed) == 1)
                core.CoreWakeup.notify_one();
        }
    };

    /*!
     * Temporary release main thread ownership to allow alternative thread
     * implement DF logic thread loop
     */
    //struct DFHACK_EXPORT CoreSuspendReleaseMain {
    //    CoreSuspendReleaseMain();
    //    ~CoreSuspendReleaseMain();
    //};

    /*!
     * Temporary claim main thread ownership. This allows caller to call
     * Core::Update from a different thread than original DF logic thread if
     * logic thread has released main thread ownership with
     * CoreSuspendReleaseMain
     */
    struct DFHACK_EXPORT CoreSuspendClaimMain {
        CoreSuspendClaimMain();
        ~CoreSuspendClaimMain();
    };

    //using CoreSuspenderClaimer = CoreSuspender;
    //typedef CoreSuspender CoreSuspenderClaimer;
}
