#include "pch.h"


namespace GUI
{
    namespace Elements
    {

        bool asset_item::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
        {
            if (action == mouse_action::UP && button == mouse_button::RIGHT)
            {
                menu_list::ptr menu(new menu_list());
                menu->add_item("Test")->on_click = [this](menu_list_element::ptr e)
                {
                    auto material = asset->get_asset()->get_ptr<MaterialAsset>();

                    if (material)
                    {
                        auto universal = std::dynamic_pointer_cast<materials::universal_material>(material);

                        if (universal)
                            universal->test();
                    }
                };
                menu->add_item("Edit")->on_click = [this](menu_list_element::ptr e)
                {
                    if (!asset->get_asset()) return;

                    auto material = asset->get_asset()->get_ptr<MaterialAsset>();

                    if (material)
                    {
                        auto universal = std::dynamic_pointer_cast<materials::universal_material>(material);

                        if (universal)
                        {
                            run_on_ui([this, universal]()
                            {
                                window::ptr wnd(new window);
                                user_ui->add_child(wnd);
                                dock_base::ptr dock(new dock_base);
                                wnd->add_child(dock);
                                dock->get_tabs()->add_button(FlowGraph::manager::get().add_graph(universal->get_graph()));
                                wnd->pos = { 200, 200 };
                                wnd->size = { 300, 300 };
                            });
                        }
                    }

                    auto texture = asset->get_asset()->get_ptr<TextureAsset>();

                    if (texture)
                    {
                        auto t = texture->get_texture()->get_desc();
                        std::stringstream s;
                        s << "mips: " << t.MipLevels;
                        user_ui->message_box("info", s.str(), [](bool v) {});
                    }
                };
                auto texture = asset->get_asset()->get_ptr<TextureAsset>();

                if (texture)
                    menu->add_item("Compress")->on_click = [this](menu_list_element::ptr e)
                {
                    auto texture = asset->get_asset()->get_ptr<TextureAsset>();
                    texture->compress();
                };

                menu->add_item("refresh")->on_click = [this](menu_list_element::ptr e)
                {
                    asset->update_preview();
                };

                menu->pos = vec2(pos);

                run_on_ui([this, menu]()
                {
                    menu->self_open(user_ui);
                    menu->add_item("Reload")->on_click = [this](menu_list_element::ptr e)
                    {
                        run_on_ui([this]() { asset->get_asset()->reload_resource();});
                    };
                });
            }

            return true;
        }


        void GUI::Elements::asset_item::draw(Render::context& c)
        {
			if (is_hovered())
				back_im->texture = hover;

			else //if (is_hovered())
				//back_im->texture = hover;
		//	else
				back_im->texture = back;


            // if (asset->need_update_preview())
            //  asset->update_preview();
            //   im->texture = asset->get_preview();
        }

        GUI::Elements::asset_item::asset_item(AssetStorage::ptr asset)
        {
            size = { 64, 64 };

			back = Skin::get().DefaultImagePanel.Normal;
			hover = Skin::get().DefaultImagePanel.Hover;
			this->asset = asset;
            GUI::Elements::image::ptr img(new GUI::Elements::image);
            img->texture = Skin::get().DefaultImagePanel.Normal;
            img->padding = { 9, 9, 9, 9 };
            img->docking = GUI::dock::FILL;
            add_child(img);

			back_im = img;
            im.reset(new image());
            im->docking = GUI::dock::FILL;
            im->width_size = GUI::size_type::SQUARE;
            img->add_child(im);
            name.reset(new label());
            name->text = convert(asset->get_name());
            name->docking = GUI::dock::FILL;
			name->y_type = pos_y_type::BOTTOM;
			name->height_size = size_type::FIXED;
			img->add_child(name);
            set_package("asset");
            auto i = im;
            asset->on_preview.register_handler(this, [i](Render::Texture::ptr preview)
            {
                i->texture = preview;
            });
        }

