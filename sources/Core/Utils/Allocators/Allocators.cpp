module Core:Allocators;


import :Debug;
import :Utils;
import :Math;
import stl.core;
import stl.threading;



CommonAllocator::CommonAllocator(uint64 start_region, uint64 end_region) :size(end_region - start_region), start_region(start_region), end_region(end_region)
{
	reset_id = 0;
	Reset();
		check();
}
CommonAllocator::CommonAllocator(uint64 size /*= std::numeric_limits<uint64>::max()*/) : size(size), start_region(0), end_region(size)
{
	reset_id = 0;
	Reset();
		check();
}

uint64 CommonAllocator::get_max_usage() const
{
	return max_usage;
}

std::optional<CommonAllocator::Handle>  CommonAllocator::TryAllocate(uint64 size, uint64 align)
{
	ASSERT_SINGLETHREAD
		//std::lock_guard<std::mutex> g(m);
		if (size == 0)
		{
			//uint64 id;
			//MemoryInfo& res = tracker.place_handle(id);
			//res.aligned_offset = start_region;
			//res.offset = start_region;
			//res.size = start_region;
			////	res.owner = this;
			//res.reset_id = reset_id;
			return Handle(MemoryInfo(start_region, 0,0), this);
		}

		check();
	auto make_free_block = [this](block& new_block)
	{

		assert(fences.find(new_block.begin)==fences.end());
		assert(fences.find(new_block.end)==fences.end());


		if (new_block.begin <= new_block.end) {
			auto [new_free, inserted] = free_blocks.insert(new_block);
			fences[new_block.begin] = &*new_free;
			fences[new_block.end] = &*new_free;
		}
				check();

	};


	for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
	{
		auto& free_block = *it;
		block orig_block = free_block;
		auto aligned_offset = Math::roundUp(free_block.begin, align);

		if (free_block.end > aligned_offset)
			if ( aligned_offset + size - 1 <= free_block.end)
			{

				block used_block;

				used_block.begin = aligned_offset;
				used_block.end = aligned_offset + size - 1;


				auto free_begin = fences.find(free_block.begin);
				assert(free_begin != fences.end());
				fences.erase(free_begin);
				if (free_block.end != free_block.begin)
				{
					auto free_end = fences.find(free_block.end);
					assert(free_end != fences.end());
					fences.erase(free_end);
				}
				free_blocks.erase(it);

				check();

				if (aligned_offset > used_block.begin)
				{
					auto prev_free = orig_block;
					prev_free.end = used_block.begin - 1;
					make_free_block(prev_free);
				}

				auto next_free = orig_block;

				next_free.begin = used_block.end + 1;
				make_free_block(next_free);


				check();

				assert(used_block.end < end_region);

				max_usage = std::max(max_usage, used_block.end + 1);

				/*	if (max_usage < this->size / 2)
						assert(!free_blocks.empty());*/
				return Handle(MemoryInfo(aligned_offset, size, reset_id), this);
			}

	}
	return std::nullopt;
}
uint64 CommonAllocator::merge_prev(uint64 start)
{
	if (start == start_region) return start_region;


	auto prev_free = fences.find(start - 1);

	if (prev_free != fences.end())
	{
		auto& prev_block = prev_free->second;
	

		uint64 result = prev_block->begin;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == prev_block->begin;
			});

			fences.erase(prev_block->begin);
		fences.erase(prev_block->end);
		free_blocks.erase(it);

	

		return result;
	}

	return start;
}
uint64 CommonAllocator::merge_next(uint64 end)
{

	if (end == end_region) return end_region;

	auto next_free = fences.find(end + 1);

	if (next_free != fences.end())
	{
		auto& next_block = next_free->second;
		
		uint64 result = next_block->end;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == next_block->begin;
			});
		fences.erase(next_block->begin);
		fences.erase(next_block->end);
		free_blocks.erase(it);


		return result;
	}

	return end;
}

void CommonAllocator::Free(Handle& handle)
{
	ASSERT_SINGLETHREAD
		//std::lock_guard<std::mutex> g(m);
		if (!handle)
			return;

	if (handle.get_reset_id() != reset_id) return;

	assert(handle.get_owner() == this);
	assert(handle.get_reset_id() == reset_id);
assert(handle.get_size()>0);

	

	block my_block;

	my_block.begin = handle.get_info().offset;
	my_block.end = handle.get_info().offset + handle.get_size() - 1;
	
#ifdef DEV
	for(auto &b:free_blocks)
	{
		if(b.begin>=my_block.begin&&b.begin<=my_block.end) assert(false);
		if(b.end>=my_block.begin&&b.end<=my_block.end) assert(false);
		if(b.begin<my_block.begin&&b.end>my_block.end) assert(false);
		
	}
#endif

			check();
	my_block.begin = merge_prev(my_block.begin);
		check();
	my_block.end = merge_next(my_block.end);
		check();
		
	assert(my_block.end >= my_block.begin);
	{
		auto [elem, inserted] = free_blocks.insert(my_block);

		assert(inserted);
		auto* block_ptr = &*elem;



		assert(block_ptr->end >= block_ptr->begin);
		fences[my_block.begin] = block_ptr;
		fences[my_block.end] = block_ptr;
	}
	//tracker.free(handle->get_provider)
	//	handle.owner = nullptr;
	check();

}
void  CommonAllocator::check()
{
#ifdef DEV
	//	return;
	const block* prev = nullptr;;
	for (auto& f : fences)
	{
		if(prev)
		{
			assert(prev==f.second);
			prev=nullptr;
		}
		else
			prev=f.second;

		if(prev)
		{
			if(prev->begin==prev->end)
				prev=nullptr;
		}

	}
	for (auto& f : fences)
	{
		bool found = false;

		for (auto& free : free_blocks)
		{
			if (&free == &*f.second)
			{
				found = true;
				break;
			}
		}

		assert(found);


		assert(f.first == f.second->begin || f.first == f.second->end);


	}


	for (auto& f : free_blocks)
	{
		assert(fences[f.begin] == &f);
		assert(fences[f.end] == &f);
	}
		#endif
}
void CommonAllocator::Reset()
{
	ASSERT_SINGLETHREAD
		free_blocks.clear();
	fences.clear();
	auto [elem, inserted] = free_blocks.insert(block{ start_region,end_region - 1 });
	fences[start_region] = &*elem;
	fences[end_region - 1] = &*elem;

	max_usage = 0;
	reset_id++;
	check();

}

bool AllocatorHanle::operator!=(const AllocatorHanle& h) const
{
	return owner != h.owner || info != h.info;
}

bool AllocatorHanle::operator==(const AllocatorHanle& h) const
{
	return owner == h.owner && info == h.info;
}

AllocatorHanle::AllocatorHanle(const MemoryInfo& info, Allocator* owner) :info(info), owner(owner)
{
	//	assert(provider);
}

uint64 AllocatorHanle::get_offset() const
{
	return info.offset;
}

uint64 AllocatorHanle::get_size() const
{
	return info.size;
}

AllocatorHanle::operator bool() const
{
	return !!owner;
}

uint64 AllocatorHanle::get_reset_id() const
{
	return  info.reset_id;
}

Allocator* AllocatorHanle::get_owner() const
{
	return owner;
}

void AllocatorHanle::Free()
{
	if (owner) owner->Free(*this);
}
void AllocatorHanle::FreeAndClear()
{
	if (owner) owner->Free(*this);
	owner = nullptr;
}
