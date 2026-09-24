#pragma once

enum ValueType
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
	std::string path;

	my_stream(std::string dir, std::string filename);
	~my_stream();

	template <class T>
	std::stringstream& operator<<(const T& data);
};

// Deletes generated files under `roots` that this run didn't write. Only files
// carrying the DO-NOT-EDIT banner are touched: hand-written files live in these
// directories too (FrameGraph/autogen/PassNodeBase.h). Returns the removed paths.
std::vector<std::string> remove_stale_outputs(const std::vector<std::string>& roots);

template <class T>
std::stringstream& my_stream::operator<<(const T& data)
{
	stream << data;
	return stream;
}

struct table_offsets : public std::vector<int>
{
	table_offsets()
	{
		resize(ValueType::COUNT);
	}
};

struct SourceLocation
{
	std::string file;
	size_t line = 0;
	size_t column = 0;
};

struct parsed_type
{
	bool debug = false;
	SourceLocation loc; // for diagnostics only; not serialized, so it cannot reach generated output
	virtual ~parsed_type() = default;
};

struct have_name : public virtual parsed_type
{
	std::string name;
	std::string source_file; // path of the .prism file that defined this item
	SourceLocation name_loc; // the name token itself; loc may point at a leading [option]
	// Enclosing namespace path of a top-level declaration ("UI", "Debug::Tools");
	// empty at file scope. qn is the name qualified by it ("UI::Text::Glyph");
	// generated C++ names a declaration only through qn, since there is no flat
	// alias on the C++ side.
	std::string ns, qn;
	// Markers a template puts around a definition (set_namespace_text in
	// Main.cpp); my_stream expands them (layout_namespaces, Parsed.cpp).
	// ns_close adds the HLSL flat alias `using UI::Text::Name;`, ns_braces
	// (C++) none. All empty at file scope, so a file without namespaces
	// renders byte-identically.
	std::string ns_open, ns_close, ns_braces;

	~have_name() override = default;


	SERIALIZE()
	{
		ar& NVP(name);
		ar& NVP(source_file);
		ar& NVP(ns);
		ar& NVP(qn);
		ar& NVP(ns_open);
		ar& NVP(ns_close);
		ar& NVP(ns_braces);
	}
};


struct have_hlsl		  : public virtual parsed_type
{
	std::string hlsl;        // everything pasted into the generated HLSL: %{ }% text and [HLSL] functions
	std::string inserted;    // only the %{ }% text, for diagnostics; not serialized
	SourceLocation hlsl_loc; // where the %{ }% block starts; diagnostics only

	SERIALIZE()
	{
		ar& NVP(hlsl);
	}
};

struct inherited	  : public virtual parsed_type
{
	std::vector<std::string> parent;

	SERIALIZE()
	{
		ar& NVP(parent);
	}
};

struct have_options;

struct have_type	: public virtual parsed_type
{
	bool u_norm = false;
	//std::string type;
	std::string class_no_template;
	std::string template_arg;
	ValueType value_type;
	std::string type;

	bool pointer = false;
	bool bindless = false;
	virtual void detect_type(have_options* options = nullptr);


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
		ar& NVP(type);

		/// todo
	}
};


template <class T>
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

	template <class... Args>
	auto& emplace_back(Args... a)
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

	T* find(std::string s) requires (!std::is_base_of_v<have_name, T> && std::is_base_of_v<have_type, T>)
	{
		for (auto& d : container)
		{
			if (d.class_no_template == s)
				return &d;
		}
		return nullptr;
	}

	const T* find(std::string s) const requires (!std::is_base_of_v<have_name, T> && std::is_base_of_v<have_type, T>)
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
		//ar&NVP(container);

		if constexpr (std::is_base_of_v<have_name, T>)
		{
			for (const auto& i : container)
				ar& cereal::make_nvp(i.name, i);
		}
		else if constexpr (!std::is_base_of_v<have_name, T> && std::is_base_of_v<have_type, T>)
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


	std::string generate_array()
	{
		if (as_array)
		{
			if (array_count)
			{
				return std::format("[{}]", array_count);
			}
			else
			{
				return "";
			}
		}
		return "";
	}


	std::string generate_cpp_array()
	{
		if (as_array && array_count == 0)
			return "&";
		if (as_array && array_count > 0)
			return "*";
		return "&";
	}


	SERIALIZE()
	{
		ar& NVP(as_array);
		ar& NVP(array_count);

		std::string array = generate_array();
		ar& NVP(array);

		std::string cpp_array = generate_cpp_array();
		ar& NVP(cpp_array);
	}
};


