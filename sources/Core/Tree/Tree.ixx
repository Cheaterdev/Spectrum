export module Tree;
export import Singleton;
export import Events;
export import stl.core;

export class base_tree
{

public:
	Events::Event<void> event_on_add;
	Events::Event<void> event_on_remove;
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
struct tree_selector<P, T> : public std::enable_shared_from_this<T>
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
	VariableBase(std::string name):name(name)
	{
		
	}
	virtual ~VariableBase() = default;
	std::string get_name()
	{
		return name;
	}
};

export class VariableContext:public tree<VariableContext, std::set<VariableContext*> >, public Singleton<VariableContext>
{
	friend class Singleton<VariableContext>;
	std::wstring name;

	
	
	VariableContext():name(L"global") {};
protected:

	VariableContext(std::wstring name) : name(name)
	{
		Singleton<VariableContext>::get().add_child(this);
	}
	
	virtual ~VariableContext()
	{
		remove_from_parent();
	}
public:

	std::wstring get_name() const override
	{
		return name;
	}
	
	std::set<VariableBase*> variables;
	void add(VariableBase* v)
	{
		variables.insert(v);
	}
	void remove(VariableBase* v)
	{
		variables.erase(v);
	}
};

export template<class T>
class Variable:public VariableBase
{
	T value;
	VariableContext* context;
public:

	Variable(const T& def, std::string name, VariableContext * context) :value(def), VariableBase(name), context(context)
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
		return value = r;
	}

};

