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

			// ── zoom_overlay: left-drag to select a time range to zoom into ─────
			class zoom_overlay : public base
			{
				TimeGraph* owner;
				bool  selecting = false;
				float sel_x0    = 0;
				float sel_x1    = 0;
			public:
				zoom_overlay(TimeGraph* o) : owner(o)
				{
					docking   = dock::FILL;
					clickable = true;
				}

				bool on_mouse_action(mouse_action action, mouse_button btn, vec2 pos) override
				{
					if (btn != mouse_button::LEFT) return false;
					if (action == mouse_action::DOWN)
					{
						selecting = true;
						sel_x0 = sel_x1 = pos.x;
						return true;
					}
					if (action == mouse_action::UP)
					{
						bool do_zoom = selecting && std::abs(sel_x1 - sel_x0) > 5.0f;
						selecting = false;
						if (do_zoom) owner->zoom_to_selection(sel_x0, sel_x1);
						return true;
					}
					return false;
				}

				bool on_mouse_move(vec2 pos) override
				{
					if (selecting)
					{
						sel_x1 = pos.x;
						return true;
					}
					return false;
				}

				void draw(Context& c) override
				{
					if (!selecting || std::abs(sel_x1 - sel_x0) < 2.0f) return;
					rect   b    = get_render_bounds();
					float  lo   = std::min(sel_x0, sel_x1);
					float  hi   = std::max(sel_x0, sel_x1);
					float  w    = hi - lo;
					bool   zout = sel_x1 < sel_x0;
					float4 fill = zout ? float4{1.0f, 0.5f, 0.2f, 0.15f} : float4{0.3f, 0.6f, 1.0f, 0.15f};
					float4 edge = zout ? float4{1.0f, 0.6f, 0.3f, 0.8f}  : float4{0.4f, 0.7f, 1.0f, 0.8f};
					c.renderer->draw_color(c, fill, {lo,        b.pos.y, w,    b.size.y});
					c.renderer->draw_color(c, edge, {lo,        b.pos.y, 1.5f, b.size.y});
					c.renderer->draw_color(c, edge, {hi - 1.5f, b.pos.y, 1.5f, b.size.y});
				}
			};

			// ── TimeGraph::rebuild_display ────────────────────────────────────────

			void TimeGraph::rebuild_display()
			{
				float  scaler = build_scaler;
				float  t0_px  = float(build_t0) * scaler;

				run_on_ui([this, scaler, t0_px]()
				{
					front->remove_all();
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

					float frame_s     = float(std::chrono::duration<double>(end - start).count());
					float total_width = scaler * frame_s;

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

						float frame_ms   = frame_s * 1000.0f;
						float vw         = filled->get_render_bounds().w;
						float visible_ms = (vw > 0 && scaler > 0) ? 1000.0f * vw / scaler : frame_ms;
						float tick_ms;
						if      (visible_ms > 30.0f) tick_ms = 5.0f;
						else if (visible_ms >  8.0f) tick_ms = 2.0f;
						else if (visible_ms >  2.0f) tick_ms = 1.0f;
						else if (visible_ms >  0.5f) tick_ms = 0.25f;
						else                         tick_ms = 0.05f;

						for (float ms = 0; ms <= frame_ms + tick_ms * 0.5f; ms += tick_ms)
						{
							float px = scaler * ms / 1000.0f;

							base::ptr tick(new base());
							tick->docking     = dock::NONE;
							tick->x_type      = pos_x_type::LEFT;
							tick->y_type      = pos_y_type::TOP;
							tick->pos         = {px, 0};
							tick->width_size  = size_type::FIXED;
							tick->height_size = size_type::FIXED;
							tick->size        = {1, 8};
							ruler->add_child(tick);

							label::ptr lbl(new label());
							lbl->docking     = dock::NONE;
							lbl->x_type      = pos_x_type::LEFT;
							lbl->y_type      = pos_y_type::TOP;
							lbl->pos         = {px + 2, 8};
							lbl->width_size  = size_type::FIXED;
							lbl->height_size = size_type::FIXED;
							lbl->size        = {60, 12};
							if (tick_ms < 1.0f)
								lbl->text = std::to_string(ms).substr(0, 5) + "ms";
							else
								lbl->text = std::to_string(int(ms)) + "ms";
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
					const wchar_t* queue_names[3]      = {L"GPU · Direct", L"GPU · Compute", L"GPU · Copy"};
					const wchar_t* queue_lane_names[3] = {L"Direct",       L"Compute",       L"Copy"};
					float4         queue_colors[3]     = {
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

					// Schedule scroll so the selection start appears just after the names column.
					// pending_scroll_x is applied in resized() after children are laid out
					// (so the clamping uses the correct new scaled_size, not the stale one).
					// We subtract front->padding->left so that content x=0 (time=0) aligns
					// with the same origin used in zoom_to_selection.
					float names_w      = names_col ? names_col->get_render_bounds().w : 0.0f;
					float padding_l    = float(front->padding->left);
					pending_scroll_x   = names_w - padding_l - t0_px;
					has_pending_scroll = true;
				});
			}

			// ── TimeGraph::build ──────────────────────────────────────────────────

			void TimeGraph::build()
			{
				Profiler::get().enabled = false;
				ended = true;
				end   = std::chrono::high_resolution_clock::now();
				rebuild_display();
			}

			// ── TimeGraph::zoom_to_selection ──────────────────────────────────────

			void TimeGraph::zoom_to_selection(float x0, float x1)
			{
				float names_w    = names_col ? names_col->get_render_bounds().w : 0.0f;
				float eff_vw     = filled->get_render_bounds().w - names_w;
				if (eff_vw <= 0) return;

				float origin = front->get_render_bounds().x + float(front->padding->left);

				if (x1 > x0)
				{
					// zoom in: selected range fills viewport
					double dt0 = double(x0 - origin) / build_scaler;
					double dt1 = double(x1 - origin) / build_scaler;
					if (dt1 - dt0 < 1e-9) return;
					build_scaler = eff_vw / float(dt1 - dt0);
					build_t0     = dt0;
				}
				else
				{
					// zoom out: drag_width maps inversely; center stays at viewport center
					float  drag_px  = x0 - x1;
					double center_t = double((x0 + x1) * 0.5f - origin) / build_scaler;
					build_scaler    = build_scaler * drag_px / eff_vw;
					build_t0        = center_t - double(eff_vw * 0.5f) / build_scaler;
				}

				rebuild_display();
			}

			// ── TimeGraph::zoom_reset ─────────────────────────────────────────────

			void TimeGraph::zoom_reset()
			{
				build_scaler = 500000.0f;
				build_t0     = 0.0;
				rebuild_display();
			}

			// ── TimeGraph::resized ───────────────────────────────────────────────
			// scroll_container::update_layout calls resized() BEFORE children are
			// laid out, so scaled_size is stale and clamps the wrong range.
			// A second resized() fires from c_contents::on_size_changed (after
			// layout) with the correct scaled_size — that one does the real clamp.
			// We write pending_scroll_x in the first call so the second call clamps
			// the right value against the freshly-computed content width.

			void TimeGraph::resized()
			{
				scroll_container::resized();
				if (has_pending_scroll)
				{
					has_pending_scroll = false;
					contents->pos = {pending_scroll_x, contents->pos->y};
				}
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
				btn_start->size        = {80, 26};
				btn_start->on_click = [this](button::ptr) { need_start = true; Profiler::get().set_cpu_listener(this);};
				btn_start->get_label()->text = "capture";
				toolbar->add_child(btn_start);

				button::ptr btn_zoom_out(new button);
				btn_zoom_out->docking     = dock::LEFT;
				btn_zoom_out->width_size  = size_type::FIXED;
				btn_zoom_out->height_size = size_type::FIXED;
				btn_zoom_out->size        = {80, 26};
				btn_zoom_out->on_click    = [this](button::ptr) { zoom_reset(); };
				btn_zoom_out->get_label()->text = "zoom out";
				toolbar->add_child(btn_zoom_out);

				// ── max_level controls ─────────────────────────────────────────
				{
					label::ptr lbl(new label());
					lbl->docking     = dock::LEFT;
					lbl->width_size  = size_type::FIXED;
					lbl->height_size = size_type::FIXED;
					lbl->size        = {60, 26};
					lbl->text        = "  depth:";
					lbl->color       = {0.6f, 0.6f, 0.6f, 1.0f};
					toolbar->add_child(lbl);

					// decrement
					button::ptr btn_dec(new button);
					btn_dec->docking     = dock::LEFT;
					btn_dec->width_size  = size_type::FIXED;
					btn_dec->height_size = size_type::FIXED;
					btn_dec->size        = {22, 26};
					btn_dec->on_click    = [this](button::ptr) {
						auto& ml = Profiler::get().max_level;
						if (ml > 1 && ml != std::numeric_limits<int>::max()) --ml;
						max_level_lbl->text = ml == std::numeric_limits<int>::max() ? "inf" : std::to_string(ml);
					};
					btn_dec->get_label()->text = "-";
					toolbar->add_child(btn_dec);

					// current value label
					label::ptr val_lbl(new label());
					val_lbl->docking     = dock::LEFT;
					val_lbl->width_size  = size_type::FIXED;
					val_lbl->height_size = size_type::FIXED;
					val_lbl->size        = {24, 26};
					val_lbl->text        = "inf";
					val_lbl->color       = {1.0f, 1.0f, 1.0f, 1.0f};
					toolbar->add_child(val_lbl);
					max_level_lbl = val_lbl;

					// increment
					button::ptr btn_inc(new button);
					btn_inc->docking     = dock::LEFT;
					btn_inc->width_size  = size_type::FIXED;
					btn_inc->height_size = size_type::FIXED;
					btn_inc->size        = {22, 26};
					btn_inc->on_click    = [this](button::ptr) {
						auto& ml = Profiler::get().max_level;
						if (ml == std::numeric_limits<int>::max()) ml = 1;
						else ++ml;
						max_level_lbl->text = std::to_string(ml);
					};
					btn_inc->get_label()->text = "+";
					toolbar->add_child(btn_inc);

					// reset to inf
					button::ptr btn_inf(new button);
					btn_inf->docking     = dock::LEFT;
					btn_inf->width_size  = size_type::FIXED;
					btn_inf->height_size = size_type::FIXED;
					btn_inf->size        = {22, 26};
					btn_inf->on_click    = [this](button::ptr) {
						Profiler::get().max_level = std::numeric_limits<int>::max();
						max_level_lbl->text = "inf";
					};
					btn_inf->get_label()->text = "inf";
					toolbar->add_child(btn_inf);
				}

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

				// ── Zoom selection overlay (left-drag on the mover layer) ──────
				{
					auto zo = std::make_shared<zoom_overlay>(this);
					over_filled->add_child(zo);
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

				
			}
		}
	}
}

void GUI::Elements::Debug::TimeGraph::on_cpu_start(TimedBlock* block)
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
}

void GUI::Elements::Debug::TimeGraph::on_cpu_end(TimedBlock* block)
{
	if (ended) return;
	if (block->id < 0 || block->id >= int(data.blocks.size())) return;
	data.blocks[block->id].end_time = block->cpu_counter.end_time;
}
