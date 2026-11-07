import Clutter from 'gi://Clutter';
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import Meta from 'gi://Meta';
import Shell from 'gi://Shell';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

const SERVICE = 'io.github.QTCopyHistory';
const PATH = '/io/github/QTCopyHistory';

export default class ClipboardHistory extends Extension {
    enable() {
        // global.backend is Meta.Backend; the seat belongs to Clutter.Backend.
        this._keyboard = global.stage.get_context().get_backend().get_default_seat()
            .create_virtual_device(Clutter.InputDeviceType.KEYBOARD_DEVICE);
        this._settings = this.getSettings();
        this._notificationSettings = new Gio.Settings({schema_id: 'org.gnome.shell.keybindings'});
        // GNOME uses Super+V for its notification tray. Restore it on disable.
        this._oldBinding = this._notificationSettings.get_strv('toggle-message-tray');
        this._notificationSettings.set_strv('toggle-message-tray',
            this._oldBinding.filter(key => !['<Super>v', '<Super>V'].includes(key)));
        Main.wm.addKeybinding('show-history', this._settings, Meta.KeyBindingFlags.NONE,
            Shell.ActionMode.NORMAL, () => this._show());
        this._mapId = global.window_manager.connect('map', (_wm, actor) => {
            const window = actor.meta_window;
            if (!this._pending || window.get_title() !== 'Clipboard History') return;
            this._pending = false;
            const monitor = Main.layoutManager.monitors.find(m =>
                this._x >= m.x && this._x < m.x + m.width &&
                this._y >= m.y && this._y < m.y + m.height) ?? Main.layoutManager.primaryMonitor;
            const area = Main.layoutManager.getWorkAreaForMonitor(monitor.index);
            const frame = window.get_frame_rect();
            const width = Math.min(frame.width, area.width - 8);
            const height = Math.min(frame.height, area.height - 8);
            const x = Math.max(area.x + 4, Math.min(this._x, area.x + area.width - width - 4));
            let y = this._y + 12;
            if (y + height > area.y + area.height - 4) y = this._y - height - 12;
            y = Math.max(area.y + 4, Math.min(y, area.y + area.height - height - 4));
            window.move_resize_frame(true, x, y, width, height);
            Main.activateWindow(window);
        });
        this._signalId = Gio.DBus.session.signal_subscribe(SERVICE, SERVICE, 'pasteRequested',
            PATH, null, Gio.DBusSignalFlags.NONE, () => this._paste());
    }

    _show() {
        if (this._timer) return;
        const focused = global.display.focus_window;
        if (focused?.get_title() === 'Clipboard History') return;
        this._target = focused;
        [this._x, this._y] = global.get_pointer();
        this._pending = true;
        Gio.DBus.session.call(SERVICE, PATH, SERVICE, 'showAtCursor', null, null,
            Gio.DBusCallFlags.NONE, 2000, null, (connection, result) => {
                try { connection.call_finish(result); }
                catch (error) {
                    this._pending = false;
                    Main.notify('Qt Copy History', '请先启动 QTCopyHistory 程序。');
                    console.error(error);
                }
            });
    }

    _paste() {
        const target = this._target;
        this._target = null;
        if (!target || this._timer) return;
        if (!global.get_window_actors().some(actor => actor.meta_window === target)) return;
        Main.activateWindow(target);
        let attempts = 0;
        this._timer = GLib.timeout_add(GLib.PRIORITY_DEFAULT, 30, () => {
            const [, , modifiers] = global.get_pointer();
            const held = modifiers & (Clutter.ModifierType.SHIFT_MASK |
                Clutter.ModifierType.CONTROL_MASK | Clutter.ModifierType.MOD1_MASK |
                Clutter.ModifierType.SUPER_MASK);
            if (global.display.focus_window !== target || held) {
                if (++attempts < 30) return GLib.SOURCE_CONTINUE;
            } else {
                const time = GLib.get_monotonic_time();
                this._keyboard.notify_keyval(time, Clutter.KEY_Control_L, Clutter.KeyState.PRESSED);
                this._keyboard.notify_keyval(time, Clutter.KEY_v, Clutter.KeyState.PRESSED);
                this._keyboard.notify_keyval(time, Clutter.KEY_v, Clutter.KeyState.RELEASED);
                this._keyboard.notify_keyval(time, Clutter.KEY_Control_L, Clutter.KeyState.RELEASED);
            }
            this._timer = 0;
            return GLib.SOURCE_REMOVE;
        });
    }

    disable() {
        Main.wm.removeKeybinding('show-history');
        if (this._mapId) global.window_manager.disconnect(this._mapId);
        if (this._signalId) Gio.DBus.session.signal_unsubscribe(this._signalId);
        this._mapId = 0;
        this._signalId = 0;
        if (this._timer) GLib.source_remove(this._timer);
        this._timer = 0;
        if (this._notificationSettings && this._oldBinding)
            this._notificationSettings.set_strv('toggle-message-tray', this._oldBinding);
        this._notificationSettings = null;
        this._settings = null;
        this._keyboard = null;
        this._target = null;
        this._pending = false;
    }
}
