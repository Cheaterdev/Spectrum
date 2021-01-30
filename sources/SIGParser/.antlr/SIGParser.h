
// Generated from SIG.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  SIGParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, OR = 26, 
    AND = 27, EQ = 28, NEQ = 29, GT = 30, LT = 31, GTEQ = 32, LTEQ = 33, 
    PLUS = 34, MINUS = 35, MULT = 36, DIV = 37, MOD = 38, POW = 39, NOT = 40, 
    SCOL = 41, ASSIGN = 42, OPAR = 43, CPAR = 44, OBRACE = 45, CBRACE = 46, 
    OSBRACE = 47, CSBRACE = 48, TRUE = 49, FALSE = 50, LOG = 51, LAYOUT = 52, 
    STRUCT = 53, COMPUTE_PSO = 54, GRAPHICS_PSO = 55, SLOT = 56, RT = 57, 
    RTV = 58, DSV = 59, ROOTSIG = 60, ID = 61, INT_SCALAR = 62, FLOAT_SCALAR = 63, 
    STRING = 64, COMMENT = 65, SPACE = 66, INSERT_START = 67, INSERT_END = 68, 
    INSERT_BLOCK = 69
  };

  enum {
    RuleParse = 0, RuleBind_option = 1, RuleOptions_assign = 2, RuleOption = 3, 
    RuleOption_block = 4, RuleArray_count_id = 5, RuleArray = 6, RuleValue_declaration = 7, 
    RuleSlot_declaration = 8, RuleSampler_declaration = 9, RuleDefine_declaration = 10, 
    RuleRtv_formats_declaration = 11, RuleBlends_declaration = 12, RulePso_param = 13, 
    RuleType_with_template = 14, RuleInherit_id = 15, RuleName_id = 16, 
    RuleType_id = 17, RuleOption_id = 18, RuleOwner_id = 19, RuleTemplate_id = 20, 
    RuleValue_id = 21, RuleInsert_block = 22, RulePath_id = 23, RuleInherit = 24, 
    RuleLayout_stat = 25, RuleLayout_block = 26, RuleLayout_definition = 27, 
    RuleTable_stat = 28, RuleTable_block = 29, RuleTable_definition = 30, 
    RuleRt_color_declaration = 31, RuleRt_ds_declaration = 32, RuleRt_stat = 33, 
    RuleRt_block = 34, RuleRt_definition = 35, RuleArray_value_holder = 36, 
    RuleArray_value_ids = 37, RuleRoot_sig = 38, RuleShader = 39, RuleCompute_pso_stat = 40, 
    RuleCompute_pso_block = 41, RuleCompute_pso_definition = 42, RuleGraphics_pso_stat = 43, 
    RuleGraphics_pso_block = 44, RuleGraphics_pso_definition = 45, RuleShader_type = 46, 
    RulePso_param_id = 47, RuleBool_type = 48
  };

  SIGParser(antlr4::TokenStream *input);
  ~SIGParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


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
    std::vector<antlr4::tree::TerminalNode *> COMMENT();
    antlr4::tree::TerminalNode* COMMENT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Option_blockContext* option_block();

  class  Array_count_idContext : public antlr4::ParserRuleContext {
  public:
    Array_count_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT_SCALAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pso_paramContext* pso_param();

  class  Type_with_templateContext : public antlr4::ParserRuleContext {
  public:
    Type_with_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *LT();
    Template_idContext *template_id();
    antlr4::tree::TerminalNode *GT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Type_with_templateContext* type_with_template();

  class  Inherit_idContext : public antlr4::ParserRuleContext {
  public:
    Inherit_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Inherit_idContext* inherit_id();

  class  Name_idContext : public antlr4::ParserRuleContext {
  public:
    Name_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Name_idContext* name_id();

  class  Type_idContext : public antlr4::ParserRuleContext {
  public:
    Type_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_with_templateContext *type_with_template();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Type_idContext* type_id();

  class  Option_idContext : public antlr4::ParserRuleContext {
  public:
    Option_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Option_idContext* option_id();

  class  Owner_idContext : public antlr4::ParserRuleContext {
  public:
    Owner_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Owner_idContext* owner_id();

  class  Template_idContext : public antlr4::ParserRuleContext {
  public:
    Template_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Value_idContext* value_id();

  class  Insert_blockContext : public antlr4::ParserRuleContext {
  public:
    Insert_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INSERT_BLOCK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Rt_definitionContext* rt_definition();

  class  Array_value_holderContext : public antlr4::ParserRuleContext {
  public:
    Array_value_holderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_idContext *value_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Graphics_pso_definitionContext* graphics_pso_definition();

  class  Shader_typeContext : public antlr4::ParserRuleContext {
  public:
    Shader_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Shader_typeContext* shader_type();

  class  Pso_param_idContext : public antlr4::ParserRuleContext {
  public:
    Pso_param_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Bool_typeContext* bool_type();


private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

