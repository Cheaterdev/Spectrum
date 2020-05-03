#pragma once
#include <fstream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <array>


enum   ValueType 
{
	CB,
	SRV,
	UAV,
	SMP,
	STRUCT,
	COUNT

};

struct my_stream
{
	std::ofstream stream;
	char levels[256] = { 0 };
	int level = 0;
	my_stream(std::string dir, std::string filename);
	~my_stream();
	void push();


	void pop();


	template<class T>
	std::ofstream& operator<<(const T& data);

};

template<class T>
std::ofstream& my_stream::operator<<(const T& data)
{
	stream << levels << data;
	return stream;
}

using table_offsets = std::array<int, ValueType::COUNT>;


struct have_name
{
	std::string name;

	virtual ~have_name() = default;
};
struct inherited
{
	std::vector<std::string> parent;
};

struct have_options;
struct have_type
{
	std::string type;
	std::string template_arg;
	ValueType value_type;

	bool bindless = false;
	void detect_type(have_options* options = nullptr);

};


struct have_array
{
	int as_array = false;
	int array_count = 1;
};



struct have_owner
{
	std::string owner_name;
};

struct have_expr
{
	std::string expr;
};

struct ValueAtom :public have_name, have_owner
{

};

struct option :public have_name
{
	ValueAtom value_atom;
};


struct have_options
{
	std::vector<option> options;

	const option* find_option(std::string s) const
	{
		for (auto& o : options)
		{
			if (o.name == s)
				return &o;
		}

		return nullptr;
	}
};


struct Value :public have_name, have_options, have_type, have_expr, have_array
{
	int offset = 0;
};

struct Layout;

struct Sampler : public have_name, have_expr
{

};

struct Slot : public have_options, have_name
{
	Layout* layout = nullptr;
	int id;
	table_offsets max_counts;
	table_offsets ids;

	Slot();
};

std::string get_name_for(ValueType type);

std::string get_cpp_for(Value v);


struct Layout : public inherited, have_options, have_name
{
	Layout* parent_ptr = nullptr;
	std::list< Layout*> child_layouts;
	std::list<Slot> slots;
	std::list<Sampler> samplers;


	int types_counts;
	Slot* find_slot(std::string name);

	void set_slots(int offset = 0);

	void setup();
	template<class T>
	void recursive_slots(T f);

	template<class T>
	void recursive_samplers(T f);

};

template<class T>
void Layout::recursive_slots(T f)
{
	if (parent_ptr) parent_ptr->recursive_slots(f);


	for (auto& s : slots)
	{
		f(s);
	}
}

template<class T>
void Layout::recursive_samplers(T f)
{
	if (parent_ptr) parent_ptr->recursive_samplers(f);

	for (auto& s : samplers)
	{
		f(s);
	}
}

struct Parsed;
struct Table : public inherited, have_options, have_name
{
	Slot* slot = nullptr;
	std::string path;
	std::list<Value> values;
	std::set<std::string> used_tables;

	table_offsets offsets;
	table_offsets counts;
	Value* bindless_srv = nullptr;
	Table* bindless_table = nullptr;


	bool cb_provided = false;
	bool cb_raw = false;
	void setup(Parsed* all);
};

struct Parsed
{
	std::list<Layout> layouts;
	std::list<Table> tables;
	std::list<Layout*> root_layouts;

	Layout* find_layout(std::string name);
	Table* find_table(std::string name);

	void setup();

	void merge(Parsed& r)
	{
		layouts.splice(layouts.end(), r.layouts);
		tables.splice(tables.end(), r.tables);
	}
};
