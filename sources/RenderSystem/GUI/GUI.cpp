#include "pch.h"
#include "GUI.h"
#include "Renderer/Renderer.h"
#include "Elements/FlowGraph/ParameterWindow.h"
#include "Elements/Window.h"


namespace GUI
{
    class dark : public base
    {
        public:
            virtual void draw(DX12::context&) override;
    };

    void base::on_add(base* base_parent)
    {
        tree_base::on_add(base_parent);

        if (!base_parent) return;

        user_ui = base_parent->user_ui;
        renderer = parent->renderer;
        map_to_pixels = map_to_pixels && base_parent->map_to_pixels;
        need_update_layout = true;

        if (user_ui && thinkable)
            user_ui->thinking.insert(this);

        if (user_ui && drag_listener)
            user_ui->drag.dragdrop.insert(this);

        for (auto c : childs)
            c->on_add(this);
    }

    void base::on_remove()
    {
        tree_base::on_remove();

        //  if (user_ui)
        //      user_ui->remove_base(this);
        if (user_ui)
            user_ui->thinking.erase(this);

        if (user_ui)
            user_ui->drag.dragdrop.erase(this);

        user_ui = nullptr;

        for (auto c : childs)
            c->on_remove();
    }


    // ReSharper disable once CppPossiblyUninitializedMember
    base::base() :
        render_bounds(std::bind(&base::on_bounds_changed, this, std::placeholders::_1)),
        size(std::bind(&base::on_local_size_changed, this, std::placeholders::_1)),
        pos(std::bind(&base::on_pos_changed, this, std::placeholders::_1)),
        docking(std::bind(&base::on_dock_changed, this, std::placeholders::_1)),
        visible(std::bind(&base::on_visible_changed, this, std::placeholders::_1)),
        margin(std::bind(&base::on_margin_changed, this, std::placeholders::_1)),
        padding(std::bind(&base::on_padding_changed, this, std::placeholders::_1)),
		scaled_size(std::bind(&base::on_size_changed, this, std::placeholders::_1))
	{
        minimal_size = { 0, 0 };
        clip_child = true;
        clickable = true;
        margin = { 0, 0, 0, 0 };
        padding = { 0, 0, 0, 0 };
        size = { 0, 0 };
		scaled_size = { 0, 0 };

        pos = { 0, 0 };
        docking = dock::NONE;
        visible = true;
        size.filter_func = [this](const vec2 & r)->vec2
        {
            vec2 res = vec2::max(vec2((r)), minimal_size);

            if (clip_to_parent && parent)
            {
                //		res = vec2::min(res, vec2(parent->render_bounds->size) - vec2(parent->padding->left + parent->padding->right, parent->padding->right + parent->padding->bottom));
            }

            /*
                        if (width_size == size_type::FIXED)
                            res.x = size->x;

                        if (height_size == size_type::FIXED)
                            res.y = size->y;*/
            //	Log::get() << "size " << res << " " << r << Log::endl;
            return res;
        };


        scaled_size.filter_func = [this](const vec2& r)->vec2
        {
            vec2 res = vec2::max(vec2((r)), minimal_size);

            if (clip_to_parent && parent)
            {
                //		res = vec2::min(res, vec2(parent->render_bounds->size) - vec2(parent->padding->left + parent->padding->right, parent->padding->right + parent->padding->bottom));
            }

            /*
                        if (width_size == size_type::FIXED)
                            res.x = size->x;

                        if (height_size == size_type::FIXED)
                            res.y = size->y;*/
                            //	Log::get() << "size " << res << " " << r << Log::endl;
            return res;
        };


        pos.filter_func = [this](const vec2 & r)->vec2
        {
            vec2 res = (r);

            if (clip_to_parent && parent)
            {
                //	res = vec2::max(vec2(0, 0), res);
                //	res = vec2::min(res, vec2(parent->render_bounds->size) - size.get() - vec2(parent->padding->left + parent->padding->right, parent->padding->right + parent->padding->bottom));
            }

            /*
            if (width_size == size_type::FIXED)
            res.x = size->x;

            if (height_size == size_type::FIXED)
            res.y = size->y;*/
            //	Log::get() << "size " << res << " " << r << Log::endl;
            return res;
        };
        render_bounds.filter_func = [this](const rect & r)->rect
        {
            rect res = r;

            if (width_size == size_type::FIXED || width_size == size_type::MATCH_CHILDREN || width_size == size_type::MATCH_PARENT || width_size == size_type::MATCH_PARENT_CHILDREN)
                res.size.x = scaled_size->x * result_scale;

            if (height_size == size_type::FIXED || height_size == size_type::MATCH_CHILDREN || height_size == size_type::MATCH_PARENT || width_size == size_type::MATCH_PARENT_CHILDREN)
                res.size.y = scaled_size->y * result_scale;

            if (x_type == pos_x_type::LEFT)
                res.pos.x = r.pos.x;
            else if (x_type == pos_x_type::RIGHT)
                res.pos.x = r.pos.x + r.size.x - res.size.x;

            if (x_type == pos_x_type::CENTER)
                res.pos.x = r.pos.x + ((r.size.x - res.size.x) / 2);

            if (y_type == pos_y_type::TOP)
                res.pos.y = r.pos.y;
            else if (y_type == pos_y_type::BOTTOM)
                res.pos.y = r.pos.y + r.size.y - res.size.y;

            if (y_type == pos_y_type::CENTER)
                res.pos.y = r.pos.y + ((r.size.y - res.size.y) / 2);

            if (clip_to_parent && parent)
            {
                //float2 size = float2(res.size);
                //float2 pos = float2(res.pos);
                float2 size = float2::min(float2(res.size), float2(parent->get_render_bounds().size) - float2(parent->padding->left + parent->padding->right, parent->padding->top + parent->padding->bottom) * result_scale);
                float2 pos = float2::max(float2(parent->get_render_bounds().pos) + float2(parent->padding->left, parent->padding->top) * result_scale, float2(res.pos));
                pos = float2::min(pos, float2(parent->get_render_bounds().pos) + float2(parent->get_render_bounds().size) - size - float2(parent->padding->right, parent->padding->bottom) * result_scale);
                res = rect(pos, size);
            }

            if (map_to_pixels || result_scale == 1)
                res = ivec4(res);

            //res *= result_scale;
            return res;
        };
    }

