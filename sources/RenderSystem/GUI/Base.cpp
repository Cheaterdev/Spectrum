module GUI;
import RenderSystem;
import Graphics;


import <windows/windows.h>;
import windows;

import HAL;
import FrameGraph;
import TextEngine;

#include "../FrameGraph/autogen/pass_defaults.h"


using namespace FrameGraph;

// TEMP-POPUPLOG: scratch logging for "popups don't close on an outside click".
// Appends to popup_log.temp (gitignored) in the working directory; remove with the
// other TEMP-POPUPLOG marks (Base.cpp, MenuList.cpp, Ribbon.cpp) once it is solved.
namespace
{
    void popup_log(const std::string& line)
    {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> guard(log_mutex);
        std::ofstream out("popup_log.temp", std::ios::app);
        out << line << "\n";
    }
}

static const LPCTSTR cursors[] =
{
    IDC_ARROW,
    IDC_IBEAM,
    IDC_SIZENS,
    IDC_SIZEWE,
    IDC_SIZENWSE,
    IDC_SIZENESW,
    IDC_SIZEALL,
    IDC_NO ,
    IDC_WAIT,
    IDC_HAND
};

// Forward-declared here (global scope, matching where it's actually defined
// below, alongside the other PassDefault<T> specializations) so
// user_interface::create_graph (namespace GUI, below) can call it before its
// definition -- unqualified lookup from inside a namespace still finds a
// global-scope name.
static uint32_t ui_per_thread(uint32_t size);

namespace GUI
{
    class dark : public base
    {
        public:
            virtual void draw(Context&) override;
    };

    void base::on_add(base* base_parent)
    {
        tree_base::on_add(base_parent);

        if (!base_parent) return;

        user_ui = base_parent->user_ui;
        map_to_pixels = map_to_pixels && base_parent->map_to_pixels;
        need_update_layout = true;

        if (fade_on_add && user_ui && user_ui->fade_enabled && user_ui->window_animations)
        {
            fade_closing = false;
            layer_opacity = 0.0f;
            fade_to(1.0f, user_ui->fade_in_duration);
        }

        if (user_ui && thinkable)
            user_ui->thinking.insert(this);

        if (user_ui && drag_listener)
            user_ui->drag.dragdrop.insert(this);

        if (user_ui)
            user_ui->add_base(this);

        for (auto c : childs)
            c->on_add(this);
    }

    void base::on_remove()
    {
        tree_base::on_remove();

          if (user_ui)
              user_ui->remove_base(this);
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

       
            return res;
        };


        scaled_size.filter_func = [this](const vec2& r)->vec2
        {
            vec2 res = vec2::max(vec2((r)), minimal_size);

        
            return res;
        };


