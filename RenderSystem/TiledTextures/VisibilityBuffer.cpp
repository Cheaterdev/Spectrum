#include "pch.h"



VisibilityBuffer::VisibilityBuffer(ivec3 sizes) :sizes(sizes)
{
	buffer.reset(new Render::ByteBuffer(sizes.x*sizes.y*sizes.z * 4));
	buffer->set_name("VisibilityBuffer::buffer");
	clear_data.resize(sizes.x*sizes.y*sizes.z*4, 255);
	buffer->set_data(0, clear_data);
}

std::function<void(const Handle&)> VisibilityBuffer::uav()
{
	return buffer->uav();
}

void VisibilityBuffer::wait_for_results()
{
	if (waiter.valid())
		waiter.get();
}

void VisibilityBuffer::update(CommandList::ptr& list)
{

	list->transition_uav(buffer.get());
	/*list->read_buffer(buffer.get(), 0, buffer->get_size(), [this](const char* data, UINT64 size)
	{
		std::vector<task<bool>> tasks;


		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//	const int* i_data = reinterpret_cast<const int*>(data);
		size /= sizeof(int);

		int thread_count = std::min(sizes.y, 8);
		int one_thread = size / thread_count;
		//memcpy(resolved_data.data(), data, size);
		//   auto int_data = reinterpret_cast<const unsigned int*>(data);
		std::atomic_int counter = 0;
		// if (Application::get().is_alive())
		for (int thread = 0; thread < thread_count; thread++)
			tasks.emplace_back(create_task([this, thread, one_thread, data,&counter]()
		{


			for (int i = one_thread*thread; i < one_thread*thread + one_thread; i++)
			{


				int x = i % sizes.x;
				int z = i / (sizes.y*sizes.x);
				int y = ((i - x) / sizes.x) % sizes.y;
				if (data[4*i] == 0)
					++counter;
				process_tile_readback({ x,y,z }, data[4 * i]);

			}
			return true;
		}));

			for (auto &t : tasks)
			t.wait();

	Log::get() << "counter " << counter.load() << Log::endl;

	});*/

	waiter = list->get_copy().read_buffer(buffer.get(), 0, buffer->get_size(), [this](const char* data, UINT64 size)
	{


		std::vector<task<std::vector<vec3>>> tasks;

	
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		//	const int* i_data = reinterpret_cast<const int*>(data);
		size /= sizeof(int);

		int thread_count = std::min(sizes.y, 8);
		int one_thread = size / thread_count;
		//memcpy(resolved_data.data(), data, size);
		//   auto int_data = reinterpret_cast<const unsigned int*>(data);
		std::atomic_int counter = 0;
		// if (Application::get().is_alive())
		for (int thread = 0; thread < thread_count; thread++)
			tasks.emplace_back(create_task([this, thread, one_thread, data, &counter]()
		{

			std::vector<vec3> poses;
			for (int i = one_thread*thread; i < one_thread*thread + one_thread; i++)
			{


				int x = i % sizes.x;
				int z = i / (sizes.y*sizes.x);
				int y = ((i - x) / sizes.x) % sizes.y;
				if (data[4 * i] == 0)
					poses.emplace_back(float3(x,y,z));
					++counter;
			//	process_tile_readback({ x,y,z }, data[4 * i]);

			}
			return poses;
		}));

		for (auto &t : tasks)
		{
			auto result = t.get();
			for(auto &p:result)
			process_tile_readback(p,0);

		}
			
		Log::get() << "counter " << counter.load() << Log::endl;

	});


	list->transition_uav(buffer.get());
	list->transition(buffer.get(), Render::ResourceState::COPY_DEST);
	list->get_copy().update_buffer(buffer.get(), 0, reinterpret_cast<char*>(clear_data.data()), clear_data.size());
	list->transition(buffer.get(), Render::ResourceState::UNORDERED_ACCESS);
	list->transition_uav(buffer.get());

	
}

void VisibilityBufferUniversal::process_tile_readback(ivec3 pos, char level)
{
	on_process(pos, level);
}
