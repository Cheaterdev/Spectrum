import Core;
import jinja2cpp;
import cereal.json;


#include "Parsed.h"
#include "Parsing.h"

static const std::string cpp_path = "../../sources/HAL/autogen";
static const std::string shaders_path = "../../workdir/shaders";
static const std::string hlsl_path = shaders_path + "/autogen";


static const std::string cpp_path_render = "../../sources/RenderSystem/FrameGraph/autogen";

// Mirrors FrameGraph::WRITEABLE_FLAGS (sources/RenderSystem/FrameGraph/FrameGraph.Base.ixx).
// SIGParser can't include that C++ module, so keep these two lists in sync by hand -
// used to derive [Always=X] fields' write-ness for resource_accesses[].
static const std::set<std::string> WRITEABLE_FLAG_NAMES = {
	"CopyDest", "UnorderedAccess", "RenderTarget", "DepthStencil"
};

// A PassNode field of one of these types is plain per-frame CPU state (e.g.
// an index a [Multiple] instance's own setup lambda writes for a later
// [Optional=...] guard to read), not a FrameGraph resource -- no Handlers::/
// ResourceID::, not part of resource_accesses[]/get_all_resources().
static const std::set<std::string> PRIMITIVE_SCALAR_TYPES = {
	"int", "uint", "bool", "float"
};


using namespace jinja2;

void iterate_files(std::filesystem::path path, std::function<void(std::filesystem::path)> f)
{
	using namespace std::filesystem;

	recursive_directory_iterator dir(path), end;

	while (dir != end)
	{
		// do other stuff here.
		if (!is_directory(*dir))
			f(dir->path());

		++dir;
	}
}

jinja2::TemplateEnv global;

rapidjson::Document make_map(auto& p)
{
	std::stringstream s;
	{
		cereal::JSONOutputArchive archive(s);

		p.serialize(archive);
	}
	rapidjson::Document doc;

	auto str = s.str();
	doc.Parse(str.c_str());

	return doc;
}

class TemplatesLibrary
{
	std::map<std::wstring, jinja2::Template> templates;

public:
	TemplatesLibrary(std::string dir)
	{
		iterate_files("templates/" + dir + "/", [this](std::filesystem::path path)
		{
			std::ifstream stream;
			stream.open(path);

			if (!stream.is_open())
				return;


			auto& tpl = templates[path.stem()];
			tpl = jinja2::Template(&global);
			tpl.Load(stream);

			stream.close();
		});
	}

	auto generate(std::wstring filename, const ValuesMap& params)
	{
		jinja2::Template& tpl = templates[filename];
		auto res = tpl.RenderAsString(params);

		if (res.has_value())
			return res.value();

		std::cout << res.error().ToString() << std::endl;
	}

	auto generate(std::wstring filename)
	{
		return generate(filename, ValuesMap{});
	}

	auto generate2(std::wstring filename, std::string name, auto& p)
	{
		auto dp = make_map(p);

		ValuesMap params = {
			{name, Reflect(dp)}
		};

		return generate(filename, params);
	}
};


// Options whose value is a boolean expression over contexts rather than a
// single name or flag set. Only these get rendered from ExprTerm; every other
// option keeps its existing single-atom code path (resolve_size_expr,
// resolve_flags_expr, ...) untouched, so this cannot regress them.
static const std::set<std::string> CONDITION_OPTIONS = {
	"SetupCondition", "RenderCondition", "Optional"
};

// Options that feed graph structure but whose value CANNOT be expressed as a
// set of context fields -- anything resolving a ResourceID at runtime, since
// there is no finite bit that represents "some arbitrary resource". A pass
// carrying one of these must report deps_complete = false, or it claims a
// provable dependency set while its resource set silently varies.
//
// Empty right now: [NeedDynamic] was the only member and UI_Render, its only
// user, moved to a declared field plus Graph::override_resource (ui.sig). Kept
// so that reintroducing such an option cannot quietly produce a lying mask.
static const std::set<std::string> UNPROVABLE_STRUCTURAL_OPTIONS = {
};

// Renders one parsed expression back to C++ and records which Table:: contexts
// it read.
//
// Spacing is not cosmetic: single spaces around binary operators, none after a
// prefix '!' or inside parentheses, reproduces byte-for-byte what the
// hand-written backtick conditions produced. That is what makes "the generated
// pass_defaults.cpp is unchanged" a usable acceptance test for the migration --
// any difference in the output is a real difference in meaning, not whitespace.
static void render_expr(const Parsed& parsed, have_expr& e)
{
	// A backtick span is opaque C++: it may read contexts this function cannot
	// see, so the dependency set stops being provable. Say so rather than
	// reporting an empty/partial list a cache could trust.
	if (e.is_raw)
	{
		e.deps_complete = false;
		return;
	}

	if (e.terms.empty())
		return;

	std::string out;
	bool suppress_space = true; // no leading space

	auto append = [&](const std::string& s, bool tight_after)
	{
		if (!suppress_space && !out.empty() && s != ")")
			out += ' ';
		out += s;
		suppress_space = tight_after;
	};

	for (const auto& t : e.terms)
	{
		switch (t.kind)
		{
		case ExprTerm::Qualified:
			// Owner::name is a context field only if Owner is a SIG-declared
			// struct. Anything else (an enum value, a Constants:: entry) names
			// no runtime state, so it passes through and records no dependency.
			if (parsed.tables.find(t.owner))
			{
				append("builder.graph->get_context<Table::" + t.owner + ">()." + t.text, false);
				auto& ref = e.field_refs.emplace_back();
				ref.owner = t.owner;
				ref.field = t.text;
			}
			else
			{
				append(t.owner + "::" + t.text, false);
			}
			break;

		case ExprTerm::Member:
			append(t.owner + "." + t.text, false);
			break;

		case ExprTerm::Function:
		{
			// exists(X) is sugar for the builder.exists(data.X) guard that
			// [Optional] uses constantly; it reads graph structure, not a
			// context, so it adds no field dependency.
			const std::string& fn = t.text;
			if (fn.rfind("exists(", 0) == 0)
				append("builder." + fn.substr(0, 7) + "data." + fn.substr(7), false);
			else
				append(fn, false);
			break;
		}

		case ExprTerm::Op:
			// '!' and '(' bind tight to what follows; ')' to what precedes.
			append(t.text, t.text == "!" || t.text == "(");
			break;

		default:
			append(t.text, false);
			break;
		}
	}

	e.expr = out;
}