    vec2 base::to_global(vec2 p)
    {
        if (parent)
        {
            // If our parent has an innerpanel and we're a child of it
            // add its offset onto us.
            //
            /*	if (m_Parent->m_InnerPanel && m_Parent->m_InnerPanel->IsChild(this))
            {
            x -= m_Parent->m_InnerPanel->X();
            y -= m_Parent->m_InnerPanel->Y();
            }*/
            return parent->to_global(p + local_bounds.pos);
        }

        return p;
    }

    vec2 base::to_local(vec2 p)
    {
        if (parent)
        {
            // If our parent has an innerpanel and we're a child of it
            // add its offset onto us.
            //
            /*	if (m_Parent->m_InnerPanel && m_Parent->m_InnerPanel->IsChild(this))
            	{
            		x -= m_Parent->m_InnerPanel->X();
            		y -= m_Parent->m_InnerPanel->Y();
            	}*/
            return parent->to_local(p - local_bounds.pos);
        }

        return p;
    }

    void base::to_front()
    {
        if (parent)
        {
            auto tmp_ptr = get_ptr();
            parent->childs.erase(tmp_ptr);
            parent->childs.insert(tmp_ptr);
        }

        set_update_layout();
    }

    void base::remove_child(ptr obj)
    {
        auto&& g = lock();
        tree_base::remove_child(obj);
        set_update_layout();
    }

    void base::update_cursor() const
    {
        ::SetCursor(LoadCursor(nullptr, cursors[static_cast<int>(cursor)]));
    }

    void base::add_child(ptr obj)
    {
        auto&& g = lock();
        tree_base::add_child(obj);
        set_update_layout();
        rect r2 = get_render_bounds() + rect(padding->left, padding->top, -padding->right - padding->left, -padding->bottom - padding->top);
        obj->on_parent_size_changed(r2);
    }

    bool base::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
    {
        if (button == mouse_button::LEFT)
            pressed = action == mouse_action::DOWN;

        return true;
    }

    bool base::on_wheel(mouse_wheel type, float value, vec2 pos)
    {
        return false;
    }

    void base::on_mouse_leave(vec2 pos)
    {
    }

    void base::on_mouse_enter(vec2 pos)
    {
    }

    bool base::on_mouse_move(vec2 pos)
    {
        return true;
    }

