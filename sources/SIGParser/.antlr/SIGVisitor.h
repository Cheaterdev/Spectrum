
// Generated from SIG.g4 by ANTLR 4.11.1

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
    virtual std::any visitParse(SIGParser::ParseContext *context) = 0;

    virtual std::any visitBind_option(SIGParser::Bind_optionContext *context) = 0;

    virtual std::any visitOptions_assign(SIGParser::Options_assignContext *context) = 0;

    virtual std::any visitOption(SIGParser::OptionContext *context) = 0;

    virtual std::any visitOption_block(SIGParser::Option_blockContext *context) = 0;

    virtual std::any visitArray_count_id(SIGParser::Array_count_idContext *context) = 0;

    virtual std::any visitArray(SIGParser::ArrayContext *context) = 0;

    virtual std::any visitValue_declaration(SIGParser::Value_declarationContext *context) = 0;

    virtual std::any visitSlot_declaration(SIGParser::Slot_declarationContext *context) = 0;

    virtual std::any visitSampler_declaration(SIGParser::Sampler_declarationContext *context) = 0;

    virtual std::any visitDefine_declaration(SIGParser::Define_declarationContext *context) = 0;

    virtual std::any visitRtv_formats_declaration(SIGParser::Rtv_formats_declarationContext *context) = 0;

    virtual std::any visitBlends_declaration(SIGParser::Blends_declarationContext *context) = 0;

    virtual std::any visitPointer(SIGParser::PointerContext *context) = 0;

    virtual std::any visitPso_param(SIGParser::Pso_paramContext *context) = 0;

    virtual std::any visitClass_no_template(SIGParser::Class_no_templateContext *context) = 0;

    virtual std::any visitType_with_template(SIGParser::Type_with_templateContext *context) = 0;

    virtual std::any visitInherit_id(SIGParser::Inherit_idContext *context) = 0;

    virtual std::any visitName_id(SIGParser::Name_idContext *context) = 0;

    virtual std::any visitOption_id(SIGParser::Option_idContext *context) = 0;

    virtual std::any visitOwner_id(SIGParser::Owner_idContext *context) = 0;

    virtual std::any visitTemplate_id(SIGParser::Template_idContext *context) = 0;

    virtual std::any visitFunction_id(SIGParser::Function_idContext *context) = 0;

    virtual std::any visitValue_id(SIGParser::Value_idContext *context) = 0;

    virtual std::any visitValue_id_ignore(SIGParser::Value_id_ignoreContext *context) = 0;

    virtual std::any visitType_id(SIGParser::Type_idContext *context) = 0;

    virtual std::any visitInsert_block(SIGParser::Insert_blockContext *context) = 0;

    virtual std::any visitPath_id(SIGParser::Path_idContext *context) = 0;

    virtual std::any visitInherit(SIGParser::InheritContext *context) = 0;

    virtual std::any visitLayout_stat(SIGParser::Layout_statContext *context) = 0;

    virtual std::any visitLayout_block(SIGParser::Layout_blockContext *context) = 0;

    virtual std::any visitLayout_definition(SIGParser::Layout_definitionContext *context) = 0;

    virtual std::any visitTable_stat(SIGParser::Table_statContext *context) = 0;

    virtual std::any visitTable_block(SIGParser::Table_blockContext *context) = 0;

    virtual std::any visitTable_definition(SIGParser::Table_definitionContext *context) = 0;

    virtual std::any visitRt_color_declaration(SIGParser::Rt_color_declarationContext *context) = 0;

    virtual std::any visitRt_ds_declaration(SIGParser::Rt_ds_declarationContext *context) = 0;

    virtual std::any visitRt_stat(SIGParser::Rt_statContext *context) = 0;

    virtual std::any visitRt_block(SIGParser::Rt_blockContext *context) = 0;

    virtual std::any visitRt_definition(SIGParser::Rt_definitionContext *context) = 0;

    virtual std::any visitArray_value_holder(SIGParser::Array_value_holderContext *context) = 0;

    virtual std::any visitArray_value_ids(SIGParser::Array_value_idsContext *context) = 0;

    virtual std::any visitRoot_sig(SIGParser::Root_sigContext *context) = 0;

    virtual std::any visitShader(SIGParser::ShaderContext *context) = 0;

    virtual std::any visitCompute_pso_stat(SIGParser::Compute_pso_statContext *context) = 0;

    virtual std::any visitCompute_pso_block(SIGParser::Compute_pso_blockContext *context) = 0;

    virtual std::any visitCompute_pso_definition(SIGParser::Compute_pso_definitionContext *context) = 0;

    virtual std::any visitGraphics_pso_stat(SIGParser::Graphics_pso_statContext *context) = 0;

    virtual std::any visitGraphics_pso_block(SIGParser::Graphics_pso_blockContext *context) = 0;

    virtual std::any visitGraphics_pso_definition(SIGParser::Graphics_pso_definitionContext *context) = 0;

    virtual std::any visitRtx_pso_stat(SIGParser::Rtx_pso_statContext *context) = 0;

    virtual std::any visitRtx_pso_block(SIGParser::Rtx_pso_blockContext *context) = 0;

    virtual std::any visitRtx_pso_definition(SIGParser::Rtx_pso_definitionContext *context) = 0;

    virtual std::any visitNode_param_id(SIGParser::Node_param_idContext *context) = 0;

    virtual std::any visitNode_param(SIGParser::Node_paramContext *context) = 0;

    virtual std::any visitNode_output_decl(SIGParser::Node_output_declContext *context) = 0;

    virtual std::any visitNode_stat(SIGParser::Node_statContext *context) = 0;

    virtual std::any visitNode_block(SIGParser::Node_blockContext *context) = 0;

    virtual std::any visitNode_definition(SIGParser::Node_definitionContext *context) = 0;

    virtual std::any visitWorkgraph_pso_stat(SIGParser::Workgraph_pso_statContext *context) = 0;

    virtual std::any visitWorkgraph_pso_block(SIGParser::Workgraph_pso_blockContext *context) = 0;

    virtual std::any visitWorkgraph_pso_definition(SIGParser::Workgraph_pso_definitionContext *context) = 0;

    virtual std::any visitRtx_pass_stat(SIGParser::Rtx_pass_statContext *context) = 0;

    virtual std::any visitRtx_pass_block(SIGParser::Rtx_pass_blockContext *context) = 0;

    virtual std::any visitRtx_pass_definition(SIGParser::Rtx_pass_definitionContext *context) = 0;

    virtual std::any visitRtx_raygen_stat(SIGParser::Rtx_raygen_statContext *context) = 0;

    virtual std::any visitRtx_raygen_block(SIGParser::Rtx_raygen_blockContext *context) = 0;

    virtual std::any visitRtx_raygen_definition(SIGParser::Rtx_raygen_definitionContext *context) = 0;

    virtual std::any visitView_declaration(SIGParser::View_declarationContext *context) = 0;

    virtual std::any visitView_stat(SIGParser::View_statContext *context) = 0;

    virtual std::any visitView_block(SIGParser::View_blockContext *context) = 0;

    virtual std::any visitView_definition(SIGParser::View_definitionContext *context) = 0;

    virtual std::any visitPass_definition(SIGParser::Pass_definitionContext *context) = 0;

    virtual std::any visitPipeline_stat(SIGParser::Pipeline_statContext *context) = 0;

    virtual std::any visitPipeline_block(SIGParser::Pipeline_blockContext *context) = 0;

    virtual std::any visitPipeline_definition(SIGParser::Pipeline_definitionContext *context) = 0;

    virtual std::any visitShader_type(SIGParser::Shader_typeContext *context) = 0;

    virtual std::any visitPso_param_id(SIGParser::Pso_param_idContext *context) = 0;

    virtual std::any visitBool_type(SIGParser::Bool_typeContext *context) = 0;


};

