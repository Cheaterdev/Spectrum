module Graphics:AssetExplorer;
import :Materials.UniversalMaterial;
import :MeshAsset;
import Core;

import GUI;

namespace GUI
{
	namespace Elements
	{
		class dummy_context :public ::FlowGraph::GraphContext
		{

		};
		using dummy_options = ::FlowGraph::ContextOptions<dummy_context>;

		class dummy_node : public dummy_options::NodeType, public GUI::Elements::FlowGraph::VisualGraph
		{
			virtual  void operator()(dummy_options::ContextType* type)
			{

			};
		public:
			dummy_node()
			{

				register_input(/*::FlowGraph::data_types::INT,*/ "Ref")->only_one_input = false;
				register_output(/*::FlowGraph::data_types::INT,*/ "Ref");
			}
			using ptr = std::shared_ptr<dummy_node>;
			GUI::Elements::image::ptr img_inner;
			AssetStorage::ptr asset_storage;

			GUI::base::ptr create_editor_window() override
			{
				GUI::Elements::image::ptr img(new GUI::Elements::image);
				img->texture = Skin::get().Shadow;

				img->padding = img->texture.padding;
				img->width_size = GUI::size_type::MATCH_CHILDREN;
				img->height_size = GUI::size_type::MATCH_CHILDREN;
				//   img->size = { 64, 64 };
				img_inner.reset(new GUI::Elements::image);


				img_inner->texture.texture = asset_storage->get_preview();
				img_inner->docking = GUI::dock::TOP;
				img_inner->size = { 128, 128 };
				img->add_child(img_inner);
				return img;
			}



		};
		bool asset_item::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
		{
			if (action == mouse_action::UP && button == mouse_button::RIGHT)
			{
				auto loaded_asset = asset->get_asset();
				if (!loaded_asset) return true;
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
							window::ptr wnd(new window);
							user_ui->add_child(wnd);
							dock_base::ptr dock(new dock_base);
							wnd->add_child(dock);
							dock->get_tabs()->add_button(FlowGraph::manager::get().add_graph(universal->get_graph()));
							wnd->pos = { 200, 200 };
							wnd->size = { 300, 300 };
						}
					}

					auto texture = asset->get_asset()->get_ptr<TextureAsset>();

					if (texture)
					{
						auto t = texture->get_texture()->get_desc().as_texture();
						std::stringstream s;
						s << "mips: " << t.MipLevels;
						user_ui->message_box("info", s.str(), [](bool v) {});
					}


					auto mesh = asset->get_asset()->get_ptr<MeshAsset>();

					if (mesh)
					{
						//	auto t = mesh->get_texture()
						std::stringstream s;
						size_t total_vertexes = 0;
						size_t total_indices = 0;
						for (auto& info : mesh->meshes)
						{
							s << "mesh vertex: " << info.vertex_count << " index: " << info.index_count << std::endl;
							total_vertexes += info.vertex_count;
							total_indices += info.index_count;

						}

						s << "TOTAL vertex: " << total_vertexes << " index: " << total_indices << std::endl;



						//			s << "mips: " << t.MipLevels;
						user_ui->message_box("info", s.str(), [](bool v) {});
					}


				};
				auto texture = loaded_asset->get_ptr<TextureAsset>();

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

				menu->add_item("show references")->on_click = [this](menu_list_element::ptr e)
				{

					dummy_options::GraphType::ptr graph(new dummy_options::GraphType());

					graph->register_input(/*::FlowGraph::data_types::INT, */"Ref");

					std::function<void(::FlowGraph::parameter::ptr, AssetStorage::ptr)> generate_references;


					std::map<AssetStorage::ptr, dummy_node::ptr> ready_nodes;
					generate_references = [&generate_references, &graph, &ready_nodes](::FlowGraph::parameter::ptr prm, AssetStorage::ptr asset)
					{	auto references = asset->get_references();

					for (auto& ref : references)
					{

						auto asset_storage = AssetManager::get().get_storage(ref);

						dummy_node::ptr& node = ready_nodes[asset_storage];

						if (!node)
						{
							node.reset(new dummy_node());
							node->name = convert(asset_storage->get_name());
							node->asset_storage = asset_storage;
							graph->register_node(node);
							generate_references(node->get_output(0), asset_storage);
						}



						prm->link(node->get_input(0));



					}

					};
					generate_references(graph->get_input(0), asset);

					run_on_ui([this, graph]()
						{
							window::ptr wnd(new window);
							user_ui->add_child(wnd);
							dock_base::ptr dock(new dock_base);
							wnd->add_child(dock);
							dock->get_tabs()->add_button(FlowGraph::manager::get().add_graph(graph));
							wnd->pos = { 200, 200 };
							wnd->size = { 300, 300 };
						});

				};

				menu->pos = vec2(pos);


