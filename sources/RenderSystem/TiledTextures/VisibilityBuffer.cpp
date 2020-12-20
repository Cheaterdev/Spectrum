#include "pch.h"



VisibilityBuffer::VisibilityBuffer(ivec3 sizes) :sizes(sizes)
{
	buffer.reset(new Render::ByteBuffer(sizes.x*sizes.y*sizes.z * 4));
	buffer->set_name("VisibilityBuffer::buffer");
	clear_data.resize(sizes.x*sizes.y*sizes.z*4, 255);
	buffer->set_data(0, clear_data);
}

std::function<void( Handle&)> VisibilityBuffer::uav()
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
//TODO: GPU!!
	waiter = list->get_copy().read_buffer(buffer.get(), 0, buffer->get_size(), [this](const char* data, UINT64 size)
	{
		std::vector<task<std::vector<ivec3>>> tasks;	
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		size /= sizeof(int);

		int thread_count = std::min(sizes.y, 8);
		UINT one_thread = UINT(size / thread_count);

		for (int thread = 0; thread < thread_count; thread++)
			tasks.emplace_back(create_task([this, thread, one_thread, data]()
		{

			std::vector<ivec3> poses;
			for (UINT i = one_thread*thread; i < one_thread*thread + one_thread; i++)
			{

				ivec3 t = sizes;
				int x = i % t.x;
				int z = i / (t.y*t.x);
				int y = ((i - x) / t.x) % t.y;
				if (data[4 * i] == 0)
					poses.emplace_back(ivec3(x,y,z));
					
			//	process_tile_readback({ x,y,z }, data[4 * i]);

			}
			return poses;
		}));

		int counter = 0;
		for (auto &t : tasks)
		{
			auto result = t.get();
			for(auto &p:result)
			process_tile_readback(p,0);

			counter += (UINT)result.size();
		}
			
		Log::get() << "counter " << counter << Log::endl;

	});

	list->get_copy().update_buffer(buffer.get(), 0, reinterpret_cast<char*>(clear_data.data()), (UINT)clear_data.size());
}

void VisibilityBufferUniversal::process_tile_readback(ivec3 pos, char level)
{
	on_process(pos, level);
}
