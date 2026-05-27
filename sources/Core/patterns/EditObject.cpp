module Core:EditObject;

import stl.core;
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
            {
                save(f);
            }
    }
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
