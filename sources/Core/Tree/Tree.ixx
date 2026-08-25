export module Core:Tree;
export import :Singleton;
export import :Events;
export import stl.core;
export import :Data;
import magic_enum;
export class base_tree
{

public:
	Events::Event<> event_on_add;
	Events::Event<> event_on_remove;
	Events::Event<base_tree*> event_on_add_child;
	Events::Event<base_tree*> event_on_remove_child;

protected:
	virtual void on_add()
	{
		event_on_add();
	}

	virtual void on_remove()
	{
		event_on_remove();
	}

	virtual void added_child(base_tree* obj)
	{
		event_on_add_child(obj);
	}


	virtual void removed_child(base_tree* obj)
	{
		event_on_remove_child(obj);
	}

public:

	virtual std::wstring get_name() const
	{
		return L"some_tree_item";
	}
};

template <class P, class T> class tree_selector;

template <class T>
concept pointer_type = std::is_pointer<T>::value;

template <class T>
concept shared_ptr_type = !std::is_pointer<T>::value;


template<pointer_type P, class T>
struct tree_selector<P, T>

{

};

template<shared_ptr_type P, class T>
struct tree_selector<P, T> : public SharedObject<T>
{
};

export template<class T, class _child_holder, class  ...Args >
class tree : public  tree_selector<typename _child_holder::value_type, tree<T, _child_holder, Args...>>, public base_tree, public Args...
{

	friend typename T;
	friend typename tree<T, _child_holder, Args...>;

protected:

	using tree_base = tree<T, _child_holder, Args...>;
	_child_holder childs;

	_child_holder real_childs;

	using element_type = typename _child_holder::value_type;
	T* parent = nullptr;
	bool ownered = false;


public:

	virtual bool is_parent(base_tree* obj)
	{
		if (static_cast<base_tree*>(this) == obj)
			return true;

		if (!parent)
			return false;

		return parent->is_parent(obj);
	}

	
	virtual T* get_child(int i)
	{
		auto it = get_childs().begin();
		std::advance(it, i);
		if constexpr (std::is_pointer<element_type>::value)
			return *it;
		else
			return it->get();
	}
	virtual size_t get_child_count()
	{
		return get_childs().size();
	}
	

	_child_holder& get_childs()
	{
		return childs;
	}

	virtual ~tree()
	{
		for (auto& c : childs)
		{
		
			c->on_remove();
			c->on_parent_removed();
		}
	}
virtual 	int calculate_depth() 
	{
		int counter = 0;

		tree_base* p = parent;
		while (p)
		{
			counter++;
			p = p->parent;
		}

		return counter;
	}
	virtual void remove_all()
	{
		for (auto& c : childs)
			c->on_remove();

		childs.clear();
		real_childs.clear();
	}

		bool has_parent()	 const
	{

		return !!parent;
	}

	element_type get_parent()
	{
		if (parent)
			return parent->get_ptr();

		return nullptr;
	}
	template<class T2 = T>
	auto get_root()
	{
		auto root = this;

		while (root->parent)
			root = root->parent;

		return root->get_ptr<T2>();
	}

	virtual void on_add(T* _parent)
	{
		base_tree::on_add();
		this->parent = _parent;
	}

	virtual void on_parent_removed()
	{
		parent = nullptr;
	}
	virtual void on_remove()
	{
		base_tree::on_remove();
	}

	virtual void add_child(element_type obj)
	{
		obj->remove_from_parent();

		if (!obj->ownered)
			real_childs.insert(obj);

		childs.insert(obj);
		obj->on_add(static_cast<T*>(this));
	}

	virtual void remove_child(element_type obj)
	{
		childs.erase(obj);
		real_childs.erase(obj);
		obj->on_parent_removed();
		obj->on_remove();
	}

	virtual void remove_from_parent()
	{
		if (parent)
			parent->remove_child(get_ptr());
	}
	template <class M = T>
	auto get_ptr()
	{
		if constexpr (std::is_pointer<element_type>::value)
			return dynamic_cast<M*>(this);
		else
		return std::dynamic_pointer_cast<M>(this->shared_from_this());
	}

	virtual void iterate2(std::function<bool(T*)>& f)
	{
		if (f(static_cast<T*>(this)))
		{
			for (auto& c : childs)
				c->iterate(f);
		}
	}
	virtual void iterate(std::function<bool(T*)> f)
	{
		iterate2(f);
	}

};


class VariableBase
{
	std::string name;

public:
	VariableBase(std::string name);
	virtual ~VariableBase() = default;
	std::string get_name();

	// Enum support: GUI code (ParameterWindow) needs to build a combo box for
	// whichever concrete enum a Variable<T> wraps, but Core can't depend on
	// GUI, and there's no way to dynamic_cast to "some unknown enum type" the
	// way it does for the known concrete types bool/float -- so the enum
	// listing/get/set has to be exposed generically here instead, with
	// Variable<T> supplying the real implementation only when T is an enum.
	// A non-enum Variable leaves these at their harmless defaults, which GUI
	// code reads as "not an enum" (empty name list).
	virtual std::vector<std::string> get_enum_names() const { return {}; }
	virtual int get_enum_index() const { return -1; }
	virtual void set_enum_index(int index) {}
};

