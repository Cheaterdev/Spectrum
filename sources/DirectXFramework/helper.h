

#include "patterns/Singleton.h"


class string_heap : public Singleton<string_heap>
{
	friend class Singleton<string_heap>;

	std::set<std::string> strings;

public:
	const char* get_string(std::string str)
	{
		auto data = strings.find(str);

		if (data == strings.end())
			data = strings.insert(strings.end(), str);

		return data->c_str();
	}

};
