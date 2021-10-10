#include "pch_core.h"
#include "EditObject.h"

import stl.threading;

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
