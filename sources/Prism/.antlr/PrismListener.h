
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PrismParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by PrismParser.
 */
class  PrismListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterParse(PrismParser::ParseContext *ctx) = 0;
  virtual void exitParse(PrismParser::ParseContext *ctx) = 0;

  virtual void enterConst_definition(PrismParser::Const_definitionContext *ctx) = 0;
  virtual void exitConst_definition(PrismParser::Const_definitionContext *ctx) = 0;

  virtual void enterBind_option(PrismParser::Bind_optionContext *ctx) = 0;
  virtual void exitBind_option(PrismParser::Bind_optionContext *ctx) = 0;

  virtual void enterCond_expr(PrismParser::Cond_exprContext *ctx) = 0;
  virtual void exitCond_expr(PrismParser::Cond_exprContext *ctx) = 0;

  virtual void enterCond_term(PrismParser::Cond_termContext *ctx) = 0;
  virtual void exitCond_term(PrismParser::Cond_termContext *ctx) = 0;

  virtual void enterCall(PrismParser::CallContext *ctx) = 0;
  virtual void exitCall(PrismParser::CallContext *ctx) = 0;

  virtual void enterCall_arg(PrismParser::Call_argContext *ctx) = 0;
  virtual void exitCall_arg(PrismParser::Call_argContext *ctx) = 0;

  virtual void enterQualified_ref(PrismParser::Qualified_refContext *ctx) = 0;
  virtual void exitQualified_ref(PrismParser::Qualified_refContext *ctx) = 0;

  virtual void enterMember_ref(PrismParser::Member_refContext *ctx) = 0;
  virtual void exitMember_ref(PrismParser::Member_refContext *ctx) = 0;

  virtual void enterCond_op(PrismParser::Cond_opContext *ctx) = 0;
  virtual void exitCond_op(PrismParser::Cond_opContext *ctx) = 0;

  virtual void enterFlag_value_holder(PrismParser::Flag_value_holderContext *ctx) = 0;
  virtual void exitFlag_value_holder(PrismParser::Flag_value_holderContext *ctx) = 0;

  virtual void enterRaw_value(PrismParser::Raw_valueContext *ctx) = 0;
  virtual void exitRaw_value(PrismParser::Raw_valueContext *ctx) = 0;

  virtual void enterOptions_assign(PrismParser::Options_assignContext *ctx) = 0;
  virtual void exitOptions_assign(PrismParser::Options_assignContext *ctx) = 0;

  virtual void enterOption(PrismParser::OptionContext *ctx) = 0;
  virtual void exitOption(PrismParser::OptionContext *ctx) = 0;

  virtual void enterOption_block(PrismParser::Option_blockContext *ctx) = 0;
  virtual void exitOption_block(PrismParser::Option_blockContext *ctx) = 0;

  virtual void enterArray_count_id(PrismParser::Array_count_idContext *ctx) = 0;
  virtual void exitArray_count_id(PrismParser::Array_count_idContext *ctx) = 0;

  virtual void enterArray(PrismParser::ArrayContext *ctx) = 0;
  virtual void exitArray(PrismParser::ArrayContext *ctx) = 0;

  virtual void enterValue_declaration(PrismParser::Value_declarationContext *ctx) = 0;
  virtual void exitValue_declaration(PrismParser::Value_declarationContext *ctx) = 0;

  virtual void enterSlot_declaration(PrismParser::Slot_declarationContext *ctx) = 0;
  virtual void exitSlot_declaration(PrismParser::Slot_declarationContext *ctx) = 0;

  virtual void enterSampler_declaration(PrismParser::Sampler_declarationContext *ctx) = 0;
  virtual void exitSampler_declaration(PrismParser::Sampler_declarationContext *ctx) = 0;

  virtual void enterDefine_declaration(PrismParser::Define_declarationContext *ctx) = 0;
  virtual void exitDefine_declaration(PrismParser::Define_declarationContext *ctx) = 0;

  virtual void enterRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext *ctx) = 0;
  virtual void exitRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext *ctx) = 0;

  virtual void enterBlends_declaration(PrismParser::Blends_declarationContext *ctx) = 0;
  virtual void exitBlends_declaration(PrismParser::Blends_declarationContext *ctx) = 0;

  virtual void enterPointer(PrismParser::PointerContext *ctx) = 0;
  virtual void exitPointer(PrismParser::PointerContext *ctx) = 0;

  virtual void enterPso_param(PrismParser::Pso_paramContext *ctx) = 0;
  virtual void exitPso_param(PrismParser::Pso_paramContext *ctx) = 0;

  virtual void enterClass_no_template(PrismParser::Class_no_templateContext *ctx) = 0;
  virtual void exitClass_no_template(PrismParser::Class_no_templateContext *ctx) = 0;

  virtual void enterType_with_template(PrismParser::Type_with_templateContext *ctx) = 0;
  virtual void exitType_with_template(PrismParser::Type_with_templateContext *ctx) = 0;

  virtual void enterInherit_id(PrismParser::Inherit_idContext *ctx) = 0;
  virtual void exitInherit_id(PrismParser::Inherit_idContext *ctx) = 0;

  virtual void enterName_id(PrismParser::Name_idContext *ctx) = 0;
  virtual void exitName_id(PrismParser::Name_idContext *ctx) = 0;

  virtual void enterOption_id(PrismParser::Option_idContext *ctx) = 0;
  virtual void exitOption_id(PrismParser::Option_idContext *ctx) = 0;

  virtual void enterOwner_id(PrismParser::Owner_idContext *ctx) = 0;
  virtual void exitOwner_id(PrismParser::Owner_idContext *ctx) = 0;

  virtual void enterTemplate_id(PrismParser::Template_idContext *ctx) = 0;
  virtual void exitTemplate_id(PrismParser::Template_idContext *ctx) = 0;

  virtual void enterFunction_id(PrismParser::Function_idContext *ctx) = 0;
  virtual void exitFunction_id(PrismParser::Function_idContext *ctx) = 0;

  virtual void enterValue_id(PrismParser::Value_idContext *ctx) = 0;
  virtual void exitValue_id(PrismParser::Value_idContext *ctx) = 0;

  virtual void enterValue_id_ignore(PrismParser::Value_id_ignoreContext *ctx) = 0;
  virtual void exitValue_id_ignore(PrismParser::Value_id_ignoreContext *ctx) = 0;

  virtual void enterType_id(PrismParser::Type_idContext *ctx) = 0;
  virtual void exitType_id(PrismParser::Type_idContext *ctx) = 0;

  virtual void enterInsert_block(PrismParser::Insert_blockContext *ctx) = 0;
  virtual void exitInsert_block(PrismParser::Insert_blockContext *ctx) = 0;

  virtual void enterShader_path(PrismParser::Shader_pathContext *ctx) = 0;
  virtual void exitShader_path(PrismParser::Shader_pathContext *ctx) = 0;

  virtual void enterInherit(PrismParser::InheritContext *ctx) = 0;
  virtual void exitInherit(PrismParser::InheritContext *ctx) = 0;

  virtual void enterLayout_stat(PrismParser::Layout_statContext *ctx) = 0;
  virtual void exitLayout_stat(PrismParser::Layout_statContext *ctx) = 0;

  virtual void enterLayout_block(PrismParser::Layout_blockContext *ctx) = 0;
  virtual void exitLayout_block(PrismParser::Layout_blockContext *ctx) = 0;

  virtual void enterLayout_definition(PrismParser::Layout_definitionContext *ctx) = 0;
  virtual void exitLayout_definition(PrismParser::Layout_definitionContext *ctx) = 0;

  virtual void enterTable_stat(PrismParser::Table_statContext *ctx) = 0;
  virtual void exitTable_stat(PrismParser::Table_statContext *ctx) = 0;

  virtual void enterFunction_definition(PrismParser::Function_definitionContext *ctx) = 0;
  virtual void exitFunction_definition(PrismParser::Function_definitionContext *ctx) = 0;

  virtual void enterFunction_params(PrismParser::Function_paramsContext *ctx) = 0;
  virtual void exitFunction_params(PrismParser::Function_paramsContext *ctx) = 0;

  virtual void enterFunction_semantic(PrismParser::Function_semanticContext *ctx) = 0;
  virtual void exitFunction_semantic(PrismParser::Function_semanticContext *ctx) = 0;

  virtual void enterTable_block(PrismParser::Table_blockContext *ctx) = 0;
  virtual void exitTable_block(PrismParser::Table_blockContext *ctx) = 0;

  virtual void enterTable_definition(PrismParser::Table_definitionContext *ctx) = 0;
  virtual void exitTable_definition(PrismParser::Table_definitionContext *ctx) = 0;

  virtual void enterRt_color_declaration(PrismParser::Rt_color_declarationContext *ctx) = 0;
  virtual void exitRt_color_declaration(PrismParser::Rt_color_declarationContext *ctx) = 0;

  virtual void enterRt_ds_declaration(PrismParser::Rt_ds_declarationContext *ctx) = 0;
  virtual void exitRt_ds_declaration(PrismParser::Rt_ds_declarationContext *ctx) = 0;

  virtual void enterRt_stat(PrismParser::Rt_statContext *ctx) = 0;
  virtual void exitRt_stat(PrismParser::Rt_statContext *ctx) = 0;

  virtual void enterRt_block(PrismParser::Rt_blockContext *ctx) = 0;
  virtual void exitRt_block(PrismParser::Rt_blockContext *ctx) = 0;

  virtual void enterRt_definition(PrismParser::Rt_definitionContext *ctx) = 0;
  virtual void exitRt_definition(PrismParser::Rt_definitionContext *ctx) = 0;

  virtual void enterArray_value_holder(PrismParser::Array_value_holderContext *ctx) = 0;
  virtual void exitArray_value_holder(PrismParser::Array_value_holderContext *ctx) = 0;

  virtual void enterArray_value_ids(PrismParser::Array_value_idsContext *ctx) = 0;
  virtual void exitArray_value_ids(PrismParser::Array_value_idsContext *ctx) = 0;

  virtual void enterRoot_sig(PrismParser::Root_sigContext *ctx) = 0;
  virtual void exitRoot_sig(PrismParser::Root_sigContext *ctx) = 0;

  virtual void enterShader(PrismParser::ShaderContext *ctx) = 0;
  virtual void exitShader(PrismParser::ShaderContext *ctx) = 0;

  virtual void enterCompute_pso_stat(PrismParser::Compute_pso_statContext *ctx) = 0;
  virtual void exitCompute_pso_stat(PrismParser::Compute_pso_statContext *ctx) = 0;

  virtual void enterCompute_pso_block(PrismParser::Compute_pso_blockContext *ctx) = 0;
  virtual void exitCompute_pso_block(PrismParser::Compute_pso_blockContext *ctx) = 0;

  virtual void enterCompute_pso_definition(PrismParser::Compute_pso_definitionContext *ctx) = 0;
  virtual void exitCompute_pso_definition(PrismParser::Compute_pso_definitionContext *ctx) = 0;

  virtual void enterGraphics_pso_stat(PrismParser::Graphics_pso_statContext *ctx) = 0;
  virtual void exitGraphics_pso_stat(PrismParser::Graphics_pso_statContext *ctx) = 0;

  virtual void enterGraphics_pso_block(PrismParser::Graphics_pso_blockContext *ctx) = 0;
  virtual void exitGraphics_pso_block(PrismParser::Graphics_pso_blockContext *ctx) = 0;

  virtual void enterGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext *ctx) = 0;
  virtual void exitGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext *ctx) = 0;

  virtual void enterRtx_pso_stat(PrismParser::Rtx_pso_statContext *ctx) = 0;
  virtual void exitRtx_pso_stat(PrismParser::Rtx_pso_statContext *ctx) = 0;

  virtual void enterRtx_pso_block(PrismParser::Rtx_pso_blockContext *ctx) = 0;
  virtual void exitRtx_pso_block(PrismParser::Rtx_pso_blockContext *ctx) = 0;

  virtual void enterRtx_pso_definition(PrismParser::Rtx_pso_definitionContext *ctx) = 0;
  virtual void exitRtx_pso_definition(PrismParser::Rtx_pso_definitionContext *ctx) = 0;

  virtual void enterNode_param_id(PrismParser::Node_param_idContext *ctx) = 0;
  virtual void exitNode_param_id(PrismParser::Node_param_idContext *ctx) = 0;

  virtual void enterNode_param(PrismParser::Node_paramContext *ctx) = 0;
  virtual void exitNode_param(PrismParser::Node_paramContext *ctx) = 0;

  virtual void enterNode_output_decl(PrismParser::Node_output_declContext *ctx) = 0;
  virtual void exitNode_output_decl(PrismParser::Node_output_declContext *ctx) = 0;

  virtual void enterNode_stat(PrismParser::Node_statContext *ctx) = 0;
  virtual void exitNode_stat(PrismParser::Node_statContext *ctx) = 0;

  virtual void enterNode_block(PrismParser::Node_blockContext *ctx) = 0;
  virtual void exitNode_block(PrismParser::Node_blockContext *ctx) = 0;

  virtual void enterNode_definition(PrismParser::Node_definitionContext *ctx) = 0;
  virtual void exitNode_definition(PrismParser::Node_definitionContext *ctx) = 0;

  virtual void enterWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext *ctx) = 0;
  virtual void exitWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext *ctx) = 0;

  virtual void enterWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext *ctx) = 0;
  virtual void exitWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext *ctx) = 0;

  virtual void enterWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext *ctx) = 0;
  virtual void exitWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext *ctx) = 0;

  virtual void enterRtx_pass_stat(PrismParser::Rtx_pass_statContext *ctx) = 0;
  virtual void exitRtx_pass_stat(PrismParser::Rtx_pass_statContext *ctx) = 0;

  virtual void enterRtx_pass_block(PrismParser::Rtx_pass_blockContext *ctx) = 0;
  virtual void exitRtx_pass_block(PrismParser::Rtx_pass_blockContext *ctx) = 0;

  virtual void enterRtx_pass_definition(PrismParser::Rtx_pass_definitionContext *ctx) = 0;
  virtual void exitRtx_pass_definition(PrismParser::Rtx_pass_definitionContext *ctx) = 0;

  virtual void enterRtx_raygen_stat(PrismParser::Rtx_raygen_statContext *ctx) = 0;
  virtual void exitRtx_raygen_stat(PrismParser::Rtx_raygen_statContext *ctx) = 0;

  virtual void enterRtx_raygen_block(PrismParser::Rtx_raygen_blockContext *ctx) = 0;
  virtual void exitRtx_raygen_block(PrismParser::Rtx_raygen_blockContext *ctx) = 0;

  virtual void enterRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext *ctx) = 0;
  virtual void exitRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext *ctx) = 0;

  virtual void enterView_declaration(PrismParser::View_declarationContext *ctx) = 0;
  virtual void exitView_declaration(PrismParser::View_declarationContext *ctx) = 0;

  virtual void enterView_stat(PrismParser::View_statContext *ctx) = 0;
  virtual void exitView_stat(PrismParser::View_statContext *ctx) = 0;

  virtual void enterView_block(PrismParser::View_blockContext *ctx) = 0;
  virtual void exitView_block(PrismParser::View_blockContext *ctx) = 0;

  virtual void enterView_definition(PrismParser::View_definitionContext *ctx) = 0;
  virtual void exitView_definition(PrismParser::View_definitionContext *ctx) = 0;

  virtual void enterPass_definition(PrismParser::Pass_definitionContext *ctx) = 0;
  virtual void exitPass_definition(PrismParser::Pass_definitionContext *ctx) = 0;

  virtual void enterPipeline_stat(PrismParser::Pipeline_statContext *ctx) = 0;
  virtual void exitPipeline_stat(PrismParser::Pipeline_statContext *ctx) = 0;

  virtual void enterPipeline_block(PrismParser::Pipeline_blockContext *ctx) = 0;
  virtual void exitPipeline_block(PrismParser::Pipeline_blockContext *ctx) = 0;

  virtual void enterPipeline_definition(PrismParser::Pipeline_definitionContext *ctx) = 0;
  virtual void exitPipeline_definition(PrismParser::Pipeline_definitionContext *ctx) = 0;

  virtual void enterEnum_value_declaration(PrismParser::Enum_value_declarationContext *ctx) = 0;
  virtual void exitEnum_value_declaration(PrismParser::Enum_value_declarationContext *ctx) = 0;

  virtual void enterEnum_stat(PrismParser::Enum_statContext *ctx) = 0;
  virtual void exitEnum_stat(PrismParser::Enum_statContext *ctx) = 0;

  virtual void enterEnum_block(PrismParser::Enum_blockContext *ctx) = 0;
  virtual void exitEnum_block(PrismParser::Enum_blockContext *ctx) = 0;

  virtual void enterEnum_definition(PrismParser::Enum_definitionContext *ctx) = 0;
  virtual void exitEnum_definition(PrismParser::Enum_definitionContext *ctx) = 0;

  virtual void enterShader_type(PrismParser::Shader_typeContext *ctx) = 0;
  virtual void exitShader_type(PrismParser::Shader_typeContext *ctx) = 0;

  virtual void enterPso_param_id(PrismParser::Pso_param_idContext *ctx) = 0;
  virtual void exitPso_param_id(PrismParser::Pso_param_idContext *ctx) = 0;

  virtual void enterBool_type(PrismParser::Bool_typeContext *ctx) = 0;
  virtual void exitBool_type(PrismParser::Bool_typeContext *ctx) = 0;


};

