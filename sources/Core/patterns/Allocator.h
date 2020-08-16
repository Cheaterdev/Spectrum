//template <class T>
class Allocator
{
	struct Handle
	{
	private:
		size_t offset;
		size_t size;
	};

	size_t limit;

	std::list<Handle> blocks;

	
	Handle AllocateNew(size_t size, size_t align)
	{
		
	}

public:

	void SetSize(size_t limit)
	{
		this->limit = limit;
	}


	Handle Allocate(size_t size, size_t align)
	{
		
	}

	void Free(Handle handle)
	{
		
	}

};