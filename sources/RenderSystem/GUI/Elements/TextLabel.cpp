module GUI:TextLabel;
import :Renderer;

import GUI;

namespace GUI
{
    namespace Elements
    {
        text_label::text_label()
            : text([this](const std::string&) { update_size(); })
            , font_size([this](const float&) { update_size(); })
        {
            clickable = false;
            font_size = 16;
        }

        // Sized here rather than from on_pre_render: the tree walk skips
        // on_pre_render for an element with empty bounds, so a label that only
        // sized itself there would stay 0x0 and never be drawn.
        void text_label::update_size()
        {
            size = Text::Engine::get().measure(text.get(), { font_size.get() });
        }

        void text_label::on_pre_render(Context& c)
        {
            PROFILE(L"text_label_build");

            // Rebuilt every frame even when unchanged: requesting the glyphs is
            // what keeps them alive through the atlas' per-frame eviction.
            Text::Engine::get().build(text.get(), { font_size.get() * c.scale }, layout);

            if (Text::Engine::get().has_pending_upload())
                user_ui->pre_draw_infos.emplace_back(this);
        }

        void text_label::pre_draw(HAL::CommandList::ptr list)
        {
            Text::Engine::get().upload(list);
        }

        void text_label::draw(Context& c)
        {
            base::draw(c);

            // Text is issued directly, not through the batched NinePatch path, so
            // anything queued ahead of it must reach the list first.
            c.renderer->flush(c);

            const rect bounds = get_render_bounds();
            Text::Engine::get().draw(c.command_list, layout, bounds.pos + c.offset, color, c.ui_clipping, c.window_size);
        }
    }
}
