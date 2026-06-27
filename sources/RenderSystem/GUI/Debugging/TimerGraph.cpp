module GUI:Debug.TimerGraph;
import RenderSystem;

import :Label;
import :Button;
  import :Renderer;

import HAL;

using namespace HAL;

namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{
			void collected_data::reset()
			{
				block_id = 0;
				gpu_block_id = 0;
				blocks.resize(4096 * 128);
				gpu_blocks.resize(4096 * 256);
			}
		}
	}
}

namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{
			// ── left_column: fixed left overlay that pins row names ──────────────
			// dock::PARENT → placed in update_childs_layout_after, so all rows have
			// valid render_bounds by the time we sync label positions.
			class left_column : public solid_rect
			{
				struct entry { label::ptr lbl; base::wptr row; };
				std::vector<entry> entries;
			public:
				left_column()
				{
					color   = {0.07f, 0.07f, 0.09f, 0.95f};
					docking = dock::PARENT;
					margin  = {0, 30, 0, 0}; // 30px top = below toolbar
					// width_size/height_size stay NONE so filter_func in render_bounds
					// does NOT override the size we set in update_layout.
				}

				void add_entry(const std::wstring& text, base::ptr row, float4 text_color = {1,1,1,0.85f})
				{
					label::ptr lbl(new label);
					lbl->docking     = dock::NONE;
					lbl->x_type      = pos_x_type::LEFT;
					lbl->y_type      = pos_y_type::TOP;
					lbl->text        = convert(text);
					lbl->color       = text_color;
					lbl->width_size  = size_type::FIXED;
					lbl->height_size = size_type::FIXED;
					lbl->size        = {116, 18};
					add_child(lbl);
					entries.push_back({lbl, row});
				}

				void clear_entries()
				{
					remove_all();
					entries.clear();
				}

				// Always re-sync: row render_bounds change on every scroll event.
				sizer update_layout(sizer r, float scale) override
				{
					need_update_layout = true;
					sizer remaining = base::update_layout(r, scale);
					// Constrain width to 120px; height is driven by dock::PARENT margins.
					rect rb = get_render_bounds();
					render_bounds = rect{rb.pos, 120.0f, rb.size.y};
					return remaining;
				}

				// Called during update_layout when need_update_layout is true.
				// At this point all rows already have correct render_bounds (dock::PARENT
				// guarantees we run after update_childs_layout_after of the parent).
				void update_childs_layout(sizer& my, float scale) override
				{
					float my_top = get_render_bounds().pos.y;
					for (auto& e : entries)
					{
						if (auto row = e.row.lock())
						{
							auto rb    = row->get_render_bounds();
							e.lbl->pos  = {4.0f, rb.pos.y - my_top};
							e.lbl->size = {112.0f, rb.size.y};
						}
					}
					base::update_childs_layout(my, scale);
				}
			};

			// ── Per-thread hue (HSV → RGB, dark, saturated) ───────────────────────
			static float4 thread_hue_color(size_t thread_id)
			{
				float hue = std::fmod(float(thread_id - 1) * 0.618033f, 1.0f);
				float s = 0.55f, v = 0.11f;
				float hh = hue * 6.0f;
				int   i  = int(hh) % 6;
				float f  = hh - float(int(hh));
				float p = v*(1-s), q = v*(1-s*f), t = v*(1-s*(1-f));
				switch (i) {
					case 0: return {v,t,p,1};
					case 1: return {q,v,p,1};
					case 2: return {p,v,t,1};
					case 3: return {p,q,v,1};
					case 4: return {t,p,v,1};
					default: return {v,p,q,1};
				}
			}

			// ── Color from name hash (HSV→RGB) ────────────────────────────────────
			static float4 name_color(const std::wstring& name)
			{
				size_t h = 0;
				for (wchar_t c : name) h = h * 131 + static_cast<size_t>(c);
				float hue = float(h % 1009) / 1009.0f;
				float s = 0.55f, v = 0.72f;
				float hh = hue * 6.0f;
				int   i  = int(hh) % 6;
				float f  = hh - float(int(hh));
				float p = v*(1-s), q = v*(1-s*f), t = v*(1-s*(1-f));
				switch (i) {
					case 0: return {v, t, p, 1};
					case 1: return {q, v, p, 1};
					case 2: return {p, v, t, 1};
					case 3: return {p, q, v, 1};
					case 4: return {t, p, v, 1};
					default:return {v, p, q, 1};
				}
			}

			// ── solid_rect ────────────────────────────────────────────────────────

			void solid_rect::draw(Context& c)
			{
				c.renderer->draw_color(c, color, get_render_bounds());
			}

			// ── GraphElement ──────────────────────────────────────────────────────

			static void setup_block(GraphElement* e, const std::wstring& block_name)
			{
				label::ptr lbl(new label);
				lbl->docking = dock::FILL;
				lbl->text    = convert(block_name);
				lbl->color   = {1.0f, 1.0f, 1.0f, 0.9f};
				e->add_child(lbl);
			}

			void GraphElement::draw(Context& c)
			{
				auto bounds = get_render_bounds();
				// 1px darker border, then fill
				float4 border = { block_color.x * 0.6f, block_color.y * 0.6f, block_color.z * 0.6f, 1.0f };
				c.renderer->draw_color(c, border, bounds);
				rect inner =  {bounds.pos.x + 1, bounds.pos.y + 1, bounds.size.x - 2, bounds.size.y - 2} ;
				c.renderer->draw_color(c, block_color, inner);
			}

			GraphElement::GraphElement(block_data* data)
			{
				clickable   = true;
				name        = data->name;
				block_color = name_color(name);
				setup_block(this, name);
				start_time = std::chrono::duration<double>(data->start_time.time_since_epoch()).count();
				end_time   = std::chrono::duration<double>(data->end_time.time_since_epoch()).count();
			}

			GraphElement::GraphElement(gpu_block_data* data)
			{
				clickable   = true;
				draw_helper = true;
				name        = data->name;
				block_color = name_color(name);
				setup_block(this, name);
				start_time  = data->start_time;
				end_time    = data->end_time;
			}

			void GraphElement::on_mouse_enter(vec2 pos)
			{
				if (info) return;

				solid_rect::ptr back(new solid_rect());
				back->color      = {0.10f, 0.10f, 0.13f, 0.95f};
				back->width_size     = size_type::MATCH_CHILDREN;
				back->height_size    = size_type::MATCH_CHILDREN;
				back->padding        = {6, 4, 6, 4};

				label::ptr name_lbl(new label);
				name_lbl->docking = dock::TOP;
				name_lbl->text    = convert(this->name);
				back->add_child(name_lbl);

				label::ptr time_lbl(new label);
				time_lbl->docking = dock::TOP;
				time_lbl->text    = std::to_string(1000.0 * (end_time - start_time)) + " ms";
				time_lbl->color   = {0.6f, 0.6f, 0.6f, 1.0f};
				back->add_child(time_lbl);

				user_ui->add_child(back);
				info     = back;
				info->pos = pos + float2(12, 12);
			}

			bool GraphElement::on_mouse_move(vec2 pos)
			{
				if (info) info->pos = pos + float2(12, 12);
				return true;
			}

			void GraphElement::on_mouse_leave(vec2 pos)
			{
				if (info) info->remove_from_parent();
				info = nullptr;
			}

			// ── TimeGraph::build ──────────────────────────────────────────────────

			void TimeGraph::build()
			{
				const float scaler = 500000.0f;
				Profiler::get().enabled = false;
				ended = true;
				end   = std::chrono::high_resolution_clock::now();

				run_on_ui([this, scaler]()
				{
					auto* col = static_cast<left_column*>(names_col.get());
					col->clear_entries();

					std::map<std::thread::id, size_t> thread_ids;

					int blocks_size     = data.block_id;
					int gpu_blocks_size = data.gpu_block_id;

					for (int i = 0; i < blocks_size; i++)
					{
						auto& block = data.blocks[i];
						if (thread_ids.count(block.native_id) == 0)
							thread_ids[block.native_id] = thread_ids.size() + 1;
						block.thread_id = thread_ids[block.native_id];
					}

					float total_width = scaler * float(std::chrono::duration<double>(end - start).count());

					// ── Helpers ────────────────────────────────────────────────
					auto add_section = [&](const std::wstring& title, float4 bg)
					{
						solid_rect::ptr hdr(new solid_rect());
						hdr->color       = bg;
						hdr->docking     = dock::TOP;
						hdr->height_size = size_type::FIXED;
						hdr->width_size  = size_type::FIXED;
						hdr->x_type      = pos_x_type::LEFT;
						hdr->size        = {total_width, 18};
						hdr->padding     = {8, 0, 0, 0};

						label::ptr lbl(new label());
						lbl->docking = dock::FILL;
						lbl->text    = convert(title);
						lbl->color   = {1.0f, 1.0f, 1.0f, 0.85f};
						hdr->add_child(lbl);
						front->add_child(hdr);
					};

					auto add_lane = [&](const std::wstring& lane_name = L"", float4 lane_color = {0.05f, 0.05f, 0.07f, 1.0f}) -> base::ptr
					{
						solid_rect::ptr row(new solid_rect());
						row->color       = lane_color;
						row->docking     = dock::TOP;
						row->height_size = size_type::MATCH_CHILDREN;
						row->width_size  = size_type::FIXED;
						row->x_type      = pos_x_type::LEFT;
						row->size        = {total_width, 0};
						row->padding     = {0, 2, 0, 2};
						front->add_child(row);
						if (!lane_name.empty())
							col->add_entry(lane_name, row);
						return row;
					};

					auto add_separator = [&]()
					{
						base::ptr sep(new base());
						sep->docking     = dock::TOP;
						sep->height_size = size_type::FIXED;
						sep->size        = {total_width, 4};
						front->add_child(sep);
					};

					// ── Time ruler ────────────────────────────────────────────
					{
						solid_rect::ptr ruler(new solid_rect());
						ruler->color       = {0.08f, 0.08f, 0.10f, 1.0f};
						ruler->docking     = dock::TOP;
						ruler->height_size = size_type::FIXED;
						ruler->width_size  = size_type::FIXED;
						ruler->x_type      = pos_x_type::LEFT;
						ruler->size        = {total_width, 20};
						front->add_child(ruler);

						float frame_ms = 1000.0f * float(std::chrono::duration<double>(end - start).count());
						float tick_ms  = frame_ms > 30.0f ? 5.0f : (frame_ms > 8.0f ? 2.0f : 1.0f);

						for (float ms = 0; ms <= frame_ms + tick_ms * 0.5f; ms += tick_ms)
						{
							float px = scaler * ms / 1000.0f;

							// tick line
							base::ptr tick(new base());
							tick->docking     = dock::NONE;
							tick->x_type     = pos_x_type::LEFT;
							tick->y_type     = pos_y_type::TOP;
							tick->pos         = {px, 0};
							tick->width_size  = size_type::FIXED;
							tick->height_size = size_type::FIXED;
							tick->size        = {1, 8};
							ruler->add_child(tick);

							// tick label
							label::ptr lbl(new label());
							lbl->docking     = dock::NONE;
							lbl->x_type     = pos_x_type::LEFT;
							lbl->y_type     = pos_y_type::TOP;
							lbl->pos         = {px + 2, 8};
							lbl->width_size  = size_type::FIXED;
							lbl->height_size = size_type::FIXED;
							lbl->size        = {60, 12};
							lbl->text        = std::to_string(int(ms)) + "ms";
							lbl->color       = {0.5f, 0.5f, 0.5f, 0.8f};
							ruler->add_child(lbl);
						}
					}

					// ── CPU threads ───────────────────────────────────────────
					if (blocks_size > 0)
					{
						add_section(L"CPU", {0.10f, 0.20f, 0.38f, 1.0f});

						std::vector<base::ptr> thread_backs(thread_ids.size());
						for (size_t ti = 0; ti < thread_backs.size(); ti++)
						{
							std::wstring tname = L"Thread " + std::to_wstring(ti + 1);
							thread_backs[ti] = add_lane(tname, thread_hue_color(ti + 1));
						}

						for (int i = 0; i < blocks_size; i++)
						{
							auto& block = data.blocks[i];
							GraphElement::ptr obj(new GraphElement(&block));
							float x = scaler * float(std::chrono::duration<double>(block.start_time - start).count());
							float w = scaler * float(std::chrono::duration<double>(block.end_time - block.start_time).count());
							obj->pos  = {x, 24.0f * float(block.depth - 1)};
							obj->size = {std::max(w, 2.0f), 23};
							thread_backs[block.thread_id - 1]->add_child(obj);
						}
					}

					// ── GPU queues ────────────────────────────────────────────
					const wchar_t* queue_names[3]     = {L"GPU · Direct", L"GPU · Compute", L"GPU · Copy"};
					const wchar_t* queue_lane_names[3] = {L"Direct",      L"Compute",       L"Copy"};
					float4         queue_colors[3]  = {
						{0.18f, 0.10f, 0.32f, 1.0f},
						{0.08f, 0.22f, 0.16f, 1.0f},
						{0.08f, 0.18f, 0.28f, 1.0f}
					};

					base::ptr gpu_lanes[3] = {};

					bool any_gpu = gpu_blocks_size > 0;
					if (any_gpu)
					{
						add_separator();
						for (int q = 0; q < 3; q++)
						{
							add_section(queue_names[q], queue_colors[q]);
							gpu_lanes[q] = add_lane(queue_lane_names[q]);
						}
					}

					for (int i = 0; i < gpu_blocks_size; i++)
					{
						auto& block = data.gpu_blocks[i];
						if (!gpu_lanes[int(block.queue_type)]) continue;

						GraphElement::ptr obj(new GraphElement(&block));

						double gpu_start = double(clock_info[block.queue_type].gpu_time) / clock_info[block.queue_type].frequency;
						float  x = scaler * float(block.start_time - gpu_start);
						float  w = scaler * float(block.end_time - block.start_time);
						obj->pos  = {x, 24.0f * float(block.depth)};
						obj->size = {std::max(w, 3.0f), 23};
						gpu_lanes[int(block.queue_type)]->add_child(obj);
					}
				});
			}

			// ── TimeGraph constructor ─────────────────────────────────────────────

			TimeGraph::TimeGraph()
			{
				docking = dock::FILL;
				ended   = true;

				// ── Toolbar ────────────────────────────────────────────────────
				solid_rect::ptr toolbar(new solid_rect());
				toolbar->color       = {0.08f, 0.08f, 0.10f, 1.0f};
				toolbar->docking     = dock::TOP;
				toolbar->height_size = size_type::FIXED;
				toolbar->size        = {0, 30};
				toolbar->padding     = {6, 0, 6, 0};
				base::add_child(toolbar);

				button::ptr btn_start(new button);
				btn_start->docking     = dock::LEFT;
				btn_start->width_size  = size_type::FIXED;
				btn_start->height_size = size_type::FIXED;
				btn_start->size        = {80, 22};
				btn_start->on_click = [this](button::ptr) { need_start = true; };
				toolbar->add_child(btn_start);

				label::ptr btn_lbl(new label());
				btn_lbl->docking = dock::FILL;
				btn_lbl->text    = "capture";
				btn_start->add_child(btn_lbl);

				// ── Scrollable content ─────────────────────────────────────────
				image::ptr back(new image());
				back->height_size  = size_type::MATCH_CHILDREN;
				back->width_size   = size_type::MATCH_CHILDREN;
				back->texture      = Skin::get().DebugBack;
				back->padding      = back->texture.padding;
				scroll_container::add_child(back);
				front = back;

				// ── Fixed left names column (dock::PARENT overlay, non-scrolling) ──
				{
					auto col = std::make_shared<left_column>();
					names_col = col;
					base::add_child(col);
				}

				// ── Profiler hooks ─────────────────────────────────────────────
				Profiler::get().on_frame.register_handler(this, [this](UINT64 frame)
				{
					PROFILE(L"Profiler");
					m.lock();
					if (need_start)
					{
						Profiler::get().enabled = true;
						need_start = false;
						data.reset();
						front->remove_all();

						clock_info[CommandListType::DIRECT]  = RenderSystem::get().device().get_queue(CommandListType::DIRECT)->get_clock_time();
						clock_info[CommandListType::COMPUTE] = RenderSystem::get().device().get_queue(CommandListType::COMPUTE)->get_clock_time();
						clock_info[CommandListType::COPY]    = RenderSystem::get().device().get_queue(CommandListType::COPY)->get_clock_time();
						start         = std::chrono::high_resolution_clock::now();
						ended         = false;
						started_frame = frame;
					}
					m.unlock();

					if (!ended && (frame - started_frame > 10))
						run_on_ui([this]() { build(); });
				});

				Profiler::get().on_gpu_timer.register_handler(this, [this](std::pair<TimedBlock*, GPUTimerInterface*> p)
				{
					if (ended) return;
					auto& timer  = *static_cast<GPUTimer*>(p.second);
					auto  my_id  = data.gpu_block_id.fetch_add(1);
					ASSERT(my_id < 4096 * 256);
					auto& d      = data.gpu_blocks[my_id];
					d.name       = p.first->get_name();
					d.depth      = p.first->level;
					d.start_time = static_cast<double>(timer.get_start()) / clock_info[timer.queue_type].frequency;
					d.end_time   = static_cast<double>(timer.get_end())   / clock_info[timer.queue_type].frequency;
					d.queue_type = timer.queue_type;
				});

				Profiler::get().on_cpu_timer_start.register_handler(this, [this](TimedBlock* block)
				{
					if (ended) return;
					auto  my_id  = data.block_id.fetch_add(1);
					ASSERT(my_id < 4096 * 128);
					auto& d      = data.blocks[my_id];
					d.name       = block->get_name();
					d.depth      = block->level;
					d.native_id  = std::this_thread::get_id();
					block->id    = my_id;
					d.start_time = block->cpu_counter.start_time;
				});

				Profiler::get().on_cpu_timer_end.register_handler(this, [this](TimedBlock* block)
				{
					if (ended) return;
					if (block->id < 0 || block->id >= int(data.blocks.size())) return;
					data.blocks[block->id].end_time = block->cpu_counter.end_time;
				});
			}
		}
	}
}
