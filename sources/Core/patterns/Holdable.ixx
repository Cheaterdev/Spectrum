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
			std::any&& obj = objects[std::type_index(typeid(T))];

			if (!obj.exists())
			{
				return nullptr;
			}

			return &obj.get<T>();
		}


		Holder clone()
		{
			Holder result;
			for(const auto &[a,b]:objects)
			{
				result.objects[a] = b;
			}

			return result;
		}
	};


	class UniversalContext
	{
		Holder holder;


	public:
		template<class T>
		T& get_context()
		{
			return holder.get_or_create<T>();
		}


		UniversalContext clone()
		{
			UniversalContext result;
			result.holder = holder.clone();
			return result;
		}
	};
}
