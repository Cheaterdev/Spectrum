
// Generated from SIG.g4 by ANTLR 4.7.1

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

  virtual void enterOptions(SIGParser::OptionsContext *ctx) = 0;
  virtual void exitOptions(SIGParser::OptionsContext *ctx) = 0;

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

  virtual void enterTemplated(SIGParser::TemplatedContext *ctx) = 0;
  virtual void exitTemplated(SIGParser::TemplatedContext *ctx) = 0;

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


};

