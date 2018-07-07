#pragma once
#include <any>
#include <map>
#include <atomic>
#include <typeindex>




class Holder
{

	std::map<std::type_index, variant> objects;
public:
	template<class T>
	T& get_or_create()
	{
		variant& obj = objects[std::type_index(typeid(T))];

		if (!obj.exists())
		{
			obj.create<T>();
		}

		return obj.get<T>();
	}

	template<class T>
	T* get_or_null()
	{
		std::any obj&& = objects[std::type_index(typeid(T))];

		if (!obj.exists())
		{
			return nullptr;
		}

		return &obj.get<T>();
	}

};
