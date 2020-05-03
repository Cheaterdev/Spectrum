#include "pch.h"

void com_deleter(IUnknown* pComResource)
{
    if (pComResource)
        pComResource->Release();
}

std::wstring convert(const std::string& s)
{
    std::wstring w;
    w.resize(s.size());
    std::transform(s.begin(), s.end(), w.begin(), btowc);
    return w;
}

std::string convert(const std::wstring& s)
{
    std::string w;
    w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), [](wint_t c) {return static_cast<char>(wctob(c)); });
    return w;
}


HRESULT test(HRESULT hr, std::string str)
{
    if (FAILED(hr))
        Log::get().crash_error(hr, str);

    return hr;
}

std::string to_lower(const std::string& str)
{
    std::string lowered;
    lowered.resize(str.size());
    std::transform(str.begin(),
                   str.end(),
                   lowered.begin(),
                   ::tolower);
    return lowered;
}


std::wstring to_lower(const std::wstring& str)
{
    std::wstring lowered;
    lowered.resize(str.size());
    std::transform(str.begin(),
                   str.end(),
                   lowered.begin(),
                   ::towlower);
    return lowered;
}


std::size_t operator "" _t(unsigned long long int x)
{
	return x;
}

CommonAllocator::CommonAllocator(size_t size /*= std::numeric_limits<size_t>::max()*/) :size(size)
{
	reset_id = 0;
	Reset();
}

size_t CommonAllocator::get_max_usage()
{
	return max_usage;
}

Allocator::Handle CommonAllocator::Allocate(size_t size, size_t align)
{
	//std::lock_guard<std::mutex> g(m);
	if (size == 0)
	{
		Allocator::Handle res;
		res.aligned_offset = 0;
		res.offset = 0;
		res.size = 0;
		res.owner = nullptr;

		return res;
	}

	for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
	{
		auto& free_block = *it;

		auto aligned_offset = Math::AlignUp(free_block.begin, align);

		if ((free_block.end +1 - aligned_offset) >= size)
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
				auto& [new_free, inserted] = free_blocks.insert(new_block);
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
			Allocator::Handle res;
			res.aligned_offset = aligned_offset;
			res.offset = used_block.begin;
			res.size = size;
			res.owner = this;
			res.reset_id = reset_id;
			assert(used_block.end < this->size);

			max_usage = std::max(max_usage, used_block.end+1);

			return res;
		}

	}
	assert(false);
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

	auto next_free = fences.find(end+1);

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

void CommonAllocator::Free( Allocator::Handle& handle)
{
	//std::lock_guard<std::mutex> g(m);
	if (!handle.owner)
		return;

	assert(handle.owner == this);
	assert(handle.reset_id == reset_id);


	block my_block;

	my_block.begin = handle.offset;
	my_block.end = handle.offset + handle.size - 1;

	my_block.begin = merge_prev(my_block.begin);
	my_block.end = merge_next(my_block.end);

	{
		auto& [elem, inserted] = free_blocks.insert(my_block);
		auto* block_ptr = &*elem;

		fences[my_block.begin] = block_ptr;
		fences[my_block.end] = block_ptr;
	}

//	handle.owner = nullptr;
	check();

}
void  CommonAllocator::check()
{
	return;
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
	

		assert(f.first==f.second->begin|| f.first == f.second->end);
	}


	for (auto& f : free_blocks)
	{
		assert(fences[f.begin] == &f);
		assert(fences[f.end] == &f);
	}
	
}
void CommonAllocator::Reset()
{
	free_blocks.clear();
	fences.clear();
	auto& [elem, inserted] =free_blocks.insert(block{ 0,size - 1 });
	fences[0] = &*elem;
	fences[size - 1] = &*elem;

	max_usage = 0;
	reset_id++;
	check();

}