    sizer base::update_layout(sizer o_r, float scale)
    {
        if (!visible.get()) return o_r;

        result_scale = scale;
        //	scale = 1;
        sizer r = o_r;
        int max_iterations = 3;

        if (parent_sizer != sizer_long(o_r))
            need_update_layout = true;

        parent_sizer = o_r;

        do
        {
            sizer all_pos = { pos.get(), pos.get() + scaled_size.get() };
            all_pos +=
            {	-margin->left, -margin->top, margin->right, margin->bottom};
            float2 all_size = { all_pos.right - all_pos.left, all_pos.bottom - all_pos.top };
            all_pos *= scale;
            all_size *= scale;
            r = o_r;

            if (parent)
            {
                if (docking.get() == dock::TOP || docking.get() == dock::BOTTOM || docking.get() == dock::FILL)
                    if (parent->width_size == size_type::MATCH_CHILDREN)
                        r.right = r.left + std::max(all_size.x, r.right - r.left);

                if (docking.get() == dock::LEFT || docking.get() == dock::RIGHT || docking.get() == dock::FILL)
                    if (parent->height_size == size_type::MATCH_CHILDREN)
                        r.bottom = r.top + std::max(all_size.y, r.bottom - r.top);
            }

            sizer my = r;

            switch (docking.get())
            {
                case dock::FILL:
                    my = r;
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    break;

                case dock::NONE:
                    my.left = r.left + all_pos.left;
                    my.right = r.left + all_pos.right;
                    my.top = r.top + all_pos.top;
                    my.bottom = r.top + all_pos.bottom;
                    //	my += { margin->left, margin->top, -margin->right, -margin->bottom };
                    break;

                case dock::TOP:
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    my.bottom = my.top + scaled_size->y * scale;
                    r.top += all_size.y;
                    break;

                case dock::BOTTOM:
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    my.top = my.bottom - scaled_size->y * scale;
                    r.bottom -= all_size.y;
                    break;

                case dock::LEFT:
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    my.right = my.left + scaled_size->x * scale;
                    r.left += all_size.x;
                    break;

                case dock::RIGHT:
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    my.left = my.right - scaled_size->x * scale;
                    r.right -= all_size.x;
                    break;

                case dock::PARENT:
                    my = r; // r means full parent, how to do best?
                    my += scale * float4(margin->left, margin->top, -margin->right, -margin->bottom);
                    break;
            };

            render_bounds = math::convert(my);
            my = math::convert(render_bounds.get());
            my += scale * float4(padding->left, padding->top, -padding->right, -padding->bottom);
            sizer orig_my = my;

            if (!need_update_layout) break;

            need_update_layout = false;

            for (auto && c : childs)
                if (c->docking == dock::NONE) c->update_layout(my, result_scale * this->scale);

            for (auto && c : childs)
                if (c->docking != dock::FILL && c->docking != dock::NONE && c->docking != dock::PARENT) my = c->update_layout(my, result_scale * this->scale);

            for (auto && c : childs)
                if (c->docking == dock::FILL && c->docking != dock::NONE && c->docking != dock::PARENT) c->update_layout(my, result_scale * this->scale);

            vec2 new_size = size.get();

            if ((width_size == size_type::MATCH_CHILDREN || width_size == size_type::MATCH_PARENT_CHILDREN))
            {
                if (childs.size())
                {
					std::optional<float> min_x;
					std::optional<float> max_x;

                    for (auto && c : childs)
                    {
                        if (!c->visible.get()) continue;

                        if (c->width_size == size_type::MATCH_PARENT) continue;

                        if (c->width_size == size_type::SQUARE && c->height_size == size_type::SQUARE) continue;


						float current_min = c->get_render_bounds().x - padding->left * scale - c->margin->left * scale;
						float current_max = c->get_render_bounds().x + c->get_render_bounds().w + c->margin->right * scale + padding->right * scale;



						min_x = min_x ? std::min(*min_x, current_min) : current_min;
						max_x = max_x ? std::max(*max_x, current_max) : current_max;
					}

					if (max_x&&min_x)
						new_size.x = (*max_x - *min_x) / scale;
					else
						new_size.x = 0;

                    if (parent && width_size == size_type::MATCH_PARENT_CHILDREN) new_size.x = std::max(new_size.x, parent->get_render_bounds().w/scale);
                }

                else
                {
                    new_size =
                    { 0, new_size.y };
                }
            }

            if ((height_size == size_type::MATCH_CHILDREN || height_size == size_type::MATCH_PARENT_CHILDREN))
            {
                if (childs.size())
                {
					std::optional<float> min_y;
					std::optional<float> max_y;

                    for (auto && c : childs)
                    {
                        if (!c->visible.get()) continue;

                        if (c->height_size == size_type::MATCH_PARENT) continue;

                        if (c->height_size == size_type::SQUARE && c->width_size == size_type::SQUARE) continue;
						
						float current_min = c->get_render_bounds().y - padding->top * scale - c->margin->top * scale;
						float current_max = c->get_render_bounds().y + c->get_render_bounds().h + padding->bottom * scale + c->margin->bottom * scale;


                        min_y = min_y?std::min(*min_y, current_min): current_min;
                        max_y = max_y?std::max(*max_y, current_max) : current_max;
                    }

					if (max_y&&min_y)
						new_size.y = (*max_y - *min_y) / scale;
					else
						new_size.y = 0;

                    if (parent && height_size == size_type::MATCH_PARENT_CHILDREN) new_size.y = std::max(new_size.y, parent->get_render_bounds().h / scale);
                }

                else
                {
                    new_size =
                    { new_size.x, 0 };
                }
            }

            if (width_size == size_type::SQUARE)
            {
                new_size.x = new_size.y;// *proportion;
            }

            if (height_size == size_type::SQUARE)
            {
                new_size.y = new_size.x;// / proportion;
            }

            if (new_size.y != new_size.y) new_size.y = 0;

            scaled_size = new_size; ///scale;
            max_iterations--;

            for (auto && c : childs)
                if (c->docking == dock::PARENT) c->update_layout(orig_my, result_scale * this->scale);

            //	hack_layout();
            if (max_iterations == 0) break;
        }
        while (need_update_layout);

        return r;
    }

    void base::on_margin_changed(const sizer& r)
    {
        if (parent)
            parent->set_update_layout();
    }

    void base::on_padding_changed(const sizer& r)
    {
        set_update_layout();
    }

    void base::on_visible_changed(const bool& r)
    {
        if (parent)
            parent->set_update_layout();
    }

