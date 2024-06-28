#pragma once

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

struct table_offsets :public std::vector<int>
{
	table_offsets()
	{
		resize(ValueType::COUNT);
	}
};

struct parsed_type
{
	bool debug = false;
	virtual ~parsed_type() = default;
};
struct have_name :public parsed_type
{
	std::string name;

	virtual ~have_name() = default;



	SERIALIZE()
	{
		ar& NVP(name);
	}
};


struct have_hlsl
{

	std::string hlsl;
	SERIALIZE()
	{

		ar& NVP(hlsl);
	}
};
struct inherited
{
	std::vector<std::string> parent;

	SERIALIZE()
	{
		ar& NVP(parent);
	}
};

struct have_options;
struct have_type
{
	bool u_norm = false;
	//std::string type;
	std::string class_no_template;
	std::string template_arg;
	ValueType value_type;
	std::string type;
	bool pointer = false;
	bool bindless = false;
	void detect_type(have_options* options = nullptr);

	std::string get_type() const
	{

		//if(pointer) return "uint";

		std::string res;
		res = class_no_template;

		if (!template_arg.empty())
		{
			res += "<";
			res += template_arg;
			res += ">";
		}

		return res;
	}



	SERIALIZE()
	{
		ar& NVP(class_no_template);
		ar& NVP(template_arg);
		ar& NVP(value_type);
		ar& NVP(pointer);
		ar& NVP(bindless);
		ar& NVP(value_type);
				ar& NVP(type);

		/// todo
	}
};


template<class T>
class my_container 
{
	std::list<T> container;
public:
	

	auto begin() const
	{
	return container.begin();
	}


	auto end() const
	{
	return container.end();
	}
	auto begin() 
	{
	return container.begin();
	}


	auto end() 
	{
	return container.end();
	}

	auto size() const
	{
	return container.size();
	}

	template<class ...Args>
	auto&emplace_back(Args... a)
	{
	return container.emplace_back(a...);
	}


	void merge(my_container& other)
	{
		container.splice(container.end(), other.container);
	}


	 T* find(std::string s) requires (std::is_base_of_v<have_name, T>)
	{
		for (auto& d : container)
		{
			if (d.name == s)
				return &d;
		}
		return nullptr;
	}

	const T* find(std::string s) const requires (std::is_base_of_v<have_name, T>)
	{
		for (auto& d : container)
		{
			if (d.name == s)
				return &d;
		}
		return nullptr;
	}
	
	 T* find(std::string s) requires (!std::is_base_of_v<have_name, T>&&std::is_base_of_v<have_type, T>)
	{
		for (auto& d : container)
		{
			if (d.class_no_template == s)
				return &d;
		}
		return nullptr;
	}

	const T* find(std::string s) const requires (!std::is_base_of_v<have_name, T>&&std::is_base_of_v<have_type, T>)
	{
		for (auto& d : container)
		{
			if (d.class_no_template == s)
				return &d;
		}
		return nullptr;
	}


	SERIALIZE()
	{
		if constexpr (std::is_base_of_v<have_name, T>)
		{
			for (const auto& i : container)
				ar& cereal::make_nvp(i.name, i);


		}
		else if constexpr (!std::is_base_of_v<have_name, T>&&std::is_base_of_v<have_type, T>)
		{
			for (const auto& i : container)
				ar& cereal::make_nvp(i.class_no_template, i);

		}
	}


};



struct have_array
{
	int as_array = false;
	int array_count = 1;


	SERIALIZE()
	{
		ar& NVP(as_array);
		ar& NVP(array_count);
	}


};



struct have_owner
{
	std::string owner_name;

	SERIALIZE()
	{

		ar& NVP(owner_name);
	}
};

struct have_expr
{
	std::string expr;

	SERIALIZE()
	{
		ar& NVP(expr);
	}
};

struct ValueAtom :public have_expr, have_owner, have_name
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_expr);
		SAVE_PARENT_MERGED(have_owner);
		SAVE_PARENT_MERGED(have_name);

		//ar& NVP(value_atom);
	}
};

struct option :public have_name
{
	ValueAtom value_atom;


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);

		ar& NVP(value_atom);
	}
};


struct have_options
{
	my_container<option> options;

	const option* find_option(std::string s) const
	{
		return options.find(s);
	}


	SERIALIZE()
	{
		ar& NVP(options);
	}
};


struct Value :public have_name, have_options, have_type, have_expr, have_array
{
	int offset = 0;
	int size = 0;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_type);
		SAVE_PARENT_MERGED(have_expr);
		SAVE_PARENT_MERGED(have_array);

		ar& NVP(offset);
		ar& NVP(size);
	}
};

struct Layout;

struct Sampler : public have_name, have_expr
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_expr);

	}
};

struct Slot : public have_options, have_name
{
	Layout* layout = nullptr;
	int id;
	table_offsets max_counts;
	table_offsets ids;

	Slot();


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		ar& NVP(max_counts);
		ar& NVP(ids);
		ar& NVP(id);

	}
};

std::string get_name_for(ValueType type);

std::string get_cpp_for(Value v);


struct Layout : public inherited, have_options, have_name
{
	Layout* parent_ptr = nullptr;
	std::list< Layout*> child_layouts;
	my_container<Slot> slots;
	my_container<Sampler> samplers;


