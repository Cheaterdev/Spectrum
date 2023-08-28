export module Core:Trackable;
import :Data;
import <stl/core.h>;

import :StateContext;
export
{

	struct TrackedObjectState
	{
		bool used = false;
		void reset()
		{
			used = false;
		}
	};

	template<class T>
	class Trackable : public ObjectState<TrackedObjectState>
	{
	public:
		mutable std::shared_ptr<T> tracked_info;
		Trackable()
		{
			tracked_info = std::make_shared<T>();
		}
		virtual ~Trackable() {};

		std::shared_ptr<T> get_tracked() const
		{
			return tracked_info;
		}

	};

	template <class T>
	concept TrackableClass = std::is_base_of<ObjectState<TrackedObjectState>, T>::value;

	class TrackedObject
	{
	public:
		using ptr = std::shared_ptr<TrackedObject>;
		virtual ~TrackedObject() {};
	};


}
