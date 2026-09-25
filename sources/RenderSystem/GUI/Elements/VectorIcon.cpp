module GUI:VectorIcon;
import :Renderer;

import GUI;

namespace GUI
{
    namespace Elements
    {
        vector_icon::vector_icon(std::string icon) : icon(std::move(icon))
        {
            clickable = false;
        }

        void vector_icon::set_icon(std::string name)
        {
            std::lock_guard<std::mutex> guard(m);
            icon = std::move(name);
        }

        void vector_icon::on_pre_render(Context& c)
        {
            std::string name;
            {
                std::lock_guard<std::mutex> guard(m);
                name = icon;
            }

            // Rebuilt every frame even when unchanged: requesting the icon is
            // what keeps it alive through the atlas' per-frame eviction.
            Text::Engine::get().build_icon(name, vec2(get_render_bounds().size), layout);

            if (Text::Engine::get().has_pending_upload())
                user_ui->pre_draw_infos.emplace_back(this);
        }

        void vector_icon::pre_draw(HAL::CommandList::ptr list)
        {
            Text::Engine::get().upload(list);
        }

        void vector_icon::draw(Context& c)
        {
            base::draw(c);

            rect bounds = get_render_bounds();
            bounds.x += c.offset.x;
            bounds.y += c.offset.y;

            const sizer clip = intersect(c.ui_clipping, math::convert(bounds));
            if (clip.left >= clip.right || clip.top >= clip.bottom)
                return;

            // Issued directly, not through the batched NinePatch path, so
            // anything queued ahead of it must reach the list first.
            c.renderer->flush(c);
            Text::Engine::get().draw(c.command_list, layout, bounds.pos, color, clip, c.window_size);
        }
    }
}
