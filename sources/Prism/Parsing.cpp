import Core;

#include "Parsing.h"
#include "Diagnostics.h"
#include "PrismAst.h"

using Ast = PrismNode;

// Walks the AST in source order and builds the Parsed model. Every rule of the
// grammar is an AST node, so each handler sees the same nodes in the same order
// the ANTLR parse tree walker used to deliver them: `enter_x` before a node's
// children, `exit_x` after.
class TreeShapeListener
{
public:
	Parsed& parsed;
	std::string file;
	const std::string& source; // the buffer that was parsed; token views point into it

	TreeShapeListener(Parsed& parsed, std::string file, const std::string& source)
		: parsed(parsed), file(std::move(file)), source(source)
	{
		parsed.loc.file = this->file;
		setup_elem(parsed);
	}

	void walk(const Ast& node)
	{
		enter(node);
		for (const auto& child : node.nodes)
			walk(*child);
		leave(node);
	}

	// ---- AST access ---------------------------------------------------------

	static void collect_text(const Ast& node, std::string& out)
	{
		if (node.is_token)
			out += node.token;
		else
			for (const auto& child : node.nodes)
				collect_text(*child, out);
	}

	// The concatenated token texts, which is what ANTLR's getText() returned:
	// whitespace and comments between tokens are not part of it.
	static std::string text_of(const Ast& node)
	{
		std::string out;
		collect_text(node, out);
		return out;
	}

	static const Ast* child_of(const Ast& node, std::string_view name)
	{
		for (const auto& child : node.nodes)
			if (child->name == name)
				return child.get();
		return nullptr;
	}

	static const Ast& require_child(const Ast& node, std::string_view name)
	{
		if (auto* c = child_of(node, name))
			return *c;
		throw std::logic_error(std::format("internal: {} has no {}", node.name, name));
	}

	static const Ast* first_leaf(const Ast& node)
	{
		const Ast* n = &node;
		while (!n->is_token)
		{
			if (n->nodes.empty())
				return nullptr;
			n = n->nodes.front().get();
		}
		return n;
	}

	SourceLocation loc_of(const Ast& node, size_t shift = 0) const
	{
		return SourceLocation{ file, node.line, node.column + shift };
	}

	size_t offset_of(const char* p) const
	{
		return static_cast<size_t>(p - source.data());
	}

	void stamp(const Ast& ctx)
	{
		elems.back().elem->loc = loc_of(ctx);
	}

	// ---- element stack ------------------------------------------------------

	struct elem_info
	{
		parsed_type* elem;
		std::function<void()> on_end;
	};

	std::list<elem_info> elems;

	template <class T>
	T& setup_map(my_container<T>& m)
	{
		auto& l = m.emplace_back();
		setup_elem(l);

		return l;
	}

	template <class T>
	T& setup_list(std::list<T>& m)
	{
		auto& l = m.emplace_back();
		setup_elem(l);

		return l;
	}

	// Paths of the namespace blocks currently open, innermost last.
	std::vector<std::string> ns_stack;

	template <class T>
	void setup_elem(T& e, std::function<void()> f = nullptr)
	{
		// With the Parsed on top, `e` is a top-level declaration (a Namespace
		// header included, which records its parent).
		if (!elems.empty() && !ns_stack.empty() && dynamic_cast<Parsed*>(elems.back().elem))
			if (auto* named = dynamic_cast<have_name*>(static_cast<parsed_type*>(&e)))
				named->ns = ns_stack.back();
		elems.emplace_back(&e, f);
	}

	void end_elem()
	{
		auto& e = elems.back();

		if (e.on_end)
			e.on_end();
		elems.pop_back();
	}

	template <class T>
	bool check()
	{
		return find_elem<T>() != nullptr;
	}

	// For traits that are genuinely optional at a given rule (e.g. an RTV has a
	// type but no options).
	template <class T>
	T* find_elem()
	{
		return dynamic_cast<T*>(elems.back().elem);
	}