    void base::on_dock_changed(const dock& r)
    {
        if (parent)
            parent->set_update_layout();
    }


    void base::on_pos_changed(const vec2& r)
    {
        if (parent)
            parent->set_update_layout();

        //else
        set_update_layout();
    }

    void base::on_size_changed(const vec2& r)
    {
        if (parent)
            parent->set_update_layout();

        //  else
        set_update_layout();
    }

	void base::on_local_size_changed(const vec2& r)
	{
		scaled_size = r;
	}

    bool base::test_local_visible()
    {
        return render_bounds->w > 0 && render_bounds->h > 0;
    }

    std::lock_guard<std::recursive_mutex> base::lock()
    {
        if (user_ui)
        {
            user_ui->childs_locker_mutex.lock();
            return{ user_ui->childs_locker_mutex, std::adopt_lock };
        }

        childs_locker_mutex.lock();
        return{ childs_locker_mutex, std::adopt_lock };
    }

    void base::think(float dt)
    {
    }

    std::vector<base::ptr> base::find_control(vec2 at, bool click_only)
    {
        std::vector<base::ptr> res;

        if (!test_local_visible() || !visible.get())
            return res;

        if (clip_child && (render_bounds->pos.x > at.x || render_bounds->pos.y > at.y || render_bounds->pos.x + render_bounds->w < at.x || render_bounds->pos.y + render_bounds->h < at.y))
            return res;

        for (auto it = childs.rbegin(); it != childs.rend(); ++it)
        {
            auto c = *it;
            std::vector<base::ptr> cur = c->find_control(at /*- vec2(c->render_bounds->pos)*/, click_only);
            /*   if (cur.size())
               {
            	if (click_only && !clickable)
                   cur.push_back(get_ptr());
                   return cur;
               }*/
            res.insert(res.end(), cur.begin(), cur.end());
        }

        if (click_only && !clickable)
            return res;

        if (!(render_bounds->pos.x > at.x || render_bounds->pos.y > at.y || render_bounds->pos.x + render_bounds->w < at.x || render_bounds->pos.y + render_bounds->h < at.y))
            res.push_back(get_ptr());

        return res;
    }

    void base::on_touch()
    {
        if (parent)
            parent->on_child_touched(get_ptr());
    }

    void base::on_child_touched(ptr child)
    {
        on_touch();
    }

    void base::on_bounds_changed(const rect& r)
    {
        if (!parent)
            local_bounds = r;
        else
        {
            vec2 p = vec2(r.pos) - parent->render_bounds->pos;
            vec2 s = vec2(r.size);
            local_bounds = { p, s };
        }

        //   set_update_layout();
        rect r2 = r + rect(padding->left, padding->top, -padding->right - padding->left, -padding->bottom - padding->top);

        for (auto c : childs)
            c->on_parent_size_changed(r2);
    }

    void base::set_movable(bool value)
    {
        if (!user_ui)
            return;

        if (value)
        {
            user_ui->mouse_focus = get_ptr();
            user_ui->set_capture(true);
        }

        else
        {
            user_ui->mouse_focus.reset();
            user_ui->set_capture(false);
        }
    }


    vec2 base::get_desired_size(vec2 cur)
    {
        return cur;
    }

    void base::focus()
    {
        user_ui->focused = get_ptr();
    }

    void base::unfocus()
    {
        user_ui->focused.reset();
    }

    void base::set_update_layout()
    {
        need_update_layout = true;

        if (user_ui && !user_ui->is_updating_layout && parent)
            parent->set_update_layout();
    }

    void base::on_key_action(long key)
    {
    }

    void base::run_on_ui(std::function<void()> f)
    {
        if (user_ui)
        {
            user_ui->add_task(std::bind([](ptr component, std::function<void()> f2)
            {
                f2();
            }, get_ptr(), f));
        }

        else
            f();
    }

    void base::draw_recursive(RecursiveContext& rec_c, base* p)
    {
        if (!test_local_visible())
            return;

		auto &c = rec_c.render_context;

        bool visibility = true;
        rect r_bounds = render_bounds.get();
        r_bounds.x += c.offset.x;
        r_bounds.y += c.offset.y;

        if (p)
        {
            visibility = c.ui_clipping.top < c.ui_clipping.bottom && c.ui_clipping.left < c.ui_clipping.right;
            sizer intersected = intersect(math::convert(r_bounds), c.ui_clipping);
            visibility = visibility && (intersected.top < intersected.bottom && intersected.left < intersected.right);
        }

        else
        {
            c.ui_clipping = user_ui->get_render_bounds();

			rec_c.execute(c.commit_scissor());
			
        }

        c.scale = result_scale;

        if (visibility)
		{

			rec_c.execute([this](DX12::context &c) {
				draw(c);
			});
		}
       

        if (clip_child && !visibility)
            return;

        sizer orig = c.ui_clipping;
        sizer s;
        s.left = c.offset.x + render_bounds->x + padding->left * result_scale;
        s.top = c.offset.y + render_bounds->y + padding->top * result_scale;
        s.right = c.offset.x + render_bounds->x + render_bounds->w - padding->right * result_scale;
        s.bottom = c.offset.y + render_bounds->y + render_bounds->h - padding->bottom * result_scale;

        if (clip_child)
        {
            if (p)
                c.ui_clipping = intersect(s, c.ui_clipping);
            else
                c.ui_clipping = s;
        }

        if (!clip_child || c.ui_clipping.left < c.ui_clipping.right)
            if (!clip_child || c.ui_clipping.top < c.ui_clipping.bottom)
            {
       
                for (auto& child : childs)
                    if (child->visible.get())
                        child->draw_recursive(rec_c, this);
            }

        c.ui_clipping = orig;
		c.scale = result_scale;


    //    if (visibility)
    //        draw_after(c);

		rec_c.execute([this](DX12::context &c) {
			draw_after(c);
		});
    }

