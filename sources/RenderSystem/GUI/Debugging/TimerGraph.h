#pragma once

namespace GUI
{
	namespace Elements
	{
		namespace Debug
		{
			struct block_data
			{
				TimedBlock *block;
				std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
				std::chrono::time_point<std::chrono::high_resolution_clock> end_time;

				std::thread::id native_id;
				size_t thread_id;
			};


			struct gpu_block_data
			{
				TimedBlock *block;

				double start_time;
				double end_time;


			};
			struct collected_data
			{
				std::vector<block_data> blocks;
				std::vector<gpu_block_data> gpu_blocks;



				std::atomic_int block_id;

				std::atomic_int gpu_block_id;
				void reset()
				{
					block_id = 0;
					gpu_block_id = 0;
					blocks.resize(4096 * 128);
					gpu_blocks.resize(4096 * 128);
				}
			};
			class GraphElement :public image
			{
			//	stack_trace trace;
				std::wstring name;


				base::ptr info;
				double start_time;
				double end_time;
			public:


				using ptr = std::shared_ptr<GraphElement>;
				GraphElement(block_data* block);
				GraphElement(gpu_block_data* block);

				void on_mouse_enter(vec2 pos) override;
				bool on_mouse_move(vec2 pos)override;
				void on_mouse_leave(vec2 pos) override;

			};


			class TimeGraph : public scroll_container
			{
				std::chrono::time_point<std::chrono::high_resolution_clock>  start;
				std::chrono::time_point<std::chrono::high_resolution_clock>  end;

				double gpu_start;
				std::mutex m;

			//	std::map<TimedBlock*, block_data*> current_data;
			//	std::map<std::thread::id, size_t> threads;
			//	std::map<TimedBlock*, size_t> thread_ids;

				collected_data data;

				base::ptr front;
				void build();
				bool ended = false;
				bool need_start = false;

				UINT64 started_frame;

				//static thread_local block_data* data;

			public:

				TimeGraph();


			};
		}
	}
}