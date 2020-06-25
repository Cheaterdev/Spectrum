#include "pch.h"



namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{


			GraphElement::GraphElement(block_data * data)
			{
			//	trace = get_stack_trace();

				clickable = true;
			//	draw_helper = true;


				label::ptr name_lbl(new label);

				name_lbl->docking = dock::FILL;
				add_child(name_lbl);


				name_lbl->text = convert(data->block->get_name());

				texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/background.png"));
				texture.padding = {2,2,2,2 };

				name = data->block->get_name();


		//		get_label()->text = convert(name);

				start_time = std::chrono::duration<double>(data->start_time.time_since_epoch()).count();
					end_time = std::chrono::duration<double>(data->end_time.time_since_epoch()).count();

			}


			GraphElement::GraphElement(gpu_block_data * data)
			{
				//	trace = get_stack_trace();

				clickable = true;
				draw_helper = true;


				label::ptr name_lbl(new label);

				name_lbl->docking = dock::FILL;
				add_child(name_lbl);
				texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/background.png"));
				texture.padding = {2,2,2,2};

				name_lbl->text = convert(data->block->get_name());
				name = data->block->get_name();

				start_time = data->start_time;
					end_time = data->end_time;

			}

			void GraphElement::on_mouse_enter(vec2 pos)
			{
				if (info)
				{
					return;
				}

				image::ptr back(new image);
				back->texture.texture = Render::Texture::get_resource({ "textures/gui/background.png", false, false });

				back->width_size = size_type::MATCH_CHILDREN;
				back->height_size = size_type::MATCH_CHILDREN;

				label::ptr name(new label);
				name->docking = dock::TOP;
				name->text = convert(this->name);

				back->add_child(name);

				{
					label::ptr time(new label);
					time->docking = dock::TOP;
					time->text = std::to_string(1000 * (end_time - start_time)) + " ms";

					back->add_child(time);
				}



				user_ui->add_child(back);
				info = back;
				info->pos = pos + float2(10, 10);
			}

			bool GraphElement::on_mouse_move(vec2 pos)
			{
				info->pos = pos + float2(10, 10);
				return true;
			}

			void GraphElement::on_mouse_leave(vec2 pos)
			{
				info->remove_from_parent();
				info = nullptr;
			}

			void TimeGraph::build()
			{

				Profiler::get().enabled = false;
				ended = true;
				end = std::chrono::high_resolution_clock::now();
				run_on_ui([this]() {

					std::vector<base::ptr> thread_backs(threads.size());


					for (auto&&e : thread_backs)
					{

						image::ptr back(new image());
						back->docking = dock::TOP;
						back->height_size = size_type::MATCH_CHILDREN;
						back->width_size = size_type::FIXED;
						//	back->width_size = size_type::MATCH_CHILDREN;
						back->x_type = pos_x_type::LEFT;
					//	back->texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/debug_back.png"));
					//	back->texture.padding = { 16,16,16,16 };
						back->padding = { 4,4,4,4 };
						back->size = { 80000 * std::chrono::duration<double>(end - start).count() ,0 };
						front->add_child(back);
						e = back;



						image::ptr line(new image());
						line->docking = dock::TOP;
						line->height_size = size_type::FIXED;
						line->width_size = size_type::MATCH_PARENT;
						line->texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/background.png"));

						//	back->texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/debug_back.png"));
							back->texture.padding = { 1,1,1,1 };
						line->size = {1,1 };
						front->add_child(line);
						e = back;
					}


					for (auto& block : data.blocks)
					{
						GraphElement::ptr obj(new GraphElement(&block));
						obj->pos = { 80000 * std::chrono::duration<double>(block.start_time - start).count(), 50 * (block.block->calculate_depth() - 1) };
						obj->size = { 80000 * std::chrono::duration<double>(block.end_time - block.start_time).count() ,50 };

						thread_backs[block.thread_id - 1]->add_child(obj);

					}


					base::ptr gpu_block(new base);


					{

						image::ptr back(new image());
						back->docking = dock::TOP;
						back->height_size = size_type::MATCH_CHILDREN;
						back->width_size = size_type::FIXED;
						back->x_type = pos_x_type::LEFT;
					//	back->texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/debug_back.png"));
					//	back->texture.padding = { 16,16,16,16 };
					//	back->padding = { 4,4,4,4 };
						back->size = { 80000 * std::chrono::duration<double>(end - start).count() ,0 };
						front->add_child(back);
						gpu_block = back;
					}



					for (auto& block : data.gpu_blocks)
					{
						GraphElement::ptr obj(new GraphElement(&block));
						obj->pos = { 80000 * std::chrono::duration<double>(block.start_time - gpu_start).count(),  50 * (block.block->calculate_depth() - 1) };
						obj->size = { 80000 * std::chrono::duration<double>(block.end_time - block.start_time).count() ,50 };

						gpu_block->add_child(obj);

					}

				});
			}

			TimeGraph::TimeGraph()
			{
				docking = dock::FILL;

				start = std::chrono::high_resolution_clock::now();

				ended = true;

				button::ptr btn_start(new button);
				{
					base::add_child(btn_start);
					btn_start->size = { 25,25 };
					btn_start->on_click = [this](button::ptr b) {

						need_start = true;
						//	run_on_ui([this]() {

						//	});


					};
				}

				image::ptr back(new image());
				//back->docking = dock::TOP;
				back->height_size = size_type::MATCH_CHILDREN;
				back->width_size = size_type::MATCH_CHILDREN;
			//	back->x_type = pos_x_type::LEFT;
				back->texture.texture = Render::Texture::get_resource(Render::texure_header("textures/gui/debug_back.png"));
				back->texture.padding = { 16,16,16,16 };
				back->padding = { 4,4,4,4 };
				//back->size = {100,100 };
				scroll_container::add_child(back);

				front = back;
				Profiler::get().on_frame.register_handler(this, [this](UINT64 frame) {
					m.lock();
					if (need_start)
					{
						Profiler::get().enabled = true;
						need_start = false;

						data.reset();
						front->remove_all();

						gpu_start = GPUTimeManager::get().get_now();
						start = std::chrono::high_resolution_clock::now();

						ended = false;

						started_frame = frame;
					}

					m.unlock();


					if (!ended&&(frame - started_frame > 10))
					{
						build();
					}
				});


				Profiler::get().on_gpu_timer.register_handler(this, [this](TimedBlock* block) {

					
					if (ended) return;

					GPUBlock * b = dynamic_cast<GPUBlock*>(block);
					if (!b) return;

				

					m.lock();

					this->data.gpu_blocks.emplace_back();
					auto data = &this->data.gpu_blocks.back();
					//	data->thread_id = thread_id;
					data->block = block;


					data->start_time = b->gpu_counter.timer.get_start();
					data->end_time = b->gpu_counter.timer.get_end();

					m.unlock();

				});


				Profiler::get().on_cpu_timer_start.register_handler(this, [this](TimedBlock* block) {
					if (ended) return;
					auto this_id = std::this_thread::get_id();





					m.lock();
					auto &data = current_data[block];
					auto &thread_id = thread_ids[block];

				//	if (thread_id == 0)
					{
						auto & 	id2 = threads[this_id];
						if (id2 == 0)
							id2 = threads.size();
						thread_id = id2;
						thread_ids[block] = thread_id;
					}

					this->data.blocks.emplace_back();
					data = &this->data.blocks.back();
					data->thread_id = thread_id;
					data->block = block;
					data->start_time = block->cpu_counter.start_time;
					m.unlock();





				});



				Profiler::get().on_cpu_timer_end.register_handler(this, [this](TimedBlock* block) {
					if (ended) return;

					m.lock();
					auto &data = current_data[block];
					if (data)
					{

						data->end_time = block->cpu_counter.end_time;
			
					}
					m.unlock();

				});


			}

		}
	}
}