#pragma once


class IdGeneratorUnsafe
{
	int max_counter = 0;
	std::list<int> free;
	
public:
	

	int get()
	{
	
		if (free.empty())
			return max_counter++;

		int res = free.front();
		free.pop_front();
		return res;
	}


	void put(int i)
	{
		assert(std::find(free.begin(), free.end(), i) == free.end());
		free.emplace_back(i);
	}
};
class IdGenerator: public IdGeneratorUnsafe
{
     
        std::mutex m;
    public:
       

        int get()
        {
			int res = -1;

			{
				std::lock_guard<std::mutex> g(m);
				res = IdGeneratorUnsafe::get();
			}

		   return res;
        }


        void put(int i)
        {
            std::lock_guard<std::mutex> g(m);
			IdGeneratorUnsafe::put(i);
        }

};