    bool base::need_drag_drop()
    {
        return false;
    }

    void base::on_dragdrop_start(drag_n_drop_package::ptr)
    {
    }

    void base::on_dragdrop_end()
    {
    }

    void base::on_drag_start()
    {
    }

    void base::on_drag_end()
    {
    }

    void base::set_package(std::string name)
    {
        if (!package) package.reset(new drag_n_drop_package());

        package->name = name;
    }

    void base::on_parent_size_changed(const rect& r)
    {
        vec2 new_size = scaled_size.get();

        if (width_size == size_type::MATCH_PARENT)
            new_size.x = r.w;

        if (height_size == size_type::MATCH_PARENT)
            new_size.y = r.h;

        size = new_size;
    }

    bool base::can_accept(drag_n_drop_package::ptr)
    {
        return false;
    }

    void base::on_drop_enter(drag_n_drop_package::ptr)
    {
    }

    void base::on_drop_move(drag_n_drop_package::ptr, vec2)
    {
    }

    void base::on_drop_leave(drag_n_drop_package::ptr)
    {
    }

    bool base::on_drop(drag_n_drop_package::ptr, vec2)
    {
        return false;
    }

    drag_n_drop_package::ptr base::get_package()
    {
        return package;
    }

    rect base::get_render_bounds()
    {
        return render_bounds.get();
    }

    user_interface* base::get_user_ui()
    {
        return user_ui;
    }

    bool base::is_hovered()
    {
        return user_ui->hovered.lock().get() == this;
    }

    bool base::is_pressed()
    {
        return pressed;
    }

    bool base::is_on_top()
    {
        if (!parent)
            return true;

        return parent->childs.back().get() == this;
    }

    void base::set_skin(Renderer_ptr skin)
    {
        if (skin != this->renderer)
            render_data.reset();

        this->renderer = skin;

        for (auto c : childs)
            c->set_skin(skin);
    }

    GUI::Renderer_ptr base::get_skin()
    {
        return renderer;
    }

    bool base::is_menu_component()
    {
        if (parent)
            return parent->is_menu_component();

        return false;
    }

    void base::close_menus()
    {
        auto copy = childs;

        for (auto c : copy)
            c->close_menus();
    }

    void base::draw(DX12::context& c) //
    {
        if (draw_helper)
            renderer->draw_virtual(get_ptr(), c);
    }

    void base::draw_after(DX12::context&)
    {
    }

    void base::init_properties(Elements::ParameterWindow* wnd)
    {
        wnd->add_param(std::make_shared<Elements::property_base>("base"));
        wnd->add_param(std::make_shared<Elements::property_base>("size"));
        wnd->add_param(std::make_shared<Elements::property_enum>(docking, "docker", GUI::dock::NONE, "NONE"));
        wnd->add_param(std::make_shared<Elements::property_base>("visible"));
    }

     void user_interface::process_ui(float delta_time)
    {

		 for (int i = 0; i < 3; i++)
		 {
			 press_interpret[i] = false;
			 release_interpret[i] = false;
		 }
		 {
			 while (true)
			 {
				 tasks_mutex.lock();
				 std::vector<std::function<void()> > copy;
				 std::swap(copy, tasks);
				 tasks_mutex.unlock();

				 if (copy.empty())break;

				 for (auto t : copy)
					 t();
			 }
		 }

	


		 auto&& g2 = lock();
		 {
		//	 auto timer = c.command_list->start(L"think");
			 for (auto t : thinking)
				 t->think(delta_time);

		 }
		 is_updating_layout = true;
		 {
             PROFILE(L"layout");

			 update_layout({ pos.get(), pos.get() + size.get() }, scale);
			 is_updating_layout = false;
		 }


    }


