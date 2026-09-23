
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PrismListener.h"


/**
 * This class provides an empty implementation of PrismListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  PrismBaseListener : public PrismListener {
public:

  virtual void enterParse(PrismParser::ParseContext * /*ctx*/) override { }
  virtual void exitParse(PrismParser::ParseContext * /*ctx*/) override { }

  virtual void enterConst_definition(PrismParser::Const_definitionContext * /*ctx*/) override { }
  virtual void exitConst_definition(PrismParser::Const_definitionContext * /*ctx*/) override { }

  virtual void enterBind_option(PrismParser::Bind_optionContext * /*ctx*/) override { }
  virtual void exitBind_option(PrismParser::Bind_optionContext * /*ctx*/) override { }

  virtual void enterCond_expr(PrismParser::Cond_exprContext * /*ctx*/) override { }
  virtual void exitCond_expr(PrismParser::Cond_exprContext * /*ctx*/) override { }

  virtual void enterCond_term(PrismParser::Cond_termContext * /*ctx*/) override { }
  virtual void exitCond_term(PrismParser::Cond_termContext * /*ctx*/) override { }

  virtual void enterQualified_ref(PrismParser::Qualified_refContext * /*ctx*/) override { }
  virtual void exitQualified_ref(PrismParser::Qualified_refContext * /*ctx*/) override { }

  virtual void enterMember_ref(PrismParser::Member_refContext * /*ctx*/) override { }
  virtual void exitMember_ref(PrismParser::Member_refContext * /*ctx*/) override { }

  virtual void enterCond_op(PrismParser::Cond_opContext * /*ctx*/) override { }
  virtual void exitCond_op(PrismParser::Cond_opContext * /*ctx*/) override { }

  virtual void enterFlag_value_holder(PrismParser::Flag_value_holderContext * /*ctx*/) override { }
  virtual void exitFlag_value_holder(PrismParser::Flag_value_holderContext * /*ctx*/) override { }

  virtual void enterRaw_value(PrismParser::Raw_valueContext * /*ctx*/) override { }
  virtual void exitRaw_value(PrismParser::Raw_valueContext * /*ctx*/) override { }

  virtual void enterOptions_assign(PrismParser::Options_assignContext * /*ctx*/) override { }
  virtual void exitOptions_assign(PrismParser::Options_assignContext * /*ctx*/) override { }

  virtual void enterOption(PrismParser::OptionContext * /*ctx*/) override { }
  virtual void exitOption(PrismParser::OptionContext * /*ctx*/) override { }

  virtual void enterOption_block(PrismParser::Option_blockContext * /*ctx*/) override { }
  virtual void exitOption_block(PrismParser::Option_blockContext * /*ctx*/) override { }

  virtual void enterArray_count_id(PrismParser::Array_count_idContext * /*ctx*/) override { }
  virtual void exitArray_count_id(PrismParser::Array_count_idContext * /*ctx*/) override { }

  virtual void enterArray(PrismParser::ArrayContext * /*ctx*/) override { }
  virtual void exitArray(PrismParser::ArrayContext * /*ctx*/) override { }

  virtual void enterValue_declaration(PrismParser::Value_declarationContext * /*ctx*/) override { }
  virtual void exitValue_declaration(PrismParser::Value_declarationContext * /*ctx*/) override { }

  virtual void enterSlot_declaration(PrismParser::Slot_declarationContext * /*ctx*/) override { }
  virtual void exitSlot_declaration(PrismParser::Slot_declarationContext * /*ctx*/) override { }

  virtual void enterSampler_declaration(PrismParser::Sampler_declarationContext * /*ctx*/) override { }
  virtual void exitSampler_declaration(PrismParser::Sampler_declarationContext * /*ctx*/) override { }

  virtual void enterDefine_declaration(PrismParser::Define_declarationContext * /*ctx*/) override { }
  virtual void exitDefine_declaration(PrismParser::Define_declarationContext * /*ctx*/) override { }

  virtual void enterRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext * /*ctx*/) override { }
  virtual void exitRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext * /*ctx*/) override { }

  virtual void enterBlends_declaration(PrismParser::Blends_declarationContext * /*ctx*/) override { }
  virtual void exitBlends_declaration(PrismParser::Blends_declarationContext * /*ctx*/) override { }

  virtual void enterPointer(PrismParser::PointerContext * /*ctx*/) override { }
  virtual void exitPointer(PrismParser::PointerContext * /*ctx*/) override { }

  virtual void enterPso_param(PrismParser::Pso_paramContext * /*ctx*/) override { }
  virtual void exitPso_param(PrismParser::Pso_paramContext * /*ctx*/) override { }

  virtual void enterClass_no_template(PrismParser::Class_no_templateContext * /*ctx*/) override { }
  virtual void exitClass_no_template(PrismParser::Class_no_templateContext * /*ctx*/) override { }

  virtual void enterType_with_template(PrismParser::Type_with_templateContext * /*ctx*/) override { }
  virtual void exitType_with_template(PrismParser::Type_with_templateContext * /*ctx*/) override { }

  virtual void enterInherit_id(PrismParser::Inherit_idContext * /*ctx*/) override { }
  virtual void exitInherit_id(PrismParser::Inherit_idContext * /*ctx*/) override { }

  virtual void enterName_id(PrismParser::Name_idContext * /*ctx*/) override { }
  virtual void exitName_id(PrismParser::Name_idContext * /*ctx*/) override { }

  virtual void enterOption_id(PrismParser::Option_idContext * /*ctx*/) override { }
  virtual void exitOption_id(PrismParser::Option_idContext * /*ctx*/) override { }

  virtual void enterOwner_id(PrismParser::Owner_idContext * /*ctx*/) override { }
  virtual void exitOwner_id(PrismParser::Owner_idContext * /*ctx*/) override { }

  virtual void enterTemplate_id(PrismParser::Template_idContext * /*ctx*/) override { }
  virtual void exitTemplate_id(PrismParser::Template_idContext * /*ctx*/) override { }

  virtual void enterFunction_id(PrismParser::Function_idContext * /*ctx*/) override { }
  virtual void exitFunction_id(PrismParser::Function_idContext * /*ctx*/) override { }

  virtual void enterValue_id(PrismParser::Value_idContext * /*ctx*/) override { }
  virtual void exitValue_id(PrismParser::Value_idContext * /*ctx*/) override { }

  virtual void enterValue_id_ignore(PrismParser::Value_id_ignoreContext * /*ctx*/) override { }
  virtual void exitValue_id_ignore(PrismParser::Value_id_ignoreContext * /*ctx*/) override { }

  virtual void enterType_id(PrismParser::Type_idContext * /*ctx*/) override { }
  virtual void exitType_id(PrismParser::Type_idContext * /*ctx*/) override { }

  virtual void enterInsert_block(PrismParser::Insert_blockContext * /*ctx*/) override { }
  virtual void exitInsert_block(PrismParser::Insert_blockContext * /*ctx*/) override { }

  virtual void enterShader_path(PrismParser::Shader_pathContext * /*ctx*/) override { }
  virtual void exitShader_path(PrismParser::Shader_pathContext * /*ctx*/) override { }

  virtual void enterInherit(PrismParser::InheritContext * /*ctx*/) override { }
  virtual void exitInherit(PrismParser::InheritContext * /*ctx*/) override { }

  virtual void enterLayout_stat(PrismParser::Layout_statContext * /*ctx*/) override { }
  virtual void exitLayout_stat(PrismParser::Layout_statContext * /*ctx*/) override { }

  virtual void enterLayout_block(PrismParser::Layout_blockContext * /*ctx*/) override { }
  virtual void exitLayout_block(PrismParser::Layout_blockContext * /*ctx*/) override { }

  virtual void enterLayout_definition(PrismParser::Layout_definitionContext * /*ctx*/) override { }
  virtual void exitLayout_definition(PrismParser::Layout_definitionContext * /*ctx*/) override { }

  virtual void enterTable_stat(PrismParser::Table_statContext * /*ctx*/) override { }
  virtual void exitTable_stat(PrismParser::Table_statContext * /*ctx*/) override { }

  virtual void enterFunction_definition(PrismParser::Function_definitionContext * /*ctx*/) override { }
  virtual void exitFunction_definition(PrismParser::Function_definitionContext * /*ctx*/) override { }

  virtual void enterFunction_params(PrismParser::Function_paramsContext * /*ctx*/) override { }
  virtual void exitFunction_params(PrismParser::Function_paramsContext * /*ctx*/) override { }

  virtual void enterFunction_semantic(PrismParser::Function_semanticContext * /*ctx*/) override { }
  virtual void exitFunction_semantic(PrismParser::Function_semanticContext * /*ctx*/) override { }

  virtual void enterTable_block(PrismParser::Table_blockContext * /*ctx*/) override { }
  virtual void exitTable_block(PrismParser::Table_blockContext * /*ctx*/) override { }

  virtual void enterTable_definition(PrismParser::Table_definitionContext * /*ctx*/) override { }
  virtual void exitTable_definition(PrismParser::Table_definitionContext * /*ctx*/) override { }

  virtual void enterRt_color_declaration(PrismParser::Rt_color_declarationContext * /*ctx*/) override { }
  virtual void exitRt_color_declaration(PrismParser::Rt_color_declarationContext * /*ctx*/) override { }

  virtual void enterRt_ds_declaration(PrismParser::Rt_ds_declarationContext * /*ctx*/) override { }
  virtual void exitRt_ds_declaration(PrismParser::Rt_ds_declarationContext * /*ctx*/) override { }

  virtual void enterRt_stat(PrismParser::Rt_statContext * /*ctx*/) override { }
  virtual void exitRt_stat(PrismParser::Rt_statContext * /*ctx*/) override { }

  virtual void enterRt_block(PrismParser::Rt_blockContext * /*ctx*/) override { }
  virtual void exitRt_block(PrismParser::Rt_blockContext * /*ctx*/) override { }

  virtual void enterRt_definition(PrismParser::Rt_definitionContext * /*ctx*/) override { }
  virtual void exitRt_definition(PrismParser::Rt_definitionContext * /*ctx*/) override { }

  virtual void enterArray_value_holder(PrismParser::Array_value_holderContext * /*ctx*/) override { }
  virtual void exitArray_value_holder(PrismParser::Array_value_holderContext * /*ctx*/) override { }

  virtual void enterArray_value_ids(PrismParser::Array_value_idsContext * /*ctx*/) override { }
  virtual void exitArray_value_ids(PrismParser::Array_value_idsContext * /*ctx*/) override { }

  virtual void enterRoot_sig(PrismParser::Root_sigContext * /*ctx*/) override { }
  virtual void exitRoot_sig(PrismParser::Root_sigContext * /*ctx*/) override { }

  virtual void enterShader(PrismParser::ShaderContext * /*ctx*/) override { }
  virtual void exitShader(PrismParser::ShaderContext * /*ctx*/) override { }

  virtual void enterCompute_pso_stat(PrismParser::Compute_pso_statContext * /*ctx*/) override { }
  virtual void exitCompute_pso_stat(PrismParser::Compute_pso_statContext * /*ctx*/) override { }

  virtual void enterCompute_pso_block(PrismParser::Compute_pso_blockContext * /*ctx*/) override { }
  virtual void exitCompute_pso_block(PrismParser::Compute_pso_blockContext * /*ctx*/) override { }

  virtual void enterCompute_pso_definition(PrismParser::Compute_pso_definitionContext * /*ctx*/) override { }
  virtual void exitCompute_pso_definition(PrismParser::Compute_pso_definitionContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_stat(PrismParser::Graphics_pso_statContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_stat(PrismParser::Graphics_pso_statContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_block(PrismParser::Graphics_pso_blockContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_block(PrismParser::Graphics_pso_blockContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_pso_stat(PrismParser::Rtx_pso_statContext * /*ctx*/) override { }
  virtual void exitRtx_pso_stat(PrismParser::Rtx_pso_statContext * /*ctx*/) override { }

  virtual void enterRtx_pso_block(PrismParser::Rtx_pso_blockContext * /*ctx*/) override { }
  virtual void exitRtx_pso_block(PrismParser::Rtx_pso_blockContext * /*ctx*/) override { }

  virtual void enterRtx_pso_definition(PrismParser::Rtx_pso_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_pso_definition(PrismParser::Rtx_pso_definitionContext * /*ctx*/) override { }

  virtual void enterNode_param_id(PrismParser::Node_param_idContext * /*ctx*/) override { }
  virtual void exitNode_param_id(PrismParser::Node_param_idContext * /*ctx*/) override { }

  virtual void enterNode_param(PrismParser::Node_paramContext * /*ctx*/) override { }
  virtual void exitNode_param(PrismParser::Node_paramContext * /*ctx*/) override { }

  virtual void enterNode_output_decl(PrismParser::Node_output_declContext * /*ctx*/) override { }
  virtual void exitNode_output_decl(PrismParser::Node_output_declContext * /*ctx*/) override { }

  virtual void enterNode_stat(PrismParser::Node_statContext * /*ctx*/) override { }
  virtual void exitNode_stat(PrismParser::Node_statContext * /*ctx*/) override { }

  virtual void enterNode_block(PrismParser::Node_blockContext * /*ctx*/) override { }
  virtual void exitNode_block(PrismParser::Node_blockContext * /*ctx*/) override { }

  virtual void enterNode_definition(PrismParser::Node_definitionContext * /*ctx*/) override { }
  virtual void exitNode_definition(PrismParser::Node_definitionContext * /*ctx*/) override { }

  virtual void enterWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext * /*ctx*/) override { }
  virtual void exitWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext * /*ctx*/) override { }

  virtual void enterWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext * /*ctx*/) override { }
  virtual void exitWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext * /*ctx*/) override { }

  virtual void enterWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext * /*ctx*/) override { }
  virtual void exitWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_pass_stat(PrismParser::Rtx_pass_statContext * /*ctx*/) override { }
  virtual void exitRtx_pass_stat(PrismParser::Rtx_pass_statContext * /*ctx*/) override { }

  virtual void enterRtx_pass_block(PrismParser::Rtx_pass_blockContext * /*ctx*/) override { }
  virtual void exitRtx_pass_block(PrismParser::Rtx_pass_blockContext * /*ctx*/) override { }

  virtual void enterRtx_pass_definition(PrismParser::Rtx_pass_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_pass_definition(PrismParser::Rtx_pass_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_stat(PrismParser::Rtx_raygen_statContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_stat(PrismParser::Rtx_raygen_statContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_block(PrismParser::Rtx_raygen_blockContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_block(PrismParser::Rtx_raygen_blockContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext * /*ctx*/) override { }

  virtual void enterView_declaration(PrismParser::View_declarationContext * /*ctx*/) override { }
  virtual void exitView_declaration(PrismParser::View_declarationContext * /*ctx*/) override { }

  virtual void enterView_stat(PrismParser::View_statContext * /*ctx*/) override { }
  virtual void exitView_stat(PrismParser::View_statContext * /*ctx*/) override { }

  virtual void enterView_block(PrismParser::View_blockContext * /*ctx*/) override { }
  virtual void exitView_block(PrismParser::View_blockContext * /*ctx*/) override { }

  virtual void enterView_definition(PrismParser::View_definitionContext * /*ctx*/) override { }
  virtual void exitView_definition(PrismParser::View_definitionContext * /*ctx*/) override { }

  virtual void enterPass_definition(PrismParser::Pass_definitionContext * /*ctx*/) override { }
  virtual void exitPass_definition(PrismParser::Pass_definitionContext * /*ctx*/) override { }

  virtual void enterPipeline_stat(PrismParser::Pipeline_statContext * /*ctx*/) override { }
  virtual void exitPipeline_stat(PrismParser::Pipeline_statContext * /*ctx*/) override { }

  virtual void enterPipeline_block(PrismParser::Pipeline_blockContext * /*ctx*/) override { }
  virtual void exitPipeline_block(PrismParser::Pipeline_blockContext * /*ctx*/) override { }

  virtual void enterPipeline_definition(PrismParser::Pipeline_definitionContext * /*ctx*/) override { }
  virtual void exitPipeline_definition(PrismParser::Pipeline_definitionContext * /*ctx*/) override { }

  virtual void enterEnum_value_declaration(PrismParser::Enum_value_declarationContext * /*ctx*/) override { }
  virtual void exitEnum_value_declaration(PrismParser::Enum_value_declarationContext * /*ctx*/) override { }

  virtual void enterEnum_stat(PrismParser::Enum_statContext * /*ctx*/) override { }
  virtual void exitEnum_stat(PrismParser::Enum_statContext * /*ctx*/) override { }

  virtual void enterEnum_block(PrismParser::Enum_blockContext * /*ctx*/) override { }
  virtual void exitEnum_block(PrismParser::Enum_blockContext * /*ctx*/) override { }

  virtual void enterEnum_definition(PrismParser::Enum_definitionContext * /*ctx*/) override { }
  virtual void exitEnum_definition(PrismParser::Enum_definitionContext * /*ctx*/) override { }

  virtual void enterShader_type(PrismParser::Shader_typeContext * /*ctx*/) override { }
  virtual void exitShader_type(PrismParser::Shader_typeContext * /*ctx*/) override { }

  virtual void enterPso_param_id(PrismParser::Pso_param_idContext * /*ctx*/) override { }
  virtual void exitPso_param_id(PrismParser::Pso_param_idContext * /*ctx*/) override { }

  virtual void enterBool_type(PrismParser::Bool_typeContext * /*ctx*/) override { }
  virtual void exitBool_type(PrismParser::Bool_typeContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

