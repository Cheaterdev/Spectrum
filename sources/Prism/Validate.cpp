import Core;

#include "Validate.h"
#include "Diagnostics.h"

namespace
{
	// Option names each declaration kind is allowed to carry. An option nothing
	// reads is not merely dead: a misspelled [SetupConditon] parses fine and
	// then silently drops the pass's condition, so the pass runs
	// unconditionally with no compile error anywhere.
	//
	// Every name here must be read by Main.cpp, Parsed.cpp or a template. Add
	// to it in the same change that adds the consumer. The exceptions are
	// marked "unread": used in .prism files as annotations, consumed by nothing.
	const std::set<std::string> PSO_OPTIONS = { "Template", "ExcludeVulkan" };
	const std::set<std::string> RESOURCE_FIELD_OPTIONS = {
		"Always", "ArrayCount", "Format", "MipCount", "Optional", "PrevFor", "Recreate", "RecreateFlags",
		"Size", "SkipEnablement", "Write",
		"Counted", // buffer gets a counter (StructuredDesc::counted)
	};

	const std::map<std::string, std::set<std::string>> KNOWN_OPTIONS = {
		{ "struct", { "Bind", "IndirectCommand", "RenderTarget", "nobind", "raypayload", "serialize", "shader_only", "Template" } },
		{ "struct field", { "Auto", "Barrier", "DispatchSize", "dynamic", "read", "write",
			"Write" /* unread */ } },
		// [HLSL] is also the default; [CPP] (emit into the C++ struct) is not implemented yet.
		{ "function", { "HLSL" } },
		{ "layout", {} },
		{ "slot", {} },
		{ "render target", {} },
		{ "ComputePSO", PSO_OPTIONS },
		{ "GraphicsPSO", { "Template", "ExcludeVulkan", "Base" /* unread */ } },
		{ "WorkgraphPSO", PSO_OPTIONS },
		{ "RaytracePSO", PSO_OPTIONS },
		{ "RaytraceRaygen", { "Bind" } },
		{ "RaytracePass", { "Bind" } },
		{ "shader", { "EntryPoint", "Erase", "Enable16bits" } },
		{ "define", { "CS", "PS", "VS", "GS", "FS", "HS", "MS", "AS", "rename", "type", "indirect",
			"nullable" /* unread */ } },
		{ "PSO param", {} },
		{ "rtv", {} },
		{ "blend", {} },
		{ "workgraph node param", {} },
		{ "workgraph node output", { "MaxRecords" } },
		{ "view", { "SetupCondition", "RenderCondition", "Optional" } },
		{ "view field", RESOURCE_FIELD_OPTIONS },
		{ "PassNode", { "Compute", "Multiple", "PreSetup", "RenderCondition", "Required", "RunAlways", "SetupCondition", "Static" } },
		{ "PassNode field", RESOURCE_FIELD_OPTIONS },
		{ "pipeline entry", { "Async", "Async2", "Async3" } },
		{ "namespace", {} },
	};

	// Case-insensitive Levenshtein distance.
	size_t edit_distance(std::string_view a, std::string_view b)
	{
		std::vector<size_t> row(b.size() + 1);
		std::iota(row.begin(), row.end(), size_t(0));
		for (size_t i = 1; i <= a.size(); ++i)
		{
			size_t diag = row[0];
			row[0] = i;
			for (size_t j = 1; j <= b.size(); ++j)
			{
				size_t up = row[j];
				bool same = std::tolower((unsigned char)a[i - 1]) == std::tolower((unsigned char)b[j - 1]);
				row[j] = std::min({ row[j] + 1, row[j - 1] + 1, diag + (same ? 0 : 1) });
				diag = up;
			}
		}
		return row[b.size()];
	}

	std::optional<std::string> closest(const std::string& bad, const std::vector<std::string>& candidates)
	{
		// Loose enough for a typo or two, tight enough not to suggest an unrelated name.
		size_t limit = std::max<size_t>(2, bad.size() / 3);
		std::optional<std::string> best;
		size_t best_distance = limit + 1;
		for (const auto& c : candidates)
		{
			size_t d = edit_distance(bad, c);
			if (d < best_distance && c != bad)
			{
				best = c;
				best_distance = d;
			}
		}
		return best;
	}

