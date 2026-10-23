module GUI:Commands;

namespace
{
    std::string lower(std::string_view text)
    {
        std::string result(text);
        for (auto& ch : result)
            ch = char(std::tolower(static_cast<unsigned char>(ch)));
        return result;
    }

    std::string_view trim(std::string_view text)
    {
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
            text.remove_prefix(1);
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
            text.remove_suffix(1);
        return text;
    }

    struct named_key
    {
        const char* name;
        long        code;
    };

    // Windows virtual-key codes for the keys that are not a single letter or digit.
    constexpr named_key named_keys[] =
    {
        { "backspace", 0x08 }, { "tab", 0x09 }, { "enter", 0x0D }, { "esc", 0x1B }, { "space", 0x20 },
        { "pageup", 0x21 }, { "pagedown", 0x22 }, { "end", 0x23 }, { "home", 0x24 },
        { "left", 0x25 }, { "up", 0x26 }, { "right", 0x27 }, { "down", 0x28 },
        { "insert", 0x2D }, { "delete", 0x2E },
    };

    // Other spellings of the same keys.
    constexpr named_key aliases[] =
    {
        { "bksp", 0x08 }, { "return", 0x0D }, { "escape", 0x1B },
        { "pgup", 0x21 }, { "pgdn", 0x22 }, { "ins", 0x2D }, { "del", 0x2E },
    };
}

namespace GUI
{
    std::optional<key_chord> key_chord::parse(std::string_view text)
    {
        key_chord chord;
        bool has_key = false;

        size_t start = 0;
        while (start <= text.size())
        {
            const size_t end = text.find('+', start);
            const std::string part = lower(trim(text.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start)));
            start = end == std::string_view::npos ? text.size() + 1 : end + 1;

            if (part.empty())
                return std::nullopt;

            if (part == "ctrl" || part == "control") { chord.ctrl = true; continue; }
            if (part == "shift")                     { chord.shift = true; continue; }
            if (part == "alt")                       { chord.alt = true; continue; }

            // One key only.
            if (has_key)
                return std::nullopt;

            long code = 0;

            if (part.size() == 1 && std::isalnum(static_cast<unsigned char>(part[0])))
                code = std::toupper(static_cast<unsigned char>(part[0]));   // letters and digits: their ASCII codes
            else if (part.size() >= 2 && part[0] == 'f' && std::all_of(part.begin() + 1, part.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
            {
                const int number = std::atoi(part.c_str() + 1);
                if (number >= 1 && number <= 12)
                    code = 0x70 + number - 1;                                  // F1 .. F12
            }

            if (!code)
                for (auto& k : named_keys)
                    if (part == k.name)
                        code = k.code;

            if (!code)
                for (auto& k : aliases)
                    if (part == k.name)
                        code = k.code;

            if (!code)
                return std::nullopt;

            chord.key = code;
            has_key = true;
        }

        if (!has_key)
            return std::nullopt;

        return chord;
    }

    std::string key_chord::to_string() const
    {
        std::string result;
        if (ctrl)  result += "Ctrl+";
        if (shift) result += "Shift+";
        if (alt)   result += "Alt+";

        if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z'))
            result += char(key);
        else if (key >= 0x70 && key <= 0x7B)
            result += "F" + std::to_string(key - 0x70 + 1);
        else
        {
            std::string name = "?";
            for (auto& k : named_keys)
                if (k.code == key)
                {
                    name = k.name;
                    name[0] = char(std::toupper(static_cast<unsigned char>(name[0])));
                    break;
                }
            result += name;
        }

        return result;
    }

    std::string command::get_tooltip() const
    {
        std::string text = tooltip_text.empty() ? get_title() : tooltip_text;

        if (chord)
            text += " (" + chord->to_string() + ")";

        return text;
    }

    bool command::execute() const
    {
        if (!run_fn || !is_enabled())
            return false;

        run_fn();
        return true;
    }

    command& command_registry::add(std::string id)
    {
        for (auto& c : commands)
            if (c->get_id() == id)
                return *c;

        commands.push_back(std::make_shared<command>(std::move(id)));
        return *commands.back();
    }

    command::ptr command_registry::find(std::string_view id) const
    {
        for (auto& c : commands)
            if (c->get_id() == id)
                return c;

        return nullptr;
    }

    bool command_registry::run(std::string_view id) const
    {
        auto c = find(id);
        return c && c->execute();
    }

    command::ptr command_registry::find_shortcut(const key_chord& chord) const
    {
        for (auto& c : commands)
            if (c->get_shortcut() && *c->get_shortcut() == chord)
                return c;

        return nullptr;
    }

    void command_registry::install_shortcuts(user_interface& ui)
    {
        if (!hook_owner)
            hook_owner = std::make_shared<base>();

        ui.add_key_hook(hook_owner, [this, &ui](key_action action, long key, key_mods mods)
        {
            return dispatch(ui, action, key, mods);
        });
    }

    bool command_registry::dispatch(user_interface& ui, key_action action, long key, key_mods mods) const
    {
        // Key presses only (not releases or the -1 a mouse press is to a hook), and not while
        // the ribbon's Alt mode is reading the keyboard.
        if (action != key_action::DOWN || key <= 0 || ui.keys_captured)
            return false;

        // A modifier on its own is not a shortcut.
        if (key == 0x10 || key == 0x11 || key == 0x12)
            return false;

        const key_chord chord = { key, mods.ctrl, mods.shift, mods.alt };
        auto c = find_shortcut(chord);

        if (!c || !c->is_enabled())
            return false;

        // A plain key in a text field is typing.
        if (!chord.ctrl && !chord.alt && ui.has_focused_element())
            return false;

        return c->execute();
    }
}
