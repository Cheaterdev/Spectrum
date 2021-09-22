#include "pch.h"
#include "Allocators.h"

CommonAllocator::CommonAllocator(size_t size /*= std::numeric_limits<size_t>::max()*/) :size(size)
{
	reset_id = 0;
	Reset();
}

size_t CommonAllocator::get_max_usage()
{
	return max_usage;
}

std::optional<CommonAllocator::Handle>  CommonAllocator::TryAllocate(size_t size, size_t align)
{
	ASSERT_SINGLETHREAD
	//std::lock_guard<std::mutex> g(m);
	if (size == 0)
	{
		size_t id;
		MemoryInfo& res = tracker.place_handle(id);
		res.aligned_offset = 0;
		res.offset = 0;
		res.size = 0;
		//	res.owner = this;
		res.reset_id = reset_id;
		return Handle(std::make_shared<TrackedMemoryInfoProvider>(id, tracker), nullptr);
	}

	for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
	{
		auto& free_block = *it;
		auto orig_block = free_block;
		auto aligned_offset = Math::roundUp(free_block.begin, align);

		if(free_block.end> aligned_offset)
		if ((free_block.end + 1 - aligned_offset) >= size)
		{

			block used_block;

			used_block.begin = free_block.begin;
			used_block.end = used_block.begin + size + (aligned_offset - free_block.begin) - 1;


			auto free_begin = fences.find(free_block.begin);
			assert(free_begin != fences.end());
			fences.erase(free_begin);
			if (free_block.end != free_block.begin)
			{
				auto free_end = fences.find(free_block.end);
				assert(free_end != fences.end());
				fences.erase(free_end);
			}

			block new_block = free_block;
			free_blocks.erase(it);

			check();


			new_block.begin = used_block.end + 1;
			if (new_block.begin <= new_block.end) {
				auto [new_free, inserted] = free_blocks.insert(new_block);
				fences[new_block.begin] = &*new_free;
				fences[new_block.end] = &*new_free;
			}


			/*

			free_block.begin = used_block.end + 1;

			if (free_block.begin < free_block.end)
			{
				fences[free_block.begin] = &free_block;
			}
			else 	if (free_block.begin > free_block.end)
			{
				auto free_end = fences.find(free_block.end);
				assert(free_end != fences.end());
				fences.erase(free_end);


			}
			*/
			check();

			size_t id;
			MemoryInfo& res = tracker.place_handle(id);

			res.aligned_offset = aligned_offset;
			res.offset = used_block.begin;
			res.size = size;
			//	res.owner = this;
			res.reset_id = reset_id;
			assert(used_block.end < this->size);

			max_usage = std::max(max_usage, used_block.end + 1);

			if (max_usage < this->size / 2)
				assert(!free_blocks.empty());
			return Handle(std::make_shared<TrackedMemoryInfoProvider>(id, tracker), this);
		}

	}
	return std::nullopt;
}
size_t CommonAllocator::merge_prev(size_t start)
{
	if (start == 0) return 0;


	auto prev_free = fences.find(start - 1);

	if (prev_free != fences.end())
	{
		auto& prev_block = prev_free->second;

		size_t result = prev_block->begin;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == prev_block->begin;
			});

		free_blocks.erase(it);


		fences.erase(prev_free);

		return result;
	}

	return start;
}
size_t CommonAllocator::merge_next(size_t end)
{

	if (end == size) return end;

	auto next_free = fences.find(end + 1);

	if (next_free != fences.end())
	{
		auto& next_block = next_free->second;

		size_t result = next_block->end;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == next_block->begin;
			});

		free_blocks.erase(it);
		fences.erase(next_free);

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



	block my_block;

	my_block.begin = handle.get_info().offset;
	my_block.end = handle.get_info().aligned_offset + handle.get_size() - 1;

	my_block.begin = merge_prev(my_block.begin);
	my_block.end = merge_next(my_block.end);


	assert(my_block.end >= my_block.begin);
	{
		auto [elem, inserted] = free_blocks.insert(my_block);
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
//	return;
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

}
void CommonAllocator::Reset()
{
	ASSERT_SINGLETHREAD
	free_blocks.clear();
	fences.clear();
	auto [elem, inserted] = free_blocks.insert(block{ 0,size - 1 });
	fences[0] = &*elem;
	fences[size - 1] = &*elem;

	max_usage = 0;
	reset_id++;
	check();

}

bool AllocatorHanle::operator!=(const AllocatorHanle& h) const
{
	return owner != h.owner || (provider->get_info().aligned_offset != h.provider->get_info().aligned_offset);
}

bool AllocatorHanle::operator==(const AllocatorHanle& h) const
{
	return owner == h.owner && (provider->get_info().aligned_offset == h.provider->get_info().aligned_offset);
}

AllocatorHanle::AllocatorHanle(std::shared_ptr<MemoryInfoProvider> provider, Allocator* owner) :provider(provider), owner(owner)
{
	assert(provider);
}

AllocatorHanle::AllocatorHanle() = default;

size_t AllocatorHanle::get_offset() const
{
	return provider->get_info().aligned_offset;
}

size_t AllocatorHanle::get_size() 
{
	return provider->get_info().size;
}

AllocatorHanle::operator bool()
{
	return !!owner;
}

size_t AllocatorHanle::get_reset_id()
{
	return provider->get_info().reset_id;
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
