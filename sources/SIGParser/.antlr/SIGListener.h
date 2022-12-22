
// Generated from SIG.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "SIGParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by SIGParser.
 */
class  SIGListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterParse(SIGParser::ParseContext *ctx) = 0;
  virtual void exitParse(SIGParser::ParseContext *ctx) = 0;

  virtual void enterBind_option(SIGParser::Bind_optionContext *ctx) = 0;
  virtual void exitBind_option(SIGParser::Bind_optionContext *ctx) = 0;

  virtual void enterOptions_assign(SIGParser::Options_assignContext *ctx) = 0;
  virtual void exitOptions_assign(SIGParser::Options_assignContext *ctx) = 0;

  virtual void enterOption(SIGParser::OptionContext *ctx) = 0;
  virtual void exitOption(SIGParser::OptionContext *ctx) = 0;

  virtual void enterOption_block(SIGParser::Option_blockContext *ctx) = 0;
  virtual void exitOption_block(SIGParser::Option_blockContext *ctx) = 0;

  virtual void enterArray_count_id(SIGParser::Array_count_idContext *ctx) = 0;
  virtual void exitArray_count_id(SIGParser::Array_count_idContext *ctx) = 0;

  virtual void enterArray(SIGParser::ArrayContext *ctx) = 0;
  virtual void exitArray(SIGParser::ArrayContext *ctx) = 0;

  virtual void enterValue_declaration(SIGParser::Value_declarationContext *ctx) = 0;
  virtual void exitValue_declaration(SIGParser::Value_declarationContext *ctx) = 0;

  virtual void enterSlot_declaration(SIGParser::Slot_declarationContext *ctx) = 0;
  virtual void exitSlot_declaration(SIGParser::Slot_declarationContext *ctx) = 0;

  virtual void enterSampler_declaration(SIGParser::Sampler_declarationContext *ctx) = 0;
  virtual void exitSampler_declaration(SIGParser::Sampler_declarationContext *ctx) = 0;

  virtual void enterDefine_declaration(SIGParser::Define_declarationContext *ctx) = 0;
  virtual void exitDefine_declaration(SIGParser::Define_declarationContext *ctx) = 0;

  virtual void enterRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *ctx) = 0;
  virtual void exitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *ctx) = 0;

  virtual void enterBlends_declaration(SIGParser::Blends_declarationContext *ctx) = 0;
  virtual void exitBlends_declaration(SIGParser::Blends_declarationContext *ctx) = 0;

  virtual void enterPso_param(SIGParser::Pso_paramContext *ctx) = 0;
  virtual void exitPso_param(SIGParser::Pso_paramContext *ctx) = 0;

  virtual void enterType_with_template(SIGParser::Type_with_templateContext *ctx) = 0;
  virtual void exitType_with_template(SIGParser::Type_with_templateContext *ctx) = 0;

  virtual void enterInherit_id(SIGParser::Inherit_idContext *ctx) = 0;
  virtual void exitInherit_id(SIGParser::Inherit_idContext *ctx) = 0;

  virtual void enterName_id(SIGParser::Name_idContext *ctx) = 0;
  virtual void exitName_id(SIGParser::Name_idContext *ctx) = 0;

  virtual void enterType_id(SIGParser::Type_idContext *ctx) = 0;
  virtual void exitType_id(SIGParser::Type_idContext *ctx) = 0;

  virtual void enterOption_id(SIGParser::Option_idContext *ctx) = 0;
  virtual void exitOption_id(SIGParser::Option_idContext *ctx) = 0;

  virtual void enterOwner_id(SIGParser::Owner_idContext *ctx) = 0;
  virtual void exitOwner_id(SIGParser::Owner_idContext *ctx) = 0;

  virtual void enterTemplate_id(SIGParser::Template_idContext *ctx) = 0;
  virtual void exitTemplate_id(SIGParser::Template_idContext *ctx) = 0;

  virtual void enterValue_id(SIGParser::Value_idContext *ctx) = 0;
  virtual void exitValue_id(SIGParser::Value_idContext *ctx) = 0;

  virtual void enterInsert_block(SIGParser::Insert_blockContext *ctx) = 0;
  virtual void exitInsert_block(SIGParser::Insert_blockContext *ctx) = 0;

  virtual void enterPath_id(SIGParser::Path_idContext *ctx) = 0;
  virtual void exitPath_id(SIGParser::Path_idContext *ctx) = 0;

  virtual void enterInherit(SIGParser::InheritContext *ctx) = 0;
  virtual void exitInherit(SIGParser::InheritContext *ctx) = 0;

  virtual void enterLayout_stat(SIGParser::Layout_statContext *ctx) = 0;
  virtual void exitLayout_stat(SIGParser::Layout_statContext *ctx) = 0;

  virtual void enterLayout_block(SIGParser::Layout_blockContext *ctx) = 0;
  virtual void exitLayout_block(SIGParser::Layout_blockContext *ctx) = 0;

  virtual void enterLayout_definition(SIGParser::Layout_definitionContext *ctx) = 0;
  virtual void exitLayout_definition(SIGParser::Layout_definitionContext *ctx) = 0;

  virtual void enterTable_stat(SIGParser::Table_statContext *ctx) = 0;
  virtual void exitTable_stat(SIGParser::Table_statContext *ctx) = 0;

  virtual void enterTable_block(SIGParser::Table_blockContext *ctx) = 0;
  virtual void exitTable_block(SIGParser::Table_blockContext *ctx) = 0;

  virtual void enterTable_definition(SIGParser::Table_definitionContext *ctx) = 0;
  virtual void exitTable_definition(SIGParser::Table_definitionContext *ctx) = 0;

  virtual void enterRt_color_declaration(SIGParser::Rt_color_declarationContext *ctx) = 0;
  virtual void exitRt_color_declaration(SIGParser::Rt_color_declarationContext *ctx) = 0;

  virtual void enterRt_ds_declaration(SIGParser::Rt_ds_declarationContext *ctx) = 0;
  virtual void exitRt_ds_declaration(SIGParser::Rt_ds_declarationContext *ctx) = 0;

  virtual void enterRt_stat(SIGParser::Rt_statContext *ctx) = 0;
  virtual void exitRt_stat(SIGParser::Rt_statContext *ctx) = 0;

  virtual void enterRt_block(SIGParser::Rt_blockContext *ctx) = 0;
  virtual void exitRt_block(SIGParser::Rt_blockContext *ctx) = 0;

  virtual void enterRt_definition(SIGParser::Rt_definitionContext *ctx) = 0;
  virtual void exitRt_definition(SIGParser::Rt_definitionContext *ctx) = 0;

  virtual void enterArray_value_holder(SIGParser::Array_value_holderContext *ctx) = 0;
  virtual void exitArray_value_holder(SIGParser::Array_value_holderContext *ctx) = 0;

  virtual void enterArray_value_ids(SIGParser::Array_value_idsContext *ctx) = 0;
  virtual void exitArray_value_ids(SIGParser::Array_value_idsContext *ctx) = 0;

  virtual void enterRoot_sig(SIGParser::Root_sigContext *ctx) = 0;
  virtual void exitRoot_sig(SIGParser::Root_sigContext *ctx) = 0;

  virtual void enterShader(SIGParser::ShaderContext *ctx) = 0;
  virtual void exitShader(SIGParser::ShaderContext *ctx) = 0;

  virtual void enterCompute_pso_stat(SIGParser::Compute_pso_statContext *ctx) = 0;
  virtual void exitCompute_pso_stat(SIGParser::Compute_pso_statContext *ctx) = 0;

  virtual void enterCompute_pso_block(SIGParser::Compute_pso_blockContext *ctx) = 0;
  virtual void exitCompute_pso_block(SIGParser::Compute_pso_blockContext *ctx) = 0;

  virtual void enterCompute_pso_definition(SIGParser::Compute_pso_definitionContext *ctx) = 0;
  virtual void exitCompute_pso_definition(SIGParser::Compute_pso_definitionContext *ctx) = 0;

  virtual void enterGraphics_pso_stat(SIGParser::Graphics_pso_statContext *ctx) = 0;
  virtual void exitGraphics_pso_stat(SIGParser::Graphics_pso_statContext *ctx) = 0;

  virtual void enterGraphics_pso_block(SIGParser::Graphics_pso_blockContext *ctx) = 0;
  virtual void exitGraphics_pso_block(SIGParser::Graphics_pso_blockContext *ctx) = 0;

  virtual void enterGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *ctx) = 0;
  virtual void exitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *ctx) = 0;

  virtual void enterRtx_pso_stat(SIGParser::Rtx_pso_statContext *ctx) = 0;
  virtual void exitRtx_pso_stat(SIGParser::Rtx_pso_statContext *ctx) = 0;

  virtual void enterRtx_pso_block(SIGParser::Rtx_pso_blockContext *ctx) = 0;
  virtual void exitRtx_pso_block(SIGParser::Rtx_pso_blockContext *ctx) = 0;

  virtual void enterRtx_pso_definition(SIGParser::Rtx_pso_definitionContext *ctx) = 0;
  virtual void exitRtx_pso_definition(SIGParser::Rtx_pso_definitionContext *ctx) = 0;

  virtual void enterRtx_pass_stat(SIGParser::Rtx_pass_statContext *ctx) = 0;
  virtual void exitRtx_pass_stat(SIGParser::Rtx_pass_statContext *ctx) = 0;

  virtual void enterRtx_pass_block(SIGParser::Rtx_pass_blockContext *ctx) = 0;
  virtual void exitRtx_pass_block(SIGParser::Rtx_pass_blockContext *ctx) = 0;

  virtual void enterRtx_pass_definition(SIGParser::Rtx_pass_definitionContext *ctx) = 0;
  virtual void exitRtx_pass_definition(SIGParser::Rtx_pass_definitionContext *ctx) = 0;

  virtual void enterRtx_raygen_stat(SIGParser::Rtx_raygen_statContext *ctx) = 0;
  virtual void exitRtx_raygen_stat(SIGParser::Rtx_raygen_statContext *ctx) = 0;

  virtual void enterRtx_raygen_block(SIGParser::Rtx_raygen_blockContext *ctx) = 0;
  virtual void exitRtx_raygen_block(SIGParser::Rtx_raygen_blockContext *ctx) = 0;

  virtual void enterRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext *ctx) = 0;
  virtual void exitRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext *ctx) = 0;

  virtual void enterShader_type(SIGParser::Shader_typeContext *ctx) = 0;
  virtual void exitShader_type(SIGParser::Shader_typeContext *ctx) = 0;

  virtual void enterPso_param_id(SIGParser::Pso_param_idContext *ctx) = 0;
  virtual void exitPso_param_id(SIGParser::Pso_param_idContext *ctx) = 0;

  virtual void enterBool_type(SIGParser::Bool_typeContext *ctx) = 0;
  virtual void exitBool_type(SIGParser::Bool_typeContext *ctx) = 0;


};