	// An unknown name at `loc`: adds "did you mean" and a fix when a candidate is close.
	void unknown_name(const SourceLocation& loc, std::string message, const std::string& bad, const std::vector<std::string>& candidates)
	{
		if (auto best = closest(bad, candidates))
			diagnostics().error(loc, message + std::format("; did you mean '{}'?", *best), Diagnostics::Fix{ bad.size(), *best });
		else
			diagnostics().error(loc, std::move(message));
	}

	const SourceLocation& name_loc_of(const have_name& n)
	{
		return n.name_loc.line ? n.name_loc : n.loc;
	}

	void check_options(const have_options& holder, const std::string& kind, const std::string& owner_name)
	{
		auto known = KNOWN_OPTIONS.find(kind);
		std::vector<std::string> names;
		if (known != KNOWN_OPTIONS.end())
			names.assign(known->second.begin(), known->second.end());

		for (const auto& opt : holder.options)
		{
			if (known != KNOWN_OPTIONS.end() && known->second.count(opt.name))
				continue;

			unknown_name(name_loc_of(opt), std::format("unknown option [{}] on {} '{}'", opt.name, kind, owner_name), opt.name, names);
		}
	}

	template <class C>
	void check_duplicates(const C& container, const std::string& kind)
	{
		std::map<std::string, const parsed_type*> first;
		for (const auto& item : container)
		{
			auto [it, inserted] = first.emplace(item.name, &item);
			if (!inserted)
			{
				const auto& prev = it->second->loc;
				diagnostics().error(item, std::format("{} '{}' is already declared at {}({},{})",
					kind, item.name, prev.file, prev.line, prev.column));
			}
		}
	}

	const Value* find_table_field(const Parsed& parsed, const std::string& table_name, const std::string& field, int depth = 0)
	{
		const Table* table = parsed.tables.find(table_name);
		if (!table || depth > 16)
			return nullptr;

		for (const auto& v : table->values)
			if (v.name == field)
				return &v;

		for (const auto& parent : table->parent)
			if (const Value* v = find_table_field(parsed, parent, field, depth + 1))
				return v;

		return nullptr;
	}

	const View_Param* find_param(const std::list<View_Param>& params, const std::string& name)
	{
		for (const auto& p : params)
			if (p.name == name)
				return &p;
		return nullptr;
	}

	void table_field_names(const Parsed& parsed, const std::string& table_name, std::vector<std::string>& out, int depth = 0)
	{
		const Table* table = parsed.tables.find(table_name);
		if (!table || depth > 16)
			return;
		for (const auto& v : table->values)
			out.push_back(v.name);
		for (const auto& parent : table->parent)
			table_field_names(parsed, parent, out, depth + 1);
	}

	std::vector<std::string> param_names(const View& owner)
	{
		std::vector<std::string> out;
		for (const auto& p : owner.params)
			out.push_back(p.name);
		if (owner.find_option("Multiple"))
			out.push_back("pass_index");
		return out;
	}

	// Mirrors CONDITION_OPTIONS in Main.cpp.
	const std::set<std::string> CONDITION_OPTIONS = { "SetupCondition", "RenderCondition", "Optional" };

	// Qualified names in a condition that are neither a Prism struct nor a Prism
	// enum and are known to be valid C++ in the generated pass code.
	const std::set<std::string> KNOWN_CPP_SCOPES = {
	};