struct have_owner	  : public virtual parsed_type
{
	std::string owner_name;

	SERIALIZE()
	{
		ar& NVP(owner_name);
	}
};

// One token of a parsed option expression (grammar rule cond_term), kept in
// source order so codegen can render the expression back to C++ verbatim.
// Rendering is deferred to codegen rather than done while parsing because
// resolving Qualified -- deciding whether `Owner::name` is a Table:: context
// field or an enum value -- needs parsed.tables/parsed.enums fully populated,
// and the struct may be declared in a .prism file parsed after this one.
struct ExprTerm : public virtual parsed_type
{
	enum Kind
	{
		Plain,      // literal or bare identifier, pasted as-is
		Qualified,  // Owner::name -- context field OR enum value (resolved later)
		Member,     // owner.name  -- pass-local state, e.g. data.pass_index
		Function,   // exists(X) and friends, captured whole
		Op,         // && || ! == != >= <= > < ( ) + - * / % and a call's `,`
		Call        // name( of a call with expression arguments; its argument
		            // terms, `,` and `)` follow as ordinary terms
	};

	int         kind = Plain;
	std::string owner;
	std::string text;

	// Diagnostics only, not serialized: where `owner` and `text` start. For
	// Function terms text_loc is where the argument of exists(...) starts.
	SourceLocation owner_loc;
	SourceLocation text_loc;

	SERIALIZE()
	{
		ar& NVP(kind);
		ar& NVP(owner);
		ar& NVP(text);
	}
};

// One Table:: context field named by an option expression. This is the payoff
// of parsing conditions rather than pasting them: it is what lets a consumer
// know that e.g. VSM_Combine's enable decision reads exactly
// VSMSelectors::use_vsm_penumbra and nothing else.
struct FieldRef : public virtual parsed_type
{
	std::string owner;
	std::string field;

	SERIALIZE()
	{
		ar& NVP(owner);
		ar& NVP(field);
	}
};

struct have_expr: public virtual parsed_type
{
	std::string expr;

	// The expression as parsed tokens, in source order. Populated for every
	// option value; codegen only renders FROM it when there is more than one
	// term, so every single-atom option (`[Always = Read]`,
	// `[Size = ViewportContext::frame_size]`) keeps taking the exact code path
	// it always did and cannot regress.
	std::list<ExprTerm> terms;

	// Table:: context fields this expression reads, filled by codegen's
	// renderer. Empty until rendered.
	std::list<FieldRef> field_refs;

	// False once a raw backtick span is rendered: expr is then opaque C++ and
	// field_refs CANNOT be assumed to list everything it touches. A consumer
	// that keys a cache off field_refs must treat this as "assume it depends on
	// everything" -- under-reporting a dependency is the one failure mode that
	// produces a silently stale result rather than a loud one.
	bool deps_complete = true;

	// True for a value_id that matched INT_SCALAR/FLOAT_SCALAR/bool_type --
	// a bare literal, safe to interpolate as-is. False for ID/function_id/
	// array_value_ids/shader_type -- a reference to something else (a named
	// constant, or with an owner_name set, an Owner::Field context
	// reference) that codegen needs to resolve rather than paste literally.
	bool is_literal = false;

	// True for a backtick-delimited raw_value -- expr is opaque C++ text,
	// captured verbatim and pasted into the generated code as-is (an escape
	// hatch for calculations the grammar has no operators for). Codegen must
	// not apply the owner_name context-reference resolution to it.
	bool is_raw = false;

	SERIALIZE()
	{
		ar& NVP(expr);
		ar& NVP(terms);
		ar& NVP(field_refs);
		ar& NVP(deps_complete);
		ar& NVP(is_literal);
		ar& NVP(is_raw);
	}
};
			 
					 
struct have_values : public virtual parsed_type
{
	std::list<have_expr> values;

