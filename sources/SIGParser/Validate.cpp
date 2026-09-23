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
	// marked "unread": used in .sig files as annotations, consumed by nothing.
	const std::set<std::string> PSO_OPTIONS = { "Template", "ExcludeVulkan" };
	const std::set<std::string> RESOURCE_FIELD_OPTIONS = {
		"Always", "ArrayCount", "Format", "MipCount", "Optional", "PrevFor", "Recreate", "RecreateFlags",
		"Size", "SkipEnablement", "Write"
	};

	const std::map<std::string, std::set<std::string>> KNOWN_OPTIONS = {
		{ "struct", { "Bind", "IndirectCommand", "RenderTarget", "nobind", "raypayload", "serialize", "shader_only", "Template" } },
		{ "struct field", { "Auto", "Barrier", "DispatchSize", "dynamic", "read", "write",
			"Write" /* unread */ } },
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
	};

	void check_options(const have_options& holder, const std::string& kind, const std::string& owner_name)
	{
		auto known = KNOWN_OPTIONS.find(kind);

		for (const auto& opt : holder.options)
		{
			if (known != KNOWN_OPTIONS.end() && known->second.count(opt.name))
				continue;

			diagnostics().error(opt, std::format("unknown option [{}] on {} '{}'", opt.name, kind, owner_name));
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

	// Mirrors CONDITION_OPTIONS in Main.cpp.
	const std::set<std::string> CONDITION_OPTIONS = { "SetupCondition", "RenderCondition", "Optional" };

	// Qualified names in a condition that are neither a SIG struct nor a SIG
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
						diagnostics().error(opt, std::format("[{}] on '{}': struct '{}' has no field '{}'",
							opt.name, owner.name, t.owner, t.text));
				}
				else if (const Enum* e = parsed.enums.find(t.owner))
				{
					bool found = std::any_of(e->values.begin(), e->values.end(), [&](const EnumValue& v) { return v.name == t.text; });
					if (!found)
						diagnostics().error(opt, std::format("[{}] on '{}': enum '{}' has no value '{}'",
							opt.name, owner.name, t.owner, t.text));
				}
				else if (!KNOWN_CPP_SCOPES.count(t.owner))
				{
					diagnostics().error(opt, std::format("[{}] on '{}': '{}::{}' names no struct, enum or known C++ scope",
						opt.name, owner.name, t.owner, t.text));
				}
				break;

			case ExprTerm::Member:
				if (t.owner != "data")
					diagnostics().error(opt, std::format("[{}] on '{}': '{}.{}' -- only 'data.<field>' is available in a condition",
						opt.name, owner.name, t.owner, t.text));
				else if (!find_param(owner.params, t.text)
					&& !(t.text == "pass_index" && owner.find_option("Multiple"))) // implicit, see pass.jinja
					diagnostics().error(opt, std::format("[{}] on '{}': no field '{}'", opt.name, owner.name, t.text));
				break;

			case ExprTerm::Function:
				if (t.text.rfind("exists(", 0) == 0 && t.text.back() == ')')
				{
					std::string field = t.text.substr(7, t.text.size() - 8);
					if (!find_param(owner.params, field))
						diagnostics().error(opt, std::format("[{}] on '{}': exists({}) names no field of this pass",
							opt.name, owner.name, field));
				}
				break;

			default:
				break;
			}
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
				if (CONDITION_OPTIONS.count(opt.name))
					check_condition(parsed, opt, owner);

			// resolve_size_expr turns Owner::field into get_context<Table::Owner>().field
			// unconditionally, so the owner has to be a SIG struct.
			if (const option* size = p.find_option("Size"))
			{
				const auto& atom = size->value_atom;
				if (!atom.is_raw && !atom.is_literal && !atom.owner_name.empty()
					&& !find_table_field(parsed, atom.owner_name, atom.expr))
					diagnostics().error(*size, std::format("[Size] on '{}.{}': '{}::{}' is not a field of a SIG struct",
						owner.name, p.name, atom.owner_name, atom.expr));
			}

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

	// %{ }% is pasted into HLSL verbatim, so a `# note` written out of .sig
	// habit becomes an invalid preprocessor directive. Without this the error
	// surfaces only when the engine compiles the shader at load time.
	void check_hlsl_block(const have_hlsl& holder, const std::string& owner_name)
	{
		std::istringstream lines(holder.hlsl);
		std::string line;
		size_t line_no = holder.hlsl_loc.line; // the block text starts right after '%{' on this line

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
				diagnostics().error(SourceLocation{ holder.hlsl_loc.file, line_no, indent + 1 },
					std::format("'{}': %{{ }}% is raw HLSL, so this '#' line is an invalid preprocessor directive; use // for comments",
						owner_name));
		}
	}

	template <class T>
	void check_pso(const T& pso, const std::string& kind)
	{
		check_options(pso, kind, pso.name);

		for (const auto& s : pso.shaders)
			check_options(s, "shader", pso.name + "." + s.name);

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

	for (const auto& table : parsed.tables)
	{
		check_options(table, "struct", table.name);
		for (const auto& v : table.values)
			check_options(v, "struct field", table.name + "." + v.name);
		check_hlsl_block(table, table.name);
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

	for (const auto& pipeline : parsed.pipelines)
	{
		check_duplicates(pipeline.entries, "pipeline entry");
		for (const auto& entry : pipeline.entries)
		{
			check_options(entry, "pipeline entry", pipeline.name + "." + entry.name);
			if (!parsed.passes.find(entry.name))
				diagnostics().error(entry, std::format("Pipeline '{}' names unknown PassNode '{}'", pipeline.name, entry.name));
		}
	}
}
