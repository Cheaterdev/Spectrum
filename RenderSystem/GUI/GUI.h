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


enum class mouse_wheel : int
{
    VERTICAL,
    HORIZONTAL
};


struct InputHandler
{
	virtual	void mouse_move_event(vec2 pos) = 0;
	virtual	void mouse_action_event(mouse_action action, mouse_button button, vec2 pos) = 0;
	virtual	void mouse_wheel_event(mouse_wheel wheel, float value, vec2 pos) = 0;

	virtual	void key_action_event(long key) = 0;
};

namespace GUI
{
    class Renderer;
    using Renderer_ptr = s_ptr<Renderer>;
    class user_interface;

    namespace offset
    {
        static const vec2 CENTER(0.5, 0.5);
        static const vec2 LEFT_TOP(0, 0);
        static const vec2 LEFT_DOWN(0, 1);
        static const vec2 RIGHT_TOP(1, 0);
        static const vec2 RIGHT_DOWN(1, 1);
        static const vec2 LEFT_CENTER(0, 0.5);
        static const vec2 RIGHT_CENTER(1, 0.5);

        static const vec2 TOP_CENTER(0.5, 0);
        static const vec2 DOWN_CENTER(0.5, 1);
    };

    static LPCTSTR cursors[] =
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
    class base;
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
            virtual ~ParameterHolder()
            {
            }

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

            property(std::function<void(const T&)> change_func = nullptr)
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
            const T& operator=(const T& r)
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

    class RenderData
    {


        public:
            virtual ~RenderData()
            {
            }

            using ptr = s_ptr<RenderData>;

            virtual void on_render_changed(const rect& r) {};

    };
    struct Texture
    {
        Render::Texture::ptr texture;
        sizer margins;
        sizer padding;
        sizer tc;
		bool tiled = false;

		Render::HandleTable srv;
        Texture()
        {
            margins = { 0, 0, 0, 0 };
            padding = { 0, 0, 0, 0 };
            tc = { 0, 0, 1, 1 };
        }

        void operator=(const       Render::Texture::ptr& texture)
        {
            this->texture = texture;
        }
    };

    class drag_n_drop;
    class base : public tree<base, my_unique_vector<std::shared_ptr<base>>>, public ParameterHolder, public Events::prop_handler //, public Render::renderable
    {
            friend class tree_base;
            friend class user_interface;
            friend class drag_n_drop;
            friend class Elements::layouts::horizontal;
        public:
            using ptr = s_ptr<base>;
            using wptr = w_ptr<base>;

            RenderData::ptr render_data;
        protected:

            using guard = std::lock_guard<std::mutex>;


            std::recursive_mutex childs_locker_mutex;
            std::lock_guard<std::recursive_mutex> lock();

            virtual void think(float dt);;
            drag_n_drop_package::ptr package;
            bool need_update_layout = false;


            property<rect> render_bounds;
            rect local_bounds;
            bool pressed;
            bool thinkable;
            cursor_style cursor = cursor_style::ARROW;

            user_interface* user_ui = nullptr;
            sizer_long parent_sizer;
            virtual std::vector<ptr> find_control(vec2 at, bool click_only);

            virtual bool test_local_visible();

            virtual void draw_recursive(Render::context&, base* = nullptr);

            virtual void draw(Render::context&); // override;;
            virtual void draw_after(Render::context&);;


           virtual void init_properties(Elements::ParameterWindow* wnd) override;


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
			virtual sizer update_layout(sizer r, float scale);


            float scale = 1;
            float result_scale = 1;
            bool clip_child  = true;
            bool clickable;
            bool draw_helper = false;
            bool map_to_pixels = false;
            bool drag_listener = false;

            virtual void set_update_layout();


        public:
            virtual void on_key_action(long key);
            virtual bool on_mouse_move(vec2 pos);
            virtual void on_mouse_enter(vec2 pos);
            virtual void on_mouse_leave(vec2 pos);
            virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos);
            virtual bool on_wheel(mouse_wheel type, float value, vec2 pos);;
        public:
            virtual void add_child(ptr obj) override;
            virtual void remove_child(ptr obj) override;

            void update_cursor() const;

            void focus();

            void unfocus();
        protected:
            virtual void on_add(base* parent) override;
            virtual void on_remove() override;

            void set_movable(bool value);

            virtual vec2 get_desired_size(vec2 cur);

            Renderer_ptr renderer;