     void user_interface::create_graph(FrameGraph& graph)
     {
      //   return;
         process_graph(graph);
         struct pass_data
         {
             ResourceHandler* o_texture;
         };

		 graph.add_pass<pass_data>("UI RENDER",[](pass_data& data, TaskBuilder& builder) {
			 data.o_texture = builder.need_texture("swapchain", ResourceFlags::RenderTarget);

			 }, [this,&graph](pass_data& data, FrameContext& context) {
			//	 std::lock_guard<std::mutex> g(m);

				 auto command_list = context.get_list();
				
				 auto texture = context.get_texture(data.o_texture);

			//	 command_list->transition(texture.resource, DX12::ResourceState::RENDER_TARGET);
				 command_list->get_graphics().set_rtv(1, texture.get_rtv(), DX12::Handle());
				 command_list->get_graphics().set_viewports({ texture.get_viewport() });

			
                 std::shared_ptr<OVRContext> ovr_context;
                 renderer->start();

                 DX12::context c(command_list, ovr_context);
                 c.command_list = command_list;
               
				 c.offset = { 0, 0 };
				 c.window_size = scaled_size.get();
				 c.scale = 1;
                 c.delta_time = graph.time;
				 {
                     PROFILE_GPU(L"draw");
					 RecursiveContext context(c);

					 //if (!c.command_list_label)
					 {
						 c.labeled = &context.pre_executor;

						 c.command_list_label = c.command_list->get_sub_list();
						 c.command_list_label->begin("Label");
					 }


					 draw_recursive(context);

					 drag.draw(context);
					 context.stop_and_wait();
				 }

				 if (c.command_list_label)
				 {
					 c.command_list_label->end();
					 c.command_list_label->execute();
					 c.command_list_label = nullptr;
				 }

				 renderer->flush(c);

				 //command_list->transition(texture.resource, DX12::ResourceState::PRESENT);


			 });


     }

     /*
    void user_interface::draw_ui(DX12::context& c)
    {
        std::lock_guard<std::mutex> g(m);

        process_ui();


		{
			auto timer = c.command_list->start(L"draw");
			RecursiveContext context(c);

			//if (!c.command_list_label)
			{
				c.labeled = &context.pre_executor;

				c.command_list_label = c.command_list->get_sub_list();
				c.command_list_label->begin("Label");
			}


			draw_recursive(context);
			
			drag.draw(context);
			context.stop_and_wait();
		}

		if(c.command_list_label)
		{
			c.command_list_label->end();
			c.command_list_label->execute();
			c.command_list_label = nullptr;
		}
		
		renderer->flush(c);	
    }
    */
    void user_interface::mouse_action_event(mouse_action action, mouse_button button, vec2 pos)
    {
        {
            std::lock_guard<std::mutex> g(m);
            auto&& g2 = lock();
            auto i_button = static_cast<int>(button);

            if (action == mouse_action::DOWN)
            {
                if (press_interpret[i_button])
                    return;

                press_interpret[i_button] = true;
            }

            if (action == mouse_action::UP)
            {
                if (release_interpret[i_button])
                    return;

                release_interpret[i_button] = true;
            }

            std::vector<base::ptr>& controls = hovered_controls;// find_control(pos, true);
            auto m_focus = mouse_focus.lock();

            if (m_focus)
                controls.push_back(m_focus);

            if (controls.size() == 0)
                return;

            if (action == mouse_action::DOWN)
            {
                pressed[i_button].clear();

                for (unsigned int i = 0; i < controls.size(); i++)
                {
                    base::ptr control = controls[i];
                    pressed[i_button].push_back(control);

                    if (!control->on_mouse_action(action, button, pos))
                        continue;

                    if (!control->is_menu_component())
                        run_on_ui([this]() { close_menus();});
                    control->on_touch();
                    control->update_cursor();
                    drag.mouse_action_event(control, action, button, pos);
                    break;
                }
            }

            if (action == mouse_action::UP)
            {
                for (unsigned int i = 0; i < pressed[i_button].size(); i++)
                {
                    base::ptr control = pressed[i_button][i];

                    if (controls.size() > i && control == controls[i])
                        control->on_mouse_action(action, button, pos);
                    else
                        control->on_mouse_action(mouse_action::CANCEL, button, pos);
                }

                if (pressed[i_button].size())
                    drag.mouse_action_event(pressed[i_button].back(), action, button, pos);
            }

            if (action == mouse_action::CANCEL)
            {
                for (unsigned int i = 0; i < pressed[i_button].size(); i++)
                {
                    base::ptr control = pressed[i_button][i];
                    control->on_mouse_action(action, button, pos);
                }

                if (pressed[i_button].size())
                    drag.mouse_action_event(pressed[i_button].back(), action, button, pos);
            }
        }
        mouse_move_event(pos);
        /*
        if (action == mouse_action::DOWN)
        {

            if (control)
            {
                if (!control->is_menu_component())
                {
                    close_menus();
                }

                pressed[static_cast<int>(button)] = control;
                control->on_touch();
                control->on_mouse_action(mouse_action::DOWN, button, pos);
                control->update_cursor();
            }

            drag.mouse_action_event(control, action, button, pos);
        }
        else    if (action == mouse_action::UP)
        {
            auto p_control = pressed[static_cast<int>(button)].lock();

            if (p_control)
            {
                if (control == p_control)
                {
                    p_control->on_mouse_action(mouse_action::UP, button, pos);
                    p_control->update_cursor();
                }
                else
                {
                    if (p_control)
                    {
                        p_control->on_mouse_action(mouse_action::CANCEL, button, pos);
                        p_control->update_cursor();
                    }
                }
            }

            pressed[static_cast<int>(button)].reset();
            drag.mouse_action_event(p_control, action, button, pos);
        }
        else
        {
            auto p_control = pressed[static_cast<int>(button)].lock();

            if (p_control)
            {
                p_control->on_mouse_action(mouse_action::CANCEL, button, pos);
                p_control->update_cursor();
            }

            pressed[static_cast<int>(button)].reset();
            drag.mouse_action_event(p_control, action, button, pos);
        }
        */
        //	if (movable)
        //		movable->update_cursor();
    }