	SERIALIZE()
	{
		ar& NVP(values);
	}
};
struct ValueAtom : public have_expr, have_owner, have_name		 ,have_values
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_expr);
		SAVE_PARENT_MERGED(have_owner);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_values);
	}
};
struct option : public have_name
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


struct Value : public have_name, have_options, have_type, have_expr, have_array,have_values
{
	int offset = 0;
	int size = 0;
	std::string cpp_type;
	std::string qtype; // get_type() with table/enum names fully qualified, for C++ (qualify_field_types)
	void detect_type(have_options* options) override;


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_type);
		SAVE_PARENT_MERGED(have_expr);
		SAVE_PARENT_MERGED(have_array);

		ar& NVP(offset);
		ar& NVP(size);
		ar& NVP(cpp_type);
		ar& NVP(qtype);
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

std::string get_name_for(ValueType type);

std::string get_cpp_for(Value v);
class Slot;

struct Layout : public inherited, have_options, have_name
{
	Layout* parent_ptr = nullptr;
	std::list<Layout*> child_layouts;
	my_container<Slot> slots;
	my_container<Sampler> samplers;


	int types_counts;
	Slot* find_slot(std::string name);

	void set_slots(int offset = 0);

	void setup();
	template <class T>
	void recursive_slots(T f);

	template <class T>
	void recursive_samplers(T f);


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
		ar& NVP(slots);
		ar& NVP(samplers);
		ar& NVP(types_counts);

		if (parent_ptr)
			ar& NP("parent", *parent_ptr);
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

		auto layout = this->layout->name;

		ar& NVP(layout);
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

struct EnumValue : public have_name, have_expr
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_expr);
	}
};

struct Enum : public have_name
{
	std::list<EnumValue> values;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		ar& NVP(values);
	}
};

struct Shader : public have_name, have_options
{
	std::string path; // relative to workdir/shaders, without ".hlsl" -- what the templates paste

	// As written, for validation and editor support; not serialized.
	std::string path_literal;   // contents of the quoted string, or the bare sentinel
	bool path_quoted = false;
	SourceLocation path_loc;    // first character inside the quotes


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);


		ar& NVP(path);
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


struct PSO_RTV : public have_options, public have_name, public have_values
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
	}
};

struct PSO_Blend : public have_options, public have_name, public have_values
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_values);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
	}
};

struct PSO_Param : public have_options, public have_values, public have_type, public have_expr, public virtual parsed_type
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


struct View_Param : public have_options, public have_type, public virtual parsed_type, public have_name
{


	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_options);
		SAVE_PARENT_MERGED(have_type);
		SAVE_PARENT_MERGED(have_name);
	}
};

struct View:public inherited, public have_options, public have_name
{
	std::list<View_Param> params;
	SERIALIZE()
	{

		SAVE_PARENT_MERGED(inherited);
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);


		ar& NVP(params);
	}
};

struct Pass:public View
{

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(View);
	}
};

struct PSO : public inherited, public have_options, public have_name, public shader_holder, public root_holder
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

		bool compute = true;
		ar& NVP(compute);
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
		bool graphics = true;
		ar& NVP(graphics);
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
			
struct WorkgraphNodeOutput : public have_name, public have_type, public have_options, public virtual parsed_type
{
	int max_records = 1;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_type);
		SAVE_PARENT_MERGED(have_options);
		ar& NVP(max_records);
	}
};

struct WorkgraphNode : public have_name, public param_holder, public virtual parsed_type
{
	std::list<WorkgraphNodeOutput> outputs;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(param_holder);
		ar& NVP(outputs);
	}
};

struct WorkgraphPSO : public PSO
{
	my_container<WorkgraphNode> nodes;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(PSO);
		ar& NVP(nodes);
		bool workgraph = true;
		ar& NVP(workgraph);
	}
};
template <class T>
void Layout::recursive_slots(T f)
{
	if (parent_ptr) parent_ptr->recursive_slots(f);


	for (auto& s : slots)
	{
		f(s);
	}
}

