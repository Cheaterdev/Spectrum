
// Generated from SIG.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"




class  SIGParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, OR = 5, AND = 6, EQ = 7, NEQ = 8, 
    GT = 9, LT = 10, GTEQ = 11, LTEQ = 12, PLUS = 13, MINUS = 14, MULT = 15, 
    DIV = 16, MOD = 17, POW = 18, NOT = 19, SCOL = 20, ASSIGN = 21, OPAR = 22, 
    CPAR = 23, OBRACE = 24, CBRACE = 25, OSBRACE = 26, CSBRACE = 27, TRUE = 28, 
    FALSE = 29, LOG = 30, LAYOUT = 31, STRUCT = 32, SLOT = 33, RT = 34, 
    RTV = 35, DSV = 36, ID = 37, INT_SCALAR = 38, FLOAT_SCALAR = 39, STRING = 40, 
    COMMENT = 41, SPACE = 42, INSERT_START = 43, INSERT_END = 44, INSERT_BLOCK = 45
  };

  enum {
    RuleParse = 0, RuleBind_option = 1, RuleOptions_assign = 2, RuleOptions = 3, 
    RuleOption_block = 4, RuleArray_count_id = 5, RuleArray = 6, RuleValue_declaration = 7, 
    RuleSlot_declaration = 8, RuleSampler_declaration = 9, RuleTemplated = 10, 
    RuleType_with_template = 11, RuleInherit_id = 12, RuleName_id = 13, 
    RuleType_id = 14, RuleOption_id = 15, RuleOwner_id = 16, RuleTemplate_id = 17, 
    RuleValue_id = 18, RuleInsert_block = 19, RuleInherit = 20, RuleLayout_stat = 21, 
    RuleLayout_block = 22, RuleLayout_definition = 23, RuleTable_stat = 24, 
    RuleTable_block = 25, RuleTable_definition = 26, RuleRt_color_declaration = 27, 
    RuleRt_ds_declaration = 28, RuleRt_stat = 29, RuleRt_block = 30, RuleRt_definition = 31
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
  class OptionsContext;
  class Option_blockContext;
  class Array_count_idContext;
  class ArrayContext;
  class Value_declarationContext;
  class Slot_declarationContext;
  class Sampler_declarationContext;
  class TemplatedContext;
  class Type_with_templateContext;
  class Inherit_idContext;
  class Name_idContext;
  class Type_idContext;
  class Option_idContext;
  class Owner_idContext;
  class Template_idContext;
  class Value_idContext;
  class Insert_blockContext;
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
    Name_idContext *name_id();
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

  class  OptionsContext : public antlr4::ParserRuleContext {
  public:
    OptionsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    Options_assignContext *options_assign();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OptionsContext* options();

  class  Option_blockContext : public antlr4::ParserRuleContext {
  public:
    Option_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OSBRACE();
    antlr4::tree::TerminalNode *CSBRACE();
    std::vector<OptionsContext *> options();
    OptionsContext* options(size_t i);

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

  class  TemplatedContext : public antlr4::ParserRuleContext {
  public:
    TemplatedContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LT();
    Template_idContext *template_id();
    antlr4::tree::TerminalNode *GT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TemplatedContext* templated();

  class  Type_with_templateContext : public antlr4::ParserRuleContext {
  public:
    Type_with_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    TemplatedContext *templated();

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