    void user_interface::mouse_move_event(vec2 pos)
    {
        std::lock_guard<std::mutex> g(m);
        auto&& g2 = lock();
        /*	if (movable)
        	{

        		movable->on_mouse_move(pos);
        		movable->update_cursor();
            }*/
        hovered_controls = find_control(pos, true);
        auto t = mouse_focus.lock();

        if (t)
        {
            hovered = t;
            t->on_mouse_move(pos);
            t->update_cursor();
            drag.mouse_move_event(t, pos);
            return;
        }

        size_t remove_index = hovered_controls.size();

        for (size_t i = 0; i < hovered_controls.size(); i++)
        {
            base::ptr control = hovered_controls[i];
            auto hover = hovered.lock();

            if (hover != control)
            {
                if (hover)
                    hover->on_mouse_leave(pos);

                if (control)
                    control->on_mouse_enter(pos);
            }

            hovered = control;

            if (!control->on_mouse_move(pos))
            {
                control->on_mouse_leave(pos);
                continue;
            }

            control->update_cursor();
            drag.mouse_move_event(control, pos);
            remove_index = i + 1;
            break;
        }

        for (size_t i = remove_index; i < hovered_controls.size(); i++)
        {
            base::ptr control = hovered_controls[i];

            for (size_t i_button = 0; i_button < 3; i_button++)
            {
                for (size_t j = 0; j < pressed[i_button].size(); j++)
                {
                    if (pressed[i_button][j] == control)
                    {
                        control->on_mouse_action(mouse_action::CANCEL, static_cast<mouse_button>(i_button), pos);
                        pressed[i_button].erase(pressed[i_button].begin() + j);
                        break;
                    }
                }
            }
        }

        if (remove_index != hovered_controls.size())
            hovered_controls.erase(hovered_controls.begin() + remove_index, hovered_controls.end());

        /*base::ptr control = nullptr;

        if (controls.size() > 0)
            control = controls[0];

        auto hover = hovered.lock();

        if (hover != control)
        {
            if (hover)
                hover->on_mouse_leave(pos);

            if (control)
                control->on_mouse_enter(pos);
        }

        hovered = control;
        auto t = mouse_focus.lock();

        if (t)
        {
            hovered = t;
            control = t;
        }

        if (control)
        {
            control->on_mouse_move(pos);
            control->update_cursor();
        }

        drag.mouse_move_event(control, pos);*/
    }

    user_interface::user_interface(): drag(this)
    {
        docking = dock::FILL;
        renderer.reset(new Renderer());
        user_ui = this;
        back.reset(new dark());
        user_interface::add_child(back);
        back->visible = false;
        add_child(drag.holder);
    }


    void user_interface::remove_base(base* object)
    {
	/*	auto frame_gen = dynamic_cast<FrameGraphGenerator*>(object);
		if (frame_gen)
			frame_generators.emplace_back(frame_gen);*/
    }

    void user_interface::add_base(base* object)
    {
        //   components.insert(object);
      //  auto frame_gen = dynamic_cast<FrameGraphGenerator*>(object);
     //   if (frame_gen)
      //      frame_generators.emplace_back(frame_gen);
    }

    void user_interface::key_action_event(long key)
    {
        auto really_focused = focused.lock();

        if (really_focused)
            really_focused->on_key_action(key);
    }

