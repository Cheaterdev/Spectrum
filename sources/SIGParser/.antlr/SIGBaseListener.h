
// Generated from sources/SIGParser/SIG.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"
#include "SIGListener.h"


/**
 * This class provides an empty implementation of SIGListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SIGBaseListener : public SIGListener {
public:

  virtual void enterParse(SIGParser::ParseContext * /*ctx*/) override { }
  virtual void exitParse(SIGParser::ParseContext * /*ctx*/) override { }

  virtual void enterBind_option(SIGParser::Bind_optionContext * /*ctx*/) override { }
  virtual void exitBind_option(SIGParser::Bind_optionContext * /*ctx*/) override { }

  virtual void enterOptions_assign(SIGParser::Options_assignContext * /*ctx*/) override { }
  virtual void exitOptions_assign(SIGParser::Options_assignContext * /*ctx*/) override { }

  virtual void enterOption(SIGParser::OptionContext * /*ctx*/) override { }
  virtual void exitOption(SIGParser::OptionContext * /*ctx*/) override { }

  virtual void enterOption_block(SIGParser::Option_blockContext * /*ctx*/) override { }
  virtual void exitOption_block(SIGParser::Option_blockContext * /*ctx*/) override { }

  virtual void enterArray_count_id(SIGParser::Array_count_idContext * /*ctx*/) override { }
  virtual void exitArray_count_id(SIGParser::Array_count_idContext * /*ctx*/) override { }

  virtual void enterArray(SIGParser::ArrayContext * /*ctx*/) override { }
  virtual void exitArray(SIGParser::ArrayContext * /*ctx*/) override { }

  virtual void enterValue_declaration(SIGParser::Value_declarationContext * /*ctx*/) override { }
  virtual void exitValue_declaration(SIGParser::Value_declarationContext * /*ctx*/) override { }

  virtual void enterSlot_declaration(SIGParser::Slot_declarationContext * /*ctx*/) override { }
  virtual void exitSlot_declaration(SIGParser::Slot_declarationContext * /*ctx*/) override { }

  virtual void enterSampler_declaration(SIGParser::Sampler_declarationContext * /*ctx*/) override { }
  virtual void exitSampler_declaration(SIGParser::Sampler_declarationContext * /*ctx*/) override { }

  virtual void enterDefine_declaration(SIGParser::Define_declarationContext * /*ctx*/) override { }
  virtual void exitDefine_declaration(SIGParser::Define_declarationContext * /*ctx*/) override { }

  virtual void enterRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext * /*ctx*/) override { }
  virtual void exitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext * /*ctx*/) override { }

  virtual void enterBlends_declaration(SIGParser::Blends_declarationContext * /*ctx*/) override { }
  virtual void exitBlends_declaration(SIGParser::Blends_declarationContext * /*ctx*/) override { }

  virtual void enterPso_param(SIGParser::Pso_paramContext * /*ctx*/) override { }
  virtual void exitPso_param(SIGParser::Pso_paramContext * /*ctx*/) override { }

  virtual void enterType_with_template(SIGParser::Type_with_templateContext * /*ctx*/) override { }
  virtual void exitType_with_template(SIGParser::Type_with_templateContext * /*ctx*/) override { }

  virtual void enterInherit_id(SIGParser::Inherit_idContext * /*ctx*/) override { }
  virtual void exitInherit_id(SIGParser::Inherit_idContext * /*ctx*/) override { }

  virtual void enterName_id(SIGParser::Name_idContext * /*ctx*/) override { }
  virtual void exitName_id(SIGParser::Name_idContext * /*ctx*/) override { }

  virtual void enterType_id(SIGParser::Type_idContext * /*ctx*/) override { }
  virtual void exitType_id(SIGParser::Type_idContext * /*ctx*/) override { }

  virtual void enterOption_id(SIGParser::Option_idContext * /*ctx*/) override { }
  virtual void exitOption_id(SIGParser::Option_idContext * /*ctx*/) override { }

  virtual void enterOwner_id(SIGParser::Owner_idContext * /*ctx*/) override { }
  virtual void exitOwner_id(SIGParser::Owner_idContext * /*ctx*/) override { }

  virtual void enterTemplate_id(SIGParser::Template_idContext * /*ctx*/) override { }
  virtual void exitTemplate_id(SIGParser::Template_idContext * /*ctx*/) override { }

  virtual void enterValue_id(SIGParser::Value_idContext * /*ctx*/) override { }
  virtual void exitValue_id(SIGParser::Value_idContext * /*ctx*/) override { }

  virtual void enterInsert_block(SIGParser::Insert_blockContext * /*ctx*/) override { }
  virtual void exitInsert_block(SIGParser::Insert_blockContext * /*ctx*/) override { }

  virtual void enterPath_id(SIGParser::Path_idContext * /*ctx*/) override { }
  virtual void exitPath_id(SIGParser::Path_idContext * /*ctx*/) override { }

  virtual void enterInherit(SIGParser::InheritContext * /*ctx*/) override { }
  virtual void exitInherit(SIGParser::InheritContext * /*ctx*/) override { }

  virtual void enterLayout_stat(SIGParser::Layout_statContext * /*ctx*/) override { }
  virtual void exitLayout_stat(SIGParser::Layout_statContext * /*ctx*/) override { }

  virtual void enterLayout_block(SIGParser::Layout_blockContext * /*ctx*/) override { }
  virtual void exitLayout_block(SIGParser::Layout_blockContext * /*ctx*/) override { }

  virtual void enterLayout_definition(SIGParser::Layout_definitionContext * /*ctx*/) override { }
  virtual void exitLayout_definition(SIGParser::Layout_definitionContext * /*ctx*/) override { }

  virtual void enterTable_stat(SIGParser::Table_statContext * /*ctx*/) override { }
  virtual void exitTable_stat(SIGParser::Table_statContext * /*ctx*/) override { }

  virtual void enterTable_block(SIGParser::Table_blockContext * /*ctx*/) override { }
  virtual void exitTable_block(SIGParser::Table_blockContext * /*ctx*/) override { }

  virtual void enterTable_definition(SIGParser::Table_definitionContext * /*ctx*/) override { }
  virtual void exitTable_definition(SIGParser::Table_definitionContext * /*ctx*/) override { }

  virtual void enterRt_color_declaration(SIGParser::Rt_color_declarationContext * /*ctx*/) override { }
  virtual void exitRt_color_declaration(SIGParser::Rt_color_declarationContext * /*ctx*/) override { }

  virtual void enterRt_ds_declaration(SIGParser::Rt_ds_declarationContext * /*ctx*/) override { }
  virtual void exitRt_ds_declaration(SIGParser::Rt_ds_declarationContext * /*ctx*/) override { }

  virtual void enterRt_stat(SIGParser::Rt_statContext * /*ctx*/) override { }
  virtual void exitRt_stat(SIGParser::Rt_statContext * /*ctx*/) override { }

  virtual void enterRt_block(SIGParser::Rt_blockContext * /*ctx*/) override { }
  virtual void exitRt_block(SIGParser::Rt_blockContext * /*ctx*/) override { }

  virtual void enterRt_definition(SIGParser::Rt_definitionContext * /*ctx*/) override { }
  virtual void exitRt_definition(SIGParser::Rt_definitionContext * /*ctx*/) override { }

  virtual void enterArray_value_holder(SIGParser::Array_value_holderContext * /*ctx*/) override { }
  virtual void exitArray_value_holder(SIGParser::Array_value_holderContext * /*ctx*/) override { }

  virtual void enterArray_value_ids(SIGParser::Array_value_idsContext * /*ctx*/) override { }
  virtual void exitArray_value_ids(SIGParser::Array_value_idsContext * /*ctx*/) override { }

  virtual void enterRoot_sig(SIGParser::Root_sigContext * /*ctx*/) override { }
  virtual void exitRoot_sig(SIGParser::Root_sigContext * /*ctx*/) override { }

  virtual void enterShader(SIGParser::ShaderContext * /*ctx*/) override { }
  virtual void exitShader(SIGParser::ShaderContext * /*ctx*/) override { }

  virtual void enterCompute_pso_stat(SIGParser::Compute_pso_statContext * /*ctx*/) override { }
  virtual void exitCompute_pso_stat(SIGParser::Compute_pso_statContext * /*ctx*/) override { }

  virtual void enterCompute_pso_block(SIGParser::Compute_pso_blockContext * /*ctx*/) override { }
  virtual void exitCompute_pso_block(SIGParser::Compute_pso_blockContext * /*ctx*/) override { }

  virtual void enterCompute_pso_definition(SIGParser::Compute_pso_definitionContext * /*ctx*/) override { }
  virtual void exitCompute_pso_definition(SIGParser::Compute_pso_definitionContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_stat(SIGParser::Graphics_pso_statContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_stat(SIGParser::Graphics_pso_statContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_block(SIGParser::Graphics_pso_blockContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_block(SIGParser::Graphics_pso_blockContext * /*ctx*/) override { }

  virtual void enterGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext * /*ctx*/) override { }
  virtual void exitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_pso_stat(SIGParser::Rtx_pso_statContext * /*ctx*/) override { }
  virtual void exitRtx_pso_stat(SIGParser::Rtx_pso_statContext * /*ctx*/) override { }

  virtual void enterRtx_pso_block(SIGParser::Rtx_pso_blockContext * /*ctx*/) override { }
  virtual void exitRtx_pso_block(SIGParser::Rtx_pso_blockContext * /*ctx*/) override { }

  virtual void enterRtx_pso_definition(SIGParser::Rtx_pso_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_pso_definition(SIGParser::Rtx_pso_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_pass_stat(SIGParser::Rtx_pass_statContext * /*ctx*/) override { }
  virtual void exitRtx_pass_stat(SIGParser::Rtx_pass_statContext * /*ctx*/) override { }

  virtual void enterRtx_pass_block(SIGParser::Rtx_pass_blockContext * /*ctx*/) override { }
  virtual void exitRtx_pass_block(SIGParser::Rtx_pass_blockContext * /*ctx*/) override { }

  virtual void enterRtx_pass_definition(SIGParser::Rtx_pass_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_pass_definition(SIGParser::Rtx_pass_definitionContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_stat(SIGParser::Rtx_raygen_statContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_stat(SIGParser::Rtx_raygen_statContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_block(SIGParser::Rtx_raygen_blockContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_block(SIGParser::Rtx_raygen_blockContext * /*ctx*/) override { }

  virtual void enterRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext * /*ctx*/) override { }
  virtual void exitRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext * /*ctx*/) override { }

  virtual void enterShader_type(SIGParser::Shader_typeContext * /*ctx*/) override { }
  virtual void exitShader_type(SIGParser::Shader_typeContext * /*ctx*/) override { }

  virtual void enterPso_param_id(SIGParser::Pso_param_idContext * /*ctx*/) override { }
  virtual void exitPso_param_id(SIGParser::Pso_param_idContext * /*ctx*/) override { }

  virtual void enterBool_type(SIGParser::Bool_typeContext * /*ctx*/) override { }
  virtual void exitBool_type(SIGParser::Bool_typeContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

