#pragma once
#include <fstream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <array>
#include <optional>
#include <Windows.h>
#include <sstream>

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
	std::stringstream stream;
	char levels[256] = { 0 };
	int level = 0;

	std::string path;

	my_stream(std::string dir, std::string filename);
	~my_stream();
	void push();


	void pop();


	template<class T>
	std::stringstream& operator<<(const T& data);

};

template<class T>
std::stringstream& my_stream::operator<<(const T& data)
{
	stream << levels << data;
	return stream;
}

using table_offsets = std::array<int, ValueType::COUNT>;

struct parsed_type
{
	bool debug = false;
	virtual ~parsed_type() = default;
};
struct have_name:public parsed_type
{
	std::string name;

	virtual ~have_name() = default;
};


struct have_hlsl
{

	std::string hlsl;

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

struct ValueAtom :public have_expr, have_owner , have_name
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
	int size = 0;
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


struct DSV: public have_name
{

}; 

struct RTV : public have_name, have_type
{

};

struct RenderTarget : public inherited, have_options, have_name
{
	std::list<RTV> rtvs;
	std::optional<DSV> dsv;
};

struct RootSig : public have_name
{

};

struct Shader : public have_name, have_options
{
	std::string type;
};

struct have_values
{
	std::vector<Value> values;
};

struct Define : public have_options, have_name, have_values
{
};


struct PSO_RTV : public have_options, have_name , have_values
{
};

struct PSO_Blend : public have_options, have_name, have_values
{
};

struct PSO_Param : public have_options, have_values, have_type, have_expr, parsed_type
{
};

struct PSO : public inherited, have_options, have_name
{
	RootSig root_sig;
	std::vector<Define> defines;

	std::list<Shader> shader_list;
	std::map<std::string, Shader*> shaders;

	Define* find_define(std::string name)
	{

		for (auto& d : defines)
		{
			if (d.name == name)
				return &d;
		}
		return nullptr;
	}
};

struct ComputePSO : public PSO
{

	Shader* get_compute()
	{
		return shaders["compute"];
	}
};

struct GraphicsPSO : public PSO
{

	PSO_RTV rtv;
	PSO_Blend blend;
	std::vector<PSO_Param> params;

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
struct Table : public inherited, have_options, have_name , have_hlsl
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



struct RT : public inherited, have_options, have_name
{

};


struct Parsed
{
	std::list<Layout> layouts;
	std::list<Table> tables;
	std::list<Layout*> root_layouts;
	std::list<RenderTarget> rt;


	std::list<ComputePSO> compute_pso;
	std::list<GraphicsPSO> graphics_pso;

	Layout* find_layout(std::string name);
	Table* find_table(std::string name);

	void setup();

	void merge(Parsed& r)
	{
		layouts.splice(layouts.end(), r.layouts);
		tables.splice(tables.end(), r.tables);
		rt.splice(rt.end(), r.rt);
		compute_pso.splice(compute_pso.end(), r.compute_pso);
		graphics_pso.splice(graphics_pso.end(), r.graphics_pso);

	}
};
