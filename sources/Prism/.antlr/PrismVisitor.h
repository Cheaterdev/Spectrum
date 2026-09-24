
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PrismParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by PrismParser.
 */
class  PrismVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by PrismParser.
   */
    virtual std::any visitParse(PrismParser::ParseContext *context) = 0;

    virtual std::any visitDefinition(PrismParser::DefinitionContext *context) = 0;

    virtual std::any visitNamespace_definition(PrismParser::Namespace_definitionContext *context) = 0;

    virtual std::any visitNamespace_header(PrismParser::Namespace_headerContext *context) = 0;

    virtual std::any visitConst_definition(PrismParser::Const_definitionContext *context) = 0;

    virtual std::any visitBind_option(PrismParser::Bind_optionContext *context) = 0;

    virtual std::any visitCond_expr(PrismParser::Cond_exprContext *context) = 0;

    virtual std::any visitCond_term(PrismParser::Cond_termContext *context) = 0;

    virtual std::any visitCall(PrismParser::CallContext *context) = 0;

    virtual std::any visitCall_arg(PrismParser::Call_argContext *context) = 0;

    virtual std::any visitQualified_ref(PrismParser::Qualified_refContext *context) = 0;

    virtual std::any visitMember_ref(PrismParser::Member_refContext *context) = 0;

    virtual std::any visitCond_op(PrismParser::Cond_opContext *context) = 0;

    virtual std::any visitFlag_value_holder(PrismParser::Flag_value_holderContext *context) = 0;

    virtual std::any visitRaw_value(PrismParser::Raw_valueContext *context) = 0;

    virtual std::any visitOptions_assign(PrismParser::Options_assignContext *context) = 0;

    virtual std::any visitOption(PrismParser::OptionContext *context) = 0;

    virtual std::any visitOption_block(PrismParser::Option_blockContext *context) = 0;

    virtual std::any visitArray_count_id(PrismParser::Array_count_idContext *context) = 0;

    virtual std::any visitArray(PrismParser::ArrayContext *context) = 0;

    virtual std::any visitValue_declaration(PrismParser::Value_declarationContext *context) = 0;

    virtual std::any visitSlot_declaration(PrismParser::Slot_declarationContext *context) = 0;

    virtual std::any visitSampler_declaration(PrismParser::Sampler_declarationContext *context) = 0;

    virtual std::any visitDefine_declaration(PrismParser::Define_declarationContext *context) = 0;

    virtual std::any visitRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext *context) = 0;

    virtual std::any visitBlends_declaration(PrismParser::Blends_declarationContext *context) = 0;

    virtual std::any visitPointer(PrismParser::PointerContext *context) = 0;

    virtual std::any visitPso_param(PrismParser::Pso_paramContext *context) = 0;

    virtual std::any visitClass_no_template(PrismParser::Class_no_templateContext *context) = 0;

    virtual std::any visitType_with_template(PrismParser::Type_with_templateContext *context) = 0;

    virtual std::any visitInherit_id(PrismParser::Inherit_idContext *context) = 0;

    virtual std::any visitName_id(PrismParser::Name_idContext *context) = 0;

    virtual std::any visitOption_id(PrismParser::Option_idContext *context) = 0;

    virtual std::any visitOwner_id(PrismParser::Owner_idContext *context) = 0;

    virtual std::any visitTemplate_id(PrismParser::Template_idContext *context) = 0;

    virtual std::any visitFunction_id(PrismParser::Function_idContext *context) = 0;

    virtual std::any visitValue_id(PrismParser::Value_idContext *context) = 0;

    virtual std::any visitValue_id_ignore(PrismParser::Value_id_ignoreContext *context) = 0;

    virtual std::any visitType_id(PrismParser::Type_idContext *context) = 0;

    virtual std::any visitInsert_block(PrismParser::Insert_blockContext *context) = 0;

    virtual std::any visitShader_path(PrismParser::Shader_pathContext *context) = 0;

    virtual std::any visitInherit(PrismParser::InheritContext *context) = 0;

    virtual std::any visitLayout_stat(PrismParser::Layout_statContext *context) = 0;

    virtual std::any visitLayout_block(PrismParser::Layout_blockContext *context) = 0;

    virtual std::any visitLayout_definition(PrismParser::Layout_definitionContext *context) = 0;

    virtual std::any visitTable_stat(PrismParser::Table_statContext *context) = 0;

    virtual std::any visitFunction_definition(PrismParser::Function_definitionContext *context) = 0;

    virtual std::any visitFunction_params(PrismParser::Function_paramsContext *context) = 0;

    virtual std::any visitFunction_semantic(PrismParser::Function_semanticContext *context) = 0;

    virtual std::any visitTable_block(PrismParser::Table_blockContext *context) = 0;

    virtual std::any visitTable_definition(PrismParser::Table_definitionContext *context) = 0;

    virtual std::any visitRt_color_declaration(PrismParser::Rt_color_declarationContext *context) = 0;

    virtual std::any visitRt_ds_declaration(PrismParser::Rt_ds_declarationContext *context) = 0;

    virtual std::any visitRt_stat(PrismParser::Rt_statContext *context) = 0;

    virtual std::any visitRt_block(PrismParser::Rt_blockContext *context) = 0;

    virtual std::any visitRt_definition(PrismParser::Rt_definitionContext *context) = 0;

    virtual std::any visitArray_value_holder(PrismParser::Array_value_holderContext *context) = 0;

    virtual std::any visitArray_value_ids(PrismParser::Array_value_idsContext *context) = 0;

    virtual std::any visitRoot_sig(PrismParser::Root_sigContext *context) = 0;

    virtual std::any visitShader(PrismParser::ShaderContext *context) = 0;

    virtual std::any visitCompute_pso_stat(PrismParser::Compute_pso_statContext *context) = 0;

    virtual std::any visitCompute_pso_block(PrismParser::Compute_pso_blockContext *context) = 0;

    virtual std::any visitCompute_pso_definition(PrismParser::Compute_pso_definitionContext *context) = 0;

    virtual std::any visitGraphics_pso_stat(PrismParser::Graphics_pso_statContext *context) = 0;

    virtual std::any visitGraphics_pso_block(PrismParser::Graphics_pso_blockContext *context) = 0;

    virtual std::any visitGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext *context) = 0;

    virtual std::any visitRtx_pso_stat(PrismParser::Rtx_pso_statContext *context) = 0;

    virtual std::any visitRtx_pso_block(PrismParser::Rtx_pso_blockContext *context) = 0;

    virtual std::any visitRtx_pso_definition(PrismParser::Rtx_pso_definitionContext *context) = 0;

    virtual std::any visitNode_param_id(PrismParser::Node_param_idContext *context) = 0;

    virtual std::any visitNode_param(PrismParser::Node_paramContext *context) = 0;

    virtual std::any visitNode_output_decl(PrismParser::Node_output_declContext *context) = 0;

    virtual std::any visitNode_stat(PrismParser::Node_statContext *context) = 0;

    virtual std::any visitNode_block(PrismParser::Node_blockContext *context) = 0;

    virtual std::any visitNode_definition(PrismParser::Node_definitionContext *context) = 0;

    virtual std::any visitWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext *context) = 0;

    virtual std::any visitWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext *context) = 0;

    virtual std::any visitWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext *context) = 0;

    virtual std::any visitRtx_pass_stat(PrismParser::Rtx_pass_statContext *context) = 0;

    virtual std::any visitRtx_pass_block(PrismParser::Rtx_pass_blockContext *context) = 0;

    virtual std::any visitRtx_pass_definition(PrismParser::Rtx_pass_definitionContext *context) = 0;

    virtual std::any visitRtx_raygen_stat(PrismParser::Rtx_raygen_statContext *context) = 0;

    virtual std::any visitRtx_raygen_block(PrismParser::Rtx_raygen_blockContext *context) = 0;

    virtual std::any visitRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext *context) = 0;

    virtual std::any visitView_declaration(PrismParser::View_declarationContext *context) = 0;

    virtual std::any visitView_stat(PrismParser::View_statContext *context) = 0;

    virtual std::any visitView_block(PrismParser::View_blockContext *context) = 0;

    virtual std::any visitView_definition(PrismParser::View_definitionContext *context) = 0;

    virtual std::any visitPass_definition(PrismParser::Pass_definitionContext *context) = 0;

    virtual std::any visitPipeline_stat(PrismParser::Pipeline_statContext *context) = 0;

    virtual std::any visitPipeline_block(PrismParser::Pipeline_blockContext *context) = 0;

    virtual std::any visitPipeline_definition(PrismParser::Pipeline_definitionContext *context) = 0;

    virtual std::any visitEnum_value_declaration(PrismParser::Enum_value_declarationContext *context) = 0;

    virtual std::any visitEnum_stat(PrismParser::Enum_statContext *context) = 0;

    virtual std::any visitEnum_block(PrismParser::Enum_blockContext *context) = 0;

    virtual std::any visitEnum_definition(PrismParser::Enum_definitionContext *context) = 0;

    virtual std::any visitShader_type(PrismParser::Shader_typeContext *context) = 0;

    virtual std::any visitPso_param_id(PrismParser::Pso_param_idContext *context) = 0;

    virtual std::any visitBool_type(PrismParser::Bool_typeContext *context) = 0;


};