	int types_counts;
	Slot* find_slot(std::string name);

	void set_slots(int offset = 0);

	void setup();
	template<class T>
	void recursive_slots(T f);

	template<class T>
	void recursive_samplers(T f);


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		ar& NVP(slots);
		ar& NVP(samplers);
		ar& NVP(types_counts);

		if(parent_ptr)
		ar& NP("parent", *parent_ptr);
	}

};


struct DSV : public have_name
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
	}
};

struct RTV : public have_name, have_type
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_type);
	}
};

struct RenderTarget : public inherited, have_options, have_name
{
	std::list<RTV> rtvs;
	std::optional<DSV> dsv;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(inherited);
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_name);

		ar& NVP(dsv);
		ar& NVP(rtvs);
	}
};

struct RootSig : public have_name
{

};

struct Shader : public have_name, have_options
{
	std::string path;


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);


		ar& NVP(path);

	}
};

struct have_values
{
	std::list<Value> values;

	SERIALIZE()
	{
		ar& NVP(values);
	}
};

struct Define : public have_options, have_name, have_values
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);

	}

};


struct PSO_RTV : public have_options,public have_name,public have_values
{

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);

	}
};

struct PSO_Blend : public have_options,public have_name,public have_values
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);

	}
};

struct PSO_Param : public have_options,public have_values,public have_type,public have_expr,public parsed_type
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_type);
		SAVE_PARENT_MERGED(have_expr);

	}
};
struct root_holder
{
	RootSig root_sig;

	SERIALIZE()
	{
		ar& NVP(root_sig);
	}
};
struct shader_holder
{
	my_container<Shader> shaders;


	SERIALIZE()
	{
		ar& NVP(shaders);
	}
};

struct param_holder
{
	my_container<PSO_Param> params;

	PSO_Param* find_param(std::string v)
	{
		return params.find(v);
	}

	SERIALIZE()
	{
		ar& NVP(params);
	}
};
struct PSO : public inherited,public have_options,public  have_name, public shader_holder, public root_holder
{

	my_container<Define> defines;

	Define* find_define(std::string s)
	{
	return defines.find(s);
	}


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(inherited);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(root_holder);
		SAVE_PARENT_MERGED(shader_holder);
	
		ar& NVP(defines);
	}

};

struct ComputePSO : public PSO
{

	Shader* get_compute()
	{
		return shaders.find("compute");
	}
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);
	}
};

struct GraphicsPSO : public PSO, public param_holder
{

	PSO_RTV rtv;
	PSO_Blend blend;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);
		SAVE_PARENT_MERGED(param_holder);
		ar& NVP(rtv);
		ar& NVP(blend);

	}
};


struct RaytracePass : public PSO, public param_holder
{

	int index = 0;
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);
		SAVE_PARENT_MERGED(param_holder);
		ar& NVP(index);

	}
};

struct RaytraceGen : public PSO
{
	int index = 0;
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);
		ar& NVP(index);

	}
};


struct RaytracePSO : public PSO
{
	my_container<RaytraceGen> gens;
	my_container<RaytracePass> passes;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);

		ar& NVP(gens);

		ar& NVP(passes);

	}
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

	for (auto& s  : samplers)
	{
		f(s);
	}
}

struct Parsed;
struct Table : public inherited, have_options, have_name, have_hlsl
{
	Slot* slot = nullptr;
	std::string path;
	std::list<Value> values;
	std::set<std::string> used_tables;

	table_offsets offsets;
	table_offsets counts;
	Value* bindless_srv = nullptr;
	Table* bindless_table = nullptr;

	bool can_compile = true;
	bool cb_provided = false;
	bool cb_raw = false;
	void setup(Parsed* all);


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_hlsl);

		ar& NVP(path);

		ar& NVP(values);

		ar& NVP(used_tables);
		ar& NVP(offsets);
		ar& NVP(counts);
		ar& NVP(can_compile);
		ar& NVP(cb_provided);
		ar& NVP(cb_raw);

	}
};



struct Parsed : public parsed_type
{
	my_container< Layout> layouts;
	my_container<Table> tables;
	std::list<Layout*> root_layouts;
	my_container<RenderTarget> rt;


	my_container<ComputePSO> compute_pso;
	my_container<GraphicsPSO> graphics_pso;

	my_container<RaytracePSO> raytrace_pso;
	my_container<RaytracePass> raytrace_pass;
	my_container<RaytraceGen> raytrace_gen;

	Layout* find_layout(std::string name);
	Table* find_table(std::string name);
	RaytracePSO* find_rtx(std::string name);

	void setup();

	void merge(Parsed& r)
	{
layouts.merge( r.layouts);
		tables.merge(r.tables);
		rt.merge(r.rt);
		compute_pso.merge( r.compute_pso);
		graphics_pso.merge( r.graphics_pso);
		raytrace_pso.merge( r.raytrace_pso);
		raytrace_pass.merge( r.raytrace_pass);
		raytrace_gen.merge(r.raytrace_gen);
	}


	SERIALIZE()
	{

		ar& NVP(layouts);
		ar& NVP(tables);
		ar& NVP(rt);
		ar& NVP(compute_pso);
		ar& NVP(graphics_pso);
		ar& NVP(raytrace_pass);

		ar& NVP(raytrace_gen);

	}
};