				menu->self_open(user_ui);
				menu->add_item("Reload")->on_click = [this](menu_list_element::ptr e)
				{
					asset->get_asset()->reload_resource();
				};

			}

			return true;
		}


		void GUI::Elements::asset_item::draw(Context& c)
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
			im->margin = { 5,5,5,5 };
			img->add_child(im);
			name.reset(new label());
			name->text = convert(asset->get_name());
			name->docking = GUI::dock::FILL;
			name->y_type = pos_y_type::BOTTOM;
			name->height_size = size_type::FIXED;
			img->add_child(name);
			set_package("asset");
			auto i = im;
			asset->on_preview.register_handler(this, [i](HAL::Texture::ptr preview)
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
					auto files = Application::get().file_open("Choose your destiny", "", "png|*.png|" "jpg|*.jpg|" "all|*.*|"   "|");
				};
				menu->add_item("Import asset")->on_click = [this](menu_list_element::ptr e)
				{

					auto files = Application::get().file_open("Choose your destiny", "", "dds|*.dds|" "png|*.png|" "jpg|*.jpg|"  "obj|*.obj|"  "blend|*.blend|"  "all|*.*|"   "|");

					struct material_info
					{
						TextureAsset::ptr albedo;
						TextureAsset::ptr metallic;
						TextureAsset::ptr roughness;
						TextureAsset::ptr height;
						TextureAsset::ptr normals;

						void create()
						{
							MaterialGraph::ptr graph(new MaterialGraph);


							if (albedo)
							{

								auto value_node = std::make_shared<TextureNode>(albedo, true);
								graph->register_node(value_node);
								value_node->get_output(0)->link(graph->get_base_color());

								graph->get_texcoord()->link(value_node->get_input(0));
							}

							if (roughness)

							{
								auto value_node = std::make_shared<TextureNode>(roughness);
								graph->register_node(value_node);
								value_node->get_output(1)->link(graph->get_roughness());
								graph->get_texcoord()->link(value_node->get_input(0));
							}

							if (metallic)
							{
								auto value_node = std::make_shared<TextureNode>(metallic);
								graph->register_node(value_node);
								value_node->get_output(1)->link(graph->get_mettalic());
								graph->get_texcoord()->link(value_node->get_input(0));
							}

							if (normals)
							{
								auto value_node = std::make_shared<TextureNode>(normals);
								graph->register_node(value_node);
								value_node->get_output(0)->link(graph->get_normals());
								graph->get_texcoord()->link(value_node->get_input(0));

							}

							if (height)
							{

								auto mul_node = std::make_shared<MulNode>();
								auto scalar_node = std::make_shared<ScalarNode>(1.0f);
								auto value_node = std::make_shared<TextureNode>(height);
								graph->register_node(mul_node);
								graph->register_node(scalar_node);
								graph->register_node(value_node);


								graph->get_texcoord()->link(value_node->get_input(0));
								value_node->get_output(1)->link(mul_node->get_input(0));
								scalar_node->get_output(0)->link(mul_node->get_input(1));

								mul_node->get_output(0)->link(graph->get_tess_displacement());

							}

							materials::universal_material* m = (new materials::universal_material(graph));
							//m->generate_material();
							m->register_new();

						}
					};

					auto mat_info = std::make_shared<material_info>();
					std::vector<concurrency::task<void>> tasks;
					tasks.reserve(files.size());
					for (auto s : files)
					{

						s = to_lower(s);
						std::string ext = to_lower(s.substr(s.find_last_of(".") + 1));

						if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "dds" || ext == "tga")
							tasks.emplace_back(concurrency::create_task([s, mat_info]() {
							auto asset = (new TextureAsset(convert(s)));
							asset->try_register();

							if (s.find("albedo") != std::string::npos || s.find("diff") != std::string::npos || s.find("alb") != std::string::npos || s.find("base") != std::string::npos || s.find("color") != std::string::npos)
								mat_info->albedo = asset->get_ptr<TextureAsset>();

							if (s.find("metal") != std::string::npos)
								mat_info->metallic = asset->get_ptr<TextureAsset>();

							if (s.find("rough") != std::string::npos)
								mat_info->roughness = asset->get_ptr<TextureAsset>();


							if (s.find("height") != std::string::npos)
								mat_info->height = asset->get_ptr<TextureAsset>();

							if (s.find("normal") != std::string::npos)
								mat_info->normals = asset->get_ptr<TextureAsset>();



								}));
						else
							//   if (ext == "blend" || ext == "obj" || ext == "3ds" || ext == "dae" || ext == "fbx" || ext == "rw4" || ext == "stl")
							concurrency::create_task([this, s]()
								{

									auto context = std::make_shared<AssetLoadingContext>();
								//	context->ui = user_ui->get_ptr<GUI::user_interface>();
									(new MeshAsset(convert(s), context))->try_register();
								});
						ext = "";
					}

					when_all(begin(tasks), end(tasks)).then([mat_info, this]() {

						bool any = !!mat_info->albedo | !!mat_info->metallic | !!mat_info->roughness | !!mat_info->height | !!mat_info->normals;
						if (any)
							user_ui->message_box("create_material", "yes, we can", [mat_info](bool v) {
							if (v)
								mat_info->create();
								});
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

				menu->pos = vec2(but->get_render_bounds().pos);
				menu->self_open(user_ui);

			};
			AssetManager::get().add_listener(this);
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

			auto folders = std::make_shared<GUI::Elements::tree<folder_item>>();
			folders->size = { 200, 150 };
			folders->docking = GUI::dock::LEFT;
			add_child(folders);

			GUI::Elements::image::ptr divider(new 	GUI::Elements::image());
			divider->size = { 2, 2 };
			divider->docking = GUI::dock::LEFT;
			divider->texture = Skin::get().DefaultStatusBar.Normal;
			add_child(divider);


			folders->on_select = [this](folder_item* item)
			{
				table->remove_elements();
				current_folder = item;
				current_folder->iterate_assets([this](AssetStorage::ptr e)
					{
						asset_item::ptr elem(new asset_item(e));
						table->add_child(elem);
					});
			};
			folders->init(AssetManager::get().get_folders().get());
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


		void GUI::Elements::asset_explorer::draw(Context& c)
		{
			//	renderer->draw_container(get_ptr(), c);
		}
	}
}