        pos.filter_func = [this](const vec2 & r)->vec2
        {
            vec2 res = (r);

    

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

            if (height_size == size_type::FIXED || height_size == size_type::MATCH_CHILDREN || height_size == size_type::MATCH_PARENT || height_size == size_type::MATCH_PARENT_CHILDREN)
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

            if (clamp_to_parent != ParentClamp::NONE && parent)
            {
                //float2 size = float2(res.size);
                //float2 pos = float2(res.pos);
                float2 size = float2::min(float2(res.size), float2(parent->get_render_bounds().size) - float2(parent->padding->left + parent->padding->right, parent->padding->top + parent->padding->bottom) * result_scale);
                float2 pos = float2::max(float2(parent->get_render_bounds().pos) + float2(parent->padding->left, parent->padding->top) * result_scale, float2(res.pos));
            	pos = float2::min(pos, float2(parent->get_render_bounds().pos) + float2(parent->get_render_bounds().size) - size - float2(parent->padding->right, parent->padding->bottom) * result_scale);

                if (check(clamp_to_parent & ParentClamp::WIDTH)) {
                    res.pos.x = pos.x;
                    res.size.x = size.x;
                }

                if (check(clamp_to_parent & ParentClamp::HEIGHT)) {
                    res.pos.y = pos.y;
                    res.size.y = size.y;
                }

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
        if (user_ui) CHECK_THREAD(GUI);

        auto&& g = lock();
        tree_base::remove_child(obj);
        set_update_layout();
    }

    cursor_style base::update_cursor() const
    {
        return cursor;
      
    }

    void base::add_child(ptr obj)
    {
    	if(user_ui)  CHECK_THREAD(GUI);
    	
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

        return pressed;
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

	void base::update_childs_layout(sizer& my, float scale)
	{
		for (auto&& c : childs)
			if (c->docking == dock::NONE) c->update_layout(my, result_scale * this->scale);

		for (auto&& c : childs)
			if (c->docking != dock::FILL && c->docking != dock::NONE && c->docking != dock::PARENT) my = c->update_layout(my, result_scale * this->scale);

		for (auto&& c : childs)
			if (c->docking == dock::FILL && c->docking != dock::NONE && c->docking != dock::PARENT) c->update_layout(my, result_scale * this->scale);
	}

	void base::update_childs_layout_after(sizer& r, float scale)
	{
		for (auto&& c : childs)
			if (c->docking == dock::PARENT) c->update_layout(r, result_scale * this->scale);

	}
                                void base::register_listener(base* listener)
                                {
									listeners.insert(listener);
                                }

			void base::unregister_listener(base* listener)
            {
				listeners.erase(listener);
			}

  void base::on_base_change(base* b)
  {
               
	  vec2 new_size = scaled_size.get();

	  if (width_size == size_type::MATCH_PARENT)
		  if (width_sticks == b)
			  new_size.x = width_sticks->get_render_bounds().w;


	  if (height_size == size_type::MATCH_PARENT)
		  if (height_sticks == b)
			  new_size.y = height_sticks->get_render_bounds().h;


	  size = new_size;
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
                sizer{	-margin->left, -margin->top, margin->right, margin->bottom};
            float2 all_size = { all_pos.right - all_pos.left, all_pos.bottom - all_pos.top };
            all_pos *= scale;
            all_size *= scale;
            r = o_r;

            if (parent)
            {
                if (docking.get() == dock::TOP || docking.get() == dock::BOTTOM || docking.get() == dock::FILL)
                    if (width_sticks==nullptr&&parent->width_size == size_type::MATCH_CHILDREN)
                        r.right = r.left + std::max(all_size.x, r.right - r.left);

                if (docking.get() == dock::LEFT || docking.get() == dock::RIGHT || docking.get() == dock::FILL)
                    if (height_sticks==nullptr&&parent->height_size == size_type::MATCH_CHILDREN)
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

            update_childs_layout(my,  result_scale * this->scale);

            vec2 new_size = size.get();

            if ((width_size == size_type::MATCH_CHILDREN || width_size == size_type::MATCH_PARENT_CHILDREN))
            {
                if (childs.size())
                {
					std::optional<float> min_x;
					std::optional<float> max_x;

                    for (auto && c : childs)
                    {
                        auto ch=c.get();
                        if (!c->visible.get()) continue;
                        if (c->width_sticks) continue;
                        if (c->width_size == size_type::MATCH_PARENT) continue;
                             if (c->docking == dock::FILL)
                                 continue;

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
                           auto ch=c.get();
                        if (!c->visible.get()) continue;
                        if (c->height_sticks) continue;
                        if (c->height_size == size_type::MATCH_PARENT) continue;
                                           if (c->docking == dock::FILL)
                                 continue;
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

             update_childs_layout_after(orig_my,  result_scale * this->scale);

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

    // ---- Touch (ported from the NDK GUI; separate from the mouse path) --------------------------

    bool base::on_touch_action(mouse_action action, touch& t)
    {
        if (touch_id == t.id || touch_id == -1)
        {
            touch_id = action == mouse_action::DOWN ? t.id : -1;
            return on_mouse_action(action, mouse_button::LEFT, t.pos);
        }

        return false;
    }

    bool base::on_long_pressed(const touch& t)
    {
        // The original returned its catch_clicks flag, which defaulted to false and which this engine
        // has no equivalent of: elements opt in by overriding this.
        return false;
    }

    void base::on_touch_move(std::vector<touch*>& e)
    {
        e[0]->catched = on_mouse_move(e[0]->pos);
    }

    bool base::point_in(vec2 at)
    {
        return !(render_bounds->pos.x > at.x || render_bounds->pos.y > at.y
                 || render_bounds->pos.x + render_bounds->w < at.x || render_bounds->pos.y + render_bounds->h < at.y);
    }

    std::vector<std::pair<base::ptr, std::vector<touch*>>> base::find_childs_control(std::vector<touch>& touches, bool click_only)
    {
        std::vector<std::pair<base::ptr, std::vector<touch*>>> res;

        for (auto it = childs.rbegin(); it != childs.rend(); ++it)
        {
            auto c = *it;
            auto cur = c->find_control(touches, click_only);
            res.insert(res.end(), cur.begin(), cur.end());
        }

        return res;
    }

    std::vector<std::pair<base::ptr, std::vector<touch*>>> base::find_control(std::vector<touch>& touches, bool click_only)
    {
        std::vector<std::pair<base::ptr, std::vector<touch*>>> res;

        if (!test_local_visible() || !visible.get())
            return res;

        if (!clip_child)
        {
            bool all_touches_out = true;

            for (auto& at : touches)
                if (point_in(at.pos))
                {
                    all_touches_out = false;
                    break;
                }

            if (all_touches_out)
                return res;
        }

        res = find_childs_control(touches, click_only);

        if (click_only && !clickable)
            return res;

        std::vector<touch*> touches_in_me;

        for (auto& at : touches)
            if (point_in(at.pos))
                touches_in_me.push_back(&at);

        if (touches_in_me.size())
            res.push_back(std::make_pair(get_ptr(), touches_in_me));

        return res;
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

		for (auto l : listeners)
            l->on_base_change(this);
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
        auto previous = user_ui->focused.lock();
        if (previous.get() == this)
            return;

        user_ui->focused = get_ptr();
        if (previous)
            previous->on_focus_changed(false);
        on_focus_changed(true);
    }

    void base::unfocus()
    {
        if (!user_ui || user_ui->focused.lock().get() != this)
            return;

        user_ui->focused.reset();
        on_focus_changed(false);
    }

    void base::on_focus_changed(bool)
    {
    }

    bool base::is_focused()
    {
        return user_ui && user_ui->focused.lock().get() == this;
    }

    void base::set_update_layout()
    {
        need_update_layout = true;

        if (user_ui && !user_ui->is_updating_layout && parent)
            parent->set_update_layout();
    }

    void base::on_key_action(key_action action, long key, key_mods mods)
    {
    }

    void base::on_char(char32_t ch)
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

    void base::draw_recursive(GUIInfo& c, base* p)
    {
        if (!test_local_visible())
            return;


        bool visibility = true;
        const rect collected_bounds = render_bounds.get();
        rect r_bounds = collected_bounds;
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
            c.scissors = user_ui->get_render_bounds();

			//rec_c.execute(c.commit_scissor());
			
        }

        c.scale = result_scale;

        sizer_long entry_scissors = c.scissors;
        if (self_scissor.has_value())
            entry_scissors = intersect(entry_scissors, math::convert(*self_scissor));

        // Decided once per frame here, so the before and after entries agree.
        bool layered = false;
        uint32_t layer_index = 0;
        float layer_alpha = 1.0f;

        if (visibility)
		{
            update_fade(c.delta_time);
            layered = layer_wanted();
            layer_alpha = layer_opacity;

            if (layered)
                layer_index = user_ui->layer_counter++;

            for (auto& e : effects)
                if (e->enabled)
                {
                    e->collect(*this);
                    e->update(*this, c.delta_time);
                }

           on_pre_render(c);
            user_ui->draw_infos.emplace_back(c.ui_clipping, entry_scissors,c.scale,this,c.offset,true,layered,layer_index,layer_alpha,collected_bounds);
		//	rec_c.execute([this](Context &c) {
		//		draw(c);
		//	});
		}
       

        if (clip_child && !visibility)
            return;

        sizer orig            = c.ui_clipping;
        sizer_long orig_scissors = c.scissors;
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

        if (child_scissor.has_value())
        {
            sizer cs = math::convert(*child_scissor);
            c.ui_clipping = intersect(c.ui_clipping, cs);
            c.scissors    = intersect(c.scissors, cs);
        }

        if (!clip_child || c.ui_clipping.left < c.ui_clipping.right)
            if (!clip_child || c.ui_clipping.top < c.ui_clipping.bottom)
            {

                for (auto& child : childs)
                    if (child->visible.get())
                        child->draw_recursive(c, this);
            }

        c.ui_clipping = orig;
        c.scissors    = orig_scissors;
		c.scale = result_scale;


    //    if (visibility)
    //        draw_after(c);
        user_ui->draw_infos.emplace_back(c.ui_clipping, entry_scissors,c.scale, this, c.offset, false, layered, layer_index, layer_alpha, collected_bounds);

		/*rec_c.execute([this](Context &c) {
			draw_after(c);
		});*/
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
            if(width_sticks)
                      new_size.x = width_sticks->get_render_bounds().w;
            else

            new_size.x = r.w;

        if (height_size == size_type::MATCH_PARENT)
              if(height_sticks)
                      new_size.y = height_sticks->get_render_bounds().h;
            else
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

    namespace
    {
        // Set by draw_bounds_scope on the thread that is drawing an element.
        thread_local const base* draw_bounds_owner = nullptr;
        thread_local rect draw_bounds_value;
    }

    draw_bounds_scope::draw_bounds_scope(const base* owner, const rect& bounds)
        : previous_owner(draw_bounds_owner), previous_bounds(draw_bounds_value)
    {
        draw_bounds_owner = owner;
        draw_bounds_value = bounds;
    }

    draw_bounds_scope::~draw_bounds_scope()
    {
        draw_bounds_owner = previous_owner;
        draw_bounds_value = previous_bounds;
    }

    rect base::get_render_bounds()
    {
        if (draw_bounds_owner == this)
            return draw_bounds_value;

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

    void base::draw(Context& c) //
    {
        if (draw_helper)
            c.renderer->draw_virtual(get_ptr(), c);
    }

    void base::draw_after(Context&)
    {
    }

    // Without a target (hosts that draw the UI elsewhere) there is nothing to
    // snapshot, and elements fall back to their regular look.
    bool base::backdrop_active(const Context& c) const
    {
        auto* blur = find_effect<BackdropBlur>();
        return blur && blur->active(*this, c);
    }

    bool BackdropBlur::active(const base& e, const GUIInfo& c) const
    {
        if (!(enabled && e.user_ui && e.user_ui->backdrop_enabled && c.target.resource))
            return false;

        // Raw parent pointers for the same reason as control_opacity().
        if (standalone_only)
            for (const base* p = e.parent; p; p = p->parent)
                if (p->find_effect<BackdropBlur>())
                    return false;

        return true;
    }

    float base::control_opacity(const Context& c)
    {
        // Raw parent pointers, not get_parent(): that returns parent->get_ptr()
        // (shared_from_this), which throws for a root not owned by a shared_ptr
        // -- e.g. a user_interface on the stack in the GUI tests.
        for (const base* p = parent; p; p = p->parent)
            if (auto* blur = p->find_effect<BackdropBlur>(); blur && blur->glass_children)
                return blur->active(*p, c) ? blur->glass_opacity : 1.0f;
        return 1.0f;
    }

    void base::fade_to(float target, float seconds)
    {
        on_fade_end = nullptr;   // a new fade replaces whatever the old one was going to do
        fade_from = layer_opacity;
        fade_target = target;
        fade_duration = std::max(seconds, 0.001f);
        fade_t = 0.0f;
    }

    void base::update_fade(float dt)
    {
        if (fade_t >= 1.0f)
            return;

        fade_t = std::min(1.0f, fade_t + dt / fade_duration);
        const float smooth = fade_t * fade_t * (3.0f - 2.0f * fade_t);
        layer_opacity = fade_from + (fade_target - fade_from) * smooth;

        if (fade_t >= 1.0f && on_fade_end)
        {
            auto done = std::move(on_fade_end);
            on_fade_end = nullptr;
            run_on_ui(std::move(done));
        }
    }

    void base::fade_close()
    {
        if (fade_closing)
            return;

        if (!user_ui || !user_ui->fade_enabled || !user_ui->window_animations || !get_parent())
        {
            remove_from_parent();
            return;
        }

        fade_closing = true;
        fade_to(0.0f, user_ui->fade_out_duration);

        // fade_to dropped any earlier callback, so this is set after it.
        on_fade_end = [self = get_ptr()]()
        {
            if (!self->fade_closing)
                return;   // cancel_fade_close() came in between

            self->fade_closing = false;
            self->remove_from_parent();
            self->layer_opacity = 1.0f;
        };
    }

    void base::cancel_fade_close()
    {
        if (!fade_closing)
            return;

        fade_closing = false;
        fade_to(1.0f, user_ui ? float(user_ui->fade_in_duration) : 0.1f);
    }

    bool base::layer_wanted() const
    {
        return layer_opacity < 0.999f && user_ui && user_ui->fade_enabled;
    }

    void base::draw_end(Context& c)
    {
        draw_after(c);

        if (layer_open)
        {
            layer_open = false;
            c.renderer->end_layer(c, c.layer_opacity);
        }
    }

    void base::draw_before(Context& c)
    {
        // The group opens first, so the blur, shadow and the element itself are
        // all part of it. The shadow reaches outside the bounds, so the
        // snapshot is grown by it.
        layer_open = false;

        if (c.layer && c.target.resource)
        {
            float pad = 2.0f;

            for (auto& e : effects)
                if (e->enabled)
                    pad = std::max(pad, e->extent(*this));

            layer_open = c.renderer->begin_layer(c, c.layer_slot, get_render_bounds(), pad);
        }

        for (auto& e : effects)
            if (e->enabled)
                e->draw_under(c, *this);

        draw(c);

        for (auto& e : effects)
            if (e->enabled)
                e->draw_over(c, *this);
    }

    void BackdropBlur::collect(base& e)
    {
        if (e.user_ui && e.user_ui->backdrop_enabled)
            slot = e.user_ui->backdrop_counter++;
    }

    void BackdropBlur::draw_under(GUIInfo& c, base& e)
    {
        if (!active(e, c))
            return;

        BackdropBlurParams params;
        params.radius = e.user_ui->backdrop_radius;
        params.saturation = e.user_ui->backdrop_saturation;
        params.tint.w = e.user_ui->backdrop_tint;
        params.luminosity = e.user_ui->backdrop_luminosity;
        params.noise = e.user_ui->backdrop_noise;

        rect r = e.get_render_bounds();
        if (r.w > 0 && r.h > 0)
            c.renderer->draw_backdrop_blur(c, slot, r, params);
    }

    void SlidingHighlight::update(bool present, float target_y, float target_h, float dt, bool snap)
    {
        const float k = snap ? 1.0f : 1.0f - std::exp(-dt * 22.0f);

        if (!present)
        {
            alpha += (0.0f - alpha) * k;
            if (alpha < 0.003f)
                alpha = 0.0f;
            return;
        }

        // Nothing visible yet: appear where the row is, don't fly in from the last place.
        if (!valid || alpha == 0.0f)
        {
            y = target_y;
            h = target_h;
            valid = true;
        }
        else
        {
            y += (target_y - y) * k;
            h += (target_h - h) * k;
        }

        alpha += (1.0f - alpha) * k;
        if (alpha > 0.997f)
            alpha = 1.0f;
    }

    float Shadow::extent(const base& e) const
    {
        if (!e.user_ui->shadows_enabled)
            return 0.0f;

        return (3.0f * float(e.user_ui->shadow_blur) + float(e.user_ui->shadow_offset)) * (1.0f + 1.2f * lift) + 2.0f;
    }

    void Shadow::update(base&, float dt)
    {
        if (lift == lift_target)
            return;

        const float step = dt / 0.12f;
        lift = lift < lift_target ? std::min(lift + step, lift_target) : std::max(lift - step, lift_target);
    }

    void Shadow::draw_under(GUIInfo& c, base& e)
    {
        if (!e.user_ui || !e.user_ui->shadows_enabled)
            return;

        ShadowStyle s = style;
        s.color.w = std::min(1.0f, float(e.user_ui->shadow_opacity) * (1.0f + 0.4f * lift));
        s.blur    = float(e.user_ui->shadow_blur) * (1.0f + 0.8f * lift);
        s.offset  = float2(0.0f, float(e.user_ui->shadow_offset) * (1.0f + 1.2f * lift));
        c.renderer->draw_shadow(c, e.get_render_bounds(), s);
    }

    float Glow::extent(const base& e) const
    {
        return e.user_ui->glow_enabled ? 3.0f * float(e.user_ui->glow_size) + 2.0f : 0.0f;
    }

    void Glow::draw_under(GUIInfo& c, base& e)
    {
        if (!e.user_ui || !e.user_ui->glow_enabled)
            return;

        ShadowStyle g = style;
        g.color.w *= e.user_ui->glow_strength;
        g.blur     = e.user_ui->glow_size;
        g.offset   = float2(0.0f, 0.0f);
        c.renderer->draw_shadow(c, e.get_render_bounds(), g);
    }

    void LightOverlay::draw_over(GUIInfo& c, base& e)
    {
        auto* ui = e.user_ui;

        if (!ui || !(reveal || border || shine || ripple || focus || inner))
            return;

        if (!(hover > 0.003f || shine_t >= 0.0f || ripple_t >= 0.0f || focus_t > 0.003f
              || (inner && ui->inner_enabled)))
            return;

        FxParams p;
        p.radius = radius;
        p.light_color = light;
        const vec2 mouse = ui->get_mouse_pos();
        p.cursor = float2(mouse.x, mouse.y);

        if (reveal && ui->reveal_enabled)
        {
            p.reveal_strength = ui->reveal_strength * hover;
            p.reveal_size = ui->reveal_size;
        }

        if (border && ui->border_enabled)
        {
            p.border_strength = ui->border_strength * hover;
            p.border_size = ui->border_size;
        }

        if (shine && ui->shine_enabled && shine_t >= 0.0f)
        {
            p.shine_phase = shine_t;
            p.shine_strength = ui->shine_strength;
        }

        if (ripple && ui->ripple_enabled && ripple_t >= 0.0f)
        {
            p.ripple_origin = float2(ripple_origin.x, ripple_origin.y);
            p.ripple_phase = ripple_t;
            p.ripple_strength = ui->ripple_strength;
        }

        if (focus && ui->focus_enabled)
            p.focus_strength = ui->focus_strength * focus_t;

        if (inner && ui->inner_enabled)
        {
            p.inner_strength = ui->inner_strength;
            p.inner_size = ui->inner_size;
        }

        c.renderer->draw_fx(c, e.get_render_bounds(), p);
    }

    void LightOverlay::update(base& e, float dt)
    {
        // Only the cursor-driven parts have state to advance.
        if (!(reveal || border || shine || ripple || focus))
            return;

        auto* ui = e.user_ui;

        // Hovered when the cursor is over this element or anything inside it
        // (a button's label is what the hit test finds). Raw parent pointers:
        // see control_opacity().
        bool hovered_now = false;

        if (ui)
            if (auto h = ui->get_hovered().lock())
                for (const base* p = h.get(); p; p = p->parent)
                    if (p == &e)
                    {
                        hovered_now = true;
                        break;
                    }

        const float target = hovered_now ? 1.0f : 0.0f;
        const float previous = hover;
        hover += (target - hover) * (1.0f - std::exp(-dt * 14.0f));

        if (std::abs(target - hover) < 0.003f)
            hover = target;

        if (shine && ui)
        {
            if (previous <= 0.0f && hover > 0.0f)
                shine_t = 0.0f;
            else if (shine_t >= 0.0f)
            {
                shine_t += dt / std::max(float(ui->shine_duration), 0.05f);

                if (shine_t >= 1.0f)
                    shine_t = -1.0f;
            }
        }

        // A press starts the ripple at the cursor; it then runs out on its own.
        const bool pressed_now = e.pressed;
        if (ripple && ui)
        {
            if (pressed_now && !was_pressed)
            {
                ripple_t = 0.0f;
                ripple_origin = ui->get_mouse_pos();
            }
            else if (ripple_t >= 0.0f)
            {
                ripple_t += dt / std::max(float(ui->ripple_duration), 0.05f);

                if (ripple_t >= 1.0f)
                    ripple_t = -1.0f;
            }
        }

        was_pressed = pressed_now;

        if (focus)
        {
            const float focus_target = e.is_focused() ? 1.0f : 0.0f;
            focus_t += (focus_target - focus_t) * (1.0f - std::exp(-dt * 14.0f));

            if (std::abs(focus_target - focus_t) < 0.003f)
                focus_t = focus_target;
        }
    }

    

     void user_interface::process_ui(float delta_time)
    {
          	PROFILE(L"process_ui");
         dt = delta_time;
         THREAD_SCOPE(GUI);
		 for (int i = 0; i < 3; i++)
		 {
			 press_interpret[i] = false;
			 release_interpret[i] = false;
		 }

    	while (Events::Runner::has_tasks()) 
            {
                PROFILE(L"process_tasks");          
                Events::Runner::process_tasks();
            }
        


		 auto&& g2 = lock();
		 {
             	PROFILE(L"thinking");
		//	 auto timer = c.command_list->start(L"think");
			 for (auto t : thinking)
				 t->think(delta_time);

		 }

		 {
			 // Long press: a finger held for LONG_PRESSED_DELAY_IN_MS offers the press to its elements.
			 constexpr int64_t LONG_PRESSED_DELAY_IN_MS = 500;
			 const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

			 for (auto& pair : touch_infos)
			 {
				 const auto& id = pair.first;
				 auto& info = pair.second;

				 if (info.was_long_pressed || (now - info.start_time) < LONG_PRESSED_DELAY_IN_MS)
					 continue;

				 info.was_long_pressed = true;

				 touch tch;
				 tch.id = id;
				 tch.pos = info.pos;

				 for (auto& obj : info.pressed)
					 if (obj->on_long_pressed(tch))
						 break;
			 }
		 }
		 is_updating_layout = true;
		 {
             PROFILE(L"layout");

			 update_layout({ pos.get(), pos.get() + size.get() }, scale);
			 is_updating_layout = false;
		 }


    }



     void user_interface::collect_draw_infos()
     {
        PROFILE(L"collect_draw_infos");
        GUIInfo c;

        c.offset = { 0, 0 };
        c.window_size = scaled_size.get();
        c.scale = 1;
        c.delta_time = dt;

        draw_infos.clear();
        pre_draw_infos.clear();
        backdrop_counter = 0;
        layer_counter = 0;
        draw_recursive(c);
        drag.draw(c);
     }

     void user_interface::create_graph(Graph& graph)
     {
        // Before the tree walk: widgets request atlas glyphs from on_pre_render.
        Text::Engine::get().begin_frame();
        collect_draw_infos();

        auto& ui_ctx = graph.get_context<UIContext>();
        ui_ctx.draw_infos = std::move(draw_infos);
        ui_ctx.pre_draw_infos = std::move(pre_draw_infos);

        // Mirrored into Table::UI::UIState -- see its own comment (ui.prism) for
        // why this can't just be a UIContext read: UI_PreDraw's
        // [RenderCondition] reads it from autogen/pass_defaults.cpp, which
        // only sees Table:: contexts.
        // ::Table (not GUI::Elements::Table, a widget class) -- both named
        // Table, ambiguous by unqualified lookup inside module GUI.
        graph.get_context<::Table::UI::UIState>().UI_Passes_needed = (uint32_t)ui_ctx.pre_draw_infos.size();

        // Mirrored into Table::UI::UIRenderState -- see its own comment (ui.prism)
        // for why: UI_Render's [Multiple=16] instances read passes_needed
        // via data.pass_index instead of the old ui_ctx.setup_counter++.
        {
            const uint32_t size     = (uint32_t)ui_ctx.draw_infos.size();
            const uint32_t per_pass = ui_per_thread(size);

            // One contiguous range per instance. A group-opacity layer copies the
            // target when it opens and draws it back when it closes, so both
            // have to be recorded in the same command list: a range that would
            // end inside a layer runs on to the layer's end.
            auto& chunks = ui_ctx.chunks;
            chunks.clear();

            const uint32_t max_chunks = Passes::UI::UI_Render::MaxCount;
            uint32_t start = 0;
            int depth = 0;

            while (start < size)
            {
                uint32_t end = chunks.size() + 1 >= max_chunks ? size : std::min(start + per_pass, size);

                for (uint32_t i = start; i < end; i++)
                    if (ui_ctx.draw_infos[i].layer)
                        depth += ui_ctx.draw_infos[i].before ? 1 : -1;

                while (depth > 0 && end < size)
                {
                    if (ui_ctx.draw_infos[end].layer)
                        depth += ui_ctx.draw_infos[end].before ? 1 : -1;
                    end++;
                }

                chunks.emplace_back(start, end);
                start = end;
            }

            auto& ui_render_ctx = graph.get_context<::Table::UI::UIRenderState>();
            ui_render_ctx.per_pass      = per_pass;
            ui_render_ctx.passes_needed = (uint32_t)chunks.size();
        }

        ui_ctx.dt          = dt;
        ui_ctx.scaled_size = scaled_size.get();

        // Pick the source the UI composites onto the swapchain based on the
        // current debug output selection (Final = the final ResultTexture).
        auto debug_source = [](FrameGraph::DebugMode mode)
        {
            using DM = FrameGraph::DebugMode;
            switch (mode)
            {
            case DM::Albedo:        return FrameGraph::ResourceID::GBuffer_Albedo;
            case DM::Motion:        return FrameGraph::ResourceID::GBuffer_Speed;
            case DM::VoxelTrace:    return FrameGraph::ResourceID::VoxelDebug;
            case DM::RTX:           return FrameGraph::ResourceID::ColorOutput;
            // RTXIndirectDenoised/Preview only exist when NRD actually ran
            // this frame -- NRD_REBLUR_Execute (and NRD_GBufferPack) are
            // gated off under DLSS-RR now (it does its own reconstruction/
            // denoising, see RTXCombine's own comment) -- so under DLSS-RR
            // fall back to the raw RTX trace signal (RTXIndirectNoise, now
            // plain RGB+hitdist -- see raytracing.hlsl's
            // TraceIndirectDiffuse), the closest equivalent that's actually
            // produced. Selecting either debug entry under DLSS-RR without
            // this showed black (the resource was never created that
            // frame), not an error -- the handler just resolves to nothing.
            case DM::RTXIndirectDenoised:
            case DM::RTXIndirectDenoisedUnpacked:
                return g_upscaler_type == Post::Upscale::UpscalerType::DLSSRR
                    ? FrameGraph::ResourceID::RTXIndirectNoise
                    : (mode == DM::RTXIndirectDenoised ? FrameGraph::ResourceID::RTXIndirectDenoised
                                                        : FrameGraph::ResourceID::RTXIndirectDenoisedPreview);
            case DM::NRDViewZ:            return FrameGraph::ResourceID::NRD_ViewZ;
            case DM::NRDNormalRoughness:  return FrameGraph::ResourceID::NRD_NormalRoughness;
            case DM::RawDepthMips:        return FrameGraph::ResourceID::GBuffer_DepthMips;
            case DM::DDGIIndirect:        return FrameGraph::ResourceID::DDGIIndirectDebug;
            default:                return FrameGraph::ResourceID::ResultTexture;
            }
        };
        // UI_Render declares ResultTexture statically (ui.prism); anything other
        // than Final redirects that one field for this frame. Registered here,
        // during graph construction, because an override has to be in place
        // before setup() runs -- and re-registered every frame, since
        // start_new_frame() clears them.
        //
        // Only a non-default selection registers anything, so the normal path
        // leaves has_resource_overrides() false and stays cacheable. See that
        // function's comment (FrameGraph.Base.ixx) for what it buys.
        auto& dbg = graph.get_context<FrameGraph::DebugContext>();
        const auto debug_src = debug_source(dbg.mode);
        if (debug_src != FrameGraph::ResourceID::ResultTexture)
            graph.override_resource(FrameGraph::PassID::UI_Render,
                                    FrameGraph::ResourceID::ResultTexture, debug_src);

        {
            PROFILE(L"process_graph");
            process_graph(graph);
        }
     }


    // hovered_controls/pressed only keep weak refs so a control that disappears between
    // hit-test and dispatch (owner destroyed, removed from tree) is detected via lock()
    // failing rather than dispatched to a dangling object; on that, re-hit-test at pos
    // in place instead of silently dropping the event.
    static std::vector<base::wptr> to_weak_controls(const std::vector<base::ptr>& v)
    {
        std::vector<base::wptr> res;
        res.reserve(v.size());

        for (auto& p : v)
            res.emplace_back(p);

        return res;
    }

    // Touch dispatch, ported from the NDK GUI. It does not go through the mouse handlers: each finger is
    // tracked in touch_infos with the elements it pressed, those get on_touch_action(DOWN / UP / CANCEL)
    // and on_touch_move, and a finger held long enough offers a long press (process_ui).
    bool user_interface::on_touch_event(const touch_event& event)
    {
        auto&& g = lock();   // touch events arrive on the platform thread, like the mouse ones

        bool catched = false;

        if (event.flags == touch_event::UP)
        {
            for (auto& e : event.touches)
            {
                vec2 pos = { e.x, e.y };
                auto& touch_info = touch_infos[e.id];

                // Ends a drag started by this finger (the element is what the drag keys on).
                drag.mouse_action_event(touch_info.pressed.empty() ? nullptr : touch_info.pressed.front(), mouse_action::UP, mouse_button::LEFT, pos);

                touch t;
                t.id = e.id;
                t.pos = pos;

                for (unsigned int i = 0; i < touch_info.pressed.size(); i++)
                {
                    base::ptr control = touch_info.pressed[i];

                    if (!control->user_ui)
                        continue;

                    control->on_touch_action(control->point_in(pos) ? mouse_action::UP : mouse_action::CANCEL, t);
                    catched = true;
                }

                touch_infos.erase(e.id);
            }
        }

        if (event.flags == touch_event::DOWN)
        {
            for (auto& e : event.touches)
            {
                vec2 pos = { e.x, e.y };

                auto& touch_info = touch_infos[e.id];
                touch_info.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                touch_info.pos = pos;
                touch_info.was_long_pressed = false;

                auto hovered_controls = find_control(pos, true);

                touch t;
                t.id = e.id;
                t.pos = pos;
                touch_info.pressed.clear();

                for (auto& control : hovered_controls)
                {
                    touch_info.pressed.push_back(control);

                    if (!control->on_touch_action(mouse_action::DOWN, t))
                        continue;

                    catched = true;
                    control->on_touch();
                    drag.mouse_action_event(control, mouse_action::DOWN, mouse_button::LEFT, pos);

                    struct smallfuncs
                    {
                        inline static void close_menus_in_children(base::ptr obj)
                        {
                            auto copy = obj->childs;

                            for (auto& child : copy)
                                child->close_menus();
                        }

                        inline static void close_menus_in_all_neighbors_of_all_parents(base::ptr obj)
                        {
                            base::ptr parent = obj->get_parent();

                            while (parent)
                            {
                                auto copy = parent->childs;

                                for (auto& child : copy)
                                    if (child != obj)
                                        child->close_menus();

                                obj = parent;
                                parent = obj->get_parent();
                            }
                        }

                        inline static void close_menus(base::ptr obj)
                        {
                            close_menus_in_children(obj);
                            close_menus_in_all_neighbors_of_all_parents(obj);
                        }
                    };

                    smallfuncs::close_menus(control);

                    break;
                }
            }
        }

        if (event.flags == touch_event::MOVE)
        {
            if (drag.pressed.lock())
            {
                for (auto& e : event.touches)
                {
                    vec2 pos = { e.x, e.y };
                    auto drag_hovered_controls = find_control(pos, true);
                    drag.mouse_move_event(drag_hovered_controls.empty() ? nullptr : drag_hovered_controls.front(), pos);
                }
            }

            // Reserved up front: touch_ids keeps pointers into this vector, so it must not reallocate.
            std::vector<touch> touches;
            touches.reserve(event.touches.size());
            std::map<int, touch*> touch_ids;

            // The original ordered pressed elements by a draw-order id (topmost last, walked in reverse).
            // Each finger's pressed list is already front-most first, so walk those in order instead.
            std::vector<base*> ordered;

            for (auto& e : event.touches)
            {
                vec2 pos = { e.x, e.y };
                auto& touch_info = touch_infos[e.id];
                touch_info.pos = pos;
                touch t;
                t.id = e.id;
                t.pos = pos;
                touches.push_back(t);
                touch_ids[e.id] = &touches.back();
            }

            std::map<base*, std::vector<touch*>> to_send;

            for (auto& e : touch_infos)
                for (auto& t : e.second.pressed)
                    if (std::find(ordered.begin(), ordered.end(), t.get()) == ordered.end())
                        ordered.push_back(t.get());

            for (auto& e : event.touches)
            {
                auto& touch_info = touch_infos[e.id];
                touch* t = touch_ids[e.id];

                for (auto& i : touch_info.pressed)
                    to_send[i.get()].push_back(t);
            }

            for (base* elem : ordered)
            {
                auto& touch_data = to_send[elem];

                touch_data.erase(std::remove_if(touch_data.begin(), touch_data.end(), [this, &elem](touch* t) {
                    if (t->catched)
                    {
                        elem->on_touch_action(mouse_action::CANCEL, *t);

                        auto& infos = touch_infos[t->id];

                        infos.pressed.erase(std::remove_if(infos.pressed.begin(), infos.pressed.end(), [&elem](const base::ptr& b) {
                            return elem == b.get();
                        }), infos.pressed.end());
                    }

                    return t->catched;
                }), touch_data.end());

                if (!touch_data.empty())
                {
                    elem->on_touch_move(touch_data);
                    catched = true;
                }
            }
        }

        return catched;
    }

    void user_interface::mouse_action_event_internal(mouse_action action, mouse_button button, vec2 pos)
    {
     //   cursor = cursor_style::ARROW;
        {
       //     std::lock_guard<std::mutex> g(m);
            auto&& g2 = lock();
            auto i_button = static_cast<int>(button);

            if (action == mouse_action::DOWN)
            {
                if (press_interpret[i_button])
                {
                    popup_log("DOWN ignored: a press was already interpreted this frame");   // TEMP-POPUPLOG
                    return;
                }

                press_interpret[i_button] = true;
            }

            if (action == mouse_action::UP)
            {
                if (release_interpret[i_button])
                    return;

                release_interpret[i_button] = true;
            }

            std::vector<base::ptr> controls;
            controls.reserve(hovered_controls.size());

            for (auto& w : hovered_controls)
            {
                base::ptr c = w.lock();

                if (!c)
                {
                    hovered_controls = to_weak_controls(find_control(pos, true));
                    controls.clear();
                    controls.reserve(hovered_controls.size());

                    for (auto& w2 : hovered_controls)
                        if (auto c2 = w2.lock())
                            controls.push_back(c2);

                    break;
                }

                controls.push_back(c);
            }

            auto m_focus = mouse_focus.lock();

            if (m_focus)
                controls.push_back(m_focus);

            if (action == mouse_action::DOWN)   // TEMP-POPUPLOG
            {
                std::string line = "DOWN at (" + std::to_string(int(pos.x)) + "," + std::to_string(int(pos.y)) + ") controls=" + std::to_string(controls.size()) + " [";
                for (auto& c : controls)
                    line += std::string(typeid(*c).name()) + (c->is_menu_component() ? "(menu) " : " ");
                popup_log(line + "] ui children=" + std::to_string(childs.size()));
            }

            if (controls.size() == 0)
            {
                // A press on nothing clickable still dismisses open menus and popups.
                if (action == mouse_action::DOWN)
                {
                    popup_log("  no controls: close_menus()");   // TEMP-POPUPLOG
                    close_menus();
                }

                return;
            }

            if (action == mouse_action::DOWN)
            {
                pressed[i_button].clear();
                const auto focused_before = focused.lock();
                base::ptr handled;

                for (unsigned int i = 0; i < controls.size(); i++)
                {
                    base::ptr control = controls[i];
                    pressed[i_button].push_back(control);

                    const bool took_it = control->on_mouse_action(action, button, pos);
                    popup_log("  " + std::string(typeid(*control).name()) + " on_mouse_action -> " + (took_it ? "handled" : "not handled"));   // TEMP-POPUPLOG

                    if (!took_it)
                        continue;

                    if (!control->is_menu_component())
                    {
                        popup_log("  not a menu component: close_menus()");   // TEMP-POPUPLOG
                        close_menus();
                    }
                    else
                        popup_log("  menu component: menus stay");   // TEMP-POPUPLOG
                    control->on_touch();
                    cursor = control->update_cursor();
                    drag.mouse_action_event(control, action, button, pos);
                    handled = control;
                    break;
                }

                if (!handled)
                    popup_log("  nobody handled the press (no close)");   // TEMP-POPUPLOG

                // A press anywhere else takes keyboard focus away: only fields
                // call focus(), so clicking a button or empty space used to
                // leave the last field focused and still receiving keys (and a
                // field that commits on focus loss never committing). Menus
                // keep it -- the edit field's own context menu acts on it.
                if (focused_before && focused.lock() == focused_before
                    && std::find(controls.begin(), controls.end(), focused_before) == controls.end()
                    && !(handled && handled->is_menu_component()))
                    focused_before->unfocus();
            }

            if (action == mouse_action::UP)
            {
                for (unsigned int i = 0; i < pressed[i_button].size(); i++)
                {
                    base::ptr control = pressed[i_button][i].lock();

                    if (!control)
                        continue;

                    if (controls.size() > i && control == controls[i])
                        control->on_mouse_action(action, button, pos);
                    else
                        control->on_mouse_action(mouse_action::CANCEL, button, pos);
                }

                if (pressed[i_button].size())
                {
                    if (auto last = pressed[i_button].back().lock())
                        drag.mouse_action_event(last, action, button, pos);
                }
            }

            if (action == mouse_action::CANCEL)
            {
                for (unsigned int i = 0; i < pressed[i_button].size(); i++)
                {
                    base::ptr control = pressed[i_button][i].lock();

                    if (control)
                        control->on_mouse_action(action, button, pos);
                }

                if (pressed[i_button].size())
                {
                    if (auto last = pressed[i_button].back().lock())
                        drag.mouse_action_event(last, action, button, pos);
                }
            }
        }
        mouse_move_event(pos);

    }

    void user_interface::mouse_move_event_internal(vec2 pos)
    {
    //    std::lock_guard<std::mutex> g(m);
        auto&& g2 = lock();
        last_mouse_pos = pos;
        /*	if (movable)
        	{

        		movable->on_mouse_move(pos);
        		movable->update_cursor();
            }*/
        hovered_controls = to_weak_controls(find_control(pos, true));
        auto t = mouse_focus.lock();

        if (t)
        {
            hovered = t;
            t->on_mouse_move(pos);
           cursor =  t->update_cursor();
            drag.mouse_move_event(t, pos);
            return;
        }

        size_t remove_index = hovered_controls.size();

        for (size_t i = 0; i < hovered_controls.size(); i++)
        {
            base::ptr control = hovered_controls[i].lock();

            if (!control)
                continue;

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

            cursor = control->update_cursor();
            drag.mouse_move_event(control, pos);
            remove_index = i + 1;
            break;
        }

        for (size_t i = remove_index; i < hovered_controls.size(); i++)
        {
            base::ptr control = hovered_controls[i].lock();

            if (!control)
                continue;

            for (size_t i_button = 0; i_button < 3; i_button++)
            {
                for (size_t j = 0; j < pressed[i_button].size(); j++)
                {
                    if (pressed[i_button][j].lock() == control)
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

    }

    user_interface::user_interface(): drag(this)
    {
        THREAD_SCOPE(GUI);
        docking = dock::FILL;
        user_ui = this;
        back.reset(new dark());
        user_interface::add_child(back);
        back->visible = false;
        add_child(drag.holder);
    }


    void user_interface::remove_base(base* object)
    {
		auto frame_gen = dynamic_cast<GraphGenerator*>(object);
		if (frame_gen)
			frame_generators.erase(frame_gen);
    }

    void user_interface::add_base(base* object)
    {
        //   components.insert(object);
        auto frame_gen = dynamic_cast<GraphGenerator*>(object);
        if (frame_gen)
            frame_generators.insert(frame_gen);
    }
	void user_interface::key_action_event_internal(key_action action, long key, key_mods mods)
	{
		auto really_focused = focused.lock();

		if (really_focused)
			really_focused->on_key_action(action, key, mods);
	}

	void user_interface::char_event_internal(char32_t ch)
	{
		auto really_focused = focused.lock();

		if (really_focused)
			really_focused->on_char(ch);
	}
    
	void user_interface::mouse_move_event(vec2 pos)
	{
		run_on_ui([this, pos]()
			{
				mouse_move_event_internal(pos);
			});

		apply_cursor();
	}

	void user_interface::apply_cursor()
	{
		::SetCursor(LoadCursor(nullptr, cursors[static_cast<int>(cursor)]));
	}

	void user_interface::mouse_action_event(mouse_action action, mouse_button button, vec2 pos)
	{
		run_on_ui([this, action, button, pos]()
			{
				mouse_action_event_internal(action, button, pos);
			});
		::SetCursor(LoadCursor(nullptr, cursors[static_cast<int>(cursor)]));
	}
	
	void user_interface::mouse_wheel_event(mouse_wheel type, float value, vec2 pos)
	{
		run_on_ui([this, type, value, pos]()
			{
				mouse_wheel_event_internal(type, value, pos);
			});
		::SetCursor(LoadCursor(nullptr, cursors[static_cast<int>(cursor)]));
	}

    void user_interface::key_action_event(key_action action, long key, key_mods mods)
    {
        run_on_ui([this, action, key, mods]()
        {
                key_action_event_internal(action, key, mods);
        });
    }

    void user_interface::char_event(char32_t ch)
    {
        run_on_ui([this, ch]()
        {
                char_event_internal(ch);
        });
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
            exit_window->layer_opacity = 0.0f;
            exit_window->fade_to(1.0f, fade_in_duration);
            add_child(exit_window);
        });
        return pr->get_future();
    }

    void user_interface::on_size_changed(const vec2& r)
    {
        base::on_size_changed(r);
        back->size = r;
    }

    void user_interface::mouse_wheel_event_internal(mouse_wheel type, float value, vec2 pos)
    {
        bool refreshed = false;

        for (int i = (int)hovered_controls.size() - 1; i >= 0; i--)
        {
            base::ptr control = hovered_controls[i].lock();

            if (!control)
            {
                if (refreshed)
                    continue;

                refreshed = true;
                hovered_controls = to_weak_controls(find_control(pos, true));
                i = (int)hovered_controls.size();
                continue;
            }

            if (control->on_wheel(type, value, pos))
                break;
        }
    }


    void dark::draw(Context& c)
    {
        c.renderer->draw_color(c, vec4(0.0, 0.0, 0.0, 0.5f), get_render_bounds());
    }

    drag_n_drop::~drag_n_drop()
    {
    }

    void drag_n_drop::draw(GUIInfo& c)
    {
        if (!dragging) return;

        auto p = pressed.lock();

        if (!p)
            return;

        c.offset = cur_pos - start_pos;

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

        // generate_container decides what the holder shows; the default and
        // tab_button re-parent the element itself (and reset its pos), while
        // e.g. edit_text only puts a preview of the dragged text there.
        if (!elem->drag_n_drop_copy)
        {
            elem->generate_container(holder);
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

GUIInfo::GUIInfo(HAL::CommandList::ptr list) : command_list(list)
{
    delta_time = 0;
}

namespace GUI
{
    Texture::Texture()
    {
        margins = { 0, 0, 0, 0 };
        padding = { 0, 0, 0, 0 };
        tc = { 0, 0, 1, 1 };
        mul_color = { 1,1,1,1 };
        add_color = { 0,0,0,0 };
    }

    void Texture::operator=(const HAL::Texture2DView& tex)
    {
        this->texture = tex;
    }

    ParameterHolder::~ParameterHolder()
    {
    }

    void base::pre_draw(HAL::CommandList::ptr list)
    {
    }

    void base::generate_container(base::ptr obj)
    {
        obj->add_child(get_ptr());
        pos = { 0, 0 };
    }

    void base::on_pre_render(Context& context)
    {
    }

    std::vector<base::ptr> drag_holder::find_control(vec2 at, bool click_only)
    {
        return std::vector<base::ptr>();
    }

    drag_holder::drag_holder()
    {
        docking = dock::NONE;
        width_size = size_type::FIXED;
        height_size = size_type::FIXED;
    }

    drag_n_drop::drag_n_drop(user_interface* ui) : user_ui(ui)
    {
        holder.reset(new drag_holder);
        //  holder->visible = false;
    }

    user_interface::~user_interface()
    {
        remove_all();
    }

    void user_interface::add_task(std::function<void()> f)
    {
        Events::Runner::run(f);
    }

    void user_interface::process_graph(FrameGraph::Graph& graph)
    {
        for (auto& gen : frame_generators)
        {
            gen->generate(graph);
        }
    }

    void user_interface::run_pre_draw(std::vector<base*>& infos, HAL::CommandList::ptr list)
    {
        for (auto& e : infos)
            e->pre_draw(list);
    }
}




// ============================================================
// PassDefault<Passes::UI::UI_PreDraw>
// ============================================================

// setup() is fully generated (ui.prism's own [RenderCondition]).
// UI_PreDraw_Sync's ResourceChain must reset every frame (create() is the
// only thing that calls reset_frame()) even with nothing to pre-draw, or
// exists() keeps reporting true from a prior frame and UI_Render's need()
// keeps appending onto an un-cleared states list -- [Always]/[Size] on the
// field itself handles that create() unconditionally; [RenderCondition]
// alone (no [SetupCondition]) just skips render() on empty frames.

void PassDefault<Passes::UI::UI_PreDraw>::render(
    Passes::UI::UI_PreDraw::Context& data, FrameGraph::FrameContext& context)
{
    auto& ui_ctx = context.graph->get_context<GUI::UIContext>();
    auto command_list = context.get_list();

    PROFILE(L"pre_draw");
    GUI::user_interface::run_pre_draw(ui_ctx.pre_draw_infos, command_list);
}


// ============================================================
// PassDefault<Passes::UI::UI_Render>
// ============================================================

static uint32_t ui_per_thread(uint32_t size)
{
	constexpr uint32_t max_threads = Passes::UI::UI_Render::MaxCount;
	constexpr uint32_t min_per_thread = 64;
	const uint32_t per_thread = (size + max_threads - 1) / max_threads;
	const uint32_t clamped_per_thread = std::max(min_per_thread, per_thread);

    return std::max(clamped_per_thread, (size + 7) / 8);
}

// setup() is fully generated (ui.prism's own [SetupCondition] on data.pass_index,
// plus [NeedDynamic] for the debug-view source -- the one resource here whose
// identity is picked at runtime).

void PassDefault<Passes::UI::UI_Render>::render(
    Passes::UI::UI_Render::Context& data, FrameGraph::FrameContext& context)
{
    // UI_Render reads cache state (label cache textures, etc.) that UI_PreDraw
    // writes. The FrameGraph's shared-resource dependency edge (see
    // PassDefault<Passes::UI::UI_PreDraw>::setup) only drives GPU barrier ordering,
    // not CPU dispatch order — Graph::render() enqueues every pass's render()
    // without waiting on prev_passes. Wait on it explicitly here instead.
    if (auto* pre_draw = context.graph->builder.get_pass(Passes::UI::UI_PreDraw::Name))
        pre_draw->wait();

    auto& ui_ctx = context.graph->get_context<GUI::UIContext>();

    // This instance's range, chosen in create_graph so no layer is split.
    if (data.pass_index >= ui_ctx.chunks.size())
        return;

    const uint32_t start     = ui_ctx.chunks[data.pass_index].first;
    const uint32_t end       = ui_ctx.chunks[data.pass_index].second;

    auto command_list = context.get_list();
    auto texture = (*data.swapchain);

    {
        PROFILE(L"setup_rt");
        RT::Frame::SingleColor rt;
        rt.GetColor() = texture.renderTarget;
        const auto rt_options = (data.pass_index == 0)
            ? HAL::RTOptions::Default | HAL::RTOptions::ClearColor
            : HAL::RTOptions::Default;
        command_list->get_graphics().set_rtv(rt, rt_options);
    }

    const float ui_scale = [&]
    {
        const auto& display = context.graph->get_context<::Table::UI::DisplayState>();
        return display.hdr ? display.sdr_white_nits / 80.0f : 1.0f;
    }();

    {
        Slots::UI::DisplayOutput output;
        output.GetUi_scale() = ui_scale;
        command_list->get_graphics().set(output);
    }

    Renderer renderer;
    GUIInfo c;
    c.renderer   = &renderer;
    c.command_list = command_list;
    c.target     = texture;
    c.ui_scale   = ui_scale;
    c.delta_time = ui_ctx.dt;
    // data.ResultTexture, not a context lookup: need() resolved this field to
    // whatever the override selected (or to ResultTexture itself), so setup and
    // render read the same resource by construction rather than by both
    // agreeing to consult the same handle.
    if (data.ResultTexture)
        c.result_texture_srv = *data.ResultTexture;

    {
        PROFILE(L"draw_elements");
        PROFILE_GPU(L"draw");
        for (uint32_t i = start; i < end; i++)
        {
            auto& e = ui_ctx.draw_infos[i];
            if (c.scissors != e.scissors)
            {
                renderer.flush(c);
                command_list->get_graphics().set_scissors(e.scissors);
            }
            c.scale       = e.scale;
            c.ui_clipping = e.clip;
            c.offset      = e.offset;
            c.scissors    = e.scissors;
            c.window_size = ui_ctx.scaled_size;
            c.layer         = e.layer;
            c.layer_slot    = e.layer_slot;
            c.layer_opacity = e.layer_opacity;

            // The element is drawn where it was when this entry was collected,
            // not where the GUI thread has since moved it.
            draw_bounds_scope bounds_scope(e.elem, e.bounds);

            if (e.before)
                e.elem->draw_before(c);
            else
                e.elem->draw_end(c);
        }
    }

    {
        PROFILE(L"flush");
        renderer.flush(c);
    }
}