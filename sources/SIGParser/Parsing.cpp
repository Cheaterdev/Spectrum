import Core;
import antlr4;
#undef EOF
#include ".antlr/SIGLexer.h"
#include ".antlr/SIGParser.h"
#include ".antlr/SIGBaseListener.h"
#include ".antlr/SIGBaseVisitor.h"

using namespace antlr4;
#include "Parsing.h"
#include "Diagnostics.h"

// ANTLR's default listener prints to stderr and then recovers, so a malformed
// .sig would otherwise yield a half-built model that generates plausible output.
class CollectingErrorListener : public BaseErrorListener
{
	std::string file;

public:
	explicit CollectingErrorListener(std::string file) : file(std::move(file)) {}

	void syntaxError(Recognizer*, Token*, size_t line, size_t column, const std::string& msg, std::exception_ptr) override
	{
		diagnostics().error(SourceLocation{ file, line, column + 1 }, msg);
	}
};

class TreeShapeListener : public SIGBaseListener
{
public:
	Parsed& parsed;
	std::string file;

	TreeShapeListener(Parsed& parsed, std::string file) : parsed(parsed), file(std::move(file))
	{
		parsed.loc.file = this->file;
		setup_elem(parsed);
	}

	void stamp(antlr4::ParserRuleContext* ctx)
	{
		auto* start = ctx->getStart();
		elems.back().elem->loc = SourceLocation{ file, start->getLine(), start->getCharPositionInLine() + 1 };
	}

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

	template <class T>
	void setup_elem(T& e, std::function<void()> f = nullptr)
	{
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

#define GENERATE(x) \
	virtual void exit##x##(SIGParser::##x##Context * ctx) override { \
		end_elem();\
	}\
	virtual void enter##x##(SIGParser::##x##Context* ctx) override { \
		enter_body_##x(ctx);\
		stamp(ctx);\
	}\
	void enter_body_##x(SIGParser::##x##Context* ctx)


#define EXIT(x) \
	virtual void exit##x##(SIGParser::##x##Context * ctx) override

