export module Core:Holdable;
import :Data;
import stl.core;

export
{
	class Holder
	{

		std::map<std::type_index, MyVariant> objects;
	public:
		template<class T>
		T& get_or_create()
		{
			MyVariant& obj = objects[std::type_index(typeid(T))];

			if (!obj.exists())
			{
				obj.create<T>();
			}

			return obj.get<T>();
		}

		template<class T>
		T* get_or_null()
		{
			MyVariant& obj = objects[std::type_index(typeid(T))];

			if (!obj.exists())
			{
				return nullptr;
			}

			return &obj.get<T>();
		}


		Holder clone();
	};


	class UniversalContext
	{
		Holder holder;


	public:

		UniversalContext() = default;
	
		template<class T>
		T& get_context()
		{
			return holder.get_or_create<T>();
		}


		UniversalContext clone();
	};
}
