export module GUI:SyntaxHighlight;
import Core;

// Syntax highlighters for Text::Editor::highlighter / edit_text::highlighter:
// color one line (without its line break) given the lexer state at its start,
// and return the state at its end. colors holds one packed RGBA8 (r in the
// low byte, 0 = default) per codepoint of the line. Colors suit the light
// edit-box skin.
export namespace GUI::Syntax
{
    // State: 0 = normal, 1 = inside a /* block comment */.
    uint32_t highlight_hlsl(std::u32string_view line, uint32_t state, std::span<uint32_t> colors);
}
