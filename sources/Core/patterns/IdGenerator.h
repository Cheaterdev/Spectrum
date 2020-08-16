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
		free.emplace_back(i);
	}
};
class IdGenerator: public IdGeneratorUnsafe
{
     
        std::mutex m;
    public:
       

        int get()
        {
            std::lock_guard<std::mutex> g(m);

          
            return IdGeneratorUnsafe::get();
        }


        void put(int i)
        {
            std::lock_guard<std::mutex> g(m);
			IdGeneratorUnsafe::put(i);
        }

};