	void check_condition(const Parsed& parsed, const option& opt, const View& owner)
	{
		for (const auto& t : opt.value_atom.terms)
		{
			switch (t.kind)
			{
			case ExprTerm::Qualified:
				if (parsed.tables.find(t.owner))
				{
					if (!find_table_field(parsed, t.owner, t.text))
					{
						std::vector<std::string> fields;
						table_field_names(parsed, t.owner, fields);
						unknown_name(t.text_loc, std::format("[{}] on '{}': struct '{}' has no field '{}'",
							opt.name, owner.name, t.owner, t.text), t.text, fields);
					}
				}
				else if (const Enum* e = parsed.enums.find(t.owner))
				{
					bool found = std::any_of(e->values.begin(), e->values.end(), [&](const EnumValue& v) { return v.name == t.text; });
					if (!found)
					{
						std::vector<std::string> values;
						for (const auto& v : e->values)
							values.push_back(v.name);
						unknown_name(t.text_loc, std::format("[{}] on '{}': enum '{}' has no value '{}'",
							opt.name, owner.name, t.owner, t.text), t.text, values);
					}
				}
				else if (!KNOWN_CPP_SCOPES.count(t.owner))
				{
					std::vector<std::string> scopes;
					for (const auto& table : parsed.tables)
						scopes.push_back(table.name);
					for (const auto& e : parsed.enums)
						scopes.push_back(e.name);
					unknown_name(t.owner_loc, std::format("[{}] on '{}': '{}::{}' names no struct, enum or known C++ scope",
						opt.name, owner.name, t.owner, t.text), t.owner, scopes);
				}
				break;

			case ExprTerm::Member:
				if (t.owner != "data")
					diagnostics().error(t.owner_loc, std::format("[{}] on '{}': '{}.{}' -- only 'data.<field>' is available in a condition",
						opt.name, owner.name, t.owner, t.text));
				else if (!find_param(owner.params, t.text)
					&& !(t.text == "pass_index" && owner.find_option("Multiple"))) // implicit, see pass.jinja
					unknown_name(t.text_loc, std::format("[{}] on '{}': no field '{}'", opt.name, owner.name, t.text),
						t.text, param_names(owner));
				break;

			case ExprTerm::Function:
				if (t.text.rfind("exists(", 0) == 0 && t.text.back() == ')')
				{
					std::string field = t.text.substr(7, t.text.size() - 8);
					if (!find_param(owner.params, field))
						unknown_name(t.text_loc, std::format("[{}] on '{}': exists({}) names no field of this pass",
							opt.name, owner.name, field), field, param_names(owner));
				}
				break;

			default:
				break;
			}
		}
	}

	// A [Size]/[ArrayCount]/[MipCount] expression: every Owner::name must resolve
	// (a struct field, an enum value or a const) and every call must be a known
	// size function.
	void check_size_expression(const Parsed& parsed, const option& opt, const std::string& where)
	{
		const std::string& o = opt.name;

		std::vector<std::string> functions;
		for (const auto& [name, cpp] : size_functions())
			functions.push_back(name);

		auto check_function = [&](const std::string& name, const SourceLocation& loc)
		{
			if (!size_functions().count(name))
				unknown_name(loc, std::format("[{}] on '{}': unknown function '{}'", o, where, name), name, functions);
		};

		for (const auto& t : opt.value_atom.terms)
		{
			switch (t.kind)
			{
			case ExprTerm::Qualified:
				if (parsed.tables.find(t.owner))
				{
					if (!find_table_field(parsed, t.owner, t.text))
					{
						std::vector<std::string> fields;
						table_field_names(parsed, t.owner, fields);
						unknown_name(t.text_loc, std::format("[{}] on '{}': struct '{}' has no field '{}'", o, where, t.owner, t.text),
							t.text, fields);
					}
				}
				else if (t.owner == "Constants")
				{
					if (!parsed.consts.find(t.text))
					{
						std::vector<std::string> consts;
						for (const auto& c : parsed.consts)
							consts.push_back(c.name);
						unknown_name(t.text_loc, std::format("[{}] on '{}': no const '{}'", o, where, t.text), t.text, consts);
					}
				}
				else if (!parsed.enums.find(t.owner))
				{
					std::vector<std::string> owners{ "Constants" };
					for (const auto& table : parsed.tables)
						owners.push_back(table.name);
					unknown_name(t.owner_loc, std::format("[{}] on '{}': '{}::{}' names no struct, enum or Constants",
						o, where, t.owner, t.text), t.owner, owners);
				}
				break;

			case ExprTerm::Call:
				check_function(t.text, t.text_loc);
				break;

			case ExprTerm::Function:
				check_function(t.text.substr(0, t.text.find('(')), t.text_loc);
				break;

			case ExprTerm::Member:
				diagnostics().error(t.owner_loc, std::format("[{}] on '{}': '{}.{}' -- a size can't depend on pass state",
					o, where, t.owner, t.text));
				break;

			default:
				break;
			}
		}
	}

