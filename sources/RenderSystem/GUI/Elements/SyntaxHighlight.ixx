export module GUI:SyntaxHighlight;
import Core;

// Syntax highlighters for Text::Editor::highlighter / edit_text::highlighter:
// fill one packed RGBA8 color (r in the low byte, 0 = default) per codepoint
// of the whole text. Colors suit the light edit-box skin.
export namespace GUI::Syntax
{
    void highlight_hlsl(std::u32string_view text, std::vector<uint32_t>& colors);
}
