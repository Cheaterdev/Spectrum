
// Generated from SIG.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include "SIGParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SIGParser.
 */
class  SIGVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SIGParser.
   */
    virtual antlrcpp::Any visitParse(SIGParser::ParseContext *context) = 0;

    virtual antlrcpp::Any visitBind_option(SIGParser::Bind_optionContext *context) = 0;

    virtual antlrcpp::Any visitOptions_assign(SIGParser::Options_assignContext *context) = 0;

    virtual antlrcpp::Any visitOption(SIGParser::OptionContext *context) = 0;

    virtual antlrcpp::Any visitOption_block(SIGParser::Option_blockContext *context) = 0;

    virtual antlrcpp::Any visitArray_count_id(SIGParser::Array_count_idContext *context) = 0;

    virtual antlrcpp::Any visitArray(SIGParser::ArrayContext *context) = 0;

    virtual antlrcpp::Any visitValue_declaration(SIGParser::Value_declarationContext *context) = 0;

    virtual antlrcpp::Any visitSlot_declaration(SIGParser::Slot_declarationContext *context) = 0;

    virtual antlrcpp::Any visitSampler_declaration(SIGParser::Sampler_declarationContext *context) = 0;

    virtual antlrcpp::Any visitDefine_declaration(SIGParser::Define_declarationContext *context) = 0;

    virtual antlrcpp::Any visitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *context) = 0;

    virtual antlrcpp::Any visitBlends_declaration(SIGParser::Blends_declarationContext *context) = 0;

    virtual antlrcpp::Any visitPso_param(SIGParser::Pso_paramContext *context) = 0;

    virtual antlrcpp::Any visitType_with_template(SIGParser::Type_with_templateContext *context) = 0;

    virtual antlrcpp::Any visitInherit_id(SIGParser::Inherit_idContext *context) = 0;

    virtual antlrcpp::Any visitName_id(SIGParser::Name_idContext *context) = 0;

    virtual antlrcpp::Any visitType_id(SIGParser::Type_idContext *context) = 0;

    virtual antlrcpp::Any visitOption_id(SIGParser::Option_idContext *context) = 0;

    virtual antlrcpp::Any visitOwner_id(SIGParser::Owner_idContext *context) = 0;

    virtual antlrcpp::Any visitTemplate_id(SIGParser::Template_idContext *context) = 0;

    virtual antlrcpp::Any visitValue_id(SIGParser::Value_idContext *context) = 0;

    virtual antlrcpp::Any visitInsert_block(SIGParser::Insert_blockContext *context) = 0;

    virtual antlrcpp::Any visitPath_id(SIGParser::Path_idContext *context) = 0;

    virtual antlrcpp::Any visitInherit(SIGParser::InheritContext *context) = 0;

    virtual antlrcpp::Any visitLayout_stat(SIGParser::Layout_statContext *context) = 0;

    virtual antlrcpp::Any visitLayout_block(SIGParser::Layout_blockContext *context) = 0;

    virtual antlrcpp::Any visitLayout_definition(SIGParser::Layout_definitionContext *context) = 0;

    virtual antlrcpp::Any visitTable_stat(SIGParser::Table_statContext *context) = 0;

    virtual antlrcpp::Any visitTable_block(SIGParser::Table_blockContext *context) = 0;

    virtual antlrcpp::Any visitTable_definition(SIGParser::Table_definitionContext *context) = 0;

    virtual antlrcpp::Any visitRt_color_declaration(SIGParser::Rt_color_declarationContext *context) = 0;

    virtual antlrcpp::Any visitRt_ds_declaration(SIGParser::Rt_ds_declarationContext *context) = 0;

    virtual antlrcpp::Any visitRt_stat(SIGParser::Rt_statContext *context) = 0;

    virtual antlrcpp::Any visitRt_block(SIGParser::Rt_blockContext *context) = 0;

    virtual antlrcpp::Any visitRt_definition(SIGParser::Rt_definitionContext *context) = 0;

    virtual antlrcpp::Any visitArray_value_holder(SIGParser::Array_value_holderContext *context) = 0;

    virtual antlrcpp::Any visitArray_value_ids(SIGParser::Array_value_idsContext *context) = 0;

    virtual antlrcpp::Any visitRoot_sig(SIGParser::Root_sigContext *context) = 0;

    virtual antlrcpp::Any visitShader(SIGParser::ShaderContext *context) = 0;

    virtual antlrcpp::Any visitCompute_pso_stat(SIGParser::Compute_pso_statContext *context) = 0;

    virtual antlrcpp::Any visitCompute_pso_block(SIGParser::Compute_pso_blockContext *context) = 0;

    virtual antlrcpp::Any visitCompute_pso_definition(SIGParser::Compute_pso_definitionContext *context) = 0;

    virtual antlrcpp::Any visitGraphics_pso_stat(SIGParser::Graphics_pso_statContext *context) = 0;

    virtual antlrcpp::Any visitGraphics_pso_block(SIGParser::Graphics_pso_blockContext *context) = 0;

    virtual antlrcpp::Any visitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *context) = 0;

    virtual antlrcpp::Any visitShader_type(SIGParser::Shader_typeContext *context) = 0;

    virtual antlrcpp::Any visitPso_param_id(SIGParser::Pso_param_idContext *context) = 0;

    virtual antlrcpp::Any visitBool_type(SIGParser::Bool_typeContext *context) = 0;


};

