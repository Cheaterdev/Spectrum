module Core:Tree;

import stl.core;
import :Singleton;
import :Events;
import :Data;

VariableBase::VariableBase(std::string name) : name(name) {}

std::string VariableBase::get_name()
{
	return name;
}

VariableContext::VariableContext() : name(L"global") {}

VariableContext::VariableContext(std::wstring name) : name(name)
{
	Singleton<VariableContext>::get().add_child(this);
}

VariableContext::~VariableContext()
{
	remove_from_parent();
}

std::wstring VariableContext::get_name() const
{
	return name;
}

void VariableContext::add(VariableBase* v)
{
	variables.insert(v);
}

void VariableContext::remove(VariableBase* v)
{
	variables.erase(v);
}