// Walks every condition-valued option on every pass and pass field and renders
// it. Runs after parsed.setup() so parsed.tables is complete across all .sig
// files -- a condition may name a context declared in a file parsed later.
static void render_condition_options(Parsed& parsed)
{
	auto do_options = [&](have_options& holder)
	{
		for (auto& opt : holder.options)
			if (CONDITION_OPTIONS.count(opt.name))
				render_expr(parsed, opt.value_atom);
	};

	for (auto& pass : parsed.passes)
	{
		do_options(pass);
		for (auto& param : pass.params)
			do_options(param);
	}

	for (auto& view : parsed.views)
	{
		do_options(view);
		for (auto& param : view.params)
			do_options(param);
	}
}


int main()
{
	std::map<std::string, ValuesList> user_lists;

	try
	{
		Parsed parsed;

		iterate_files("sigs/", [&](std::wstring filename)
		{
			auto p = parse(filename);

			// Stamp every top-level named item with its source .sig path so
			// templates can emit an autogenerated-from comment in the output.
			std::string sig_path = "sources/SIGParser/"
			    + std::filesystem::path(filename).generic_string();
			auto tag = [&](auto& container)
			{
				for (auto& item : container)
					item.source_file = sig_path;
			};
			tag(p.tables);
			tag(p.layouts);
			tag(p.compute_pso);
			tag(p.graphics_pso);
			tag(p.workgraph_pso);
			tag(p.raytrace_pso);
			tag(p.rt);
			tag(p.views);
			tag(p.passes);
			tag(p.enums);

			parsed.merge(p);
		});

		parsed.setup();

		// Turns parsed condition terms into the C++ the templates paste, and
		// collects each condition's Table:: field dependencies on the way.
		render_condition_options(parsed);

		rapidjson::Document parsed_doc = make_map(parsed);

		jinja2::Value parsed_map = Reflect(parsed_doc);
		global.AddGlobal("parsed", parsed_map);
		global.AddGlobal("ValueType", ValuesMap{
			                 {"CB", ValueType::CB}, {"SRV", ValueType::SRV}, {"UAV", ValueType::UAV},
			                 {"SMP", ValueType::SMP}, {"STRUCT", ValueType::STRUCT}
		                 });

		global.AddGlobal("recursive_slots", jinja2::MakeCallable(
			                 [&](const std::string& name)
			                 {
				                 auto* layout = parsed.find_layout(name);
				                 ValuesList list;
				                 layout->recursive_slots([&](Slot& slot)
				                 {
					                 list.emplace_back(slot.name);
				                 });


				                 return list;
			                 },
			                 ArgInfo{"name"}
		                 ));

		global.AddGlobal("recursive_samplers", jinja2::MakeCallable(
			                 [&](const std::string& name)
			                 {
				                 auto* layout = parsed.find_layout(name);
				                 ValuesList list;


				                 layout->recursive_samplers([&](Sampler& slot)
				                 {
					                 list.emplace_back("HAL::Samplers::" + slot.expr);
				                 });


				                 return list;
			                 },
			                 ArgInfo{"name"}
		                 ));

		global.AddGlobal("get_name_for", jinja2::MakeCallable(
			                 [&](const int& i)
			                 {
				                 return get_name_for((ValueType)i);
			                 },
			                 ArgInfo{"i"}
		                 ));

		global.AddGlobal("create_list", jinja2::MakeCallable(
			                 [&](const std::string& b)
			                 {
				                 auto& list = user_lists[b];
				                 list.clear();
				                 //	 a.asList().emplace_back(b);
				                 return list;
			                 },
			                 ArgInfo{"b"}
		                 ));

		global.AddGlobal("get_list", jinja2::MakeCallable(
			                 [&](const std::string& b)
			                 {
				                 auto& list = user_lists[b];

				                 return list;
			                 },
			                 ArgInfo{"b"}
		                 ));

		// Is this pipeline entry tagged [Async]? Placement is a per-pipeline
		// decision, so it lives on the entry rather than the PassNode. Resolved in
		// C++ because the template's view of an entry does not expose its options.
		global.AddGlobal("entry_is_async", jinja2::MakeCallable(
			                 [&](const std::string& pipeline_name, const std::string& entry_name)
			                 {
				                 Pipeline* pipeline_ptr = parsed.pipelines.find(pipeline_name);
				                 if (!pipeline_ptr) return false;

				                 const PipelineEntry* entry = pipeline_ptr->entries.find(entry_name);
				                 return entry && entry->find_option("Async") != nullptr;
			                 },
			                 ArgInfo{"pipeline"}, ArgInfo{"entry"}
		                 ));

		// [Async2] / [Async3] pick an additional compute queue for this entry.
		// Both imply [Async]: the pass still needs PassFlags::Compute to leave
		// the direct queue at all.
		auto entry_has = [&](const std::string& pipeline_name, const std::string& entry_name, const char* opt)
		{
			Pipeline* pipeline_ptr = parsed.pipelines.find(pipeline_name);
			if (!pipeline_ptr) return false;

			const PipelineEntry* entry = pipeline_ptr->entries.find(entry_name);
			return entry && entry->find_option(opt) != nullptr;
		};

		global.AddGlobal("entry_is_async2", jinja2::MakeCallable(
			                 [entry_has](const std::string& pipeline_name, const std::string& entry_name)
			                 {
				                 return entry_has(pipeline_name, entry_name, "Async2");
			                 },
			                 ArgInfo{"pipeline"}, ArgInfo{"entry"}
		                 ));

		global.AddGlobal("entry_is_async3", jinja2::MakeCallable(
			                 [entry_has](const std::string& pipeline_name, const std::string& entry_name)
			                 {
				                 return entry_has(pipeline_name, entry_name, "Async3");
			                 },
			                 ArgInfo{"pipeline"}, ArgInfo{"entry"}
		                 ));

		global.AddGlobal("myappend", jinja2::MakeCallable(
			                 [&](const std::string& list_name, const std::string& b)
			                 {
				                 auto& list = user_lists[list_name];

				                 list.emplace_back(b);

				                 return list;
			                 },
			                 ArgInfo{"a"}, ArgInfo{"b"}
		                 ));

		global.AddGlobal("merge_lists", jinja2::MakeCallable(
			                 [](const GenericList& a, const GenericList& b)
			                 {
				                 ValuesList list;
				                 for (const auto& e : a)
					                 list.emplace_back(e);

				                 for (const auto& e : b)
					                 list.emplace_back(e);

				                 return list;
			                 },
			                 ArgInfo{"a"}, ArgInfo{"b"}
		                 ));
							  global.AddGlobal("merge_lists3", jinja2::MakeCallable(
			                 [](const GenericList& a, const GenericList& b, const GenericList& c)
			                 {
				                 ValuesList list;
				                 for (const auto& e : a)
					                 list.emplace_back(e);

				                 for (const auto& e : b)
					                 list.emplace_back(e);
								      for (const auto& e : c)
					                 list.emplace_back(e);

				                 return list;
			                 },
			                 ArgInfo{"a"}, ArgInfo{"b"}		   , ArgInfo{"c"}
		                 ));
		global.AddGlobal("lowerize", jinja2::MakeCallable(
			                 [](const std::string& name)
			                 {
				                 std::string lowcameled = name;
				                 lowcameled[0] = std::tolower(lowcameled[0]);
				                 return lowcameled;
			                 },
			                 ArgInfo{"name"}
		                 ));

		global.AddGlobal("camel", jinja2::MakeCallable(
			                 [](const std::string& name)
			                 {
				                 std::string lowcameled = name;
				                 lowcameled[0] = std::toupper(lowcameled[0]);
				                 return lowcameled;
			                 },
			                 ArgInfo{"name"}
		                 ));


		global.AddGlobal("replace_start", jinja2::MakeCallable(
			                 [](const std::string& a, const std::string& b, const std::string& c)
			                 {
				                 std::string res = a;

				                 if (res.starts_with(b))
					                 res.replace(0, b.size(), c);

				                 return res;
			                 },
			                 ArgInfo{"a"}, ArgInfo{"b"}, ArgInfo{"c"}
		                 ));



		global.AddGlobal("get_offset", jinja2::MakeCallable(
			                 [&](const GenericList& offsets, const int& i)
			                 {
				                 return offsets.GetAccessor()->GetIndexer()->GetItemByIndex(i);
			                 },
			                
			                 ArgInfo{"offsets"}, ArgInfo{"i"}
		                 ));
		global.AddGlobal("get_all_resources", jinja2::MakeCallable(
			[&]() -> ValuesList
			{
				std::set<std::string> seen;
				ValuesList result;

				std::function<void(const std::list<View_Param>&)> collect;
				collect = [&](const std::list<View_Param>& params)
				{
					for (const auto& p : params)
					{
						View* view = parsed.views.find(p.class_no_template);
						if (view)
							collect(view->params);
						else if (PRIMITIVE_SCALAR_TYPES.count(p.class_no_template) == 0)
						{
							if (seen.insert(p.name).second)
								result.emplace_back(p.name);
						}
					}
				};

				for (const auto& pass : parsed.passes)
					collect(pass.params);

				return result;
			}
		));

		// Fingerprint of the generated ID space -- every PassID and ResourceID
		// name, in the exact order the enums emit them.
		//
		// A stored graph plan records passes and resources by ID, and editing a
		// .sig renumbers those enums. Loading a plan built against a different ID
		// space does not fail, it MISAPPLIES: wrong resource, wrong pass, wrong
		// barriers. Comparing this constant turns that silent corruption into a
		// plain cache miss.
		global.AddGlobal("id_space_hash", jinja2::MakeCallable(
			[&]() -> std::string
			{
				unsigned long long h = 1469598103934665603ull;
				auto mix = [&](const std::string& n)
				{
					for (unsigned char c : n) { h ^= c; h *= 1099511628211ull; }
					h ^= '|'; h *= 1099511628211ull;
				};

				for (const auto& pass : parsed.passes)
					mix(pass.name);

				// Same traversal resource_ids.jinja uses, so the two stay in step.
				std::set<std::string> seen;
				std::function<void(const std::list<View_Param>&)> collect;
				collect = [&](const std::list<View_Param>& params)
				{
					for (const auto& prm : params)
					{
						View* view = parsed.views.find(prm.class_no_template);
						if (view)
							collect(view->params);
						else if (PRIMITIVE_SCALAR_TYPES.count(prm.class_no_template) == 0)
						{
							if (seen.insert(prm.name).second)
								mix(prm.name);
						}
					}
				};
				for (const auto& pass : parsed.passes)
					collect(pass.params);

				return std::to_string(h) + "ull";
			}
		));

		// Shared source of truth for a pass's ordered (resource_id, write)
		// accesses. A leaf's write-ness comes from: its own [Write] if declared
		// directly, or the enclosing view usage's [Write] / [Write = {leaves...}]
		// if it's inside a view group. A recreate adds a trailing write on the
		// same id.
		auto compute_pass_accesses = [&](Pass* pass, std::vector<std::pair<std::string, bool>>& out)
		{
			std::function<void(const std::list<View_Param>&, bool, const std::function<bool(const std::string&)>&)> rec;
			rec = [&](const std::list<View_Param>& params, bool inside_view, const std::function<bool(const std::string&)>& parent_is_write)
			{
				for (const auto& p : params)
				{
					View* view = parsed.views.find(p.class_no_template);
					if (view)
					{
						// Build the write predicate for this view usage from its
						// [Write] option: a list -> membership; bare [Write] ->
						// whole view; absent -> nothing.
						const option* w = p.find_option("Write");
						std::function<bool(const std::string&)> pred;
						if (!w)
						{
							pred = [](const std::string&) { return false; };
						}
						else
						{
							std::set<std::string> s;
							if (!w->value_atom.values.empty())
								for (const auto& v : w->value_atom.values) s.insert(v.expr);
							else if (!w->value_atom.expr.empty())
								s.insert(w->value_atom.expr);

							if (s.empty()) pred = [](const std::string&) { return true; };
							else            pred = [s](const std::string& n) { return s.count(n) > 0; };
						}
						rec(view->params, true, pred);
					}
					else if (PRIMITIVE_SCALAR_TYPES.count(p.class_no_template) > 0)
					{
						// Plain CPU-side state (e.g. a [Multiple] instance index) --
						// not a FrameGraph resource, not part of resource_accesses[].
					}
					else
					{
						// A leaf's own [Always=X] reflects the View's generic default
						// for a consumer that doesn't otherwise say -- but the
						// enclosing pass's own [Write=...] on THIS usage describes
						// what THIS pass specifically does, and must win: a leaf
						// marked [Always=Read] at the View level (the common case,
						// since most consumers only read it) is still a write for
						// whichever pass's own [Write={leaf,...}] actually creates
						// it. Checking [Always] first and ignoring parent_is_write
						// entirely for such leaves is what let a real producer like
						// AssetGBuffer get recorded as declaring a read on
						// GBuffer_Albedo, which it write-creates -- verify_declared_
						// access (FrameGraph.cpp) then ASSERTs the mismatch at
						// runtime.
						bool always_write = false;
						if (const option* always = p.find_option("Always"))
						{
							// [Always = A | B]: write if ANY of the OR'd flags is
							// writeable, matching FrameGraph::ResourceFlags's own
							// bitwise-OR semantics.
							if (!always->value_atom.values.empty())
								always_write = std::any_of(always->value_atom.values.begin(), always->value_atom.values.end(),
									[](const have_expr& v) { return WRITEABLE_FLAG_NAMES.count(v.expr) > 0; });
							else
								always_write = WRITEABLE_FLAG_NAMES.count(always->value_atom.expr) > 0;
						}

						bool own_write = inside_view ? parent_is_write(p.name)
						                              : (p.find_option("Write") != nullptr);
						bool write = always_write || own_write;
						out.emplace_back(p.name, write);
						if (p.find_option("Recreate"))
							out.emplace_back(p.name, true);
					}
				}
			};
			rec(pass->params, false, {});
		};

		global.AddGlobal("get_pass_accesses", jinja2::MakeCallable(
			[&, compute_pass_accesses](const std::string& pass_name) -> ValuesList
			{
				ValuesList result;
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return result;

				std::vector<std::pair<std::string, bool>> acc;
				compute_pass_accesses(pass, acc);
				for (const auto& [id, write] : acc)
				{
					ValuesMap m;
					m["id"] = id;
					m["write"] = write;
					result.push_back(std::move(m));
				}
				return result;
			},
			ArgInfo{"pass_name"}
		));

		// ---- Context-field dependency data -------------------------------
		// Every Table:: context field named by a condition, plus, per pass,
		// which of them its enable decision and its resource selection read.
		// This is what a FrameGraph computation cache keys off: a stage whose
		// field set is unchanged since last frame cannot have a different
		// result, so the stage can be reused instead of recomputed.
		//
		// Field IDs are assigned in (struct declaration, field declaration)
		// order over every struct a condition actually references, so the
		// numbering is deterministic across runs.
		// Owners split by ROLE, not merged: a field that only ever feeds a [Size]
		// changes a resource's geometry but never the pass set, so keying the
		// topology plan on it would fork a separate plan per window size --
		// N enable-set variants x M sizes instead of N + M. Emitted as a mask so
		// compute_graph_key can hash the two halves separately.
		std::set<std::string> condition_owners;
		std::set<std::string> desc_owners;

		auto context_field_list = [&]() -> std::vector<std::pair<std::string, std::string>>
		{
			std::set<std::string> owners;
			auto scan = [&](const have_options& holder)
			{
				for (const auto& opt : holder.options)
				{
					if (CONDITION_OPTIONS.count(opt.name))
					{
						for (const auto& r : opt.value_atom.field_refs)
						{
							owners.insert(r.owner);
							condition_owners.insert(r.owner);
						}
					}

					// Descriptor options (`[Size = ViewportContext::frame_size]`)
					// read contexts too. They feed no pass's condition mask -- a
					// size change never moves the enable set -- but the SNAPSHOT
					// has to cover them, because it is what a stored graph plan is
					// keyed on and a resize does change every sized resource's
					// desc.
					//
					// Only the single-atom form resolves here; an arithmetic
					// [Size] is still a raw backtick (REFACTOR_TODO item 6). That
					// costs nothing at this level: the key hashes every field of
					// an included struct regardless of who reads it, so pulling
					// ViewportContext in covers the opaque expressions as well.
					else if (!opt.value_atom.owner_name.empty())
					{
						owners.insert(opt.value_atom.owner_name);
						desc_owners.insert(opt.value_atom.owner_name);
					}
				}
			};
			for (const auto& pass : parsed.passes)
			{
				scan(pass);
				for (const auto& param : pass.params) scan(param);
			}

			std::vector<std::pair<std::string, std::string>> out;
			for (const auto& table : parsed.tables)
				if (owners.count(table.name))
					for (const auto& v : table.values)
						out.emplace_back(table.name, v.name);
			return out;
		}();

		auto field_index = [context_field_list](const std::string& owner, const std::string& field) -> int
		{
			for (size_t i = 0; i < context_field_list.size(); ++i)
				if (context_field_list[i].first == owner && context_field_list[i].second == field)
					return (int)i;
			return -1;
		};

		// Which fields belong to which half of the split cache. A field can be in
		// both (a struct read by a condition AND a [Size]); it then counts as a
		// condition input, since anything that can move the pass set has to
		// invalidate topology.
		global.AddGlobal("get_desc_only_fields", jinja2::MakeCallable(
			[context_field_list, condition_owners, desc_owners]() -> ValuesList
			{
				ValuesList result;
				for (const auto& [owner, field] : context_field_list)
					if (desc_owners.count(owner) && !condition_owners.count(owner))
						result.push_back(owner + "_" + field);
				return result;
			}
		));

		global.AddGlobal("get_context_fields", jinja2::MakeCallable(
			[context_field_list]() -> ValuesList
			{
				ValuesList result;
				for (const auto& [owner, field] : context_field_list)
				{
					ValuesMap m;
					m["owner"] = owner;
					m["field"] = field;
					m["id"]    = owner + "_" + field;
					result.push_back(std::move(m));
				}
				return result;
			}
		));

		global.AddGlobal("get_pass_context_deps", jinja2::MakeCallable(
			[&, field_index](const std::string& pass_name) -> ValuesMap
			{
				ValuesMap out;
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return out;

				// Two separate sets on purpose. A change to a condition field
				// can add or remove passes, which invalidates the whole
				// dependency graph; a change to an Optional field only moves
				// which resources an already-enabled pass touches. Callers that
				// cache those two stages separately need to tell them apart.
				std::set<int> cond, opt_fields;
				bool cond_complete = true, opt_complete = true;

				auto take = [&](const have_options& holder, std::set<int>& into, bool& complete)
				{
					for (const auto& o : holder.options)
					{
						if (!CONDITION_OPTIONS.count(o.name)) continue;
						if (!o.value_atom.deps_complete) complete = false;
						for (const auto& r : o.value_atom.field_refs)
						{
							int i = field_index(r.owner, r.field);
							if (i >= 0) into.insert(i);
							else complete = false;
						}
					}
				};

				take(*pass, cond, cond_complete);
				for (const auto& param : pass->params)
					take(param, opt_fields, opt_complete);

				for (const auto& o : pass->options)
					if (UNPROVABLE_STRUCTURAL_OPTIONS.count(o.name))
						cond_complete = false;

				// Emits the field NAMES, not indices: the generated table then
				// reads as the condition it came from instead of as a pile of
				// shift amounts, and it stops depending on the enum's ordering
				// -- inserting a field into a struct renumbers every bit after
				// it, which would silently rewrite every mask below it if the
				// table stored raw indices.
				auto to_list = [context_field_list](const std::set<int>& s)
				{
					ValuesList l;
					for (int i : s)
						l.push_back(context_field_list[i].first + "_" + context_field_list[i].second);
					return l;
				};

				out["condition_fields"]   = to_list(cond);
				out["condition_complete"] = cond_complete;
				out["optional_fields"]    = to_list(opt_fields);
				out["optional_complete"]  = opt_complete;
				return out;
			},
			ArgInfo{"pass_name"}
		));

		// [Always = A] or [Always = A | B] on a leaf PassNode field, expanded
		// into a ready-to-interpolate "FrameGraph::ResourceFlags::A |
		// FrameGraph::ResourceFlags::B" expression for need_always()/
		// create_always() codegen. Resolved in C++ rather than jinja since
		// joining a variable-length reflected list with a per-item prefix
		// isn't something to rely on the jinja2cpp dialect supporting.
		global.AddGlobal("resolve_flags_expr", jinja2::MakeCallable(
			[&](const std::string& pass_name, const std::string& field_name) -> std::string
			{
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return "";

				for (const auto& p : pass->params)
				{
					if (p.name != field_name) continue;

					const option* always = p.find_option("Always");
					if (!always) return "";

					// [SkipEnablement]: an access flag rather than a usage, so it is
					// appended to whatever [Always] declared instead of replacing it.
					// See ResourceFlags::SkipEnablement.
					const std::string tail = p.find_option("SkipEnablement")
						? " | FrameGraph::ResourceFlags::SkipEnablement" : "";

					if (always->value_atom.values.empty())
						return "FrameGraph::ResourceFlags::" + always->value_atom.expr + tail;

					std::string result;
					bool first = true;
					for (const auto& v : always->value_atom.values)
					{
						if (!first) result += " | ";
						result += "FrameGraph::ResourceFlags::" + v.expr;
						first = false;
					}
					return result + tail;
				}
				return "";
			},
			ArgInfo{"pass_name"}, ArgInfo{"field_name"}
		));

		// [RecreateFlags = X] or [RecreateFlags = A | B]: the flags for the
		// builder.recreate() call on a [Recreate=X] field's new chain link,
		// resolved the same way [Always] is for resolve_flags_expr -- kept as
		// a separate option because [Always] is already spoken for by the
		// need() on the field's original chain link (recreate() always needs
		// a different flags value than the read it replaces).
		global.AddGlobal("resolve_recreate_flags_expr", jinja2::MakeCallable(
			[&](const std::string& pass_name, const std::string& field_name) -> std::string
			{
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return "";

				for (const auto& p : pass->params)
				{
					if (p.name != field_name) continue;

					const option* recreate_flags = p.find_option("RecreateFlags");
					if (!recreate_flags) return "";

					if (recreate_flags->value_atom.values.empty())
						return "FrameGraph::ResourceFlags::" + recreate_flags->value_atom.expr;

					std::string result;
					bool first = true;
					for (const auto& v : recreate_flags->value_atom.values)
					{
						if (!first) result += " | ";
						result += "FrameGraph::ResourceFlags::" + v.expr;
						first = false;
					}
					return result;
				}
				return "";
			},
			ArgInfo{"pass_name"}, ArgInfo{"field_name"}
		));

		// For a PassNode field whose type is a View (e.g. `GBuffer gbuffer;`),
		// resolves every leaf the View's OWN declaration marks [Always=X] into
		// a {name, flags} pair -- EXCEPT leaves this specific pass usage's own
		// [Write] / [Write={leaves...}] already covers (a leaf being written
		// elsewhere in this same setup() shouldn't also get a blanket Read
		// need()). Lets a consumer just declare `GBuffer gbuffer;` (or a
		// producer declare `[Write={...}] GBuffer gbuffer;`) and get the
		// equivalent of a hand-written bulk need() call for every common leaf
		// for free -- same one-source-of-truth reasoning as everything else
		// [Always] already covers, just recursed one level into a view group.
		global.AddGlobal("get_view_needs", jinja2::MakeCallable(
			[&](const std::string& pass_name, const std::string& field_name) -> ValuesList
			{
				ValuesList result;
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return result;

				for (const auto& p : pass->params)
				{
					if (p.name != field_name) continue;

					View* view = parsed.views.find(p.class_no_template);
					if (!view) return result;

					std::set<std::string> write_set;
					bool write_all = false;
					if (const option* w = p.find_option("Write"))
					{
						if (!w->value_atom.values.empty())
							for (const auto& v : w->value_atom.values) write_set.insert(v.expr);
						else if (!w->value_atom.expr.empty())
							write_set.insert(w->value_atom.expr);
						else
							write_all = true;
					}

					for (const auto& leaf : view->params)
					{
						if (write_all || write_set.count(leaf.name)) continue;

						const option* always = leaf.find_option("Always");
						if (!always) continue;

						std::string flags;
						if (always->value_atom.values.empty())
							flags = "FrameGraph::ResourceFlags::" + always->value_atom.expr;
						else
						{
							bool first = true;
							for (const auto& v : always->value_atom.values)
							{
								if (!first) flags += " | ";
								flags += "FrameGraph::ResourceFlags::" + v.expr;
								first = false;
							}
						}

						ValuesMap m;
						m["name"] = leaf.name;
						m["flags"] = flags;
						result.push_back(std::move(m));
					}
					break;
				}
				return result;
			},
			ArgInfo{"pass_name"}, ArgInfo{"field_name"}
		));

		// [Size = 128] (a literal) resolves to "128" as-is. [Size =
		// ViewportContext::frame_size] (owner_name set, not a literal)
		// resolves to a get_context<Table::Owner>().field call -- the exact
		// call ContextTypeFor's static-upcast redirect (Core:Holdable) makes
		// correct regardless of which real C++ type actually implements
		// Table::Owner. A bare identifier with no owner (no known use yet)
		// passes through as-is, on the assumption it names something
		// visible in scope (e.g. a compile-time constant).
		global.AddGlobal("resolve_size_expr", jinja2::MakeCallable(
			[&](const std::string& pass_name, const std::string& field_name) -> std::string
			{
				Pass* pass = parsed.passes.find(pass_name);
				if (!pass) return "";

				for (const auto& p : pass->params)
				{
					if (p.name != field_name) continue;

					const option* size = p.find_option("Size");
					if (!size) return "";

					// Raw (backtick) text is pasted verbatim -- the .sig author
					// already wrote a complete, valid C++ expression, so no
					// literal/owner-reference resolution applies to it.
					if (size->value_atom.is_raw)
						return size->value_atom.expr;

					if (size->value_atom.is_literal || size->value_atom.owner_name.empty())
						return size->value_atom.expr;

					return "builder.graph->get_context<Table::" + size->value_atom.owner_name + ">()." + size->value_atom.expr;
				}
				return "";
			},
			ArgInfo{"pass_name"}, ArgInfo{"field_name"}
		));

		global.AddGlobal("get_pipeline_resources", jinja2::MakeCallable(
			[&](const std::string& pipeline_name) -> ValuesList
			{
				Pipeline* pipeline_ptr = parsed.pipelines.find(pipeline_name);
				if (!pipeline_ptr) return {};

				std::set<std::string> seen;
				ValuesList result;

				std::function<void(const std::list<View_Param>&)> collect;
				collect = [&](const std::list<View_Param>& params)
				{
					for (const auto& p : params)
					{
						View* view = parsed.views.find(p.class_no_template);
						if (view)
						{
							collect(view->params);
						}
						else
						{
							if (seen.insert(p.name).second)
								result.emplace_back(p.name);

							const option* recreate = p.find_option("Recreate");
							if (recreate)
							{
								if (seen.insert(recreate->value_atom.expr).second)
									result.emplace_back(recreate->value_atom.expr);
							}
						}
					}
				};

				for (const auto& entry : pipeline_ptr->entries)
				{
					Pass* pass = parsed.passes.find(entry.name);
					if (pass)
						collect(pass->params);
				}

				return result;
			},
			ArgInfo{"pipeline_name"}
		));

		// Precomputes each pipeline's per-resource RW-state timeline (mirroring the
		// runtime add_pass grouping). Returns:
		//   { "resources": [ {id, states:[{write,begin,count}]}... ],
		//     "passes":    [ {pass, index}... ] }         // referenced by begin/count
		// where each pass ref is a PassID name + [Multiple] instance index.
		global.AddGlobal("get_pipeline_states", jinja2::MakeCallable(
			[&, compute_pass_accesses](const std::string& pipeline_name) -> ValuesMap
			{
				ValuesMap out;
				out["resources"] = ValuesList{};

				Pipeline* pipeline_ptr = parsed.pipelines.find(pipeline_name);
				if (!pipeline_ptr) return out;

				// A single pass touch: which pass (name + Multiple instance) and
				// whether it writes.
				struct Touch { std::string pass; uint32_t index; bool write; };

				// Per resource version (id + chain_index), ordered touches. A
				// recreate() starts a new chain version: within one pass instance,
				// the 2nd+ access to the same id is the recreate, and it plus every
				// later touch go to the new version. Separate first-seen order
				// keeps output deterministic.
				using Key = std::pair<std::string, uint32_t>;   // (id, chain_index)
				std::map<Key, std::vector<Touch>> touches;
				std::vector<Key>                  resource_order;
				std::map<std::string, uint32_t>   version;       // current chain version per id

				for (const auto& entry : pipeline_ptr->entries)
				{
					Pass* pass = parsed.passes.find(entry.name);
					if (!pass) continue;

					uint32_t count = 1;
					if (const option* m = pass->find_option("Multiple"))
						count = (uint32_t)std::max(1, atoi(m->value_atom.expr.c_str()));

					std::vector<std::pair<std::string, bool>> accesses;
					compute_pass_accesses(pass, accesses);

					for (uint32_t inst = 0; inst < count; ++inst)
					{
						std::set<std::string> seen_in_pass;
						for (const auto& [id, write] : accesses)
						{
							if (!seen_in_pass.insert(id).second)
								++version[id];              // recreate -> next chain version

							Key key{ id, version[id] };
							auto it = touches.find(key);
							if (it == touches.end())
							{
								resource_order.push_back(key);
								it = touches.emplace(key, std::vector<Touch>{}).first;
							}
							it->second.push_back({ entry.name, inst, write });
						}
					}
				}

				// Group touches into states (new state on write, first touch, or
				// read-after-write). Each resource version owns its own states and
				// pass refs; state begin/count are offsets into that version's refs.
				ValuesList resources;
				for (const auto& key : resource_order)
				{
					const auto& id = key.first;
					const auto& ts = touches[key];

					ValuesList states;
					ValuesList pass_refs;
					bool have_state = false;
					bool cur_write = false;
					uint32_t begin = 0;
					uint32_t cnt = 0;

					auto flush = [&]()
					{
						if (!have_state) return;
						ValuesMap s;
						s["write"] = cur_write;
						s["begin"] = (int64_t)begin;
						s["count"] = (int64_t)cnt;
						states.push_back(std::move(s));
					};

					for (const auto& t : ts)
					{
						bool need_new = !have_state || t.write || cur_write;
						if (need_new)
						{
							flush();
							have_state = true;
							cur_write = t.write;
							begin = (uint32_t)pass_refs.size();
							cnt = 0;
						}
						ValuesMap pr;
						pr["pass"] = t.pass;
						pr["index"] = (int64_t)t.index;
						pass_refs.push_back(std::move(pr));
						++cnt;
					}
					flush();

					ValuesMap r;
					r["pass_refs"] = std::move(pass_refs);
					r["id"] = id;
					r["chain_index"] = (int64_t)key.second;
					r["states"] = std::move(states);
					resources.push_back(std::move(r));
				}

				out["resources"] = std::move(resources);
				return out;
			},
			ArgInfo{"pipeline_name"}
		));

		// Precomputes each pipeline's pass instances with queue type and prev-pass
		// dependency edges (the codegen equivalent of resolve_dependencies, run
		// over the same per-resource state timelines). Returns a list of
		//   { pass, index, compute, prev:[{pass,index}...] }
		// in pipeline order.
		global.AddGlobal("get_pipeline_passes", jinja2::MakeCallable(
			[&, compute_pass_accesses](const std::string& pipeline_name) -> ValuesList
			{
				ValuesList result;
				Pipeline* pipeline_ptr = parsed.pipelines.find(pipeline_name);
				if (!pipeline_ptr) return result;

				using Ref = std::pair<std::string, uint32_t>;   // (pass name, instance)
				struct Touch { Ref ref; bool write; };

				std::vector<std::pair<Ref, bool>>       order;    // (ref, compute) in pipeline order
				std::map<std::string, std::vector<Touch>> touches; // per resource, ordered
				std::vector<std::string>                resource_order;

				for (const auto& entry : pipeline_ptr->entries)
				{
					Pass* pass = parsed.passes.find(entry.name);
					if (!pass) continue;

					uint32_t count = 1;
					if (const option* m = pass->find_option("Multiple"))
						count = (uint32_t)std::max(1, atoi(m->value_atom.expr.c_str()));
					// [Compute] on the PassNode says the pass CAN run on the compute
					// queue; [Async] on this pipeline entry says it SHOULD. Async is
					// opt-in: a compute pass with no [Async] runs inline on the direct
					// queue, which costs nothing extra and avoids a fence pair for work
					// that has nothing to overlap with.
					bool compute = pass->find_option("Compute") != nullptr
					            && (entry.find_option("Async") != nullptr
					             || entry.find_option("Async2") != nullptr
					             || entry.find_option("Async3") != nullptr);

					std::vector<std::pair<std::string, bool>> accesses;
					compute_pass_accesses(pass, accesses);

					for (uint32_t inst = 0; inst < count; ++inst)
					{
						Ref ref{ entry.name, inst };
						order.emplace_back(ref, compute);
						for (const auto& [id, write] : accesses)
						{
							auto it = touches.find(id);
							if (it == touches.end())
							{
								resource_order.push_back(id);
								it = touches.emplace(id, std::vector<Touch>{}).first;
							}
							it->second.push_back({ ref, write });
						}
					}
				}

				// prev-pass edges: for each resource, walk its states accumulating
				// 'related' (passes of prior states); each pass in the current
				// state depends on all of related. Aggregate across resources.
				std::map<Ref, std::set<Ref>> deps;
				for (const auto& id : resource_order)
				{
					const auto& ts = touches[id];
					std::set<Ref>         related;
					std::vector<Ref>      cur;      // current state's passes
					bool have = false, cur_write = false;

					auto flush = [&]() { for (const auto& r : cur) related.insert(r); cur.clear(); };

					for (const auto& t : ts)
					{
						bool need_new = !have || t.write || cur_write;
						if (need_new) { flush(); have = true; cur_write = t.write; }
						for (const auto& r : related)
							if (r != t.ref) deps[t.ref].insert(r);
						cur.push_back(t.ref);
					}
				}

				for (const auto& [ref, compute] : order)
				{
					ValuesMap m;
					m["pass"] = ref.first;
					m["index"] = (int64_t)ref.second;
					m["compute"] = compute;

					ValuesList prev;
					auto it = deps.find(ref);
					if (it != deps.end())
						for (const auto& r : it->second)   // std::set -> deterministic order
						{
							ValuesMap pr;
							pr["pass"] = r.first;
							pr["index"] = (int64_t)r.second;
							prev.push_back(std::move(pr));
						}
					m["prev"] = std::move(prev);
					result.push_back(std::move(m));
				}
				return result;
			},
			ArgInfo{"pipeline_name"}
		));

		global.GetSettings().extensions.Do = true;


		TemplatesLibrary cpp_templates("cpp");
		TemplatesLibrary hlsl_templates("hlsl");

		// Tables
		for (auto& table : parsed.tables)
		{
			if (table.find_option("nobind"))
				my_stream(hlsl_path + "/tables", table.name + ".h") << hlsl_templates.generate2(
					L"nobind_table", "table", table);
			else
			{
				my_stream(hlsl_path + "/tables", table.name + ".h") << hlsl_templates.generate2(
					L"table", "table", table);

				if (table.slot)
				{
					my_stream(hlsl_path, table.name + ".h") << hlsl_templates.generate2(L"slot", "table", table);
				}
			}

			if (!table.find_option("shader_only"))
			{
				my_stream(cpp_path + "/tables", table.name + ".table.ixx") << cpp_templates.generate2(
					L"table", "table", table);

				if (table.slot)
				{
					my_stream(cpp_path + "/slots", table.name + ".ixx") << cpp_templates.generate2(
						L"slot", "table", table);
				}
			}

			if (table.find_option("RenderTarget"))
			{
				my_stream(hlsl_path + "/rt", table.name + ".h") << hlsl_templates.generate2(L"rt", "rt", table);
				my_stream(cpp_path + "/rt", table.name + ".rt.ixx") << cpp_templates.generate2(L"rt", "rt", table);
				std::filesystem::remove(cpp_path + "/rt/" + table.name + ".h");
				std::filesystem::remove(cpp_path + "/rt/" + table.name + ".ixx");
			}
		}

		// Layout
		for (auto& layout : parsed.layouts)
		{
			my_stream(hlsl_path + "/layout", layout.name + ".h") << hlsl_templates.generate2(
				L"layout", "layout", layout);
			my_stream(cpp_path + "/layout", layout.name + ".layout.ixx") << cpp_templates.generate2(
				L"layout", "layout", layout);
		}

		// PSO
		auto remove_old_pso_h = [&](const std::string& name)
		{
			std::filesystem::remove(cpp_path + "/pso/" + name + ".h");
			std::filesystem::remove(cpp_path + "/pso/" + name + ".ixx");
		};

		for (auto& pso : parsed.compute_pso)
		{
			my_stream(cpp_path + "/pso", pso.name + ".pso.ixx") << cpp_templates.generate2(L"pso", "pso", pso);
			remove_old_pso_h(pso.name);
		}

		for (auto& pso : parsed.graphics_pso)
		{
			my_stream(cpp_path + "/pso", pso.name + ".pso.ixx") << cpp_templates.generate2(L"pso", "pso", pso);
			remove_old_pso_h(pso.name);
		}

		for (auto& pso : parsed.workgraph_pso)
		{
			// No native SimpleWorkgraphPSO (.pso.ixx) generated any more --
			// this engine only ever runs the emulation path (per-node
			// ComputePSOs below); native D3D12 work graphs are being phased
			// out of the API and never built here. workgraph_nodes.jinja
			// still emits both branches of WG_* HLSL macros (toggled by the
			// WORKGRAPH_EMULATION compile define, see workgraph_node_pso.jinja),
			// since dev/workgraph_test.hlsl is shared source either way.
			std::filesystem::remove(cpp_path + "/pso/" + pso.name + ".pso.ixx");
			remove_old_pso_h(pso.name);
			my_stream(hlsl_path + "/workgraph", pso.name + ".h") << hlsl_templates.generate2(L"workgraph_nodes", "pso", pso);

			for (auto& node : pso.nodes)
			{
				auto dp = make_map(pso);
				auto dn = make_map(node);
				ValuesMap params = {
					{"pso", Reflect(dp)},
					{"node", Reflect(dn)},
				};
				my_stream(cpp_path + "/pso", pso.name + "_" + node.name + ".pso.ixx") << cpp_templates.generate(L"workgraph_node_pso", params);
			}
		}

		// RTX
		for (auto& pso : parsed.raytrace_gen)
		{
			auto bind = pso.find_option("Bind");
			parsed.find_rtx(bind->value_atom.expr)->gens.emplace_back(pso);

			my_stream(cpp_path + "/rtx", pso.name + ".h") << cpp_templates.generate2(L"raygen_pass", "pso", pso);
		}

		for (auto& pso : parsed.raytrace_pass)
		{
			auto bind = pso.find_option("Bind");
			parsed.find_rtx(bind->value_atom.expr)->passes.emplace_back(pso);


			my_stream(cpp_path + "/rtx", pso.name + ".h") << cpp_templates.generate2(L"raytrace_pass", "pso", pso);
			my_stream(hlsl_path + "/rtx", pso.name + ".h") << hlsl_templates.generate2(L"pass", "pso", pso);
		}

		for (auto& pso : parsed.raytrace_pso)
		{
			// collect unique payload tables and local slots needed by this PSO's passes
			ValuesList payload_tables, local_slots;
			std::set<std::string> seen_payload, seen_local;
			for (auto& pass : pso.passes)
			{
				auto* payload = pass.find_param("payload");
				if (payload && seen_payload.insert(payload->expr).second)
					payload_tables.emplace_back(payload->expr);

				auto* local = pass.find_param("local");
				if (local && seen_local.insert(local->expr).second)
					local_slots.emplace_back(local->expr);
			}

			auto dp = make_map(pso);
			ValuesMap params = {
				{ "pso",            Reflect(dp)     },
				{ "payload_tables", payload_tables  },
				{ "local_slots",    local_slots     },
			};

			my_stream(cpp_path + "/rtx", pso.name + ".rtx.ixx") << cpp_templates.generate(L"rtx_pso", params);
			std::filesystem::remove(cpp_path + "/rtx/" + pso.name + ".h");
			std::filesystem::remove(cpp_path + "/rtx/" + pso.name + ".ixx");
		}


		for (auto& pass : parsed.passes)
		{
			my_stream(cpp_path_render + "/pass", pass.name + ".h") << cpp_templates.generate2(L"pass", "pass", pass);
		}
		for (auto& view : parsed.views)
		{
			my_stream(cpp_path_render + "/pass", view.name + ".h") << cpp_templates.generate2(L"pass_view", "view", view);
		}
		for (auto& pipeline : parsed.pipelines)
		{
			my_stream(cpp_path_render + "/pass", pipeline.name + ".pipeline.h") << cpp_templates.generate2(L"pipeline", "pipeline", pipeline);
		}

		my_stream(cpp_path_render, "pass_defaults.h") << cpp_templates.generate(L"pass_defaults");
		my_stream(cpp_path_render, "pass_defaults.cpp") << cpp_templates.generate(L"pass_defaults_cpp");
		my_stream(cpp_path_render, "resource_ids.h") << cpp_templates.generate(L"resource_ids");
		my_stream(cpp_path_render, "pass_ids.h") << cpp_templates.generate(L"pass_ids");
		my_stream(cpp_path_render, "context_deps.h") << cpp_templates.generate(L"context_deps");
		my_stream(cpp_path_render, "context_snapshot.cpp") << cpp_templates.generate(L"context_snapshot_cpp");

		my_stream(cpp_path, "Constants.ixx") << cpp_templates.generate(L"constants");

		// includes
		my_stream(cpp_path, "autogen.ixx") << cpp_templates.generate(L"autogen");
		my_stream(cpp_path, "autogen.cpp") << cpp_templates.generate(L"autogen_impl");
		my_stream(cpp_path, "enums.ixx") << cpp_templates.generate(L"enums");
		// Lives at the shader root (next to sig_hlsl.hlsl), not under autogen/,
		// and is included by bare filename everywhere -- see the comment on
		// those #include lines for why.
		my_stream(shaders_path, "enums.h") << hlsl_templates.generate(L"enums");
		my_stream(cpp_path, "pso.cpp") << cpp_templates.generate(L"psos");

		my_stream(cpp_path_render, "enums.h") << cpp_templates.generate(L"pass_enums");
		my_stream(cpp_path_render, "passes.ixx") << cpp_templates.generate(L"passes");
	}
	catch (std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}