	// The generator pastes these after "FrameGraph::ResourceFlags::" or
	// "HAL::Format::", so a bad name is otherwise a C++ compile error far from
	// the .prism line that caused it.
	void check_enum_option(const option& opt, const std::string& where)
	{
		const auto& atom = opt.value_atom;
		const CppEnum* e = option_enum(opt.name, opt.loc.file);
		if (!e || atom.is_raw)
			return;

		if (e->names.empty())
		{
			static std::set<std::filesystem::path> reported;
			if (reported.insert(e->source).second)
				diagnostics().error(opt, std::format("[{}] values can't be checked: no {} enum found in {}",
					opt.name, e->cpp_name, std::filesystem::path(e->source).make_preferred().string()));
			return;
		}

		auto check = [&](const std::string& value, const SourceLocation& loc)
		{
			if (std::find(e->names.begin(), e->names.end(), value) == e->names.end())
				unknown_name(loc, std::format("[{}] on '{}': '{}' is not a {} value", opt.name, where, value, e->cpp_name),
					value, e->names);
		};

		if (!atom.values.empty())
		{
			if (opt.name == "Format")
				diagnostics().error(opt, std::format("[Format] on '{}' takes a single {} value", where, e->cpp_name));
			for (const auto& v : atom.values)
				check(v.expr, v.loc);
		}
		else if (!atom.owner_name.empty() || atom.terms.size() > 1)
			diagnostics().error(opt, std::format("[{}] on '{}' takes a bare {} name{}", opt.name, where, e->cpp_name,
				opt.name == "Format" ? "" : ", or several joined with |"));
		else
			check(atom.expr, atom.terms.empty() ? opt.loc : atom.terms.front().text_loc);
	}

	// A namespace reopened in several blocks has one set of options: a block
	// either repeats them exactly or writes none, so which block a member is in
	// can never change how it's treated.
	void check_namespaces(const Parsed& parsed)
	{
		auto text = [](const Namespace& n)
		{
			std::string s;
			for (const auto& o : n.options)
				s += "[" + o.name + "=" + o.value_atom.expr + "]";
			return s;
		};

		// A namespace becomes Table::<ns>, PSOS::<ns>, ..., so from inside any
		// generated body it hides a same-named outer scope: a namespace `HAL`
		// makes every `HAL::RWStructuredBuffer` in Table resolve to Table::HAL.
		// These are the scopes the templates qualify names with; a declaration
		// name counts too (DefaultLayout::Instance0, Constants-style owners).
		static const std::set<std::string> RESERVED = {
			"Constants", "Context", "ContextField", "Core", "FrameGraph", "HAL", "HLSL", "Handlers",
			"IndirectCommands", "Layouts", "Math", "PSO", "PSOS", "PassID", "Passes", "Pipelines",
			"RT", "RTX", "ResourceFlags", "ResourceID", "SlotID", "Slots", "Table", "Tables",
			"concurrency", "std",
		};
		std::set<std::string> declared;
		auto add = [&](const auto& container) { for (const auto& d : container) declared.insert(d.name); };
		add(parsed.tables); add(parsed.layouts); add(parsed.rt); add(parsed.compute_pso); add(parsed.graphics_pso);
		add(parsed.workgraph_pso); add(parsed.raytrace_pso); add(parsed.raytrace_pass); add(parsed.raytrace_gen);
		add(parsed.views); add(parsed.passes); add(parsed.pipelines); add(parsed.enums); add(parsed.consts);

		std::map<std::string, const Namespace*> with_options;
		for (const auto& n : parsed.namespaces)
		{
			if (RESERVED.count(n.name))
				diagnostics().error(name_loc_of(n), std::format("namespace '{}': the name is used by generated code "
					"(it would hide {}:: inside every generated type); pick another", n.name, n.name));
			else if (declared.count(n.name))
				diagnostics().error(name_loc_of(n), std::format("namespace '{}' has the same name as a declaration; "
					"generated code would mistake one for the other", n.name));

			check_options(n, "namespace", n.path);
			if (n.options.size() == 0)
				continue;

			auto [it, inserted] = with_options.emplace(n.path, &n);
			if (!inserted && text(*it->second) != text(n))
			{
				const auto& prev = it->second->loc;
				diagnostics().error(n, std::format("namespace '{}' is reopened with different options than at {}({},{}); "
					"write them on one block, or repeat them exactly", n.path, prev.file, prev.line, prev.column));
			}
		}
	}

