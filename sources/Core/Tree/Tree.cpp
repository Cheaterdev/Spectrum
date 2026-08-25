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

namespace
{
	// Per-thread so the (thread-confined, single-threaded-at-setup) push/pop
	// discipline can't be corrupted by another thread constructing a named
	// VariableContext concurrently.
	thread_local std::vector<VariableContext*> g_context_scope_stack;
}

VariableContext::VariableContext() : name(L"global") {}

VariableContext::VariableContext(std::wstring name) : name(name)
{
	VariableContext* parent = g_context_scope_stack.empty()
		? &Singleton<VariableContext>::get()
		: g_context_scope_stack.back();

	parent->add_child(this);
}

VariableContext::~VariableContext()
{
	remove_from_parent();
}

std::unique_ptr<VariableContext> VariableContext::create(std::wstring name)
{
	return std::unique_ptr<VariableContext>(new VariableContext(std::move(name)));
}

VariableContext::Scope::Scope(VariableContext& context) : context(context)
{
	g_context_scope_stack.push_back(&context);
}

VariableContext::Scope::~Scope()
{
	ASSERT(!g_context_scope_stack.empty() && g_context_scope_stack.back() == &context);
	g_context_scope_stack.pop_back();
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
