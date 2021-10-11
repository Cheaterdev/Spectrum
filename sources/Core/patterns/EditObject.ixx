export module EditObject;

import stl.core;
import stl.threading;

export
{

class EditContainer;

class EditObject
{
	friend class EditContainer;
    EditContainer* owner = nullptr;
    bool changed = false;
protected:
   virtual void mark_changed();

   virtual  void on_save();

    virtual ~EditObject();

public:
    virtual void save();
	virtual bool is_changed()
	{
		return changed;
	}
};


class EditContainer
{
    std::set<EditObject*> objects;

protected:
	virtual void save(EditObject* obj);;
public:
    void add(EditObject* obj);

    void remove(EditObject* obj);

    virtual bool promt(EditObject* obj);
    void save_all(bool need_promt = false);
};
}


module:private;


void EditObject::save()
{
    on_save();
}

EditObject::~EditObject()
{
    if (owner)
        owner->remove(this);
}

void EditObject::on_save()
{
    changed = false;
}

void EditObject::mark_changed()
{
    changed = true;
}

void EditContainer::save_all(bool need_promt /*= false*/)
{
    std::vector<std::future<bool>> tasks;

    for (auto f : objects)
    {
        if (f->changed)
            if (!need_promt || promt(f))
                // tasks.emplace_back(thread_pool::get().enqueue([this, f]()
            {
                save(f);
                //  return true;
            };//));
    }

    //for (auto && t : tasks)
    //    t.wait();
}

bool EditContainer::promt(EditObject*)
{
    return true;
}

void EditContainer::remove(EditObject* obj)
{
    objects.erase(obj);
}

void EditContainer::add(EditObject* obj)
{
    objects.insert(obj);
}

void EditContainer::save(EditObject*)
{
}
