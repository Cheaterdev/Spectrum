
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

	}
	inherited* inherited_ptr = nullptr;
	have_name* name_ptr;
	have_options* options_ptr;
	have_type* type_ptr;
	have_expr* expr_ptr;
	have_owner* owner_ptr;
	have_array* array_ptr;
	Layout* layout_ptr;
	Table* table_ptr;
	Slot* slot_ptr;
	option* option_ptr;
	have_hlsl* hlsl_ptr;
	have_values* value_array;


	PSO* current_pso;
	Shader* shader;

	std::stack<parsed_type*> elems;


	template<class T>
	void setup_elem(T& e)
	{
		elems.push(&e);
		propagate_elem();

		{
			parsed_type* e = elems.top();
			auto shader = dynamic_cast<Shader*>(e);
			auto current_pso = dynamic_cast<PSO*>(e);

			if (current_pso) this->current_pso = current_pso;
			if (shader) this->shader = shader;
		}

	}

	void end_elem()
	{
		{parsed_type* e = elems.top();
		auto shader = dynamic_cast<Shader*>(e);
		auto current_pso = dynamic_cast<PSO*>(e);
		if (current_pso == this->current_pso) this->current_pso = nullptr;
		if (shader == this->shader) this->shader = nullptr;
	}
		elems.pop();
		propagate_elem();
	}

	void propagate_elem()
	{
		if (elems.empty()) return;

		parsed_type* e = elems.top();
		inherited_ptr = dynamic_cast<inherited*>(e);
		name_ptr = dynamic_cast<have_name*>(e);
		options_ptr = dynamic_cast<have_options*>(e);
		layout_ptr = dynamic_cast<Layout*>(e);
		table_ptr = dynamic_cast<Table*>(e);
		slot_ptr = dynamic_cast<Slot*>(e);
		type_ptr = dynamic_cast<have_type*>(e);
		expr_ptr = dynamic_cast<have_expr*>(e);
		option_ptr = dynamic_cast<option*>(e);
		owner_ptr = dynamic_cast<have_owner*>(e);
		array_ptr = dynamic_cast<have_array*>(e);
		hlsl_ptr = dynamic_cast<have_hlsl*>(e);
		value_array = dynamic_cast<have_values*>(e);


	}
#define GENERATE(x) \
	virtual void exit##x##(SIGParser::##x##Context * ctx) override { \
		end_elem();\
	}\
	virtual void enter##x##(SIGParser::##x##Context* ctx) override 

	GENERATE(Layout_definition)
	{
		parsed.layouts.emplace_back();
		setup_elem(parsed.layouts.back());
	}

	GENERATE(Table_definition) {
		parsed.tables.emplace_back();
		setup_elem(parsed.tables.back());
	}

	GENERATE(Slot_declaration)
	{
		auto& layout = parsed.layouts.back();
		layout.slots.emplace_back();
		setup_elem(layout.slots.back());
	}



	GENERATE(Value_declaration)
	{
		auto& table = parsed.tables.back();
		table.values.emplace_back();
		setup_elem(table.values.back());
	}

	GENERATE(Option)
	{
		options_ptr->options.emplace_back();
		setup_elem(options_ptr->options.back());
	}

	GENERATE(Options_assign)
	{
		setup_elem(option_ptr->value_atom);
	}

	GENERATE(Sampler_declaration)
	{
		auto& layout = parsed.layouts.back();
		layout.samplers.emplace_back();
		setup_elem(layout.samplers.back());
	}

	GENERATE(Rt_definition)
	{
		parsed.rt.emplace_back();
		setup_elem(parsed.rt.back());
	}

	GENERATE(Rt_color_declaration)
	{
		auto& rt = parsed.rt.back();

		rt.rtvs.emplace_back();
		setup_elem(rt.rtvs.back());
	}

	GENERATE(Rt_ds_declaration)
	{
		auto& rt = parsed.rt.back();

		assert(!rt.dsv);
		rt.dsv = DSV();

		setup_elem(*rt.dsv);

	}



	GENERATE(Compute_pso_definition)
	{
		parsed.compute_pso.emplace_back();
		setup_elem(parsed.compute_pso.back());
	}

	GENERATE(Graphics_pso_definition)
	{
		parsed.graphics_pso.emplace_back();
		setup_elem(parsed.graphics_pso.back());
	}

	GENERATE(Root_sig)
	{
		
		setup_elem(current_pso->root_sig);
	}


	GENERATE(Shader)
	{
		

		current_pso->shader_list.emplace_back();
		setup_elem(current_pso->shader_list.back());
	}


	GENERATE(Define_declaration)
	{
		current_pso->defines.emplace_back();
		setup_elem(current_pso->defines.back());
	}

	GENERATE(Rtv_formats_declaration)
	{
		auto& pso = parsed.graphics_pso.back();
		//pso.rtvs.emplace_back();
		setup_elem(pso.rtv);
	}

	GENERATE(Blends_declaration)
	{
		auto& pso = parsed.graphics_pso.back();
		//pso.rtvs.emplace_back();
		setup_elem(pso.blend);
	}
