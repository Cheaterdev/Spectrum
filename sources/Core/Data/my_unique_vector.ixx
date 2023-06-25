export module Core:my_unique_vector;

import <stl/core.h>;

export template <class T>
class my_unique_vector : public std::vector<T>
{
public:
	using 	std::vector<T>::insert;

	void insert(const T& elem)
	{
		std::vector<T>::push_back(elem);
	}
	void erase(const T& elem)
	{
		auto it = std::find(std::vector<T>::begin(), std::vector<T>::end(), elem);

		if (it != std::vector<T>::end())
			std::vector<T>::erase(it);
	}
};

