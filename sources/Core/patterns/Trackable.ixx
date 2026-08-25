export module Core:Trackable;
import :Data;
import stl.core;

import :StateContext;
export
{

	struct TrackedObjectState
	{
		bool used = false;
		void reset()
		{
			used = false;
			alias_ended = false;
		}
		bool alias_ended = false;
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

	// Anything carrying per-context state that is (or derives from)
	// TrackedObjectState. Stated as a requirement on get_state rather than as
	// is_base_of<ObjectState<TrackedObjectState>, T>, because a type is free to
	// extend the state with its own data -- e.g. HAL::Resource stores its
	// per-command-list barrier tracking in a TrackedResourceState. Those give
	// ObjectState<Derived>, which is an unrelated type to the template
	// machinery even though the state itself still IS-A TrackedObjectState.
	template <class T>
	concept TrackableClass = requires (T& obj, StateContext* context)
	{
		{ obj.get_state(context) } -> std::convertible_to<TrackedObjectState&>;
	};

	class TrackedObject
	{
	public:
		using ptr = std::shared_ptr<TrackedObject>;
		virtual ~TrackedObject() {};
	};


}