            bool drag_n_drop_copy = true;
            // drag n drop
        public:
            virtual bool need_drag_drop();
            virtual void on_dragdrop_start(drag_n_drop_package::ptr);
            virtual void generate_container(base::ptr obj)
            {
                obj->add_child(get_ptr());
            }

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
            rect get_render_bounds();

            user_interface* get_user_ui();

            virtual bool is_hovered();

            virtual bool is_pressed();

            bool is_on_top();

            virtual void set_skin(Renderer_ptr skin);

            Renderer_ptr get_skin();

            virtual bool is_menu_component();

            virtual void close_menus();

            property<vec2> size;
            property<vec2> pos;
            property<dock> docking;
            property<bool> visible;
            property<sizer> margin;
            property<sizer> padding;
            vec2 minimal_size; //todo: property?
            size_type width_size = size_type::NONE;
            size_type height_size = size_type::NONE;
            pos_x_type x_type = pos_x_type::CENTER;
            pos_y_type y_type = pos_y_type::CENTER;
            vec2 childs_size;

            bool clip_to_parent = false;
            /*	protected:
            	int get_max_z*/

        protected:

            virtual void on_touch();

            virtual void on_child_touched(ptr child);

        protected:
            virtual void on_bounds_changed(const rect& r);

            virtual void on_size_changed(const vec2& r);

            virtual void on_pos_changed(const vec2& r);

            virtual void on_dock_changed(const dock& r);

            virtual void on_visible_changed(const bool& r);

            virtual void on_padding_changed(const sizer& r);

            virtual void on_margin_changed(const sizer& r);
        public:
            base();

            vec2 to_global(vec2 p);

            vec2 to_local(vec2 p);

            virtual void to_front();

            /*
            bool mouse_action_event(mouse_action action, mouse_button button, vec2 pos)
            {

            }*/
    };

    class drag_holder : public base
    {
        protected:
            virtual std::vector<base::ptr> find_control(vec2 at, bool click_only) override
            {
                return std::vector<base::ptr>();
            }


        public:
            using ptr = s_ptr<drag_holder>;
            drag_holder()
            {
                docking = dock::NONE;
                width_size = size_type::FIXED;
                height_size = size_type::FIXED;
            }
    };
    class drag_n_drop
    {
            base::wptr pressed;
            base::wptr hovered;
            drag_n_drop_package::wptr package;
            bool dragging = false;
            vec2 start_pos;
            vec2 cur_pos;
            user_interface* user_ui;
        public:
            drag_holder::ptr holder;
            drag_n_drop(user_interface* ui): user_ui(ui)
            {
                holder.reset(new drag_holder);
                //  holder->visible = false;
            }
            virtual ~drag_n_drop();

            std::set<base*> dragdrop;

            virtual void draw(Render::context& c);

            void on_drop(vec2 at) const;

            void update_hovered(base::ptr elem, vec2 pos);

            bool start_new_drag(base::ptr elem, vec2 pos);

            virtual void mouse_move_event(base::ptr elem, vec2 pos);


            virtual void mouse_action_event(base::ptr elem, mouse_action action, mouse_button button, vec2 pos);

    };


    class user_interface : public base, public InputHandler
    {
            friend class base;

            std::set<base*> components;
            std::set<base*> thinking;

            void add_base(base* object);

            void remove_base(base* object);

            base::ptr back;
            base::wptr hovered;
            base::wptr mouse_focus;

            std::array<std::vector<base::ptr>, 3> pressed;
            std::vector<base::ptr> hovered_controls;
            base::wptr focused;

            std::mutex tasks_mutex;
            std::vector<std::function<void()>> tasks;
            drag_n_drop drag;

            std::mutex m;

            std::array<bool, 3> press_interpret;
            std::array<bool, 3> release_interpret;

            bool is_updating_layout = false;

		
        public:
            using ptr = s_ptr<user_interface>;
            using wptr = w_ptr<user_interface>;


            std::function<void(bool)> set_capture;
            user_interface();
            ~user_interface()
            {
                remove_all();
            }
            virtual void mouse_move_event(vec2 pos);

            virtual void mouse_action_event(mouse_action action, mouse_button button, vec2 pos);

            virtual void mouse_wheel_event(mouse_wheel type, float value, vec2 pos);
            virtual void key_action_event(long key);

            virtual void draw_ui(Render::context&);

            void add_task(std::function<void()> f)
            {
                tasks_mutex.lock();
                tasks.push_back(f);
                tasks_mutex.unlock();
            }

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
