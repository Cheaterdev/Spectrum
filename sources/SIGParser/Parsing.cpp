
#include ".antlr/antlr4-runtime.h"
#include ".antlr/SIGLexer.h"
#include ".antlr/SIGParser.h"
#include ".antlr/SIGBaseListener.h"
#include ".antlr/SIGBaseVisitor.h"

using namespace antlr4;
#include "Parsing.h"

class TreeShapeListener : public SIGBaseListener {
public:

	Parsed& parsed;
	TreeShapeListener(Parsed& parsed) :parsed(parsed)
	{
		elems.push_back(&parsed);
	}

	std::list<parsed_type*> elems;

	template<class T>
	void setup_elem(T& e)
	{
		elems.push_back(&e);
	}

	void end_elem()
	{
		elems.pop_back();
	}

	template<class T>
	T& get_elem()
	{
		parsed_type* e = elems.back();
		return *dynamic_cast<T*>(e);
	}

	template<class T>
	T& get_parent()
	{
		auto last = std::prev(elems.end());
		auto prelast = std::prev(last);

		parsed_type* e = *prelast;
		return *dynamic_cast<T*>(e);
	}

#define GENERATE(x) \
	virtual void exit##x##(SIGParser::##x##Context * ctx) override { \
		end_elem();\
	}\
	virtual void enter##x##(SIGParser::##x##Context* ctx) override 

	GENERATE(Layout_definition)
	{
		auto& owner = get_elem<Parsed>().layouts;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Table_definition) {
		auto& owner = get_elem<Parsed>().tables;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Slot_declaration)
	{
		auto& owner = get_elem<Layout>().slots;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Value_declaration)
	{

		auto& owner = get_elem<Table>().values;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Option)
	{
		auto& owner = get_elem<have_options>().options;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Options_assign)
	{
		auto& owner = get_elem<option>();
		setup_elem(owner.value_atom);
	}

	GENERATE(Sampler_declaration)
	{
		auto& owner = get_elem<Layout>().samplers;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Rt_definition)
	{
		auto& owner = get_elem<Parsed>().rt;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Rt_color_declaration)
	{
		auto& owner = get_elem<RenderTarget>().rtvs;
		owner.emplace_back();
		setup_elem(owner.back());
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
		auto& owner = get_elem<Parsed>().compute_pso;
		owner.emplace_back();
		setup_elem(owner.back());

	}

	GENERATE(Graphics_pso_definition)
	{
		auto& owner = get_elem<Parsed>().graphics_pso;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Rtx_pso_definition)
	{
		auto& owner = get_elem<Parsed>().raytrace_pso;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Rtx_pass_definition)
	{
		auto& owner = get_elem<Parsed>().raytrace_pass;
		owner.emplace_back();

		owner.back().index = owner.size() - 1;
		setup_elem(owner.back());
	}

	GENERATE(Rtx_raygen_definition)
	{
		auto& owner = get_elem<Parsed>().raytrace_gen;
		owner.emplace_back();
		owner.back().index = owner.size() - 1;
		setup_elem(owner.back());
	}

	GENERATE(Root_sig)
	{
		setup_elem(get_elem<root_holder>().root_sig);
	}

	GENERATE(Shader)
	{
		auto& owner = get_elem<shader_holder>().shader_list;
		owner.emplace_back();
		setup_elem(owner.back());
	}


	GENERATE(Define_declaration)
	{
		auto& owner = get_elem<PSO>().defines;
		owner.emplace_back();
		setup_elem(owner.back());
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
		auto& owner = get_elem<param_holder>().params;
		owner.emplace_back();
		setup_elem(owner.back());
	}

	GENERATE(Array_value_holder)
	{
		auto& owner = get_elem<have_values>().values;
		owner.emplace_back();
		setup_elem(owner.back());
	}


	virtual void enterName_id(SIGParser::Name_idContext* ctx) override {

		auto& elem = get_elem<have_name>();
		elem.name = ctx->children[0]->getText();
	}
	virtual void enterPath_id(SIGParser::Path_idContext* ctx) override {
		auto& elem = get_elem<have_name>();

		for(auto c: ctx->children)
			elem.name += c->getText();
	}

	virtual void enterInherit_id(SIGParser::Inherit_idContext* ctx) override {
		auto& elem = get_elem<inherited>();
		elem.parent.emplace_back(ctx->children[0]->getText());
	}

	virtual void enterType_id(SIGParser::Type_idContext* ctx) override {
		auto& elem = get_elem<have_type>();
		auto& options = get_elem<have_options>();

		elem.type = ctx->children[0]->getText();
		elem.detect_type(&options);
	}

	virtual void enterOwner_id(SIGParser::Owner_idContext* ctx) override {
		auto& elem = get_elem<have_owner>();
		elem.owner_name = ctx->children[0]->getText();
	}

	virtual void enterTemplate_id(SIGParser::Template_idContext* ctx) override {
		auto& elem = get_elem<have_type>();
		elem.template_arg = ctx->children[0]->getText();
	}

	virtual void enterValue_id(SIGParser::Value_idContext* ctx) override {
		auto& elem = get_elem<have_expr>();
		elem.expr = ctx->children[0]->getText();
	}

	virtual void enterPso_param_id(SIGParser::Pso_param_idContext* ctx) override {
		auto& elem = get_elem<have_type>();
		elem.type = ctx->children[0]->getText();
	}

	virtual void enterArray(SIGParser::ArrayContext* ctx) override {
		
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


	virtual void enterArray_count_id(SIGParser::Array_count_idContext* ctx) override {
	

		{
			auto& elem = get_elem<have_array>();
			elem.array_count = std::atoi(ctx->children[0]->getText().c_str());
		}
		{
			auto& elem = get_elem<have_type>();
			elem.bindless = false;
		}
	
	}

	
	virtual void enterInsert_block(SIGParser::Insert_blockContext* ctx) override {

		auto str = ctx->children[0]->getText();
		auto& elem = get_elem<have_hlsl>();
		elem.hlsl = str.substr(2, str.size() - 4);
	}
	


	virtual void enterShader_type(SIGParser::Shader_typeContext* ctx) override {

		auto str = ctx->children[0]->getText();
		auto& shader = get_elem<Shader>();
		auto& pso = get_parent<PSO>();

		shader.type = str;
		pso.shaders[str] = &shader;
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
		tree::ParseTreeWalker walker;

		walker.walk(&listener, tree);

		stream.close();
	}
	return parsed;
}