        GUI::Elements::asset_explorer::asset_explorer()
        {
            auto assets = AssetManager::get().get_assets();
            docking = GUI::dock::FILL;
            table.reset(new GUI::Elements::container());
            table->docking = GUI::dock::FILL;
            //table->draw_helper = true;
            add_child(table);
            table->contents->width_size = size_type::MATCH_PARENT;
            /*      for (auto a : assets)
                  {
                      asset_item::ptr elem(new asset_item(a));
                      table->add_child(elem);
                  }*/
            stat_bar.reset(new GUI::Elements::status_bar());
            add_child(stat_bar);
            GUI::Elements::button::ptr add_but(new GUI::Elements::button());
            add_but->get_label()->text = "+";
            add_but->height_size = size_type::MATCH_PARENT;
            add_but->width_size = size_type::SQUARE;
            stat_bar->add_child(add_but);
            add_but->on_click = [this](button::ptr but)
            {
                menu_list::ptr menu(new menu_list());
                menu->add_item("TILE")->on_click = [this](menu_list_element::ptr e)
                {
                    run_on_ui([this]()
                    {
                        std::string s = file_open("Choose your destiny", "", "png|*.png|" "jpg|*.jpg|" "all|*.*|"   "|");

                        if (!s.empty())
                            (new TiledTexture(s))->try_register();
                    });
                };
                menu->add_item("Import asset")->on_click = [this](menu_list_element::ptr e)
                {
                    run_on_ui([this]()
                    {
                        std::string s = file_open("Choose your destiny", "", "dds|*.dds|" "png|*.png|" "jpg|*.jpg|"  "obj|*.obj|"  "blend|*.blend|"  "all|*.*|"   "|");

                        if (!s.empty())
                        {
                            std::string ext = to_lower(s.substr(s.find_last_of(".") + 1));

                            if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "dds" || ext == "tga")
                                thread_pool::get().enqueue([s]()
                            {
                                (new TextureAsset(convert(s)))->try_register();
                            }
                                                      );
							else
                         //   if (ext == "blend" || ext == "obj" || ext == "3ds" || ext == "dae" || ext == "fbx" || ext == "rw4" || ext == "stl")
                                thread_pool::get().enqueue([this,s]()
                            {

								auto context = std::make_shared<AssetLoadingContext>();
								context->ui = user_ui->get_ptr<GUI::user_interface>();
                                (new MeshAsset(convert(s), context))->try_register();
                            });
                            ext = "";
                        }
                    });
                };
                menu->add_item("Compress All")->on_click = [this](menu_list_element::ptr e)
                {
                    //    auto first_task = create_task([]() {});
                    int i = 0;

                    for (auto& a : AssetManager::get().get_assets())
                    {
                        if (a.second->get_type() == Asset_Type::TEXTURE)
                        {
                            auto texture = a.second->get_asset()->get_ptr<TextureAsset>();
                            //     first_task.then([texture]()
                            // {
                            texture->compress();
                            //  });
                            // break;
                        }
                    }
                };
                run_on_ui([this, menu, but]()
                {
                    menu->pos = vec2(but->get_render_bounds().pos);
                    menu->self_open(user_ui);
                });
            };
            AssetManager::get().add_listener(this);
            AssetRenderer::create();
            GUI::Elements::button::ptr save_all_but(new GUI::Elements::button());
            save_all_but->get_label()->text = "SAVE";
            save_all_but->height_size = size_type::MATCH_PARENT;
            save_all_but->width_size = size_type::MATCH_CHILDREN;
            stat_bar->add_child(save_all_but);
            save_all_but->on_click = [this](button::ptr but)
            {
                AssetManager::get().save_all(false);
            };
            GUI::Elements::combo_box::ptr filter_combo(new GUI::Elements::combo_box());
            filter_combo->add_item("all");
            filter_combo->add_item("textures");
            filter_combo->add_item("materials");
            filter_combo->add_item("meshes");
            filter_combo->size = { 100, 20 };
            stat_bar->add_child(filter_combo);
            GUI::Elements::tree::ptr folders(new GUI::Elements::tree());
            folders->size = { 200, 150 };
            folders->docking = GUI::dock::LEFT;
            add_child(folders);

			GUI::Elements::image::ptr divider(new 	GUI::Elements::image());
			divider->size = { 2, 2 };
			divider->docking = GUI::dock::LEFT;
			divider->texture = Skin::get().DefaultStatusBar.Normal;
			add_child(divider);


            folders->on_select = [this](tree_element::ptr elem)
            {
                run_on_ui([this, elem]()
                {
                    table->remove_elements();
                    current_folder = static_cast<folder_item*>(elem->node);
                    current_folder->iterate_assets([this](AssetStorage::ptr e)
                    {
                        asset_item::ptr elem(new asset_item(e));
                        table->add_child(elem);
                    });
                });
            };
            folders->init(AssetManager::get().get_folders().get());
            //	add_but->docking = GUI::dock::RIGHT;
        }


        void asset_explorer::on_add_asset(AssetStorage* a)
        {
            if (current_folder == a->get_folder())
            {
                asset_item::ptr elem(new asset_item(a->get_ptr()));
                run_on_ui([this, elem]()
                {
                    table->add_child(elem);
                }
                         );
            }
        }

        void asset_explorer::on_remove_asset(AssetStorage*)
        {
            //	asset_item::ptr elem(new asset_item(a));
            //	table->add_child(elem);
        }


        bool asset_item::need_drag_drop()
        {
            return true;
        }


		void GUI::Elements::asset_explorer::draw(Render::context& c)
		{
		//	renderer->draw_container(get_ptr(), c);
		}
    }
}