	// `const A = Constants::B * 2;`. A const is a constexpr in Constants.ixx, so it
	// can read only consts declared before it (C++ declaration order) and no
	// runtime state at all.
	void check_const_values(const Parsed& parsed)
	{
		std::vector<std::string> earlier;
		for (const auto& c : parsed.consts)
		{
			for (const auto& t : c.value_atom.is_raw ? std::list<ExprTerm>{} : c.value_atom.terms)
			{
				if (t.kind == ExprTerm::Qualified)
				{
					if (t.owner != "Constants")
						unknown_name(t.owner_loc, std::format("const '{}': '{}::{}' -- a const can only use Constants::",
							c.name, t.owner, t.text), t.owner, { "Constants" });
					else if (std::find(earlier.begin(), earlier.end(), t.text) == earlier.end())
					{
						if (parsed.consts.find(t.text))
							diagnostics().error(t.text_loc, std::format("const '{}': Constants::{} is declared after it", c.name, t.text));
						else
							unknown_name(t.text_loc, std::format("const '{}': no const '{}'", c.name, t.text), t.text, earlier);
					}
				}
				else if (t.kind == ExprTerm::Member)
					diagnostics().error(t.owner_loc, std::format("const '{}': '{}.{}' -- a const can't read pass state",
						c.name, t.owner, t.text));
				else if (t.kind == ExprTerm::Call || t.kind == ExprTerm::Function)
				{
					std::string name = t.text.substr(0, t.text.find('('));
					if (!size_functions().count(name))
					{
						std::vector<std::string> functions;
						for (const auto& [f, cpp] : size_functions())
							functions.push_back(f);
						unknown_name(t.text_loc, std::format("const '{}': unknown function '{}'", c.name, name), name, functions);
					}
				}
			}
			earlier.push_back(c.name);
		}
	}

	void check_view_fields(const Parsed& parsed, const View& owner)
	{
		for (const auto& opt : owner.options)
			if (CONDITION_OPTIONS.count(opt.name))
				check_condition(parsed, opt, owner);

		for (const auto& p : owner.params)
		{
			for (const auto& opt : p.options)
			{
				if (CONDITION_OPTIONS.count(opt.name))
					check_condition(parsed, opt, owner);
				check_enum_option(opt, owner.name + "." + p.name);
			}

			for (const char* name : { "Size", "ArrayCount", "MipCount" })
				if (const option* o = p.find_option(name); o && !o->value_atom.is_raw)
					check_size_expression(parsed, *o, owner.name + "." + p.name);

			if (const View* view = parsed.views.find(p.class_no_template))
			{
				if (const option* w = p.find_option("Write"))
				{
					auto check_leaf = [&](const std::string& leaf)
					{
						if (!leaf.empty() && !find_param(view->params, leaf))
							diagnostics().error(*w, std::format("[Write] on '{}.{}': view '{}' has no field '{}'",
								owner.name, p.name, view->name, leaf));
					};

					if (!w->value_atom.values.empty())
						for (const auto& v : w->value_atom.values)
							check_leaf(v.expr);
					else
						check_leaf(w->value_atom.expr);
				}
			}
		}
	}

