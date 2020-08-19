#include "df/viewscreen_dwarfmodest.h"

using df::global::ui;

struct hotkey_clear_hook : df::viewscreen_dwarfmodest {
    typedef df::viewscreen_dwarfmodest interpose_base;

    DEFINE_VMETHOD_INTERPOSE(void, render, ())
    {
        INTERPOSE_NEXT(render)();
        if (ui->main.mode == df::ui_sidebar_mode::Hotkeys)
        {
            Gui::DwarfmodeDims dims = Gui::getDwarfmodeViewDims();
            int x = dims.menu_x1 + 1, y = 19;
            OutputHotkeyString(x, y, "Clear", df::interface_key::CUSTOM_C, false, 0, COLOR_WHITE, COLOR_LIGHTRED);
        }
    }

    DEFINE_VMETHOD_INTERPOSE(void, feed, (set<df::interface_key> *input))
    {
        if (ui->main.mode == df::ui_sidebar_mode::Hotkeys &&
            input->count(df::interface_key::CUSTOM_C) &&
            !ui->main.in_rename_hotkey)
        {
            df::ui_hotkey &hotkey = ui->main.hotkeys[ui->main.selected_hotkey];
            hotkey.name = "";
            hotkey.cmd = df::ui_hotkey::T_cmd::None;
            hotkey.x = 0;
            hotkey.y = 0;
            hotkey.z = 0;
            hotkey.unit_id = 0;
            hotkey.item_id = 0;
        }
        else
        {
            INTERPOSE_NEXT(feed)(input);
        }
    }
};

IMPLEMENT_VMETHOD_INTERPOSE(hotkey_clear_hook, feed);
IMPLEMENT_VMETHOD_INTERPOSE(hotkey_clear_hook, render);