#define ENTER(x) \
	virtual void enter##x##(SIGParser::##x##Context* ctx) override


	GENERATE(Layout_definition)
	{
		setup_map(get_elem<Parsed>().layouts);
	}

	GENERATE(Table_definition)
	{
		setup_map(get_elem<Parsed>().tables);
	}

	GENERATE(Const_definition)
	{
		setup_map(get_elem<Parsed>().consts);
	}

	GENERATE(Slot_declaration)
	{
		setup_map(get_elem<Layout>().slots);
	}

	GENERATE(Value_declaration)
	{
		setup_list(get_elem<Table>().values);
	}

	GENERATE(Option)
	{
		setup_map(get_elem<have_options>().options);
	}

	GENERATE(Sampler_declaration)
	{
		setup_map(get_elem<Layout>().samplers);
	}

	GENERATE(Rt_definition)
	{
		setup_map(get_elem<Parsed>().rt);
	}

	GENERATE(Rt_color_declaration)
	{
		setup_list(get_elem<RenderTarget>().rtvs);
	}

	GENERATE(Rt_ds_declaration)
	{
		auto& owner = get_elem<RenderTarget>().dsv;
		ASSERT(!owner);
		owner = DSV();

		setup_elem(*owner);
	}


	GENERATE(Compute_pso_definition)
	{
		setup_map(get_elem<Parsed>().compute_pso);
	}

	GENERATE(Graphics_pso_definition)
	{
		setup_map(get_elem<Parsed>().graphics_pso);
	}

	GENERATE(Rtx_pso_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_pso);
	}

	GENERATE(Workgraph_pso_definition)
	{
		setup_map(get_elem<Parsed>().workgraph_pso);
	}

	GENERATE(Node_definition)
	{
		setup_map(get_elem<WorkgraphPSO>().nodes);
	}

	ENTER(Node_output_decl)
	{
		setup_list(get_elem<WorkgraphNode>().outputs);
		stamp(ctx);
	}

	EXIT(Node_output_decl)
	{
		auto& output = get_elem<WorkgraphNodeOutput>();
		if (auto* opt = output.find_option("MaxRecords"))
			output.max_records = std::atoi(opt->value_atom.expr.c_str());
		end_elem();
	}

	GENERATE(Node_param)
	{
		setup_map(get_elem<param_holder>().params);
	}


	// index is assigned after all files are merged (assign_rtx_ids in Main.cpp).
	GENERATE(Rtx_pass_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_pass);
	}

	GENERATE(Rtx_raygen_definition)
	{
		setup_map(get_elem<Parsed>().raytrace_gen);
	}


	GENERATE(Root_sig)
	{
		setup_elem(get_elem<root_holder>().root_sig);
	}


	GENERATE(Shader)
	{
		setup_map(get_elem<shader_holder>().shaders);
	}


	GENERATE(Options_assign)
	{
		auto& owner = get_elem<option>();
		setup_elem(owner.value_atom);
	}


	GENERATE(Define_declaration)
	{
		setup_map(get_elem<PSO>().defines);
	}


	GENERATE(Rtv_formats_declaration)
	{
		auto& owner = get_elem<GraphicsPSO>();
		setup_elem(owner.rtv);
	}

	GENERATE(Blends_declaration)
	{
		auto& owner = get_elem<GraphicsPSO>();
		setup_elem(owner.blend);
	}


	GENERATE(Pso_param)
	{
		setup_map(get_elem<param_holder>().params);
	}


	GENERATE(Array_value_holder)
	{
		setup_list(get_elem<have_values>().values);
	}

	GENERATE(Flag_value_holder)
	{
		setup_list(get_elem<have_values>().values);
	}

	GENERATE(View_definition)
	{
		setup_map(get_elem<Parsed>().views);
	}
	GENERATE(Pass_definition)
	{
		setup_map(get_elem<Parsed>().passes);
	}
	GENERATE(View_declaration)
	{
		setup_list(get_elem<View>().params);
	}

	GENERATE(Pipeline_definition)
	{
		setup_map(get_elem<Parsed>().pipelines);
	}
	GENERATE(Pipeline_stat)
	{
		setup_map(get_elem<Pipeline>().entries);
	}

	GENERATE(Enum_definition)
	{
		setup_map(get_elem<Parsed>().enums);
	}

	GENERATE(Enum_value_declaration)
	{
		setup_list(get_elem<Enum>().values);
	}

	void enterName_id(SIGParser::Name_idContext* ctx) override
	{
		auto& elem = get_elem<have_name>();
		elem.name = ctx->children[0]->getText();
	}

	void enterPath_id(SIGParser::Path_idContext* ctx) override
	{
		auto& elem = get_elem<Shader>();

		for (auto c : ctx->children)
			elem.path += c->getText();
	}

	void enterInherit_id(SIGParser::Inherit_idContext* ctx) override
	{
		auto& elem = get_elem<inherited>();
		elem.parent.emplace_back(ctx->children[0]->getText());
	}

	void enterType_id(SIGParser::Type_idContext* ctx) override
	{
		//elem.type = ctx->children[0]->getText();
		//	elem.detect_type(&options);
	}

	//	virtual void enterModifier(SIGParser::ModifierContext* ctx) override {
	//	auto& elem = get_elem<have_type>();
	//	auto& options = get_elem<have_options>();

	//	elem.unorm = ctx->children[0]->getText()=="unorm";
	////	elem.detect_type(&options);
	//}

	void enterClass_no_template(SIGParser::Class_no_templateContext* ctx) override
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = ctx->children[0]->getText();
		elem.detect_type(find_elem<have_options>());
	}

	void enterOwner_id(SIGParser::Owner_idContext* ctx) override
	{
		auto& elem = get_elem<have_owner>();
		elem.owner_name = ctx->children[0]->getText();
	}

	void enterTemplate_id(SIGParser::Template_idContext* ctx) override
	{
		auto& elem = get_elem<have_type>();

		if (!elem.template_arg.empty())
			elem.template_arg += ' ';
		elem.template_arg += ctx->children[0]->getText();

		elem.detect_type(find_elem<have_options>());
	}

	void enterValue_id(SIGParser::Value_idContext* ctx) override
	{
		auto& elem = get_elem<have_expr>();
		elem.expr = ctx->getText();
		elem.is_literal = ctx->INT_SCALAR() || ctx->FLOAT_SCALAR() || ctx->bool_type();
	}

	void enterRaw_value(SIGParser::Raw_valueContext* ctx) override
	{
		auto& elem = get_elem<have_expr>();
		std::string text = ctx->getText();
		elem.expr = text.substr(1, text.size() - 2); // strip the enclosing backticks
		elem.is_raw = true;
	}

	// Records each cond_term in source order. Deliberately does NOT touch
	// expr/owner_name/is_literal: enterValue_id and enterOwner_id still fire for
	// the leaves inside these terms, so a single-term option value keeps
	// producing exactly the fields it always did. Codegen renders from terms
	// only when there is more than one of them, which is what makes this change
	// incapable of altering any existing option's generated output.
	void enterCond_term(SIGParser::Cond_termContext* ctx) override
	{
		auto& elem = get_elem<have_expr>();
		auto& term = elem.terms.emplace_back();

		if (auto* q = ctx->qualified_ref())
		{
			term.kind  = ExprTerm::Qualified;
			term.owner = q->owner_id()->getText();
			term.text  = q->value_id()->getText();
		}
		else if (auto* m = ctx->member_ref())
		{
			term.kind  = ExprTerm::Member;
			term.owner = m->name_id(0)->getText();
			term.text  = m->name_id(1)->getText();
		}
		else if (ctx->function_id())
		{
			term.kind = ExprTerm::Function;
			term.text = ctx->getText();
		}
		else if (ctx->cond_op())
		{
			term.kind = ExprTerm::Op;
			term.text = ctx->getText();
		}
		else
		{
			term.kind = ExprTerm::Plain;
			term.text = ctx->getText();
		}
	}

	void enterPso_param_id(SIGParser::Pso_param_idContext* ctx) override
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = ctx->children[0]->getText();
		elem.detect_type(find_elem<have_options>());
	}

	void enterNode_param_id(SIGParser::Node_param_idContext* ctx) override
	{
		auto& elem = get_elem<have_type>();

		elem.class_no_template = ctx->children[0]->getText();
		elem.detect_type(find_elem<have_options>());
	}

	void enterArray(SIGParser::ArrayContext* ctx) override
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

	void enterPointer(SIGParser::PointerContext* ctx) override
	{
		{
			auto& elem = get_elem<have_type>();
			elem.pointer = true;

			elem.detect_type(find_elem<have_options>());
		}
	}

	void enterArray_count_id(SIGParser::Array_count_idContext* ctx) override
	{
		{
			auto& elem = get_elem<have_array>();
			elem.array_count = std::atoi(ctx->children[0]->getText().c_str());
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


	void enterInsert_block(SIGParser::Insert_blockContext* ctx) override
	{
		auto str = ctx->children[0]->getText();
		auto& elem = get_elem<have_hlsl>();
		elem.hlsl = str.substr(2, str.size() - 4);
		elem.hlsl_loc = SourceLocation{ file, ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine() + 1 };
	}


	void enterShader_type(SIGParser::Shader_typeContext* ctx) override
	{
		if (auto* shader = find_elem<Shader>())
			shader->name = ctx->children[0]->getText();
	}
};

Parsed parse(std::wstring filename)
{
	std::wcout << ((filename + L"\n")) << std::endl;
	Parsed parsed;
	std::string file = std::filesystem::absolute(filename).string();
	{
		std::ifstream stream;
		stream.open(filename);

		if (!stream.is_open())
		{
			diagnostics().error(SourceLocation{ file }, "cannot open file");
			return parsed;
		}

		CollectingErrorListener errors(file);

		ANTLRInputStream input(stream);
		SIGLexer lexer(&input);
		lexer.removeErrorListeners();
		lexer.addErrorListener(&errors);

		CommonTokenStream tokens(&lexer);
		SIGParser parser(&tokens);
		parser.removeErrorListeners();
		parser.addErrorListener(&errors);

		SIGParser::ParseContext* tree = parser.parse();

		// Walking an error-recovered tree only builds a misleading partial model
		// for the validator to complain about; the syntax errors are the report.
		if (lexer.getNumberOfSyntaxErrors() == 0 && parser.getNumberOfSyntaxErrors() == 0)
		{
			TreeShapeListener listener(parsed, file);
			antlr4::tree::ParseTreeWalker walker;

			walker.walk(&listener, tree);
		}

		stream.close();
	}
	return parsed;
}