GENERATE(Pso_param)
	{
		auto& pso = parsed.graphics_pso.back();
		pso.params.emplace_back();
		setup_elem(pso.params.back());
	}

	/*
	GENERATE(Array_value_ids)
	{
		auto& def = have_values.back();	
		setup_elem(def.value);
	}

	*/
	GENERATE(Array_value_holder)
	{
		value_array->values.emplace_back();

		setup_elem(value_array->values.back());
	}

	virtual void enterName_id(SIGParser::Name_idContext* ctx) override {
		name_ptr->name = ctx->children[0]->getText();

		name_ptr->debug = name_ptr->name == "PassData";
	}
	virtual void enterPath_id(SIGParser::Path_idContext* ctx) override {
		for(auto c: ctx->children)
		name_ptr->name += c->getText();
	}

	virtual void enterInherit_id(SIGParser::Inherit_idContext* ctx) override {
		inherited_ptr->parent.emplace_back(ctx->children[0]->getText());
	}

	virtual void enterType_id(SIGParser::Type_idContext* ctx) override {
		type_ptr->type = ctx->children[0]->getText();
		type_ptr->detect_type(options_ptr);
	}

	virtual void enterOwner_id(SIGParser::Owner_idContext* ctx) override {
		owner_ptr->owner_name = ctx->children[0]->getText();
	}

	virtual void enterTemplate_id(SIGParser::Template_idContext* ctx) override {
		type_ptr->template_arg = ctx->children[0]->getText();
	}

	virtual void enterValue_id(SIGParser::Value_idContext* ctx) override {
		expr_ptr->expr = ctx->children[0]->getText();
	}

	virtual void enterPso_param_id(SIGParser::Pso_param_idContext* ctx) override {
		type_ptr->type = ctx->children[0]->getText();
	}

	virtual void enterArray(SIGParser::ArrayContext* ctx) override {
		array_ptr->as_array = true;
		array_ptr->array_count = 0;
		type_ptr->bindless = true;
	}


	virtual void enterArray_count_id(SIGParser::Array_count_idContext* ctx) override {
	
		array_ptr->array_count =std::atoi( ctx->children[0]->getText().c_str());
		type_ptr->bindless = false;
	}

	
	virtual void enterInsert_block(SIGParser::Insert_blockContext* ctx) override {

		auto str = ctx->children[0]->getText();
		hlsl_ptr->hlsl = str.substr(2, str.size() - 4);
	}
	


	virtual void enterShader_type(SIGParser::Shader_typeContext* ctx) override {

		auto str = ctx->children[0]->getText();
		shader->type = str;
		current_pso->shaders[str] = shader;
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
		tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

		stream.close();
	}
	return parsed;
}