    std::shared_future<bool> user_interface::message_box(std::string title, std::string text, std::function<void(bool)> fun)
    {
        GUI::Elements::window::ptr exit_window(new GUI::Elements::window());
        exit_window->allow_resize(false);
        GUI::Elements::window::wptr w = exit_window;
        std::shared_ptr<std::promise<bool>> pr(new std::promise<bool>());
        auto result = [w, fun, this, pr](bool r)
        {
            auto b = back;
            run_on_ui([w, fun, r, b, pr]()
            {
                if (fun)
                    fun(r);

                pr->set_value(r);
                auto wind = w.lock();

                if (wind)
                    wind->remove_from_parent();

                b->visible = false;
            });
        };
        exit_window->size = { 250, 100 };
        exit_window->minimal_size = { 250, 100 };
        exit_window->width_size = size_type::FIXED;
        exit_window->height_size = size_type::MATCH_CHILDREN;
		exit_window->contents->docking = dock::TOP;
		exit_window->contents->height_size = size_type::MATCH_CHILDREN;
        exit_window->set_title(title);
        exit_window->pos = ivec2(size.get() / 2 - vec2(exit_window->size.get()) / 2);
        GUI::Elements::label::ptr info_text(new GUI::Elements::label());
        info_text->docking = dock::TOP;
        info_text->text = text;
        info_text->margin = { 10, 10, 10, 10 };
	    exit_window->add_child(info_text);

		base::ptr bottom_row = std::make_shared<base>();
		
		bottom_row->docking = dock::TOP;
		bottom_row->width_size = size_type::MATCH_CHILDREN;
		bottom_row->size = { 75, 25 };
		bottom_row->margin = {10,10,10,10};
		bottom_row->height_size = size_type::FIXED;
		exit_window->add_child(bottom_row);

        GUI::Elements::button::ptr yes(new GUI::Elements::button());
        yes->get_label()->text = "OK";
        yes->size = { 75, 25 };
        yes->margin = { 10, 0, 10, 0 };
        yes->on_click = [result](GUI::Elements::button::ptr) {  result(true); };
        yes->docking = dock::LEFT;


		bottom_row->add_child(yes);
        GUI::Elements::button::ptr no(new GUI::Elements::button());
        no->get_label()->text = "Cancel";
        no->size = { 75, 25 };
        no->on_click = [result](GUI::Elements::button::ptr) { result(false); };;// std::bind([](std::shared_ptr<std::promise<bool>> pr){pr->set_value(false); }, pr);
        no->docking = dock::LEFT;
        no->margin = { 10, 0, 10, 0 };

		bottom_row->add_child(no);

		exit_window->close_button->on_click = [result](GUI::Elements::button::ptr) {
			result(false);
		};
        run_on_ui([this, exit_window]()
        {
            back->to_front();
            back->visible = true;
            add_child(exit_window);
        });
        return pr->get_future();
    }

    void user_interface::on_size_changed(const vec2& r)
    {
        base::on_size_changed(r);
        back->size = r;
    }

    void user_interface::mouse_wheel_event(mouse_wheel type, float value, vec2 pos)
    {
        for (int i = (int)hovered_controls.size() - 1; i >= 0; i--)
        {
            base::ptr control = hovered_controls[i];

            if (control->on_wheel(type, value, pos))
                break;
        }
    }


    void dark::draw(DX12::context& c)
    {
        renderer->draw_color(c, vec4(0.0, 0.0, 0.0, 0.5f), get_render_bounds());
    }

    drag_n_drop::~drag_n_drop()
    {
    }

    void drag_n_drop::draw(RecursiveContext& c)
    {
        if (!dragging) return;

        auto p = pressed.lock();

        if (!p)
            return;

        c.render_context.offset = cur_pos - start_pos;
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       if (p->drag_n_drop_copy)
		   p->draw_recursive(c);
           
    }

    void drag_n_drop::on_drop(vec2 at) const
    {
        auto h = hovered.lock();
        auto p = package.lock();
        auto c = pressed.lock();

        for (auto base : dragdrop)
            base->on_dragdrop_end();

        holder->remove_from_parent();
        holder->remove_all();

        if (!h || !p || !c)
            return;

        h->on_drop_leave(p);
        h->on_drop(p, at);
    }

    void drag_n_drop::update_hovered(base::ptr elem, vec2 pos)
    {
        auto p = package.lock();

        if (!p)
            return;

        auto h = hovered.lock();

        if (h == elem)
            return;

        if (h)
            h->on_drop_leave(p);

        while (elem && !elem->can_accept(p))
            elem = elem->get_parent();

        if (elem)
            elem->on_drop_enter(p);

        hovered = elem;
    }

    bool drag_n_drop::start_new_drag(base::ptr elem, vec2 pos)
    {
        if ((start_pos - pos).length() < 20)
            return false;

        if (!elem->need_drag_drop())
            return false;

        auto package = elem->get_package();

        if (!package)
            return false;

        this->package = package;
        package->element = elem;
        dragging = true;
        elem->on_drag_start();

        for (auto c : dragdrop)
            c->on_dragdrop_start(package);

        if (!elem->drag_n_drop_copy)
        {
            elem->generate_container(holder);
            elem->pos = {0, 0};
            holder->size = { 300, 300 };
            user_ui->add_child(holder);
        }

        return true;
    }

    void drag_n_drop::mouse_move_event(base::ptr elem, vec2 pos)
    {
        auto p = pressed.lock();

        if (!p) return;

        if (dragging && !p->drag_n_drop_copy)
        {
            holder->pos = pos;//-start_pos;
        }

        if (!dragging && start_new_drag(p, pos))
            return;

        update_hovered(elem, pos);
        auto h = hovered.lock();
        auto pc = package.lock();
        cur_pos = pos;

        if (h && pc)
            h->on_drop_move(pc, pos);
    }

    void drag_n_drop::mouse_action_event(base::ptr elem, mouse_action action, mouse_button button, vec2 pos)
    {
        if (!elem) return;

        if (button != mouse_button::LEFT)
            return;

        if (action == mouse_action::DOWN)
        {
            pressed = elem;
            start_pos = pos;
        }

        else
        {
            if (dragging)
            {
                auto p = pressed.lock();

                if (p)
                    p->on_drag_end();

                on_drop(pos);
            }

            pressed.reset();
            package.reset();
            dragging = false;
        }
    }

}