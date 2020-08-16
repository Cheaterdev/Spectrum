#pragma once

class base_tree;
//class tree_contoller_base;

class tree_item_listener
{
	friend class base_tree;
	base_tree* tree = nullptr;
protected:
	virtual void tree_on_add() {};
	virtual void tree_on_remove() {};
	virtual void tree_added_child(base_tree*) {};
	virtual void tree_removed_child(base_tree*) {};

	virtual ~tree_item_listener();;
};

class tree_listener
{
	tree_item_listener* controller;
public:
	tree_listener(tree_item_listener* controller)
	{
		this->controller = controller;
	}
};



class base_tree
{

	// base_tree* parent;
protected:
	std::set<tree_item_listener*> listeners;
	virtual void on_add()
	{
		auto copy = listeners;

		for (auto l : copy)
			l->tree_on_add();
	}

	virtual void on_remove()
	{
		auto copy = listeners;

		for (auto l : copy)
			l->tree_on_remove();
	}



	virtual void added_child(base_tree* obj)
	{
		for (auto l : listeners)
			l->tree_added_child(obj);
	}



	virtual void removed_child(base_tree* obj)
	{
		for (auto l : listeners)
			l->tree_removed_child(obj);
	}


	// tree_contoller_base* controller;
public:

	virtual bool is_parent(base_tree*)
	{
		return false;
	}


	virtual void add_child(base_tree*)
	{
	}
	void register_listener(tree_item_listener* listener)
	{
		listeners.insert(listener);
		listener->tree = this;
	}
	void unregister_listener(tree_item_listener* listener)
	{
		listeners.erase(listener);
	}

	virtual std::wstring get_name() const
	{
		return L"some_tree_item";
	}

	virtual base_tree* get_child(int i) = 0;
	virtual size_t get_child_count() = 0;

	virtual int calculate_depth() = 0;

	/*virtual void on_add(base_tree* parent)
	{
	if (parent != this->parent)
	{
	controller = parent->controller;

	if (controller)
	controller->on_element_add(parent, this);
	}

	this->parent = parent;
	}

	virtual void on_remove()
	{
	if (parent && controller)
	{
	controller->on_element_remove(parent, this);
	}

	parent = nullptr;
	controller = nullptr;
	}*/
};


template<class T, class _child_holder,class  ...Args >
class tree : public  std::enable_shared_from_this<tree<T, _child_holder, Args...>>, public base_tree, public Args...
{

	friend typename T;
	friend typename tree<T, _child_holder, Args...>;

protected:

	using tree_base = tree<T, _child_holder, Args...>;
	_child_holder childs;

	_child_holder real_childs;

	T* parent = nullptr;
	bool ownered = false;

	virtual base_tree* get_child(int i)override
	{
		auto it = get_childs().begin();
		std::advance(it, i);
		return it->get();
	}
	virtual size_t get_child_count()override
	{
		return get_childs().size();
	}
	virtual void add_child(base_tree* obj) override
	{
		tree_base* s = dynamic_cast<tree_base*>(obj);

		if (s && s->parent != this)
			add_child(s->get_ptr());
	}

	virtual bool is_parent(base_tree* obj) override
	{
		if (static_cast<base_tree*>(this) == obj)
			return true;

		if (!parent)
			return false;

		return parent->is_parent(obj);
	}

public:


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
virtual 	int calculate_depth() override
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

	std::shared_ptr<T> get_parent()
	{
		if (parent)
			return parent->get_ptr();

		return nullptr;
	}
	template<class T2 = T>
	std::shared_ptr<T2> get_root()
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
		//  parent = nullptr;
		//        controller = nullptr;
	}

	virtual void add_child(std::shared_ptr<T> obj)
	{
		obj->remove_from_parent();

		if (!obj->ownered)
			real_childs.insert(obj);

		childs.insert(obj);
		obj->on_add(static_cast<T*>(this));
		base_tree::added_child(obj.get());
	}

	virtual void remove_child(std::shared_ptr<T> obj)
	{
		childs.erase(obj);
		real_childs.erase(obj);
		obj->on_parent_removed();
		obj->on_remove();
		base_tree::removed_child(obj.get());
	}

	virtual void remove_from_parent()
	{
		if (parent)
			parent->remove_child(get_ptr());
	}
	template <class M = T>
	std::shared_ptr<M> get_ptr()
	{
		return std::dynamic_pointer_cast<M>(shared_from_this());
	}

	virtual void iterate2(std::function<bool(T*)>& f)
	{
		if (f(static_cast<T*>(this)))
		{
			for (auto& c : childs)
				c->iterate(f);

			/*
			for (auto &c : ownered_childs)
			c->iterate(f);*/
		}
	}
	virtual void iterate(std::function<bool(T*)> f)
	{
		iterate2(f);
	}

};


