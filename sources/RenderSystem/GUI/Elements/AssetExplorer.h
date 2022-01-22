#pragma once
#include "Assets/Asset.h"
#include "GUI/Elements/Container.h"
#include "GUI/Elements/StatusBar.h"
#include "GUI/Elements/Image.h"

namespace GUI
{
    namespace Elements
    {
        class asset_item: public base
        {
			image::ptr back_im;

                image::ptr im;
                label::ptr name;
				Texture back;
				Texture hover;

            public:
                using ptr = s_ptr<asset_item>;
                using wptr = w_ptr<asset_item>;
                asset_item(AssetStorage::ptr asset);

                AssetStorage::ptr asset;

                void draw(Graphics::context& c);

                virtual bool need_drag_drop() override;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;


        };


        class asset_explorer: public base, public AssetListener
        {
                //class renderer;
                //friend class renderer;
                status_bar::ptr stat_bar;
                container::ptr table;
                folder_item* current_folder = nullptr;
				void draw(Graphics::context& c);

            public:
                using ptr = s_ptr<asset_explorer>;
                using wptr = w_ptr<asset_explorer>;

                asset_explorer();

                virtual void on_add_asset(AssetStorage*) override;

                virtual void on_remove_asset(AssetStorage*) override;

        };
    }
}