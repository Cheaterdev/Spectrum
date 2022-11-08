export module Core:IdGenerator;

import :Threading;

import stl.threading;
import stl.core;

export
{

	template<class LockPolicy = Thread::Lockable>
	class IdGenerator
	{
		int max_counter = 0;
		std::list<int> free;
		typename LockPolicy::mutex m;

	public:
		int get();
		void put(int i);
	};

	template <class LockPolicy>
	int IdGenerator<LockPolicy>::get()
	{
		typename LockPolicy::guard g(m);

		if (free.empty())
			return max_counter++;

		int res = free.front();
		free.pop_front();
		return res;
	}

	template <class LockPolicy>
	void IdGenerator<LockPolicy>::put(int i)
	{
		typename LockPolicy::guard g(m);

		assert(std::find(free.begin(), free.end(), i) == free.end());
		free.emplace_back(i);
	}

}