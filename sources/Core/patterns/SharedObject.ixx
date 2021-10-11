export module SharedObject;

import Utils;
import stl.core;

export
{
	template<class T> class shared_owner;

	template<class T>
	class shared_object
	{

	public:
		using ptr = s_ptr<shared_object<T>>;
		using wptr = w_ptr<shared_object<T>>;

		class owner
		{
			friend class shared_object<T>;
			std::set<shared_object<T>*> objects;
		protected:
			void add_shared(shared_object<T>* object)
			{
				objects.insert(object);
				object->owners.insert(this);
			}

			void remove_shared(shared_object<T>* object)
			{
				objects.erase(object);
				object->owners.erase(this);
			}

		public:
			using ptr = s_ptr<owner>;
			using wptr = w_ptr<owner>;

			virtual ~owner()
			{
				for (auto o : objects)
					o->owners.erase(this);
			}
		};

		virtual ~shared_object()
		{
			for (auto o : owners)
				o->objects.erase(this);
		}

		std::set<typename owner*>& get_owners()
		{
			return owners;
		}
	private:
		friend class owner;
		std::set<typename owner*> owners;
	};



}