template <class T>
void Layout::recursive_samplers(T f)
{
	if (parent_ptr) parent_ptr->recursive_samplers(f);

	for (auto& s : samplers)
	{
		f(s);
	}
}

struct Parsed;

// An HLSL function declared in a struct body. Only the signature is
// understood; `source` is the function exactly as written, pasted into the
// generated HLSL. Not serialized: templates see it through Table::hlsl.
struct Function : public have_name, public have_options, public have_type
{
	std::string params;  // parameter list as written, without the parentheses
	std::string source;  // the whole function from its return type to the closing brace, first-line indentation included
	SourceLocation body_loc; // the opening brace
};

struct Table : public inherited, have_options, have_name, have_hlsl
{
	std::list<Function> functions;

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
	bool need_compiled = false;

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
		ar& NVP(need_compiled);

		if (slot)
		{
			auto& slot = *this->slot; //->name;
			ar& NVP(slot);
		}
	}
};


// One pass slot in a Pipeline block. Options here are placement decisions for
// THIS pipeline (e.g. [Async]), as opposed to the PassNode's own options which
// describe what the pass is (e.g. [Compute] = it uses compute shaders).
struct PipelineEntry : public have_name, public have_options
{
	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		SAVE_PARENT_MERGED(have_options);
	}
};

struct Pipeline : public have_name
{
	my_container<PipelineEntry> entries;

	SERIALIZE()
	{
		SAVE_PARENT_MERGED(have_name);
		ar& NVP(entries);
	}
};

// One `[options] namespace Name { ... }` block. A namespace may be reopened, in
// the same file or another, so there can be several with the same path; the
// declarations inside record the path in have_name::ns.
struct Namespace : public have_options, have_name
{
	std::string path; // ns + "::" + name, or just name at file scope
};

struct Parsed : public parsed_type
{
	// Not serialized: templates only need each declaration's own ns.
	my_container<Namespace> namespaces;

	my_container<Layout> layouts;
	my_container<Table> tables;
	std::list<Layout*> root_layouts;
	my_container<RenderTarget> rt;


	my_container<ComputePSO> compute_pso;
	my_container<GraphicsPSO> graphics_pso;
	my_container<WorkgraphPSO> workgraph_pso;

	my_container<RaytracePSO> raytrace_pso;
	my_container<RaytracePass> raytrace_pass;
	my_container<RaytraceGen> raytrace_gen;


	my_container<View> views;
	my_container<Pass> passes;
	my_container<Pipeline> pipelines;
	my_container<Enum> enums;

	// Top-level `const Name = value;` declarations (Prism.g4's const_definition).
	// Reuses `option`'s existing name+ValueAtom shape rather than a bespoke
	// struct -- a const's value supports exactly the same literal/owner-ref/
	// raw forms a bind_option's value already does.
	my_container<option> consts;

	Layout* find_layout(std::string name);
	Table* find_table(std::string name);
	RaytracePSO* find_rtx(std::string name);
	Enum* find_enum(std::string name);

	void setup();

	void merge(Parsed& r)
	{
		namespaces.merge(r.namespaces);
		layouts.merge(r.layouts);
		tables.merge(r.tables);
		rt.merge(r.rt);
		workgraph_pso.merge(r.workgraph_pso);
		compute_pso.merge(r.compute_pso);
		graphics_pso.merge(r.graphics_pso);
		raytrace_pso.merge(r.raytrace_pso);
		raytrace_pass.merge(r.raytrace_pass);
		raytrace_gen.merge(r.raytrace_gen);
		views.merge(r.views);
		passes.merge(r.passes);
		pipelines.merge(r.pipelines);
		enums.merge(r.enums);
		consts.merge(r.consts);
	}


	SERIALIZE()
	{
		ar& NVP(layouts);
		ar& NVP(tables);
		ar& NVP(rt);
		ar& NVP(compute_pso);
		ar& NVP(graphics_pso);
		ar& NVP(workgraph_pso);
		ar& NVP(raytrace_pass);
		ar& NVP(raytrace_pso);
		ar& NVP(raytrace_gen);

		ar& NVP(views);
		ar& NVP(passes);
		ar& NVP(pipelines);
		ar& NVP(enums);
		ar& NVP(consts);
	}
};
