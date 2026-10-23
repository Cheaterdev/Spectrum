export module GUI:Base;

import Core;
import windows;
import FrameGraph;
import HAL;

export
{
    namespace GUI
    {
        class Renderer;
    }

    struct GUIInfo
    {

	    GUI::Renderer* renderer;
        HAL::CommandList::ptr command_list;

        sizer_long ui_clipping;
        sizer_long scissors;
        vec2 offset = { 0,0 };
        float delta_time;
        vec2 window_size;
        float scale = 1;
        HAL::Texture2DView result_texture_srv;

        // What the UI is drawing into. Backdrop effects snapshot it mid-pass;
        // an empty view (tests, previews that render the UI elsewhere) makes
        // them fall back to their flat tint.
        HAL::Texture2DView target;
        float ui_scale = 1;

        // Group opacity of the entry being drawn, from its draw_info.
        bool  layer = false;
        uint32_t layer_slot = 0;
        float layer_opacity = 1.0f;

        GUIInfo() = default;
        GUIInfo(HAL::CommandList::ptr list);
    };
enum class mouse_button : int
{
    LEFT,
    MIDDLE,
    RIGHT,
    UNKNOWN
};

enum class mouse_action : int
{
    DOWN,
    UP,
    CANCEL
};

enum class key_action : int
{
    DOWN,
    UP
};

// Captured on the window thread with the key message: key state queried later
// on the UI thread would be the state at processing time, not at the keypress.
struct key_mods
{
    bool shift = false;
    bool ctrl  = false;
    bool alt   = false;
};


enum class mouse_wheel : int
{
    VERTICAL,
    HORIZONTAL
};



// For a window that draws its own title bar: what a point of the client area counts as
// when the window frame asks (WM_NCHITTEST). client = the ordinary UI.
enum class window_hit : int
{
    client,
    caption,    // drags the window, a double click maximises it
    minimize,
    maximize,
    close
};

// What the UI's own minimise / maximise / close buttons ask the window to do.
enum class window_command : int
{
    minimize,
    maximize_restore,
    close
};

struct InputHandler
{
	virtual	void mouse_move_event(vec2 pos) = 0;
	virtual	void mouse_action_event(mouse_action action, mouse_button button, vec2 pos) = 0;
	virtual	void mouse_wheel_event(mouse_wheel wheel, float value, vec2 pos) = 0;

	virtual	void key_action_event(key_action action, long key, key_mods mods) = 0;
	virtual	void char_event(char32_t ch) = 0;

	// Puts the hovered element's cursor on screen. Called from WM_SETCURSOR as well as after input,
	// so the window's default arrow never gets a frame in between.
	virtual	void apply_cursor() {}

	// Called on the window thread, from WM_NCHITTEST, for a client-area point. Must not
	// touch the UI tree: it answers from what the last drawn frame recorded.
	virtual	window_hit hit_test_window(vec2 pos) { return window_hit::client; }
};

namespace GUI
{
    class user_interface;

    namespace offset
    {
         const vec2 CENTER = {0.5,0.5};
         const vec2 LEFT_TOP(0, 0);
         const vec2 LEFT_DOWN(0, 1);
         const vec2 RIGHT_TOP(1, 0);
         const vec2 RIGHT_DOWN(1, 1);
         const vec2 LEFT_CENTER(0, 0.5);
         const vec2 RIGHT_CENTER(1, 0.5);

         const vec2 TOP_CENTER(0.5, 0);
         const vec2 DOWN_CENTER(0.5, 1);
    };

  

    enum class cursor_style : int
    {
        ARROW,
        BEAM,
        NS,
        WE,
        NWSE,
        NESW,
        ALL,
        NO,
        WAIT,
        HAND
    };

    enum class dock : int
    {
        NONE,
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
        FILL,
        PARENT,
        MAGNET_CENTER
    };

    enum class size_type : int
    {
        NONE,
        FIXED,
        MATCH_CHILDREN,
        MATCH_PARENT,
        MATCH_PARENT_CHILDREN,
        SQUARE
    };

    enum class pos_x_type : int
    {
        LEFT,
        CENTER,
        RIGHT
    };

    enum class pos_y_type : int
    {
        TOP,
        CENTER,
        BOTTOM
    };


    enum class ParentClamp
        :int
    {
        NONE = 0, WIDTH = 1, HEIGHT = 2, ALL = 3
    };



    // A soft procedural shadow or glow around an element's rounded rect,
    // drawn outside the shape only (see Renderer::draw_shadow). Sizes are in
    // UI pixels at scale 1; color is sRGB, alpha = strength.
    struct ShadowStyle
    {
        float4 color   = { 0, 0, 0, 0.5f };
        float2 offset  = { 0, 0 };
        float  blur    = 12.0f;   // Gaussian sigma
        float  spread  = 0.0f;    // grows the shape before blurring
        float  radius  = 4.0f;    // corner radius
    };

    // Procedural light effects on an element's rounded rect (Renderer::draw_fx),
    // drawn over its own draw() and under its children. Which ones apply is per
    // element; strengths and sizes come from the UI Effects settings.
    struct OverlayFx
    {
        bool  reveal = false;   // soft light following the cursor while hovered
        bool  border = false;   // lit rim, brightest near the cursor
        bool  shine  = false;   // one diagonal sweep when the hover starts
        bool  ripple = false;   // a wave from the press point
        bool  focus  = false;   // accent ring while the element has keyboard focus
        bool  inner  = false;   // darkening just inside the edge (depth)
        float radius = 4.0f;    // corner radius
        float3 light = { 1.0f, 1.0f, 1.0f };   // sRGB colour of reveal / edge light / shine; tint it on light surfaces
    };

    // An eased highlight bar for single selection in lists and trees: it slides from
    // the old row to the new one instead of jumping, fades out when nothing is
    // selected, and appears in place the first time. y and h are relative to the
    // scrolled content's origin, so scrolling moves the bar with its rows (easing
    // them in screen space would make it lag behind).
    struct SlidingHighlight
    {
        float y = 0.0f, h = 0.0f;
        float alpha = 0.0f;       // 0..1, eased
        bool  valid = false;

        // `snap` skips the easing (animations off).
        void update(bool present, float target_y, float target_h, float dt, bool snap);
        // The selected row itself moved (a tree row pushed by an expanding branch):
        // the bar goes with it at once instead of trailing a few frames behind.
        void follow(float target_y, float target_h) { y = target_y; h = target_h; }
    };

    class base;

    // One visual effect of an element: backdrop blur, shadow, glow, light overlay.
    // An element holds an ordered stack of them (base::add_effect) -- most hold
    // none -- and base::draw_before runs them around the element's own draw():
    //   draw_under  before draw(), in stack order (blur first, then shadow: the blur
    //               snapshot reads a little beyond the element and a shadow already
    //               drawn there would be blurred into the glass),
    //   draw()      the element itself,
    //   draw_over   after draw(), under the children (labels stay readable).
    // collect() and update() run on the thread that builds the draw list, once per
    // visible element per frame; draw_* run on the render threads.
    // Group opacity (fades) is not an effect: it wraps the whole sequence below and
    // one layer must stay inside one command list, so it stays on the element.
    class effect
    {
        public:
            bool enabled = true;
            virtual ~effect() = default;

            virtual void  collect(base&) {}
            virtual void  update(base&, float /*dt*/) {}
            // How far outside the render bounds it draws, in UI px (sizes the fade layer).
            virtual float extent(const base&) const { return 0.0f; }
            virtual void  draw_under(GUIInfo&, base&) {}
            virtual void  draw_over(GUIInfo&, base&) {}
    };

    // Blurs what is behind the element's render bounds. Skin shadows are drawn as
    // margins outside the bounds, so the bounds are the shape.
    class BackdropBlur : public effect
    {
        public:
            // Only blur when no ancestor already does: for plain panels that are
            // their own frosted surface when standalone but just part of an
            // already back-blurred window otherwise.
            bool standalone_only = false;
            // For panels that are mostly covered by their own controls: while the
            // blur is active, the controls under it (buttons, combo boxes) draw
            // their backgrounds at glass_opacity so the blur shows through them
            // instead of only through the padding.
            bool  glass_children = false;
            float glass_opacity = 0.5f;
            uint32_t slot = 0;

            bool active(const base& e, const GUIInfo& c) const;
            void collect(base& e) override;
            void draw_under(GUIInfo& c, base& e) override;
    };

    // Soft drop shadow. Only the shape (style.radius, colour) is per element;
    // strength, softness and offset come from the UI Effects settings.
    class Shadow : public effect
    {
        public:
            ShadowStyle style;
            // 0..1 elevation: a lifted element (a dragged window) casts a larger,
            // softer, stronger shadow. Eases toward lift_target over ~0.12 s.
            float lift = 0.0f;
            float lift_target = 0.0f;

            float extent(const base& e) const override;
            void  update(base& e, float dt) override;
            void  draw_under(GUIInfo& c, base& e) override;
    };

    // Soft glow around the shape (a selection ring, for instance).
    class Glow : public effect
    {
        public:
            ShadowStyle style;

            float extent(const base& e) const override;
            void  draw_under(GUIInfo& c, base& e) override;
    };

    // Cursor-driven light on the element's rounded rect (reveal, edge light, shine,
    // ripple, focus ring, inner shadow): the flags of OverlayFx choose which.
    class LightOverlay : public effect, public OverlayFx
    {
        public:
            // hover eases 0..1 with the cursor over the element or a descendant;
            // shine_t runs 0..1 after the hover starts, -1 when idle.
            float hover = 0.0f;
            float shine_t = -1.0f;
            // ripple_t runs 0..1 after a press (-1 idle) from ripple_origin;
            // focus_t eases 0..1 with keyboard focus.
            float ripple_t = -1.0f;
            vec2  ripple_origin = { 0, 0 };
            bool  was_pressed = false;
            float focus_t = 0.0f;

            void update(base& e, float dt) override;
            void draw_over(GUIInfo& c, base& e) override;
    };

    class drag_n_drop_package
    {

        public:
            using ptr = s_ptr<drag_n_drop_package>;
            using wptr = w_ptr<drag_n_drop_package>;


            std::string name;
            w_ptr<base> element;

    };

    namespace Elements
    {
        class ParameterWindow;

    }
    class ParameterHolder
    {
        public:
            virtual ~ParameterHolder();

            using ptr = ParameterHolder*;
            virtual void init_properties(Elements::ParameterWindow* wnd) = 0;

    };


    template <class T>
    class property
    {
            T value;
            std::function<void(const T&)> on_change;

            property<T>& operator=(const property<T>&) = delete;
        public:
            std::function<T(const T&)> filter_func;

            property(std::function<void(const T&)> change_func = nullptr): value()
            {
                on_change = change_func;
             /*   filter_func = [](const T & e)->const T &
                {
                    return e;
                };*/
            }


            operator T() const
            {
                return value;
            }

#pragma warning(disable:4172)
            const T operator=(const T& r)
            {
                const T& filtered = filter_func?filter_func(r):r;

                if (value != filtered)
                {
                    value = filtered;
                    on_change(value);
                }

                return filtered;
            }
#pragma warning(default:4172)

            bool operator==(const T& r) const
            {
                return value == r;
            }

            const T* operator->() const
            {
                return &value;
            }

            const T& get() const
            {
                return value;
            }

    };

    class user_interface;

    namespace Elements
    {
        namespace layouts
        {
            class horizontal;
        }
    }

   
    struct Texture
    {
        HAL::Texture2DView texture;
        sizer margins;
        sizer padding;
        sizer tc;
		bool tiled = false;

		// The texture holds linear light (the main viewport's tonemapped
		// result) rather than sRGB-encoded UI art; see vertex_input::linearSource.
		bool linear_source = false;

        float4 mul_color;
        float4 add_color;
        Texture();

        void operator=(const HAL::Texture2DView& texture);
    };
	
    // Touch input. A separate path from the mouse one (InputHandler): user_interface::on_touch_event
    // and the elements' on_touch_action / on_touch_move. Ported from the NDK GUI.
    struct touch
    {
        vec2 pos;
        int id = 0;
        bool catched = false;
    };

    // One platform touch message: flags is MOVE, DOWN or UP, touches the fingers it concerns.
    struct touch_event
    {
        static constexpr int MOVE = 1 << 0;
        static constexpr int DOWN = 1 << 1;
        static constexpr int UP   = 1 << 2;

        struct finger
        {
            int   id;
            float x;
            float y;
        };

        int                 flags = 0;
        std::vector<finger> touches;
    };

    class drag_n_drop;
    class base : public tree<base, my_unique_vector<std::shared_ptr<base>>>, public Events::prop_handler //, public Graphics::renderable
    {
            friend tree_base;
            friend class user_interface;
            friend class drag_n_drop;
            friend class Elements::layouts::horizontal;
        public:

            using Context = GUIInfo;
            using ptr = s_ptr<base>;
            using wptr = w_ptr<base>;
             bool debug=false;
        protected:

            using guard = std::lock_guard<std::mutex>;


            std::recursive_mutex childs_locker_mutex;
            std::lock_guard<std::recursive_mutex> lock();

            virtual void think(float dt);;
            drag_n_drop_package::ptr package;
            bool need_update_layout = false;
            

            property<rect> render_bounds;
			rect local_bounds;
            bool pressed = false;
            bool thinkable = false;
            cursor_style cursor = cursor_style::ARROW;

            user_interface* user_ui = nullptr;
            sizer_long parent_sizer;
			std::set<base*> listeners;
            virtual std::vector<ptr> find_control(vec2 at, bool click_only);

            // Touch variant of find_control: every element a finger is over, with the fingers over it.
            virtual std::vector<std::pair<ptr, std::vector<touch*>>> find_control(std::vector<touch>& at, bool click_only);
            virtual std::vector<std::pair<ptr, std::vector<touch*>>> find_childs_control(std::vector<touch>& touches, bool click_only);

            bool point_in(vec2 at);

            virtual bool test_local_visible();

            virtual void draw_recursive(Context&, base* = nullptr);

            virtual void pre_draw(HAL::CommandList::ptr list); // override;;
           


     

            /*	virtual std::vector<property_base*> get_properties() override
            	{
            		std::vector<property_base*> res;

            		res.push_back(&size);
            		res.push_back(&pos);
            		res.push_back(&docking);
            		res.push_back(&visible);
            		res.push_back(&margin);
            		res.push_back(&padding);


            		return res;
            	}*/
            void run_on_ui(std::function<void()> f);
        public:

         virtual void draw(Context&); // override;;
            virtual void draw_after(Context&);;

            // The element's effect stack (see `effect`), drawn by draw_before around
            // draw(). Empty for most elements. Set up when the element is built;
            // it is read by the draw-list thread, so don't change it while the
            // element is in the tree.
            std::vector<std::unique_ptr<effect>> effects;

            // The effect of type T, or null.
            template<class T>
            T* find_effect() const
            {
                for (auto& e : effects)
                    if (auto* t = dynamic_cast<T*>(e.get()))
                        return t;
                return nullptr;
            }

            // Appends a T to the stack (or returns the one already there) and returns it.
            template<class T>
            T& add_effect()
            {
                if (auto* t = find_effect<T>())
                    return *t;
                effects.push_back(std::make_unique<T>());
                return static_cast<T&>(*effects.back());
            }

            // Whether the element's backdrop blur is on and drawing right now. Elements
            // use it to switch to a translucent look.
            bool backdrop_active(const Context& c) const;

            // The parent without get_ptr(): get_parent() throws for a root that no
            // shared_ptr owns (a user_interface on the stack in the GUI tests).
            base* get_raw_parent() const { return parent; }

            friend class effect;
            friend class BackdropBlur;
            friend class Shadow;
            friend class Glow;
            friend class LightOverlay;

            float fade_from = 1.0f;
            float fade_target = 1.0f;
            float fade_t = 1.0f;            // 0..1 along the fade, 1 = not fading
            float fade_duration = 0.1f;

            // Group opacity: while below 1 the element and everything inside it
            // is drawn as one unit and blended back with this opacity
            // (Renderer::begin_layer). fade_to animates it.
            float layer_opacity = 1.0f;
            // Runs once, as a UI task (not inside the draw walk, which iterates
            // the parent's children), when the current fade reaches its target.
            std::function<void()> on_fade_end;
            void  fade_to(float target, float seconds);
            void  update_fade(float dt);
            bool  layer_wanted() const;
            // Windows and graph nodes set this: when added they fade in as a group.
            bool  fade_on_add = false;
            // Fades out as a group and removes the element when the fade ends (it
            // stays in the tree and keeps drawing until then). Without fades it is
            // removed at once. Repeated calls are harmless.
            void  fade_close();
            // Changed mind mid fade_close(): fade back in and stay.
            void  cancel_fade_close();
            bool  is_fade_closing() const { return fade_closing; }
            bool  fade_closing = false;
            bool  layer_open = false;       // between draw_before and draw_end
            // draw_after, then closes the layer opened by draw_before.
            void  draw_end(Context& c);
            // Background opacity for a control: the nearest ancestor's
            // BackdropBlur::glass_opacity while its blur is active, else 1.
            float control_opacity(const Context& c);
            void draw_before(Context& c);
			virtual sizer update_layout(sizer r, float scale);
            virtual void update_childs_layout(sizer& r, float scale);
              virtual void update_childs_layout_after(sizer &r, float scale);


            float scale = 1;
            float result_scale = 1;
            bool clip_child  = true;
            bool can_multitouch = false;   // wants all of its fingers in one on_touch_move call
            int  touch_id = -1;            // the finger this element is currently pressed by
            bool clickable;
            // What the window frame makes of a press on this element when the window draws
            // its own title bar (user_interface::hit_test_window). The tab bar is `caption`,
            // the window buttons are minimize / maximize / close.
            window_hit window_region = window_hit::client;
            bool draw_helper = false;
            bool map_to_pixels = false;
            bool drag_listener = false;

            virtual void set_update_layout();


        public:
            virtual void on_key_action(key_action action, long key, key_mods mods);
            virtual void on_char(char32_t ch);
            virtual bool on_mouse_move(vec2 pos);

            // Touch. Like the mouse handlers, but per finger: DOWN, then UP when the finger lifts inside
            // the element or CANCEL when it slid off, with on_touch_move in between. The defaults hand a
            // single finger to the mouse handlers, as the original GUI did; override them for real touch
            // behaviour (multi-touch elements set can_multitouch).
            virtual bool on_touch_action(mouse_action action, touch& t);
            virtual bool on_long_pressed(const touch& t);
            virtual void on_touch_move(std::vector<touch*>& e);
            virtual void on_mouse_enter(vec2 pos);
            virtual void on_mouse_leave(vec2 pos);
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos);
            virtual bool on_wheel(mouse_wheel type, float value, vec2 pos);;
        public:
            virtual void add_child(ptr obj) override;
            virtual void remove_child(ptr obj) override;

            cursor_style update_cursor() const;

            void focus();

            void unfocus();

            bool is_focused();

            // Keyboard focus gained or lost; UI thread. focus() on another
            // element, or a press outside this one, takes it away.
            virtual void on_focus_changed(bool focused);
        protected:
            virtual void on_add(base* parent) override;
            virtual void on_remove() override;

            void set_movable(bool value);

            virtual vec2 get_desired_size(vec2 cur);

            bool drag_n_drop_copy = true;
            // drag n drop
        public:
            virtual bool need_drag_drop();
            virtual void on_dragdrop_start(drag_n_drop_package::ptr);
            virtual void generate_container(base::ptr obj);

            virtual void on_dragdrop_end();

            virtual void on_drag_start();

            virtual void on_drag_end();

            virtual void set_package(std::string name);
            virtual void on_parent_size_changed(const rect& r);
            virtual bool can_accept(drag_n_drop_package::ptr);
            virtual void on_drop_enter(drag_n_drop_package::ptr);

            virtual void on_drop_move(drag_n_drop_package::ptr, vec2);

            virtual void on_drop_leave(drag_n_drop_package::ptr);

            virtual bool on_drop(drag_n_drop_package::ptr, vec2);
            virtual drag_n_drop_package::ptr get_package();
        public:

            virtual void on_pre_render(Context& context);
            rect get_render_bounds();

            user_interface* get_user_ui();

            virtual bool is_hovered();

            virtual bool is_pressed();

            bool is_on_top();


            virtual bool is_menu_component();

            virtual void close_menus();

            property<vec2> size;
            property<vec2> pos;
            property<dock> docking;
            property<bool> visible;
            property<sizer> margin;
            property<sizer> padding;
			property<vec2> scaled_size;

            vec2 minimal_size; //todo: property?

            base* width_sticks = nullptr;
            base* height_sticks = nullptr;

            size_type height_size = size_type::NONE;
            size_type width_size = size_type::NONE;
            pos_x_type x_type = pos_x_type::CENTER;
            pos_y_type y_type = pos_y_type::CENTER;
            vec2 childs_size;

            ParentClamp clamp_to_parent = ParentClamp::NONE;
            std::optional<rect> child_scissor;
            std::optional<rect> self_scissor;

            std::string tooltip;
            /*	protected:
            	int get_max_z*/

        protected:

            virtual void on_touch();

            virtual void on_child_touched(ptr child);

        protected:
            virtual void on_bounds_changed(const rect& r);
			virtual void on_local_size_changed(const vec2& r);

            virtual void on_size_changed(const vec2& r);

            virtual void on_pos_changed(const vec2& r);

            virtual void on_dock_changed(const dock& r);

            virtual void on_visible_changed(const bool& r);

            virtual void on_padding_changed(const sizer& r);

            virtual void on_margin_changed(const sizer& r);
        public:
            base();
             virtual ~base() = default;
            vec2 to_global(vec2 p);

            vec2 to_local(vec2 p);

            virtual void to_front();

            void register_listener(base* listener);
			void unregister_listener(base* listener);
            virtual void on_base_change(base*);
            /*
            bool mouse_action_event(mouse_action action, mouse_button button, vec2 pos)
            {

            }*/
    };

    class drag_holder : public base
    {
        protected:
            virtual std::vector<base::ptr> find_control(vec2 at, bool click_only) override;

        public:
            using ptr = s_ptr<drag_holder>;
            drag_holder();
    };
    class drag_n_drop
    {
            friend class user_interface;   // the touch path asks whether a drag is pressed
            base::wptr pressed;
            base::wptr hovered;
            drag_n_drop_package::wptr package;
            bool dragging = false;
            vec2 start_pos;
            vec2 cur_pos;
            user_interface* user_ui;
        public:
            drag_holder::ptr holder;
            drag_n_drop(user_interface* ui);
            virtual ~drag_n_drop();

            std::set<base*> dragdrop;

            virtual void draw(base::Context& c);

            void on_drop(vec2 at) const;

            void update_hovered(base::ptr elem, vec2 pos);

            bool start_new_drag(base::ptr elem, vec2 pos);

            virtual void mouse_move_event(base::ptr elem, vec2 pos);
            virtual void mouse_action_event(base::ptr elem, mouse_action action, mouse_button button, vec2 pos);

    };


    struct draw_info
    {
        sizer clip;
        sizer_long scissors;
        float scale;
        base* elem;
        vec2 offset;
        bool before;
        // The element is a group-opacity layer: its before entry opens the
        // layer, its after entry closes it. Everything in between must be
        // recorded in the same command list.
        bool layer = false;
        // Captured with the entry: the render thread reads these, not the
        // element's live state, which the GUI thread is already updating for
        // the next frame.
        uint32_t layer_slot = 0;
        float layer_opacity = 1.0f;
        // The element's render bounds when the entry was collected. Draws use
        // these (see draw_bounds_scope), so geometry matches the clip above.
        rect bounds;
    };

    // While alive on a draw thread, owner->get_render_bounds() returns `bounds`
    // (the geometry captured when the draw list was built) instead of the live
    // value. The GUI thread lays the next frame out while this one is still being
    // recorded, so a moving element's live bounds run ahead of the clip rectangles
    // in the draw list: its blur, shadow and fill would then be placed against
    // different edges. Layout code runs on other threads and is unaffected.
    class draw_bounds_scope
    {
            const base* previous_owner;
            rect        previous_bounds;
        public:
            draw_bounds_scope(const base* owner, const rect& bounds);
            ~draw_bounds_scope();
    };

    struct UIContext
    {
        std::vector<draw_info> draw_infos;
        // [begin, end) ranges of draw_infos, one per UI_Render instance, never
        // splitting a layer.
        std::vector<std::pair<uint32_t, uint32_t>> chunks;
        std::vector<base*> pre_draw_infos;

        float dt = 0;
        vec2  scaled_size;
    };

    class user_interface : public base, public InputHandler, public Events::Runner
    {
            friend class base;
            float dt = 0;
            std::set<base*> components;
            std::set<base*> thinking;

            void add_base(base* object);

            void remove_base(base* object);

            base::ptr back;
            base::wptr hovered;
            base::wptr mouse_focus;
            vec2 last_mouse_pos;

            std::array<std::vector<base::wptr>, 3> pressed;
            std::vector<base::wptr> hovered_controls;
            base::wptr focused;

            struct touch_info
            {
                std::vector<base::ptr> pressed;
                int64_t start_time = 0;
                vec2    pos = { 0, 0 };

                bool was_long_pressed = false;
            };
            std::map<int, touch_info> touch_infos;

    
            drag_n_drop drag;

          

            std::array<bool, 3> press_interpret;
            std::array<bool, 3> release_interpret;

            bool is_updating_layout = false;

            my_unique_vector<FrameGraph::GraphGenerator*> frame_generators;
            cursor_style cursor = cursor_style::ARROW;
         
        public:


            std::vector<base*> pre_draw_infos;
            std::vector<draw_info> draw_infos;

            // Backdrop blur (any element with backdrop_blur set). Not a
            // VariableContext base: user_interface is already a GUI::base and
            // its add_child would become ambiguous.
            std::unique_ptr<VariableContext> backdrop_context = VariableContext::create(L"UI Backdrop");
            Variable<bool>  backdrop_enabled    = { true, "Blur backgrounds", backdrop_context.get() };
            Variable<float> backdrop_radius     = { 3.0f, "Blur radius (px)", backdrop_context.get(), 0.0f, 96.0f };
            Variable<float> backdrop_saturation = { 1.0f, "Blur saturation", backdrop_context.get(), 0.0f, 2.0f };
            Variable<float> backdrop_tint       = { 0.12f, "Blur tint strength", backdrop_context.get(), 0.0f, 1.0f };
            // Acrylic finish: luminosity pull toward the tint, and static grain.
            Variable<float> backdrop_luminosity = { 0.25f, "Blur luminosity", backdrop_context.get(), 0.0f, 1.0f };
            Variable<float> backdrop_noise      = { 0.03f, "Blur grain", backdrop_context.get(), 0.0f, 0.2f };

            // Procedural shadows and glow (ShadowStyle on elements).
            std::unique_ptr<VariableContext> effects_context = VariableContext::create(L"UI Effects");
            Variable<bool>  shadows_enabled  = { true, "Shadows", effects_context.get() };
            Variable<float> shadow_opacity   = { 0.45f, "Shadow opacity", effects_context.get(), 0.0f, 1.0f };
            Variable<float> shadow_blur      = { 10.0f, "Shadow softness (px)", effects_context.get(), 0.5f, 48.0f };
            Variable<float> shadow_offset    = { 3.0f, "Shadow offset (px)", effects_context.get(), 0.0f, 24.0f };
            Variable<bool>  glow_enabled     = { true, "Glow", effects_context.get() };
            Variable<float> glow_strength    = { 0.8f, "Glow strength", effects_context.get(), 0.0f, 2.0f };
            Variable<float> glow_size        = { 5.0f, "Glow size (px)", effects_context.get(), 0.5f, 32.0f };

            // Cursor-driven light effects (OverlayFx on elements).
            Variable<bool>  reveal_enabled   = { true, "Reveal highlight", effects_context.get() };
            Variable<float> reveal_strength  = { 0.16f, "Reveal strength", effects_context.get(), 0.0f, 1.0f };
            Variable<float> reveal_size      = { 48.0f, "Reveal size (px)", effects_context.get(), 4.0f, 200.0f };
            Variable<bool>  border_enabled   = { true, "Edge light", effects_context.get() };
            Variable<float> border_strength  = { 0.8f, "Edge light strength", effects_context.get(), 0.0f, 1.0f };
            Variable<float> border_size      = { 70.0f, "Edge light size (px)", effects_context.get(), 4.0f, 300.0f };
            Variable<bool>  shine_enabled    = { true, "Shine sweep", effects_context.get() };
            Variable<float> shine_strength   = { 0.28f, "Shine strength", effects_context.get(), 0.0f, 1.0f };
            Variable<float> shine_duration   = { 0.7f, "Shine duration (s)", effects_context.get(), 0.1f, 3.0f };
            Variable<bool>  ripple_enabled   = { true, "Press ripple", effects_context.get() };
            Variable<float> ripple_strength  = { 0.25f, "Ripple strength", effects_context.get(), 0.0f, 1.0f };
            Variable<float> ripple_duration  = { 0.55f, "Ripple duration (s)", effects_context.get(), 0.1f, 2.0f };
            Variable<bool>  focus_enabled    = { true, "Focus ring", effects_context.get() };
            Variable<float> focus_strength   = { 0.9f, "Focus ring strength", effects_context.get(), 0.0f, 1.0f };
            Variable<bool>  inner_enabled    = { true, "Inner shadow", effects_context.get() };
            Variable<float> inner_strength   = { 0.35f, "Inner shadow strength", effects_context.get(), 0.0f, 1.0f };
            Variable<float> inner_size       = { 10.0f, "Inner shadow size (px)", effects_context.get(), 1.0f, 48.0f };

            // Group fades (base::layer_opacity): menu popups and message boxes fade in.
            Variable<bool>  fade_enabled     = { true, "Group fades", effects_context.get() };
            Variable<float> fade_in_duration = { 0.14f, "Fade-in duration (s)", effects_context.get(), 0.02f, 1.5f };
            Variable<float> fade_out_duration = { 0.10f, "Fade-out duration (s)", effects_context.get(), 0.02f, 1.5f };
            Variable<bool>  window_animations = { true, "Window open/close fades", effects_context.get() };

            // Hands each backdrop element its own snapshot slot for the frame:
            // two elements recorded into different command lists must not
            // share a texture. Reset when the draw list is collected.
            uint32_t backdrop_counter = 0;
            // The same for group-opacity layers (one snapshot texture each).
            uint32_t layer_counter = 0;

            // Where the window frame should treat clicks as title bar or window buttons.
            // The tree walk records every visible element that has a window_region or is
            // clickable, in draw order, into building_regions (UI thread); at the end of the
            // walk the list is published to window_regions under the mutex, which the
            // window thread reads. The last one under the point wins (it is on top).
            struct frame_region
            {
                rect       bounds;
                window_hit hit;
            };
            std::vector<frame_region> building_regions;
            std::vector<frame_region> window_regions;
            std::mutex                window_regions_mutex;

            using ptr = s_ptr<user_interface>;
            using wptr = w_ptr<user_interface>;
            std::mutex m;

            std::function<void(bool)> set_capture;
            std::function<std::wstring()> get_clipboard;
            std::function<void(std::wstring_view)> set_clipboard;
            user_interface();
            ~user_interface();

            base::wptr get_hovered() const { return hovered; }
            vec2       get_mouse_pos() const { return last_mouse_pos; }

            void mouse_move_event_internal(vec2 pos);
            void mouse_action_event_internal(mouse_action action, mouse_button button, vec2 pos);
            void mouse_wheel_event_internal(mouse_wheel type, float value, vec2 pos);
            void key_action_event_internal(key_action action, long key, key_mods mods);
            void char_event_internal(char32_t ch);


            // Touch input, deliberately not routed through the mouse handlers below. Returns whether an
            // element took the touch.
            bool on_touch_event(const touch_event& event);

            virtual void mouse_move_event(vec2 pos);
            virtual void mouse_action_event(mouse_action action, mouse_button button, vec2 pos);
            virtual void mouse_wheel_event(mouse_wheel type, float value, vec2 pos);
            virtual void key_action_event(key_action action, long key, key_mods mods);
            virtual void char_event(char32_t ch);
            virtual void apply_cursor() override;

            // The window frame's hit test, answered from window_regions (see below).
            virtual window_hit hit_test_window(vec2 pos) override;

            // Set by the application when the window draws its own title bar: the UI's
            // minimise / maximise / close buttons call the handler; the query says
            // whether the window is maximised (the maximise button shows "restore").
            std::function<void(window_command)> window_command_handler;
            std::function<bool()>               window_maximized;

            // Key hooks see every key event before the focused element does, and a mouse
            // press as a key of -1 (the result of that one is ignored): for things that
            // listen to the keyboard without focus, such as the ribbon's Alt key tips. A
            // hook that returns true consumes the key. The owner is held weakly; a hook
            // whose owner is gone is skipped. While keys_captured is set, typed characters
            // go nowhere (they would otherwise land in the focused text field).
            using key_hook_fn = std::function<bool(key_action, long, key_mods)>;
            void add_key_hook(base::ptr owner, key_hook_fn hook);
            bool keys_captured = false;
            std::vector<std::pair<base::wptr, key_hook_fn>> key_hooks;

         //   virtual void draw_ui(Context&);

			virtual void process_ui(float dt);
			virtual void create_graph(FrameGraph::Graph& graph);

            // Walks the tree into draw_infos / pre_draw_infos. create_graph does this for the FrameGraph
            // passes; a host without the graph calls it directly and replays the lists itself.
            void collect_draw_infos();

            void add_task(std::function<void()> f);

            void process_graph(FrameGraph::Graph& graph);

            // base::pre_draw is protected (only user_interface is a friend) — this
            // bridges it for the UI_PreDraw FrameGraph pass, which isn't a member.
            static void run_pre_draw(std::vector<base*>& infos, HAL::CommandList::ptr list);



            std::shared_future<bool> message_box(std::string title, std::string text, std::function<void(bool)> f);

            virtual void on_size_changed(const vec2& r) override;


            /*
             virtual void close_menus()
             {
                 run_on_ui([this]
                 {
                     base::close_menus();
                 });
             }	  */
    };
}
}