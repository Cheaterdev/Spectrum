module GUI:ScrollContainer;
import :Renderer;


class c_contents : public GUI::base
{
        GUI::Elements::scroll_container* owner;
    public:
        c_contents(GUI::Elements::scroll_container* owner)
        {
            this->owner = owner;
            clickable = false;
        }

        virtual void on_size_changed(const vec2& r) override
        {
            base::on_size_changed(r);
            owner->resized();
        }
        /*
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
        	return 	owner->on_mouse_action(action, button, pos);
        }
        virtual void on_key_action(key_action action, long key, key_mods mods) override
        {
        	owner->on_key_action(action, key, mods);
        }
        virtual bool on_mouse_move(vec2 pos) override
        {
        	return 	owner->on_mouse_move(pos);
        }*/
};


class mover : public GUI::base
{
        GUI::Elements::scroll_container* owner;
        bool dragging = false;
        bool pressed = false;
        vec2 start_pos;
        vec2 prev_pos;
        vec2 speed;
//	float inertion;
    public:
        mover(GUI::Elements::scroll_container* owner)
        {
            this->owner = owner;
          //  thinkable = true;
        }

        virtual bool on_mouse_move(vec2 pos) override
        {
            if (pressed && !dragging && (pos - start_pos).length() > 5)
            {
                dragging = true;
                prev_pos = pos;
                set_movable(true);
            }

            if (dragging)
            {
                speed = pos - prev_pos;
                owner->moving(speed / owner->result_scale);
                prev_pos = pos;
            }

            return dragging;
        }
        virtual bool on_wheel(mouse_wheel type, float value, vec2 pos) override
        {
            if (type == mouse_wheel::VERTICAL)
                owner->on_wheel(type, value, pos);

            return true;
        };

        // Touch: one finger drags the content and, on release, flings it; two fingers pan and pinch.
        // Fingers that never leave the slop radius stay taps for the elements below.
        std::map<int, vec2> poses;
        bool touch_dragging = false;
        bool pinching = false;
        float pinch_prev = 0;
        vec2 pinch_center;
        vec2 velocity;
        std::chrono::steady_clock::time_point last_move;

        virtual bool on_touch_action(mouse_action action, GUI::touch& t) override
        {
            if (action == mouse_action::DOWN)
            {
                poses[t.id] = t.pos;
                owner->speed = { 0, 0 };
                owner->fling_velocity = { 0, 0 };
                velocity = { 0, 0 };
                return false;
            }

            poses.erase(t.id);
            pinch_prev = 0;

            if (poses.empty())
            {
                if (pinching)
                    owner->on_pinch(pinch_center, 1.0f, true);

                if (touch_dragging && !pinching && (std::chrono::steady_clock::now() - last_move) < std::chrono::milliseconds(80))
                    owner->fling_velocity = velocity * 0.4f;

                touch_dragging = false;
                pinching = false;
            }

            return false;
        }

        virtual void on_touch_move(std::vector<GUI::touch*>& e) override
        {
            const float scale = result_scale;
            auto now = std::chrono::steady_clock::now();

            if (e.size() >= 2)
            {
                vec2 a = e[0]->pos, b = e[1]->pos;
                vec2 center = (a + b) * 0.5f;
                float dist = (a - b).length();

                if (pinch_prev > 0)
                {
                    owner->moving((center - pinch_center) / scale);
                    if (dist > 1.0f)
                        owner->on_pinch(center, dist / pinch_prev, false);
                }

                pinch_prev = dist;
                pinch_center = center;
                pinching = touch_dragging = true;

                for (auto t : e)
                {
                    poses[t->id] = t->pos;
                    t->catched = true;
                }
                return;
            }

            auto t = e[0];

            if (!touch_dragging)
            {
                if ((t->pos - poses[t->id]).length() > 8 * scale)
                {
                    touch_dragging = true;
                    poses[t->id] = t->pos;
                    last_move = now;
                    t->catched = true;
                }
                return;
            }

            vec2 delta = (t->pos - poses[t->id]) / scale;
            poses[t->id] = t->pos;
            t->catched = true;

            float dt = std::chrono::duration<float>(now - last_move).count();
            last_move = now;

            owner->moving(delta);

            if (dt > 1e-4f)
                velocity = velocity * 0.6f + (delta / dt) * 0.4f;
        }
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
        {
            if (button == mouse_button::RIGHT)
            {
                pressed = action == mouse_action::DOWN;
                start_pos = pos;

                if (!pressed)
                {
                    dragging = false;
                    set_movable(false);
                }
            }

            return false;
        }


