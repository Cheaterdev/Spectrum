
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "PrismVisitor.h"


/**
 * This class provides an empty implementation of PrismVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  PrismBaseVisitor : public PrismVisitor {
public:

  virtual std::any visitParse(PrismParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConst_definition(PrismParser::Const_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBind_option(PrismParser::Bind_optionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond_expr(PrismParser::Cond_exprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond_term(PrismParser::Cond_termContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualified_ref(PrismParser::Qualified_refContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMember_ref(PrismParser::Member_refContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond_op(PrismParser::Cond_opContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFlag_value_holder(PrismParser::Flag_value_holderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRaw_value(PrismParser::Raw_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOptions_assign(PrismParser::Options_assignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption(PrismParser::OptionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption_block(PrismParser::Option_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_count_id(PrismParser::Array_count_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray(PrismParser::ArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_declaration(PrismParser::Value_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSlot_declaration(PrismParser::Slot_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSampler_declaration(PrismParser::Sampler_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefine_declaration(PrismParser::Define_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtv_formats_declaration(PrismParser::Rtv_formats_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlends_declaration(PrismParser::Blends_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPointer(PrismParser::PointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPso_param(PrismParser::Pso_paramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClass_no_template(PrismParser::Class_no_templateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_with_template(PrismParser::Type_with_templateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInherit_id(PrismParser::Inherit_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName_id(PrismParser::Name_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOption_id(PrismParser::Option_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOwner_id(PrismParser::Owner_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTemplate_id(PrismParser::Template_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_id(PrismParser::Function_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_id(PrismParser::Value_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_id_ignore(PrismParser::Value_id_ignoreContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_id(PrismParser::Type_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsert_block(PrismParser::Insert_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShader_path(PrismParser::Shader_pathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInherit(PrismParser::InheritContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_stat(PrismParser::Layout_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_block(PrismParser::Layout_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLayout_definition(PrismParser::Layout_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_stat(PrismParser::Table_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_definition(PrismParser::Function_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_params(PrismParser::Function_paramsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_semantic(PrismParser::Function_semanticContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_block(PrismParser::Table_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_definition(PrismParser::Table_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_color_declaration(PrismParser::Rt_color_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_ds_declaration(PrismParser::Rt_ds_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_stat(PrismParser::Rt_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_block(PrismParser::Rt_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRt_definition(PrismParser::Rt_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_value_holder(PrismParser::Array_value_holderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArray_value_ids(PrismParser::Array_value_idsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRoot_sig(PrismParser::Root_sigContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShader(PrismParser::ShaderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_stat(PrismParser::Compute_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_block(PrismParser::Compute_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompute_pso_definition(PrismParser::Compute_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_stat(PrismParser::Graphics_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_block(PrismParser::Graphics_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphics_pso_definition(PrismParser::Graphics_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_stat(PrismParser::Rtx_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_block(PrismParser::Rtx_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pso_definition(PrismParser::Rtx_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_param_id(PrismParser::Node_param_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_param(PrismParser::Node_paramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_output_decl(PrismParser::Node_output_declContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_stat(PrismParser::Node_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_block(PrismParser::Node_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode_definition(PrismParser::Node_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_stat(PrismParser::Workgraph_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_block(PrismParser::Workgraph_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWorkgraph_pso_definition(PrismParser::Workgraph_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_stat(PrismParser::Rtx_pass_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_block(PrismParser::Rtx_pass_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_pass_definition(PrismParser::Rtx_pass_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_stat(PrismParser::Rtx_raygen_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_block(PrismParser::Rtx_raygen_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRtx_raygen_definition(PrismParser::Rtx_raygen_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_declaration(PrismParser::View_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_stat(PrismParser::View_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_block(PrismParser::View_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_definition(PrismParser::View_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPass_definition(PrismParser::Pass_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_stat(PrismParser::Pipeline_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_block(PrismParser::Pipeline_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPipeline_definition(PrismParser::Pipeline_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnum_value_declaration(PrismParser::Enum_value_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnum_stat(PrismParser::Enum_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnum_block(PrismParser::Enum_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnum_definition(PrismParser::Enum_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShader_type(PrismParser::Shader_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPso_param_id(PrismParser::Pso_param_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBool_type(PrismParser::Bool_typeContext *ctx) override {
    return visitChildren(ctx);
  }


};

