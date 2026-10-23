export module GUI:Commands;
import :Base;

export namespace GUI
{
    // A keyboard shortcut: a key (a Windows virtual-key code; letters and digits are their
    // ASCII codes) with modifiers. Written and read as "Ctrl+Shift+S", "F5", "Ctrl+Del".
    struct key_chord
    {
        long key   = 0;
        bool ctrl  = false;
        bool shift = false;
        bool alt   = false;

        bool operator==(const key_chord&) const = default;

        // Modifiers and a key joined by '+', case ignored; nothing for text it can't read.
        static std::optional<key_chord> parse(std::string_view text);
        std::string to_string() const;
    };

    // One action of the application, defined once. Menus, ribbon buttons, shortcuts and
    // anything else that offers the action take their label, icon, tooltip, enabled and
    // checked state and behaviour from here, so they can't drift apart.
    //
    //   commands.add("file.save").title("Save").icon("...").shortcut("Ctrl+S")
    //           .enabled([&] { return has_scene; }).run([&] { save(); });
    class command
    {
        public:
            using ptr = std::shared_ptr<command>;

            explicit command(std::string id) : id(std::move(id)) {}

            // Setters, chainable.
            command& title(std::string value)       { title_text = std::move(value); return *this; }
            command& short_title(std::string value) { short_text = std::move(value); return *this; }
            command& icon(std::string value)        { icon_text = std::move(value); return *this; }
            // For u8"..." literals (an emoji is written u8"\U0001F4BE").
            command& icon(const char8_t* value)     { icon_text = reinterpret_cast<const char*>(value); return *this; }
            command& tooltip(std::string value)     { tooltip_text = std::move(value); return *this; }
            // An unreadable shortcut text leaves the command without one.
            command& shortcut(std::string_view text) { chord = key_chord::parse(text); return *this; }
            command& shortcut(key_chord value)       { chord = value; return *this; }
            // Asked when the state is shown or the command is about to run; without it: always.
            command& enabled(std::function<bool()> value) { enabled_fn = std::move(value); return *this; }
            // Gives the command a toggle look (a lit button, a check mark).
            command& checked(std::function<bool()> value) { checked_fn = std::move(value); return *this; }
            command& run(std::function<void()> value)     { run_fn = std::move(value); return *this; }

            const std::string& get_id() const { return id; }
            // Falls back to the id when no title was given.
            std::string get_title() const { return title_text.empty() ? id : title_text; }
            // For a small button or a shrunk group; falls back to the title.
            std::string get_short_title() const { return short_text.empty() ? get_title() : short_text; }
            const std::string& get_icon() const { return icon_text; }
            // The tooltip text, or the title; with the shortcut in brackets after it.
            std::string get_tooltip() const;
            const std::optional<key_chord>& get_shortcut() const { return chord; }

            bool is_enabled() const { return !enabled_fn || enabled_fn(); }
            bool is_toggle() const  { return bool(checked_fn); }
            bool is_checked() const { return checked_fn && checked_fn(); }

            // Runs it if it is enabled and has something to run; says whether it did.
            bool execute() const;

        private:
            std::string id;
            std::string title_text, short_text, icon_text, tooltip_text;
            std::optional<key_chord> chord;
            std::function<bool()> enabled_fn, checked_fn;
            std::function<void()> run_fn;
    };

    // All the commands of an application, by id, in the order they were added.
    class command_registry
    {
        public:
            // The command with this id; one that already exists is returned as it is, so
            // two places can add the same id without clobbering each other.
            command& add(std::string id);

            command::ptr find(std::string_view id) const;
            // False when there is no such command or it is disabled.
            bool run(std::string_view id) const;
            const std::vector<command::ptr>& all() const { return commands; }

            // The command whose shortcut is this chord, if any.
            command::ptr find_shortcut(const key_chord& chord) const;

            // Makes the registry's shortcuts work in this UI: a key hook that runs the command
            // for a chord. Keys the ribbon's Alt mode has captured are left alone, and plain
            // keys (no Ctrl or Alt) never fire while a text field has the focus, so typing
            // isn't stolen. Call it after the ribbon has been added, so the ribbon's hook
            // sees the keys first.
            void install_shortcuts(user_interface& ui);

        private:
            std::vector<command::ptr> commands;
            // The key hook holds its owner weakly: with the registry gone the hook is skipped.
            std::shared_ptr<base> hook_owner;

            bool dispatch(user_interface& ui, key_action action, long key, key_mods mods) const;
    };
}
