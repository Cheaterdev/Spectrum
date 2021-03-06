#pragma once
namespace GUI
{
    namespace Elements
    {
		/*
		class label_internal : public base
		{
		public:
			using ptr = s_ptr<label_internal>;
			using wptr = w_ptr<label_internal>;

			GUI::Texture cache;

			virtual void draw(Render::context& c) override;

		};*/

        class label : public base
        {
				/*label_internal::ptr internal_label;*/

                Fonts::Font::ptr font;
                Fonts::FontGeometry::ptr geomerty;
                Fonts::FontGeometry::ptr geomerty_shadow;
                virtual void on_text_changed(const std::string& str);
                virtual void on_size_changed(const float& str);

                virtual void on_bounds_changed(const rect& r) override;

                bool need_recalculate = false;
                bool calculated = false;
                sizer lay1, lay2, area1, area2;
                rect text_size;
                ivec2 w;
                float scaled = 1;

				GUI::Texture cache;

            public:
                int magnet_text;
                bool test = false;
                using ptr = s_ptr<label>;
                using wptr = w_ptr<label>;

                property<std::string> text;
                property<float> font_size;
                rgba8 color;
                label();
                virtual	~label()
                {
                    font = nullptr;
                }
                virtual void draw(Render::context& c) override;

                unsigned int get_index(vec3 at);
                Fonts::FontGeometry::ptr get_geometry();
                virtual void recalculate(Render::context& c);

                virtual sizer update_layout(sizer r, float scale) override;

        };
    }
}