	bool is_preprocessor_directive(std::string_view line)
	{
		static const std::array<std::string_view, 12> directives = {
			"include", "define", "undef", "if", "ifdef", "ifndef", "elif", "else", "endif", "pragma", "line", "error"
		};

		line.remove_prefix(1);
		while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
			line.remove_prefix(1);

		for (auto d : directives)
			if (line.starts_with(d) && (line.size() == d.size() || !std::isalnum((unsigned char)line[d.size()])))
				return true;
		return false;
	}

	// %{ }% blocks and function bodies are pasted into HLSL verbatim, so a
	// `# note` written out of .prism habit becomes an invalid preprocessor
	// directive. Without this the error surfaces only when the engine compiles
	// the shader at load time.
	void check_hlsl_text(const std::string& text, const SourceLocation& start, const std::string& owner_name)
	{
		std::istringstream lines(text);
		std::string line;
		size_t line_no = start.line; // the text starts on this line

		for (; std::getline(lines, line); ++line_no)
		{
			std::string_view trimmed = line;
			size_t indent = 0;
			while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
			{
				trimmed.remove_prefix(1);
				++indent;
			}

			if (trimmed.starts_with('#') && !is_preprocessor_directive(trimmed))
				diagnostics().error(SourceLocation{ start.file, line_no, indent + 1 },
					std::format("'{}': this is HLSL, so a '#' line is a preprocessor directive, and this one is invalid; use // for comments",
						owner_name));
		}
	}

	void check_shader_path(const Shader& s, const std::string& owner_name)
	{
		if (!s.path_quoted)
		{
			if (s.path_literal != "none" && s.path_literal != "null")
				diagnostics().error(s.path_loc, std::format("{}.{}: shader must be a quoted path such as \"dir/file.hlsl\" "
					"(bare words are only for `none` / `null`)", owner_name, s.name));
			return;
		}

		if (!s.path_literal.ends_with(".hlsl"))
		{
			diagnostics().error(s.path_loc, std::format("{}.{}: shader path \"{}\" must end with .hlsl", owner_name, s.name, s.path_literal));
			return;
		}

		std::filesystem::path shaders = shaders_root(s.path_loc.file);
		if (!shaders.empty() && !std::filesystem::exists(shaders / s.path_literal))
			diagnostics().error(s.path_loc, std::format("{}.{}: no shader file \"{}\" in {}", owner_name, s.name,
				s.path_literal, shaders.lexically_normal().string()));
	}

	template <class T>
	void check_pso(const T& pso, const std::string& kind)
	{
		check_options(pso, kind, pso.name);

		for (const auto& s : pso.shaders)
		{
			check_options(s, "shader", pso.name + "." + s.name);
			check_shader_path(s, pso.name);
		}

		for (const auto& d : pso.defines)
			check_options(d, "define", pso.name + "." + d.name);

		if constexpr (std::is_base_of_v<param_holder, T>)
			for (const auto& p : pso.params)
				check_options(p, "PSO param", pso.name);
	}

	template <class T>
	void check_rtx_bind(const Parsed& parsed, const T& item, const std::string& kind)
	{
		const option* bind = item.find_option("Bind");
		if (!bind)
			diagnostics().error(item, std::format("{} '{}' has no [Bind = <RaytracePSO>]", kind, item.name));
		else if (!parsed.raytrace_pso.find(bind->value_atom.expr))
			diagnostics().error(*bind, std::format("{} '{}': [Bind = {}] names no RaytracePSO", kind, item.name, bind->value_atom.expr));
	}
}

static std::filesystem::path checkout_root(const std::string& sig_file)
{
	std::error_code ec;
	for (auto p = std::filesystem::absolute(sig_file, ec).parent_path(); !p.empty() && p != p.root_path(); p = p.parent_path())
		if (std::filesystem::is_directory(p / "workdir" / "shaders", ec))
			return p;
	return {};
}

