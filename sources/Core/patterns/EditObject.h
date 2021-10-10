class EditContainer;

import stl.core;

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