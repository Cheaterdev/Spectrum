export module Core:ListAllocator;


import :Debug;
import :Utils;
import :Math;
import :Allocators;
import stl.core;
import stl.threading;



export
{
	template<class T>
	class ListAllocator
	{
		const uint64 start_region;
		const uint64 end_region;
		const uint64 size;
		uint64 max_usage;
		uint64 reset_id;
		struct Block
		{
			uint64 offset;
			uint64 size;
			T mask;


			Block* next = nullptr;
			Block* prev = nullptr;

			bool operator< (const Block& b) const
			{
				return offset < b.offset;
			}
		};

		std::mutex m;

		std::set<Block> blocks;



	public:
		using Checker = std::function<bool(const T&)>;
		using Handle = AllocatorHanle;

		ListAllocator(uint64 size = std::numeric_limits<uint64>::max()) :size(size)
		{
			Reset();
		}

		virtual ~ListAllocator() = default;
		uint64 get_max_usage() const override;

		Block* TryAllocate(uint64 size, uint64 align, Checker checker) {
			auto start_block = &blocks.first();
			Block* last_block = start_block;
			uint64 aligned_offset = 0;
			while (start_block)
			{
				bool acceptable = checker(start_block->mask);

				if (!acceptable) {
					start_block = start_block->next;
					continue;
				}

				aligned_offset = Math::roundUp(start_block->offset, align);
				uint64 available_size = start_block->offset + start_block->size - aligned_offset;
				last_block = start_block->next;
				while (last_block && checker(last_block) && available_size < size)
				{
					last_block = last_block->next;
				}

				if (available_size < size)
				{
					start_block = start_block->next;
					continue;
				}

				break;
			}

			if (!last_block)	return nullptr;

			Block start_copy = *start_block;
			Block end_copy = *last_block;

			//auto prev_block = start_block->prev;
			//auto next_block = last_block->next;

			//auto my_mask = start_block->mask;
			//auto next_mask = last_block->mask;

			//uint64 start_block_offset = start_block->offset;
			//uint64 prev_size = aligned_offset- start_block->offset;
			//uint64 after_size = aligned_offset + size - last_block->offset;


			auto block = start_block;

			do
			{
				remove_block(block);
				block = block->next;
			} while (block != last_block);


			Block* inserted_prev = nullptr;
			Block* inserted_next = nullptr;
			Block* inserted_center = nullptr;

			if (start_copy.offset != aligned_offset)
			{
				Block pre = start_copy;

				pre.size = aligned_offset - pre.offset;
				auto inserted = blocks.insert(pre);

				assert(inserted.first);

				inserted_prev = &inserted.second;
			}

			if (end_copy.offset + end_copy.size != aligned_offset + size)
			{


				Block after = end_copy;

				uint64 block_end = after.offset + after.size;

				after.offset = aligned_offset + size;
				after.size = block_end - after.offset;


				auto inserted = blocks.insert(after);

				assert(inserted.first);

				inserted_next = &inserted.second;
			}
			Block* prev_block = inserted_prev ? inserted_prev : start_copy.prev ;
				Block * after_block = inserted_next ? inserted_next : end_copy.next ;


				if (inserted_prev)
				{
					if (start_copy.prev) start_copy.prev->next = inserted_prev;
					inserted_prev->prev = start_copy.prev;
				}
			if (inserted_next)
			{
				if (end_copy.next) end_copy.next->prev = inserted_next;
				inserted_next->next = end_copy.next;

			}


			Block used_block;

			used_block.offset = aligned_offset;
			used_block.size = size;

			{
				auto inserted = blocks.insert(used_block);

				assert(inserted.first);

				inserted_center = &inserted.second;
			}


			if (prev_block)prev_block->next = inserted_center;
			if (after_block)after_block->prev = inserted_center;


			inserted_center.next = after_block;
			inserted_center.prev = prev_block;



			return inserted_center;
		}

		void adapt_block(Block* block)
		{
			merge_next(block);
			merge_prev(block);
		}
		Block* merge_next(Block* block)
		{
			auto next = block->next;

			if (!next || block->mask != next->mask) return;

			block->next = next->next;
			if (block->next) block->next->prev = block;

			block->size += next->size;

			blocks.erase(*next);

			return block;
		}

		Block* merge_prev(Block* block)
		{

			if (block->prev)
				return merge_next(block->prev);

			return block;
		}

		void Free(Handle& handle)
		{

		}

		void Reset()
		{
			blocks.clear();
			Block free;
			free.offset = 0;
			free.size = size;
			blocks.insert(free);
		}

		uint64 get_size() const
		{
			return size;
		}

	};

}