	// Only the top of the stack is inspected, deliberately: silently attaching a
	// leaf to some ancestor that happens to have the trait would be a quieter
	// version of the same bug. A grammar change that breaks the nesting a leaf
	// listener relies on fails here, naming the trait and the location.
	template <class T>
	T& get_elem()
	{
		if (T* e = find_elem<T>())
			return *e;

		const auto& loc = elems.back().elem->loc;
		throw std::logic_error(std::format("{}({},{}): internal: element on top of the parse stack is not a {}",
			loc.file, loc.line, loc.column, typeid(T).name()));
	}

	// ---- dispatch -----------------------------------------------------------
	// A rule listed here must have the matching handler below, and a handler
	// that is not listed is never called.

#define GENERATED_RULES(X) \
	X(layout_definition) X(table_definition) X(const_definition) X(slot_declaration) X(value_declaration) \
	X(option) X(sampler_declaration) X(rt_definition) X(rt_color_declaration) X(rt_ds_declaration) \
	X(compute_pso_definition) X(graphics_pso_definition) X(rtx_pso_definition) X(workgraph_pso_definition) \
	X(node_definition) X(node_param) X(rtx_pass_definition) X(rtx_raygen_definition) X(root_sig) X(shader) \
	X(options_assign) X(define_declaration) X(rtv_formats_declaration) X(blends_declaration) X(pso_param) \
	X(array_value_holder) X(flag_value_holder) X(view_definition) X(pass_definition) X(view_declaration) \
	X(pipeline_definition) X(pipeline_stat) X(enum_definition) X(enum_value_declaration)

#define ENTER_ONLY_RULES(X) \
	X(name_id) X(shader_path) X(inherit_id) X(class_no_template) X(owner_id) X(template_id) X(value_id) \
	X(raw_value) X(cond_term) X(call_arg) X(paren_group) X(pso_param_id) X(node_param_id) X(array) X(pointer) \
	X(array_count_id) X(insert_block) X(shader_type)

#define ENTER_AND_EXIT_RULES(X) X(namespace_header) X(function_definition) X(node_output_decl)

#define EXIT_ONLY_RULES(X) X(namespace_definition) X(call) X(paren_group)

	void enter(const Ast& n)
	{
		switch (n.tag)
		{
#define CASE_GENERATED(r) case prism_tag(#r): enter_body_##r(n); stamp(n); break;
#define CASE_ENTER(r) case prism_tag(#r): enter_##r(n); break;
			GENERATED_RULES(CASE_GENERATED)
			ENTER_ONLY_RULES(CASE_ENTER)
			ENTER_AND_EXIT_RULES(CASE_ENTER)
#undef CASE_GENERATED
#undef CASE_ENTER
		default:
			break;
		}
	}

	void leave(const Ast& n)
	{
		switch (n.tag)
		{
#define CASE_GENERATED(r) case prism_tag(#r): end_elem(); break;
#define CASE_EXIT(r) case prism_tag(#r): exit_##r(n); break;
			GENERATED_RULES(CASE_GENERATED)
			ENTER_AND_EXIT_RULES(CASE_EXIT)
			EXIT_ONLY_RULES(CASE_EXIT)
#undef CASE_GENERATED
#undef CASE_EXIT
		default:
			break;
		}
	}

#define GENERATE(x) void enter_body_##x([[maybe_unused]] const Ast& ctx)
#define EXIT(x) void exit_##x([[maybe_unused]] const Ast& ctx)
#define ENTER(x) void enter_##x([[maybe_unused]] const Ast& ctx)

	ENTER(namespace_header)
	{
		setup_map(get_elem<Parsed>().namespaces);
		stamp(ctx);
	}

	EXIT(namespace_header)
	{
		auto& ns = get_elem<Namespace>();
		ns.path = ns.ns.empty() ? ns.name : ns.ns + "::" + ns.name;
		end_elem();
		ns_stack.push_back(ns.path);
	}

	EXIT(namespace_definition)
	{
		ns_stack.pop_back();
	}

	GENERATE(layout_definition)
	{
		setup_map(get_elem<Parsed>().layouts);
	}

	GENERATE(table_definition)
	{
		setup_map(get_elem<Parsed>().tables);
	}

	GENERATE(const_definition)
	{
		setup_map(get_elem<Parsed>().consts);
	}

	ENTER(function_definition)
	{
		setup_list(get_elem<Table>().functions);
		stamp(ctx);
	}

