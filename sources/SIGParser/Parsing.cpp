import Core;
import <Core_defs.h>;
import antlr4;
#undef EOF
#include ".antlr/SIGLexer.h"
#include ".antlr/SIGParser.h"
#include ".antlr/SIGBaseListener.h"
#include ".antlr/SIGBaseVisitor.h"

using namespace antlr4;
#include "Parsing.h"

class TreeShapeListener : public SIGBaseListener
{
public:
	Parsed& parsed;

	TreeShapeListener(Parsed& parsed) : parsed(parsed)
	{
		setup_elem(parsed);
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
		parsed_type* e = elems.back().elem;
		return !!dynamic_cast<T*>(e);
	}
	template <class T>
	T& get_elem()
	{
		parsed_type* e = elems.back().elem;
		return *dynamic_cast<T*>(e);
	}

	template <class T>
	T& get_parent()
	{
		auto last = std::prev(elems.end());
		auto prelast = std::prev(last);

		parsed_type* e = (*prelast).elem;
		return *dynamic_cast<T*>(e);
	}

#define GENERATE(x) \
	virtual void exit##x##(SIGParser::##x##Context * ctx) override { \
		end_elem();\
	}\
	virtual void enter##x##(SIGParser::##x##Context* ctx) override


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
		assert(!owner);
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


	GENERATE(Rtx_pass_definition)
	{
		auto index = get_elem<Parsed>().raytrace_pass.size();
		auto& rtx = setup_map(get_elem<Parsed>().raytrace_pass);

		rtx.index = index;
	}

	GENERATE(Rtx_raygen_definition)
	{
		auto index = get_elem<Parsed>().raytrace_gen.size();
		auto& rtx = setup_map(get_elem<Parsed>().raytrace_gen);

		rtx.index = index;
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
		auto& elem = get_elem<have_type>();
		auto& options = get_elem<have_options>();

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
		auto& options = get_elem<have_options>();

		elem.class_no_template = ctx->children[0]->getText();
		elem.detect_type(&options);
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


		auto& options = get_elem<have_options>();
		elem.detect_type(&options);
	}

	void enterValue_id(SIGParser::Value_idContext* ctx) override
	{
		auto& elem = get_elem<have_expr>();
		elem.expr = ctx->getText();
	}

	void enterPso_param_id(SIGParser::Pso_param_idContext* ctx) override
	{
		auto& elem = get_elem<have_type>();
		auto& options = get_elem<have_options>();

		elem.class_no_template = ctx->children[0]->getText();
		elem.detect_type(&options);
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


			auto& options = get_elem<have_options>();

			elem.detect_type(&options);
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
			auto& options = get_elem<have_options>();
			elem.detect_type(&options);
		}
	}


	void enterInsert_block(SIGParser::Insert_blockContext* ctx) override
	{
		auto str = ctx->children[0]->getText();
		auto& elem = get_elem<have_hlsl>();
		elem.hlsl = str.substr(2, str.size() - 4);
	}


	void enterShader_type(SIGParser::Shader_typeContext* ctx) override
	{
		auto str = ctx->children[0]->getText();
		if(check<Shader>())
		{
						 auto& shader = get_elem<Shader>();
		auto& pso = get_parent<PSO>();

		shader.name = str;
		}
		
	}
};

Parsed parse(std::wstring filename)
{
	std::wcout << ((filename + L"\n")) << std::endl;
	Parsed parsed;
	{
		std::ifstream stream;
		stream.open(filename);

		if (!stream.is_open())
			return parsed;

		ANTLRInputStream input(stream);
		SIGLexer lexer(&input);
		CommonTokenStream tokens(&lexer);
		SIGParser parser(&tokens);
		SIGParser::ParseContext* tree = parser.parse();
		TreeShapeListener listener(parsed);
		antlr4::tree::ParseTreeWalker walker;

		walker.walk(&listener, tree);

		stream.close();
	}
	return parsed;
}
