
// Generated from SIG.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  SIGParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, OR = 35, AND = 36, EQ = 37, NEQ = 38, GT = 39, 
    LT = 40, GTEQ = 41, LTEQ = 42, PLUS = 43, MINUS = 44, MULT = 45, DIV = 46, 
    MOD = 47, POW = 48, NOT = 49, SCOL = 50, ASSIGN = 51, OPAR = 52, CPAR = 53, 
    OBRACE = 54, CBRACE = 55, OSBRACE = 56, CSBRACE = 57, TRUE = 58, FALSE = 59, 
    LOG = 60, LAYOUT = 61, STRUCT = 62, COMPUTE_PSO = 63, GRAPHICS_PSO = 64, 
    RAYTRACE_PSO = 65, RAYTRACE_RAYGEN = 66, RAYTRACE_PASS = 67, SLOT = 68, 
    RT = 69, RTV = 70, DSV = 71, ROOTSIG = 72, ID = 73, INT_SCALAR = 74, 
    FLOAT_SCALAR = 75, STRING = 76, COMMENT = 77, SPACE = 78, INSERT_START = 79, 
    INSERT_END = 80, INSERT_BLOCK = 81
  };

  enum {
    RuleParse = 0, RuleBind_option = 1, RuleOptions_assign = 2, RuleOption = 3, 
    RuleOption_block = 4, RuleArray_count_id = 5, RuleArray = 6, RuleValue_declaration = 7, 
    RuleSlot_declaration = 8, RuleSampler_declaration = 9, RuleDefine_declaration = 10, 
    RuleRtv_formats_declaration = 11, RuleBlends_declaration = 12, RulePso_param = 13, 
    RuleClass_no_template = 14, RuleType_with_template = 15, RuleInherit_id = 16, 
    RuleName_id = 17, RuleType_id = 18, RuleOption_id = 19, RuleOwner_id = 20, 
    RuleTemplate_id = 21, RuleValue_id = 22, RuleInsert_block = 23, RulePath_id = 24, 
    RuleInherit = 25, RuleLayout_stat = 26, RuleLayout_block = 27, RuleLayout_definition = 28, 
    RuleTable_stat = 29, RuleTable_block = 30, RuleTable_definition = 31, 
    RuleRt_color_declaration = 32, RuleRt_ds_declaration = 33, RuleRt_stat = 34, 
    RuleRt_block = 35, RuleRt_definition = 36, RuleArray_value_holder = 37, 
    RuleArray_value_ids = 38, RuleRoot_sig = 39, RuleShader = 40, RuleCompute_pso_stat = 41, 
    RuleCompute_pso_block = 42, RuleCompute_pso_definition = 43, RuleGraphics_pso_stat = 44, 
    RuleGraphics_pso_block = 45, RuleGraphics_pso_definition = 46, RuleRtx_pso_stat = 47, 
    RuleRtx_pso_block = 48, RuleRtx_pso_definition = 49, RuleRtx_pass_stat = 50, 
    RuleRtx_pass_block = 51, RuleRtx_pass_definition = 52, RuleRtx_raygen_stat = 53, 
    RuleRtx_raygen_block = 54, RuleRtx_raygen_definition = 55, RuleShader_type = 56, 
    RulePso_param_id = 57, RuleBool_type = 58
  };

  explicit SIGParser(antlr4::TokenStream *input);

  SIGParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~SIGParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ParseContext;
  class Bind_optionContext;
  class Options_assignContext;
  class OptionContext;
  class Option_blockContext;
  class Array_count_idContext;
  class ArrayContext;
  class Value_declarationContext;
  class Slot_declarationContext;
  class Sampler_declarationContext;
  class Define_declarationContext;
  class Rtv_formats_declarationContext;
  class Blends_declarationContext;
  class Pso_paramContext;
  class Class_no_templateContext;
  class Type_with_templateContext;
  class Inherit_idContext;
  class Name_idContext;
  class Type_idContext;
  class Option_idContext;
  class Owner_idContext;
  class Template_idContext;
  class Value_idContext;
  class Insert_blockContext;
  class Path_idContext;
  class InheritContext;
  class Layout_statContext;
  class Layout_blockContext;
  class Layout_definitionContext;
  class Table_statContext;
  class Table_blockContext;
  class Table_definitionContext;
  class Rt_color_declarationContext;
  class Rt_ds_declarationContext;
  class Rt_statContext;
  class Rt_blockContext;
  class Rt_definitionContext;
  class Array_value_holderContext;
  class Array_value_idsContext;
  class Root_sigContext;
  class ShaderContext;
  class Compute_pso_statContext;
  class Compute_pso_blockContext;
  class Compute_pso_definitionContext;
  class Graphics_pso_statContext;
  class Graphics_pso_blockContext;
  class Graphics_pso_definitionContext;
  class Rtx_pso_statContext;
  class Rtx_pso_blockContext;
  class Rtx_pso_definitionContext;
  class Rtx_pass_statContext;
  class Rtx_pass_blockContext;
  class Rtx_pass_definitionContext;
  class Rtx_raygen_statContext;
  class Rtx_raygen_blockContext;
  class Rtx_raygen_definitionContext;
  class Shader_typeContext;
  class Pso_param_idContext;
  class Bool_typeContext; 

  class  ParseContext : public antlr4::ParserRuleContext {
  public:
    ParseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<Layout_definitionContext *> layout_definition();
    Layout_definitionContext* layout_definition(size_t i);
    std::vector<Table_definitionContext *> table_definition();
    Table_definitionContext* table_definition(size_t i);
    std::vector<Rt_definitionContext *> rt_definition();
    Rt_definitionContext* rt_definition(size_t i);
    std::vector<Compute_pso_definitionContext *> compute_pso_definition();
    Compute_pso_definitionContext* compute_pso_definition(size_t i);
    std::vector<Graphics_pso_definitionContext *> graphics_pso_definition();
    Graphics_pso_definitionContext* graphics_pso_definition(size_t i);
    std::vector<Rtx_pso_definitionContext *> rtx_pso_definition();
    Rtx_pso_definitionContext* rtx_pso_definition(size_t i);
    std::vector<Rtx_pass_definitionContext *> rtx_pass_definition();
    Rtx_pass_definitionContext* rtx_pass_definition(size_t i);
    std::vector<Rtx_raygen_definitionContext *> rtx_raygen_definition();
    Rtx_raygen_definitionContext* rtx_raygen_definition(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMENT();
    antlr4::tree::TerminalNode* COMMENT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ParseContext* parse();

  class  Bind_optionContext : public antlr4::ParserRuleContext {
  public:
    Bind_optionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_idContext *value_id();
    Owner_idContext *owner_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Bind_optionContext* bind_option();

  class  Options_assignContext : public antlr4::ParserRuleContext {
  public:
    Options_assignContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    Bind_optionContext *bind_option();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Options_assignContext* options_assign();

  class  OptionContext : public antlr4::ParserRuleContext {
  public:
    OptionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    Options_assignContext *options_assign();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  OptionContext* option();

  class  Option_blockContext : public antlr4::ParserRuleContext {
  public:
    Option_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OSBRACE();
    std::vector<OptionContext *> option();
    OptionContext* option(size_t i);
    antlr4::tree::TerminalNode *CSBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Option_blockContext* option_block();

  class  Array_count_idContext : public antlr4::ParserRuleContext {
  public:
    Array_count_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT_SCALAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Array_count_idContext* array_count_id();

  class  ArrayContext : public antlr4::ParserRuleContext {
  public:
    ArrayContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OSBRACE();
    antlr4::tree::TerminalNode *CSBRACE();
    Array_count_idContext *array_count_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ArrayContext* array();

  class  Value_declarationContext : public antlr4::ParserRuleContext {
  public:
    Value_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_idContext *type_id();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    ArrayContext *array();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Value_declarationContext* value_declaration();

  class  Slot_declarationContext : public antlr4::ParserRuleContext {
  public:
    Slot_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SLOT();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Slot_declarationContext* slot_declaration();

  class  Sampler_declarationContext : public antlr4::ParserRuleContext {
  public:
    Sampler_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Sampler_declarationContext* sampler_declaration();

  class  Define_declarationContext : public antlr4::ParserRuleContext {
  public:
    Define_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    antlr4::tree::TerminalNode *ASSIGN();
    Array_value_idsContext *array_value_ids();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Define_declarationContext* define_declaration();

  class  Rtv_formats_declarationContext : public antlr4::ParserRuleContext {
  public:
    Rtv_formats_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    Array_value_idsContext *array_value_ids();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtv_formats_declarationContext* rtv_formats_declaration();

  class  Blends_declarationContext : public antlr4::ParserRuleContext {
  public:
    Blends_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ASSIGN();
    Array_value_idsContext *array_value_ids();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Blends_declarationContext* blends_declaration();

  class  Pso_paramContext : public antlr4::ParserRuleContext {
  public:
    Pso_paramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Pso_param_idContext *pso_param_id();
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Pso_paramContext* pso_param();

  class  Class_no_templateContext : public antlr4::ParserRuleContext {
  public:
    Class_no_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Class_no_templateContext* class_no_template();

  class  Type_with_templateContext : public antlr4::ParserRuleContext {
  public:
    Type_with_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_no_templateContext *class_no_template();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *GT();
    std::vector<Template_idContext *> template_id();
    Template_idContext* template_id(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Type_with_templateContext* type_with_template();

  class  Inherit_idContext : public antlr4::ParserRuleContext {
  public:
    Inherit_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Inherit_idContext* inherit_id();

  class  Name_idContext : public antlr4::ParserRuleContext {
  public:
    Name_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Name_idContext* name_id();

  class  Type_idContext : public antlr4::ParserRuleContext {
  public:
    Type_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_with_templateContext *type_with_template();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Type_idContext* type_id();

  class  Option_idContext : public antlr4::ParserRuleContext {
  public:
    Option_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Option_idContext* option_id();

  class  Owner_idContext : public antlr4::ParserRuleContext {
  public:
    Owner_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Owner_idContext* owner_id();

  class  Template_idContext : public antlr4::ParserRuleContext {
  public:
    Template_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Template_idContext* template_id();

  class  Value_idContext : public antlr4::ParserRuleContext {
  public:
    Value_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *INT_SCALAR();
    antlr4::tree::TerminalNode *FLOAT_SCALAR();
    Bool_typeContext *bool_type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Value_idContext* value_id();

  class  Insert_blockContext : public antlr4::ParserRuleContext {
  public:
    Insert_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INSERT_BLOCK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Insert_blockContext* insert_block();

  class  Path_idContext : public antlr4::ParserRuleContext {
  public:
    Path_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DIV();
    antlr4::tree::TerminalNode* DIV(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Path_idContext* path_id();

  class  InheritContext : public antlr4::ParserRuleContext {
  public:
    InheritContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Inherit_idContext *> inherit_id();
    Inherit_idContext* inherit_id(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  InheritContext* inherit();

  class  Layout_statContext : public antlr4::ParserRuleContext {
  public:
    Layout_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Slot_declarationContext *slot_declaration();
    Sampler_declarationContext *sampler_declaration();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Layout_statContext* layout_stat();

  class  Layout_blockContext : public antlr4::ParserRuleContext {
  public:
    Layout_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Layout_statContext *> layout_stat();
    Layout_statContext* layout_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Layout_blockContext* layout_block();

  class  Layout_definitionContext : public antlr4::ParserRuleContext {
  public:
    Layout_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LAYOUT();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Layout_blockContext *layout_block();
    antlr4::tree::TerminalNode *CBRACE();
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Layout_definitionContext* layout_definition();

  class  Table_statContext : public antlr4::ParserRuleContext {
  public:
    Table_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_declarationContext *value_declaration();
    Insert_blockContext *insert_block();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Table_statContext* table_stat();

  class  Table_blockContext : public antlr4::ParserRuleContext {
  public:
    Table_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Table_statContext *> table_stat();
    Table_statContext* table_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Table_blockContext* table_block();

  class  Table_definitionContext : public antlr4::ParserRuleContext {
  public:
    Table_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STRUCT();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Table_blockContext *table_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Table_definitionContext* table_definition();

  class  Rt_color_declarationContext : public antlr4::ParserRuleContext {
  public:
    Rt_color_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_idContext *type_id();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rt_color_declarationContext* rt_color_declaration();

  class  Rt_ds_declarationContext : public antlr4::ParserRuleContext {
  public:
    Rt_ds_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DSV();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rt_ds_declarationContext* rt_ds_declaration();

  class  Rt_statContext : public antlr4::ParserRuleContext {
  public:
    Rt_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Rt_color_declarationContext *rt_color_declaration();
    Rt_ds_declarationContext *rt_ds_declaration();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rt_statContext* rt_stat();

  class  Rt_blockContext : public antlr4::ParserRuleContext {
  public:
    Rt_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Rt_statContext *> rt_stat();
    Rt_statContext* rt_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rt_blockContext* rt_block();

  class  Rt_definitionContext : public antlr4::ParserRuleContext {
  public:
    Rt_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RT();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Rt_blockContext *rt_block();
    antlr4::tree::TerminalNode *CBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rt_definitionContext* rt_definition();

  class  Array_value_holderContext : public antlr4::ParserRuleContext {
  public:
    Array_value_holderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_idContext *value_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Array_value_holderContext* array_value_holder();

  class  Array_value_idsContext : public antlr4::ParserRuleContext {
  public:
    Array_value_idsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OBRACE();
    std::vector<Array_value_holderContext *> array_value_holder();
    Array_value_holderContext* array_value_holder(size_t i);
    antlr4::tree::TerminalNode *CBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Array_value_idsContext* array_value_ids();

  class  Root_sigContext : public antlr4::ParserRuleContext {
  public:
    Root_sigContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ROOTSIG();
    antlr4::tree::TerminalNode *ASSIGN();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Root_sigContext* root_sig();

  class  ShaderContext : public antlr4::ParserRuleContext {
  public:
    ShaderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Shader_typeContext *shader_type();
    antlr4::tree::TerminalNode *ASSIGN();
    Path_idContext *path_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ShaderContext* shader();

  class  Compute_pso_statContext : public antlr4::ParserRuleContext {
  public:
    Compute_pso_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Root_sigContext *root_sig();
    ShaderContext *shader();
    Define_declarationContext *define_declaration();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Compute_pso_statContext* compute_pso_stat();

  class  Compute_pso_blockContext : public antlr4::ParserRuleContext {
  public:
    Compute_pso_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Compute_pso_statContext *> compute_pso_stat();
    Compute_pso_statContext* compute_pso_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Compute_pso_blockContext* compute_pso_block();

  class  Compute_pso_definitionContext : public antlr4::ParserRuleContext {
  public:
    Compute_pso_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMPUTE_PSO();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Compute_pso_blockContext *compute_pso_block();
    antlr4::tree::TerminalNode *CBRACE();
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Compute_pso_definitionContext* compute_pso_definition();

  class  Graphics_pso_statContext : public antlr4::ParserRuleContext {
  public:
    Graphics_pso_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Root_sigContext *root_sig();
    ShaderContext *shader();
    Define_declarationContext *define_declaration();
    Rtv_formats_declarationContext *rtv_formats_declaration();
    Blends_declarationContext *blends_declaration();
    Pso_paramContext *pso_param();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Graphics_pso_statContext* graphics_pso_stat();

  class  Graphics_pso_blockContext : public antlr4::ParserRuleContext {
  public:
    Graphics_pso_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Graphics_pso_statContext *> graphics_pso_stat();
    Graphics_pso_statContext* graphics_pso_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Graphics_pso_blockContext* graphics_pso_block();

  class  Graphics_pso_definitionContext : public antlr4::ParserRuleContext {
  public:
    Graphics_pso_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GRAPHICS_PSO();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Graphics_pso_blockContext *graphics_pso_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Graphics_pso_definitionContext* graphics_pso_definition();

  class  Rtx_pso_statContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pso_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Root_sigContext *root_sig();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pso_statContext* rtx_pso_stat();

  class  Rtx_pso_blockContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pso_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Rtx_pso_statContext *> rtx_pso_stat();
    Rtx_pso_statContext* rtx_pso_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pso_blockContext* rtx_pso_block();

  class  Rtx_pso_definitionContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pso_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RAYTRACE_PSO();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Rtx_pso_blockContext *rtx_pso_block();
    antlr4::tree::TerminalNode *CBRACE();
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pso_definitionContext* rtx_pso_definition();

  class  Rtx_pass_statContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pass_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ShaderContext *shader();
    antlr4::tree::TerminalNode *COMMENT();
    Pso_paramContext *pso_param();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pass_statContext* rtx_pass_stat();

  class  Rtx_pass_blockContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pass_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Rtx_pass_statContext *> rtx_pass_stat();
    Rtx_pass_statContext* rtx_pass_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pass_blockContext* rtx_pass_block();

  class  Rtx_pass_definitionContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pass_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RAYTRACE_PASS();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Rtx_pass_blockContext *rtx_pass_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_pass_definitionContext* rtx_pass_definition();

  class  Rtx_raygen_statContext : public antlr4::ParserRuleContext {
  public:
    Rtx_raygen_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ShaderContext *shader();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_raygen_statContext* rtx_raygen_stat();

  class  Rtx_raygen_blockContext : public antlr4::ParserRuleContext {
  public:
    Rtx_raygen_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Rtx_raygen_statContext *> rtx_raygen_stat();
    Rtx_raygen_statContext* rtx_raygen_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_raygen_blockContext* rtx_raygen_block();

  class  Rtx_raygen_definitionContext : public antlr4::ParserRuleContext {
  public:
    Rtx_raygen_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *RAYTRACE_RAYGEN();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Rtx_raygen_blockContext *rtx_raygen_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Rtx_raygen_definitionContext* rtx_raygen_definition();

  class  Shader_typeContext : public antlr4::ParserRuleContext {
  public:
    Shader_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Shader_typeContext* shader_type();

  class  Pso_param_idContext : public antlr4::ParserRuleContext {
  public:
    Pso_param_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Pso_param_idContext* pso_param_id();

  class  Bool_typeContext : public antlr4::ParserRuleContext {
  public:
    Bool_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Bool_typeContext* bool_type();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