        virtual void on_key_action(key_action action, long key, key_mods mods) override
        {
        }


};



sizer GUI::Elements::scroll_container::update_layout(sizer r, float scale)
{
    resized();
    return base::update_layout(r, scale);
}

void GUI::Elements::scroll_container::resized()
{
    hor->set_sizes(filled->get_render_bounds().w, contents->scaled_size->x, contents->pos->x);
    vert->set_sizes(filled->get_render_bounds().h, contents->scaled_size->y, contents->pos->y);

    if (!allow_overflow)
        contents->pos = vec2::min(vec2(0, 0), vec2::max(contents->pos.get(), -contents->scaled_size.get() + vec2(filled->get_render_bounds().size)));

    // contents is the only unclamped measure of the real content size, so it -- not
    // filled (which clamp_to_parent shrinks to fit us) -- is what we size ourselves
    // from. The read is stale on the frame the container is first laid out, but
    // c_contents::on_size_changed re-enters resized() as soon as contents computes
    // its size, which repairs it within the same frame.
    if (auto_size)
        size = float2{ size->x, contents->scaled_size->y + contents->margin->top + contents->margin->bottom + padding->top + padding->bottom };
}

void GUI::Elements::scroll_container::remove_child(base::ptr obj)
{
    contents->remove_child(obj);
}

void GUI::Elements::scroll_container::add_child(base::ptr obj)
{
    contents->add_child(obj);
}

GUI::Elements::scroll_container::scroll_container()
{
    thinkable = true;
    docking = GUI::dock::FILL;
    vert.reset(new GUI::Elements::scroll_bar(GUI::Elements::scroll_bar::scroll_type::VERTICAL));
    vert->docking = GUI::dock::RIGHT;
    base::add_child(vert);
    hor.reset(new GUI::Elements::scroll_bar(GUI::Elements::scroll_bar::scroll_type::HORIZONTAL));
    hor->docking = GUI::dock::BOTTOM;
    base::add_child(hor);
    filled.reset(new c_contents(this));
    filled->docking = GUI::dock::FILL;
    base::add_child(filled);
    over_filled.reset(new mover(this));
    over_filled->docking = GUI::dock::FILL;
    contents.reset(new c_contents(this));
    contents->docking = GUI::dock::NONE;
    contents->size = { 00, 00 };
    contents->clamp_to_parent = ParentClamp::NONE;
    contents->width_size = size_type::MATCH_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;
    filled->add_child(contents);
    filled->add_child(over_filled);
    vert->on_move = [this](float y)
    {
        contents->pos = { contents->pos->x, -y* (contents->scaled_size->y - filled->get_render_bounds().h) };
    };
    hor->on_move = [this](float x)
    {
        contents->pos = { -x* (contents->scaled_size->x - filled->get_render_bounds().w), contents->pos->y };
    };
}

void GUI::Elements::scroll_container::moving(vec2 pos)
{
    vec2 p = contents->pos.get() + pos;

    if (!allow_overflow)
        contents->pos = vec2::min(vec2(0, 0), vec2::max(p, -contents->scaled_size.get() + vec2(filled->get_render_bounds().size)));
    else
        contents->pos = p;
}


bool GUI::Elements::scroll_container::on_wheel(mouse_wheel type, float value, vec2 pos)
{
    speed.y += value * 20;
    return true;
}

void GUI::Elements::scroll_container::think(float dt)
{
    base::think(dt);
    if (fling_velocity.length() > 8.0f)
    {
        moving(fling_velocity * dt);
        fling_velocity *= std::exp(-14 * dt);
    }

    if (speed.length() > 0.3f)
    {
        moving(100 * speed * dt);
        speed *= std::exp(-20 * dt);
    }
}
