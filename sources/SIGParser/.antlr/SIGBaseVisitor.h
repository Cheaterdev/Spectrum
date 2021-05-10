
// Generated from sources/SIGParser/SIG.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"
#include "SIGVisitor.h"


/**
 * This class provides an empty implementation of SIGVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SIGBaseVisitor : public SIGVisitor {
public:

  virtual antlrcpp::Any visitParse(SIGParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBind_option(SIGParser::Bind_optionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOptions_assign(SIGParser::Options_assignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOption(SIGParser::OptionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOption_block(SIGParser::Option_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArray_count_id(SIGParser::Array_count_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArray(SIGParser::ArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue_declaration(SIGParser::Value_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSlot_declaration(SIGParser::Slot_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSampler_declaration(SIGParser::Sampler_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDefine_declaration(SIGParser::Define_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlends_declaration(SIGParser::Blends_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPso_param(SIGParser::Pso_paramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_with_template(SIGParser::Type_with_templateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInherit_id(SIGParser::Inherit_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitName_id(SIGParser::Name_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_id(SIGParser::Type_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOption_id(SIGParser::Option_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOwner_id(SIGParser::Owner_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTemplate_id(SIGParser::Template_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitValue_id(SIGParser::Value_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInsert_block(SIGParser::Insert_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPath_id(SIGParser::Path_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInherit(SIGParser::InheritContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLayout_stat(SIGParser::Layout_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLayout_block(SIGParser::Layout_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLayout_definition(SIGParser::Layout_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTable_stat(SIGParser::Table_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTable_block(SIGParser::Table_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTable_definition(SIGParser::Table_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRt_color_declaration(SIGParser::Rt_color_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRt_ds_declaration(SIGParser::Rt_ds_declarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRt_stat(SIGParser::Rt_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRt_block(SIGParser::Rt_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRt_definition(SIGParser::Rt_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArray_value_holder(SIGParser::Array_value_holderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArray_value_ids(SIGParser::Array_value_idsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRoot_sig(SIGParser::Root_sigContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShader(SIGParser::ShaderContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompute_pso_stat(SIGParser::Compute_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompute_pso_block(SIGParser::Compute_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompute_pso_definition(SIGParser::Compute_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphics_pso_stat(SIGParser::Graphics_pso_statContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphics_pso_block(SIGParser::Graphics_pso_blockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShader_type(SIGParser::Shader_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPso_param_id(SIGParser::Pso_param_idContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBool_type(SIGParser::Bool_typeContext *ctx) override {
    return visitChildren(ctx);
  }


};