std::filesystem::path shaders_root(const std::string& sig_file)
{
	auto root = checkout_root(sig_file);
	return root.empty() ? root : root / "workdir" / "shaders";
}

// The enumerator names between the braces following `header`. Enough for the
// plain enums this reads: no nested braces, comments allowed, `= value` ignored.
static std::vector<std::string> read_cpp_enum(const std::filesystem::path& file, const std::string& header)
{
	std::ifstream f(file, std::ios::binary);
	std::string text(std::istreambuf_iterator<char>{f}, {});

	size_t at = text.find(header);
	if (at == std::string::npos) return {};
	size_t open = text.find('{', at);
	size_t close = open == std::string::npos ? open : text.find('}', open);
	if (close == std::string::npos) return {};
	std::string body = text.substr(open + 1, close - open - 1);

	std::string code;
	for (size_t i = 0; i < body.size(); ++i)
	{
		if (body.compare(i, 2, "//") == 0)
			i = std::min(body.find('\n', i), body.size()) - 1;
		else if (body.compare(i, 2, "/*") == 0)
			i = std::min(body.find("*/", i), body.size() - 2) + 1;
		else
			code += body[i];
	}

	std::vector<std::string> names;
	std::stringstream entries(code);
	for (std::string entry; std::getline(entries, entry, ',');)
	{
		auto is_ident = [](char c) { return std::isalnum((unsigned char)c) || c == '_'; };
		auto begin = std::find_if(entry.begin(), entry.end(), is_ident);
		auto end = std::find_if_not(begin, entry.end(), is_ident);
		if (begin != end)
			names.emplace_back(begin, end);
	}
	return names;
}

const CppEnum* option_enum(const std::string& option_name, const std::string& sig_file)
{
	struct Source { const char* cpp_name; const char* file; const char* header; };
	static const Source flags  = { "FrameGraph::ResourceFlags", "sources/RenderSystem/FrameGraph/FrameGraph.Base.ixx", "enum class ResourceFlags" };
	static const Source format = { "HAL::Format", "sources/HAL/HAL.Format.ixx", "enum Formats" };

	const Source* src = option_name == "Always" || option_name == "RecreateFlags" ? &flags
		: option_name == "Format" ? &format : nullptr;
	if (!src) return nullptr;

	// Keyed on the write time too: the language server lives for a whole VS
	// session, during which the enum can gain a value.
	struct Entry { std::filesystem::file_time_type time; CppEnum e; };
	static std::map<std::filesystem::path, Entry> cache;
	auto path = checkout_root(sig_file) / src->file;
	std::error_code ec;
	auto time = std::filesystem::last_write_time(path, ec);
	auto& entry = cache[path];
	if (entry.e.cpp_name.empty() || entry.time != time)
		entry = { time, CppEnum{ src->cpp_name, path, read_cpp_enum(path, src->header) } };
	return &entry.e;
}

const std::set<std::string>& known_options(const std::string& kind)
{
	static const std::set<std::string> none;
	auto it = KNOWN_OPTIONS.find(kind);
	return it != KNOWN_OPTIONS.end() ? it->second : none;
}

const std::map<std::string, std::string>& size_functions()
{
	static const std::map<std::string, std::string> functions = {
		{ "tiles", "Math::DivideByMultiple" }, // tiles(v, n): n-sized tiles covering v, per component, rounded up
		{ "area",  "Math::Area" },             // area(v): element count of a grid of size v
		{ "ivec2", "ivec2" },                  // ivec2(w, h): a non-square 2D size
	};
	return functions;
}

std::vector<std::string> option_kinds(const std::string& option_name)
{
	std::vector<std::string> out;
	for (const auto& [kind, names] : KNOWN_OPTIONS)
		if (names.count(option_name))
			out.push_back(kind);
	return out;
}