	EXIT(function_definition)
	{
		auto& fn = get_elem<Function>();

		// Token views are exact; a function_params token has no leading
		// whitespace (the `(` before it skipped it) but can carry trailing
		// whitespace up to the `)`.
		std::string_view params = require_child(ctx, "function_params").token;
		while (!params.empty() && std::isspace(static_cast<unsigned char>(params.back())))
			params.remove_suffix(1);
		if (!params.empty())
			fn.params = std::string(params);

		// From the start of the return type's line, so the pasted text keeps the
		// indentation its first line was written with.
		size_t begin = offset_of(first_leaf(require_child(ctx, "type_id"))->token.data());
		size_t line_start = begin;
		while (line_start > 0 && (source[line_start - 1] == ' ' || source[line_start - 1] == '\t'))
			--line_start;

		const Ast& body = require_child(ctx, "FUNC_BODY");
		size_t end = offset_of(body.token.data()) + body.token.size();
		fn.source = source.substr(line_start, end - line_start);
		fn.body_loc = loc_of(body);

		end_elem();

		// [HLSL] is the default; a function marked only [CPP] stays out of the shader.
		if (!fn.find_option("CPP") || fn.find_option("HLSL"))
			get_elem<Table>().hlsl += "\n" + fn.source + "\n";
	}

	GENERATE(slot_declaration)
	{
		setup_map(get_elem<Layout>().slots);
	}

	GENERATE(value_declaration)
	{
		setup_list(get_elem<Table>().values);
	}

	GENERATE(option)
	{
		setup_map(get_elem<have_options>().options);
	}

	GENERATE(sampler_declaration)
	{
		setup_map(get_elem<Layout>().samplers);
	}

	GENERATE(rt_definition)
	{
		setup_map(get_elem<Parsed>().rt);
	}

	GENERATE(rt_color_declaration)
	{
		setup_list(get_elem<RenderTarget>().rtvs);
	}

	GENERATE(rt_ds_declaration)
	{
		auto& owner = get_elem<RenderTarget>().dsv;
		ASSERT(!owner);
		owner = DSV();

		setup_elem(*owner);
	}


	GENERATE(compute_pso_definition)
	{
		setup_map(get_elem<Parsed>().compute_pso);
	}

	GENERATE(graphics_pso_definition)
	{
		setup_map(get_elem<Parsed>().graphics_pso);
	}