export class VariableContext:public tree<VariableContext, std::set<VariableContext*> >, public Singleton<VariableContext>
{
	friend class Singleton<VariableContext>;
	std::wstring name;



	VariableContext();
protected:
	VariableContext(std::wstring name);
public:
	// Public (not protected like the constructor above) so std::unique_ptr's
	// default deleter -- used by create() below to hand out an independently-
	// owned context -- can actually invoke it; the constructor stays
	// protected to keep enforcing "always give a name; go through a class
	// that derives VariableContext, or through create()".
	virtual ~VariableContext();

	std::wstring get_name() const override;

	std::set<VariableBase*> variables;
	void add(VariableBase* v);
	void remove(VariableBase* v);

	// Constructs a new named VariableContext the caller owns (VariableContext's
	// own constructor is protected, so it can't be constructed directly outside
	// classes that derive from it -- this is the way to get one as a plain,
	// independently-owned grouping node, e.g. a persistent member of a class
	// that does NOT itself derive VariableContext). The tree stores raw
	// VariableContext* internally, so the returned object must keep a stable
	// address for as long as anything is parented under it -- store the
	// unique_ptr, don't move/copy out of it.
	static std::unique_ptr<VariableContext> create(std::wstring name);

	// RAII scope: while alive, any VariableContext constructed with a name
	// (mesh_renderer, VSM, ...) attaches under `context` instead of the global
	// root -- lets call sites disambiguate otherwise identically-named siblings
	// (e.g. multiple mesh_renderer instances) by where they're constructed,
	// without threading a parent pointer through every constructor. Scopes
	// nest: a Scope opened while another is already active attaches under
	// that outer scope. Must be destroyed in strict LIFO order (ordinary
	// stack-scoped RAII use satisfies this).
	//
	// Takes an EXISTING context by reference rather than constructing and
	// owning its own: the things constructed under it (a mesh_renderer stored
	// in a long-lived shared_ptr, say) usually outlive the single statement a
	// Scope wraps, so the grouping context needs to outlive the Scope too --
	// pass a persistent context (a base-class `this`, or one obtained from
	// create() above), not a temporary. A Scope that owned and destroyed its
	// own context would remove that context -- orphaning everything nested
	// under it -- the moment the construction statement it wraps finishes.
	class Scope
	{
		VariableContext& context;
	public:
		explicit Scope(VariableContext& context);
		~Scope();
	};
};

export template<class T>
class Variable:public VariableBase
{
	T value;
	VariableContext* context;

	// Only meaningful when constrained is true (the range-taking constructor
	// was used) -- an unconstrained Variable never reads these.
	T range_min{};
	T range_max{};
	bool constrained = false;

	static T clamp(const T& v, const T& lo, const T& hi)
	{
		return v < lo ? lo : (v > hi ? hi : v);
	}

public:

	Variable(const T& def, std::string name, VariableContext * context) :value(def), VariableBase(name), context(context)
	{
		context->add(this);
	}

	// Constrained overload: value is clamped to [min, max] on construction and
	// on every subsequent assignment (GUI slider drag or a plain `var = x` in
	// code alike), so the constraint holds no matter which path writes it.
	Variable(const T& def, std::string name, VariableContext* context, T min, T max) :
		value(clamp(def, min, max)), VariableBase(name), context(context),
		range_min(min), range_max(max), constrained(true)
	{
		context->add(this);
	}

	~Variable()
	{
		context->remove(this);
	}

	operator T() const
	{
		return value;
	}

	const T operator=(const T& r)
	{
		return value = constrained ? clamp(r, range_min, range_max) : r;
	}

	bool has_range() const { return constrained; }
	T get_min() const { return range_min; }
	T get_max() const { return range_max; }

	// Real implementation only for enum T -- if constexpr discards the other
	// branch entirely (not just at runtime), so this compiles for every T
	// without needing magic_enum to accept non-enum types.
	std::vector<std::string> get_enum_names() const override
	{
		if constexpr (std::is_enum_v<T>)
		{
			auto names = magic_enum::enum_names<T>();
			return std::vector<std::string>(names.begin(), names.end());
		}
		else
			return {};
	}

	int get_enum_index() const override
	{
		if constexpr (std::is_enum_v<T>)
		{
			auto index = magic_enum::enum_index(value);
			return index.has_value() ? (int)index.value() : -1;
		}
		else
			return -1;
	}

	void set_enum_index(int index) override
	{
		if constexpr (std::is_enum_v<T>)
		{
			auto values = magic_enum::enum_values<T>();
			if (index >= 0 && index < (int)values.size())
				*this = values[index];
		}
	}
};

