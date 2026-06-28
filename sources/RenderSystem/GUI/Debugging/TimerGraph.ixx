export module GUI:Debug.TimerGraph;
import :ScrollContainer;
import :Image;
import :Label;
import Core;
export namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{
			struct block_data
			{
				//TimedBlock *block;
				std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
				std::chrono::time_point<std::chrono::high_resolution_clock> end_time;

				std::thread::id native_id;
				size_t thread_id;
		
				uint depth;
				std::wstring name;
			};


			struct gpu_block_data
			{
				//TimedBlock *block;
				uint depth;
				double start_time;
				double end_time;
				HAL::CommandListType queue_type;
				std::wstring name;
			};
			struct collected_data
			{
				std::vector<block_data> blocks;
				std::vector<gpu_block_data> gpu_blocks;



				std::atomic_int block_id;

				std::atomic_int gpu_block_id;
				void reset();
			};
			class solid_rect : public image
			{
			public:
				using ptr = std::shared_ptr<solid_rect>;
				float4 color = {1,1,1,1};
				void draw(Context& c) override;
			};

			class GraphElement :public image
			{
			//	stack_trace trace;
				std::wstring name;

				float4 block_color;
				base::ptr info;
				double start_time;
				double end_time;

			public:


				using ptr = std::shared_ptr<GraphElement>;
				GraphElement(block_data* block);
				GraphElement(gpu_block_data* block);

				void draw(Context& c) override;
				void on_mouse_enter(vec2 pos) override;
				bool on_mouse_move(vec2 pos)override;
				void on_mouse_leave(vec2 pos) override;

			};


			class TimeGraph : public scroll_container, public CPUTimerListener
			{
				std::chrono::time_point<std::chrono::high_resolution_clock>  start;
				std::chrono::time_point<std::chrono::high_resolution_clock>  end;

		
				enum_array<HAL::CommandListType, HAL::ClockCalibrationInfo> clock_info;

				std::mutex m;

			//	std::map<TimedBlock*, block_data*> current_data;
			//	std::map<std::thread::id, size_t> threads;
			//	std::map<TimedBlock*, size_t> thread_ids;

				collected_data data;

				base::ptr front;
				base::ptr names_col;
				label::ptr max_level_lbl;
				void build();
				bool ended = true;
				bool need_start = false;

				UINT64 started_frame;

				//static thread_local block_data* data;

			public:

				TimeGraph();

				void on_cpu_start(TimedBlock* block) override;
				void on_cpu_end(TimedBlock* block)   override;
			};
		}
	}
}