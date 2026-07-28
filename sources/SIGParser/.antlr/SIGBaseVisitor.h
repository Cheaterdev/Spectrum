
// Generated from SIG.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "SIGVisitor.h"


/**
 * This class provides an empty implementation of SIGVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SIGBaseVisitor : public SIGVisitor {
public:

  virtual std::any visitParse(SIGParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBind_option(SIGParser::Bind_optionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOptions_assign(SIGParser::Options_assignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption(SIGParser::OptionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption_block(SIGParser::Option_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_count_id(SIGParser::Array_count_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray(SIGParser::ArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_declaration(SIGParser::Value_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSlot_declaration(SIGParser::Slot_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSampler_declaration(SIGParser::Sampler_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefine_declaration(SIGParser::Define_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlends_declaration(SIGParser::Blends_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointer(SIGParser::PointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPso_param(SIGParser::Pso_paramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_no_template(SIGParser::Class_no_templateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_with_template(SIGParser::Type_with_templateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInherit_id(SIGParser::Inherit_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName_id(SIGParser::Name_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption_id(SIGParser::Option_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOwner_id(SIGParser::Owner_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplate_id(SIGParser::Template_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_id(SIGParser::Function_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_id(SIGParser::Value_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_id_ignore(SIGParser::Value_id_ignoreContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_id(SIGParser::Type_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsert_block(SIGParser::Insert_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPath_id(SIGParser::Path_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInherit(SIGParser::InheritContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_stat(SIGParser::Layout_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_block(SIGParser::Layout_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_definition(SIGParser::Layout_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_stat(SIGParser::Table_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_block(SIGParser::Table_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_definition(SIGParser::Table_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_color_declaration(SIGParser::Rt_color_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_ds_declaration(SIGParser::Rt_ds_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_stat(SIGParser::Rt_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_block(SIGParser::Rt_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_definition(SIGParser::Rt_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_value_holder(SIGParser::Array_value_holderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_value_ids(SIGParser::Array_value_idsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRoot_sig(SIGParser::Root_sigContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShader(SIGParser::ShaderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_stat(SIGParser::Compute_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_block(SIGParser::Compute_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_definition(SIGParser::Compute_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_stat(SIGParser::Graphics_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_block(SIGParser::Graphics_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_stat(SIGParser::Rtx_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_block(SIGParser::Rtx_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_definition(SIGParser::Rtx_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_param_id(SIGParser::Node_param_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_param(SIGParser::Node_paramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_output_decl(SIGParser::Node_output_declContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_stat(SIGParser::Node_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_block(SIGParser::Node_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_definition(SIGParser::Node_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_stat(SIGParser::Workgraph_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_block(SIGParser::Workgraph_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_definition(SIGParser::Workgraph_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_stat(SIGParser::Rtx_pass_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_block(SIGParser::Rtx_pass_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_definition(SIGParser::Rtx_pass_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_stat(SIGParser::Rtx_raygen_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_block(SIGParser::Rtx_raygen_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_declaration(SIGParser::View_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_stat(SIGParser::View_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_block(SIGParser::View_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_definition(SIGParser::View_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPass_definition(SIGParser::Pass_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_stat(SIGParser::Pipeline_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_block(SIGParser::Pipeline_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_definition(SIGParser::Pipeline_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShader_type(SIGParser::Shader_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPso_param_id(SIGParser::Pso_param_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBool_type(SIGParser::Bool_typeContext *ctx) override {
    return visitChildren(ctx);
  }


};