	GENERATE(rtx_pso_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_pso);
	}

	GENERATE(workgraph_pso_definition)
	{
		setup_map(get_elem<Parsed>().workgraph_pso);
	}

	GENERATE(node_definition)
	{
		setup_map(get_elem<WorkgraphPSO>().nodes);
	}

	ENTER(node_output_decl)
	{
		setup_list(get_elem<WorkgraphNode>().outputs);
		stamp(ctx);
	}

	EXIT(node_output_decl)
	{
		auto& output = get_elem<WorkgraphNodeOutput>();
		if (auto* opt = output.find_option("MaxRecords"))
			output.max_records = std::atoi(opt->value_atom.expr.c_str());
		end_elem();
	}

	GENERATE(node_param)
	{
		setup_map(get_elem<param_holder>().params);
	}


	// index is assigned after all files are merged (assign_rtx_ids in Main.cpp).
	GENERATE(rtx_pass_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_pass);
	}

	GENERATE(rtx_raygen_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_gen);
	}


	GENERATE(root_sig)
	{
		setup_elem(get_elem<root_holder>().root_sig);
	}


	GENERATE(shader)
	{
		setup_map(get_elem<shader_holder>().shaders);
	}


	GENERATE(options_assign)
	{
		auto& owner = get_elem<option>();
		setup_elem(owner.value_atom);
	}


	GENERATE(define_declaration)
	{
		setup_map(get_elem<PSO>().defines);
	}


	GENERATE(rtv_formats_declaration)
	{
		auto& owner = get_elem<GraphicsPSO>();
		setup_elem(owner.rtv);
	}

	GENERATE(blends_declaration)
	{
		auto& owner = get_elem<GraphicsPSO>();
		setup_elem(owner.blend);
	}


	GENERATE(pso_param)
	{
		setup_map(get_elem<param_holder>().params);
	}


	GENERATE(array_value_holder)
	{
		setup_list(get_elem<have_values>().values);
	}

	GENERATE(flag_value_holder)
	{
		setup_list(get_elem<have_values>().values);
	}

	GENERATE(view_definition)
	{
		setup_map(get_elem<Parsed>().views);
	}
	GENERATE(pass_definition)
	{
		setup_map(get_elem<Parsed>().passes);
	}
	GENERATE(view_declaration)
	{
		setup_list(get_elem<View>().params);
	}

	GENERATE(pipeline_definition)
	{
		setup_map(get_elem<Parsed>().pipelines);
	}
	GENERATE(pipeline_stat)
	{
		setup_map(get_elem<Pipeline>().entries);
	}

	GENERATE(enum_definition)
	{
		setup_map(get_elem<Parsed>().enums);
	}

	GENERATE(enum_value_declaration)
	{
		setup_list(get_elem<Enum>().values);
	}

	ENTER(name_id)
	{
		auto& elem = get_elem<have_name>();
		elem.name = text_of(ctx);
		elem.name_loc = loc_of(ctx);
	}

	ENTER(shader_path)
	{
		auto& elem = get_elem<Shader>();

		if (child_of(ctx, "STRING"))
		{
			std::string text = text_of(ctx);
			elem.path_literal = text.substr(1, text.size() - 2);
			elem.path_quoted = true;
			elem.path_loc = loc_of(ctx, 1);

			constexpr std::string_view ext = ".hlsl";
			elem.path = elem.path_literal.ends_with(ext)
				? elem.path_literal.substr(0, elem.path_literal.size() - ext.size())
				: elem.path_literal; // validate() rejects it
		}
		else
		{
			elem.path = elem.path_literal = text_of(ctx);
			elem.path_loc = loc_of(ctx);
		}
	}

	ENTER(inherit_id)
	{
		auto& elem = get_elem<inherited>();
		elem.parent.emplace_back(text_of(ctx));
	}

	ENTER(class_no_template)
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = text_of(ctx);
		elem.detect_type(find_elem<have_options>());
	}

	ENTER(owner_id)
	{
		auto& elem = get_elem<have_owner>();
		elem.owner_name = text_of(ctx);
	}

	ENTER(template_id)
	{
		auto& elem = get_elem<have_type>();

		if (!elem.template_arg.empty())
			elem.template_arg += ' ';
		elem.template_arg += text_of(ctx);

		elem.detect_type(find_elem<have_options>());
	}

	ENTER(value_id)
	{
		auto& elem = get_elem<have_expr>();
		elem.expr = text_of(ctx);
		elem.is_literal = child_of(ctx, "INT_SCALAR") || child_of(ctx, "FLOAT_SCALAR") || child_of(ctx, "bool_type");
	}

	ENTER(raw_value)
	{
		auto& elem = get_elem<have_expr>();
		std::string text = text_of(ctx);
		elem.expr = text.substr(1, text.size() - 2); // strip the enclosing backticks
		elem.is_raw = true;
	}

	// Records each cond_term in source order. Deliberately does NOT touch
	// expr/owner_name/is_literal: enter_value_id and enter_owner_id still fire for
	// the leaves inside these terms, so a single-term option value keeps
	// producing exactly the fields it always did. Codegen renders from terms
	// only when there is more than one of them, which is what makes this change
	// incapable of altering any existing option's generated output.
	ENTER(cond_term)
	{
		auto& elem = get_elem<have_expr>();
		auto& term = elem.terms.emplace_back();
		term.text_loc = loc_of(ctx);

		if (auto* q = child_of(ctx, "qualified_ref"))
		{
			const Ast& owner = require_child(*q, "owner_id");
			const Ast& value = require_child(*q, "value_id");
			term.kind  = ExprTerm::Qualified;
			term.owner = text_of(owner);
			term.text  = text_of(value);
			term.owner_loc = loc_of(owner);
			term.text_loc  = loc_of(value);
		}
		else if (auto* m = child_of(ctx, "member_ref"))
		{
			const Ast& owner = *m->nodes.front();
			const Ast& member = *m->nodes.back();
			term.kind  = ExprTerm::Member;
			term.owner = text_of(owner);
			term.text  = text_of(member);
			term.owner_loc = loc_of(owner);
			term.text_loc  = loc_of(member);
		}
		else if (child_of(ctx, "function_id"))
		{
			term.kind = ExprTerm::Function;
			term.text = text_of(ctx);
			if (term.text.rfind("exists(", 0) == 0)
				term.text_loc = loc_of(ctx, 7);
		}
		else if (auto* c = child_of(ctx, "call"))
		{
			// The argument terms follow via their own enter_cond_term; the
			// separators and closing paren come from the handlers below.
			term.kind = ExprTerm::Call;
			term.text = text_of(require_child(*c, "ID"));
		}
		else if (child_of(ctx, "cond_op"))
		{
			term.kind = ExprTerm::Op;
			term.text = text_of(ctx);
		}
		else
		{
			term.kind = ExprTerm::Plain;
			term.text = text_of(ctx);
		}
	}

	void add_op_term(std::string text, SourceLocation loc)
	{
		auto& term = get_elem<have_expr>().terms.emplace_back();
		term.kind = ExprTerm::Op;
		term.text = std::move(text);
		term.text_loc = std::move(loc);
	}

	ENTER(call_arg)
	{
		if (&require_child(*ctx.parent, "call_arg") != &ctx)
			add_op_term(",", {});
	}

	EXIT(call)
	{
		add_op_term(")", {});
	}

	// A parenthesised group inside a call's argument; a cond_op pair elsewhere.
	ENTER(paren_group)
	{
		add_op_term("(", loc_of(ctx));
	}

	EXIT(paren_group)
	{
		add_op_term(")", loc_of(*ctx.nodes.back()));
	}

	ENTER(pso_param_id)
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = text_of(ctx);
		elem.detect_type(find_elem<have_options>());
	}

	ENTER(node_param_id)
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = text_of(ctx);
		elem.detect_type(find_elem<have_options>());
	}

	ENTER(array)
	{
		{
			auto& elem = get_elem<have_array>();
			elem.as_array = true;
			elem.array_count = 0;
		}
		{
			auto& elem = get_elem<have_type>();
			elem.bindless = true;
		}
	}

	ENTER(pointer)
	{
		auto& elem = get_elem<have_type>();
		elem.pointer = true;

		elem.detect_type(find_elem<have_options>());
	}

	ENTER(array_count_id)
	{
		{
			auto& elem = get_elem<have_array>();
			elem.array_count = std::atoi(text_of(ctx).c_str());
		}
		{
			auto& elem = get_elem<have_type>();
			elem.bindless = false;
		}

		{
			auto& elem = get_elem<have_type>();
			elem.detect_type(find_elem<have_options>());
		}
	}


	ENTER(insert_block)
	{
		auto str = text_of(ctx);
		auto& elem = get_elem<have_hlsl>();
		elem.hlsl += str.substr(2, str.size() - 4);
		elem.inserted = str.substr(2, str.size() - 4);
		elem.hlsl_loc = loc_of(ctx);
	}


	ENTER(shader_type)
	{
		if (auto* shader = find_elem<Shader>())
			shader->name = text_of(ctx);
	}
};

static Parsed parse_input(const std::string& text, const std::string& file)
{
	Parsed parsed;

	std::vector<PrismSyntaxError> errors;
	auto ast = prism_parse_ast(text, file.c_str(), errors);

	// Walking a failed parse only builds a misleading partial model for the
	// validator to complain about; the syntax errors are the report.
	if (!ast)
	{
		for (const auto& e : errors)
			diagnostics().error(SourceLocation{ file, e.line, e.column }, e.message);
		return parsed;
	}

	TreeShapeListener listener(parsed, file, text);
	listener.walk(*ast);

	return parsed;
}

Parsed parse(std::wstring filename)
{
	std::string file = std::filesystem::absolute(filename).string();

	std::ifstream stream(filename);
	if (!stream.is_open())
	{
		diagnostics().error(SourceLocation{ file }, "cannot open file");
		return {};
	}

	std::ostringstream contents;
	contents << stream.rdbuf();
	return parse_input(contents.str(), file);
}

Parsed parse_text(const std::string& text, const std::string& file)
{
	return parse_input(text, file);
}

std::vector<std::string> sig_keywords()
{
	return prism_grammar_keywords();
}
