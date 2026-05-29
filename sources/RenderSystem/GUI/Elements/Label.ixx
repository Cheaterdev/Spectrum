export module GUI:Label;
import :Base;

import :ScrollContainer;

import TextSystem;

namespace HAL { class Texture; }  // forward-decl for cache_resource (defined in Graphics:Texture)
          

extern "C" typedef struct FT_FaceRec_* FT_Face;
extern "C" typedef struct FT_LibraryRec_* FT_Library;


  class FreeTypeFont:public Singleton<FreeTypeFont>
  {
  	FT_Library FtLib;
    FT_Face     face;  
  public:
      FreeTypeFont();


  };
struct Symbol
{
	GUI::Texture texture;
    float4 color;

    Symbol(char symbol)
    {
	    
    }
};

struct TextLine
{
	std::vector<Symbol> symbols;
};

struct Text
{
	std::vector<TextLine> lines;
};


export namespace GUI
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

			virtual void draw(Context& c) override;

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
                float2 text_size;
                ivec2 w;
                float scaled = 1;

				GUI::Texture cache;
				std::shared_ptr<HAL::Texture> cache_resource;  // owns the texture; cache.texture holds the view
                virtual void on_pre_render(Context& context) override;
                virtual void pre_draw(HAL::CommandList::ptr list) override;
            public:
                int magnet_text;
                bool test = false;
                using ptr = s_ptr<label>;
                using wptr = w_ptr<label>;

                property<std::string> text;
                property<float> font_size;
                float4 color;
                label();
                virtual	~label();
                virtual void draw(Context& c) override;

                unsigned int get_index(vec2 at);
                Fonts::FontGeometry::ptr get_geometry();
                virtual void recalculate(Context& c);

                virtual sizer update_layout(sizer r, float scale) override;

        };

        class MultiLineLabel: public scroll_container
        {
                                virtual void on_text_changed(const std::string& str);
            std::list<label::ptr> labels;
        public:
              using ptr = s_ptr<MultiLineLabel>;
                using wptr = w_ptr<MultiLineLabel>;


            property<std::string> text;
        	property<float> font_size;

            MultiLineLabel();
        };
    }
}