void validate(Parsed& parsed)
{
	check_duplicates(parsed.tables, "struct");
	check_duplicates(parsed.layouts, "layout");
	check_duplicates(parsed.rt, "render target");
	check_duplicates(parsed.compute_pso, "ComputePSO");
	check_duplicates(parsed.graphics_pso, "GraphicsPSO");
	check_duplicates(parsed.workgraph_pso, "WorkgraphPSO");
	check_duplicates(parsed.raytrace_pso, "RaytracePSO");
	check_duplicates(parsed.raytrace_gen, "RaytraceRaygen");
	check_duplicates(parsed.raytrace_pass, "RaytracePass");
	check_duplicates(parsed.views, "view");
	check_duplicates(parsed.passes, "PassNode");
	check_duplicates(parsed.pipelines, "Pipeline");
	check_duplicates(parsed.enums, "enum");
	check_duplicates(parsed.consts, "const");
	check_const_values(parsed);
	check_namespaces(parsed);

	for (const auto& table : parsed.tables)
	{
		check_options(table, "struct", table.name);
		for (const auto& v : table.values)
			check_options(v, "struct field", table.name + "." + v.name);
		check_hlsl_text(table.inserted, table.hlsl_loc, table.name);
		for (const auto& f : table.functions)
		{
			check_options(f, "function", table.name + "." + f.name);
			check_hlsl_text(f.source.substr(f.source.find('{')), f.body_loc, table.name + "." + f.name);
		}
	}

	for (const auto& layout : parsed.layouts)
	{
		check_options(layout, "layout", layout.name);
		for (const auto& slot : layout.slots)
			check_options(slot, "slot", layout.name + "." + slot.name);
	}

	for (const auto& rt : parsed.rt)
		check_options(rt, "render target", rt.name);

	for (const auto& pso : parsed.compute_pso)
		check_pso(pso, "ComputePSO");

	for (const auto& pso : parsed.graphics_pso)
	{
		check_pso(pso, "GraphicsPSO");
		check_options(pso.rtv, "rtv", pso.name);
		check_options(pso.blend, "blend", pso.name);
	}

	for (const auto& pso : parsed.workgraph_pso)
	{
		check_pso(pso, "WorkgraphPSO");
		for (const auto& node : pso.nodes)
		{
			for (const auto& p : node.params)
				check_options(p, "workgraph node param", pso.name + "." + node.name);
			for (const auto& out : node.outputs)
				check_options(out, "workgraph node output", pso.name + "." + node.name);
		}
	}

	for (const auto& pso : parsed.raytrace_pso)
		check_pso(pso, "RaytracePSO");

	for (const auto& gen : parsed.raytrace_gen)
	{
		check_pso(gen, "RaytraceRaygen");
		check_rtx_bind(parsed, gen, "RaytraceRaygen");
	}

	for (const auto& pass : parsed.raytrace_pass)
	{
		check_pso(pass, "RaytracePass");
		check_rtx_bind(parsed, pass, "RaytracePass");
	}

	for (const auto& view : parsed.views)
	{
		check_options(view, "view", view.name);
		for (const auto& p : view.params)
			check_options(p, "view field", view.name + "." + p.name);
		check_view_fields(parsed, view);
	}

	for (const auto& pass : parsed.passes)
	{
		check_options(pass, "PassNode", pass.name);
		for (const auto& p : pass.params)
			check_options(p, "PassNode field", pass.name + "." + p.name);
		check_view_fields(parsed, pass);
	}

	std::vector<std::string> pass_names;
	for (const auto& pass : parsed.passes)
		pass_names.push_back(pass.name);

	for (const auto& pipeline : parsed.pipelines)
	{
		check_duplicates(pipeline.entries, "pipeline entry");
		for (const auto& entry : pipeline.entries)
		{
			check_options(entry, "pipeline entry", pipeline.name + "." + entry.name);
			if (!parsed.passes.find(entry.name))
				unknown_name(name_loc_of(entry), std::format("Pipeline '{}' names unknown PassNode '{}'", pipeline.name, entry.name),
					entry.name, pass_names);
		}
	}
}
