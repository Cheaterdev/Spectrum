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


	// Redirects get_context<T>() to the real context type actually stored,
	// for a T that is a base a hand-written context inherits from (e.g. a
	// SIG-declared schema struct like Table::Frame::ViewportContext, with
	// ViewportInfo : Table::Frame::ViewportContext holding the live instance).
	// Default is identity -- get_context<T>() behaves exactly as before for
	// every T with no specialization. A one-line specialization at the
	// point where the inheritance is declared (next to the derived type)
	// is all a base type needs to become look-up-able on its own; nothing
	// about Holder's generic storage changes, and no RTTI/dynamic_cast is
	// involved -- the upcast in get_context() below is a plain static_cast,
	// exactly as safe as writing it out by hand at that specific call site.
	template<class T>
	struct ContextTypeFor { using type = T; };

	template<class T>
	using RealContextType = typename ContextTypeFor<T>::type;

	class UniversalContext
	{
		Holder holder;


	public:

		UniversalContext() = default;

		template<class T>
		T& get_context()
		{
			return static_cast<T&>(holder.get_or_create<RealContextType<T>>());
		}


		UniversalContext clone();
	};
}
