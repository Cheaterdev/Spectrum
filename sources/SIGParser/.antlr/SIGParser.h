
// Generated from sources/SIGParser/SIG.g4 by ANTLR 4.11.1

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
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, OR = 42, AND = 43, EQ = 44, NEQ = 45, 
    GT = 46, LT = 47, GTEQ = 48, LTEQ = 49, PLUS = 50, MINUS = 51, DIV = 52, 
    MOD = 53, POW = 54, NOT = 55, SCOL = 56, ASSIGN = 57, OPAR = 58, CPAR = 59, 
    OBRACE = 60, CBRACE = 61, OSBRACE = 62, CSBRACE = 63, TRUE = 64, FALSE = 65, 
    LOG = 66, LAYOUT = 67, STRUCT = 68, COMPUTE_PSO = 69, GRAPHICS_PSO = 70, 
    RAYTRACE_PSO = 71, WORKGRAPH_PSO = 72, NODE = 73, NODE_OUTPUT = 74, 
    RAYTRACE_RAYGEN = 75, RAYTRACE_PASS = 76, PASS = 77, VIEW = 78, PIPELINE = 79, 
    SLOT = 80, RT = 81, RTV = 82, DSV = 83, ROOTSIG = 84, ENUM = 85, ID = 86, 
    INT_SCALAR = 87, FLOAT_SCALAR = 88, STRING = 89, COMMENT = 90, SPACE = 91, 
    POINTER = 92, INSERT_START = 93, INSERT_END = 94, INSERT_BLOCK = 95
  };

  enum {
    RuleParse = 0, RuleBind_option = 1, RuleOptions_assign = 2, RuleOption = 3, 
    RuleOption_block = 4, RuleArray_count_id = 5, RuleArray = 6, RuleValue_declaration = 7, 
    RuleSlot_declaration = 8, RuleSampler_declaration = 9, RuleDefine_declaration = 10, 
    RuleRtv_formats_declaration = 11, RuleBlends_declaration = 12, RulePointer = 13, 
    RulePso_param = 14, RuleClass_no_template = 15, RuleType_with_template = 16, 
    RuleInherit_id = 17, RuleName_id = 18, RuleOption_id = 19, RuleOwner_id = 20, 
    RuleTemplate_id = 21, RuleFunction_id = 22, RuleValue_id = 23, RuleValue_id_ignore = 24, 
    RuleType_id = 25, RuleInsert_block = 26, RulePath_id = 27, RuleInherit = 28, 
    RuleLayout_stat = 29, RuleLayout_block = 30, RuleLayout_definition = 31, 
    RuleTable_stat = 32, RuleTable_block = 33, RuleTable_definition = 34, 
    RuleRt_color_declaration = 35, RuleRt_ds_declaration = 36, RuleRt_stat = 37, 
    RuleRt_block = 38, RuleRt_definition = 39, RuleArray_value_holder = 40, 
    RuleArray_value_ids = 41, RuleRoot_sig = 42, RuleShader = 43, RuleCompute_pso_stat = 44, 
    RuleCompute_pso_block = 45, RuleCompute_pso_definition = 46, RuleGraphics_pso_stat = 47, 
    RuleGraphics_pso_block = 48, RuleGraphics_pso_definition = 49, RuleRtx_pso_stat = 50, 
    RuleRtx_pso_block = 51, RuleRtx_pso_definition = 52, RuleNode_param_id = 53, 
    RuleNode_param = 54, RuleNode_output_decl = 55, RuleNode_stat = 56, 
    RuleNode_block = 57, RuleNode_definition = 58, RuleWorkgraph_pso_stat = 59, 
    RuleWorkgraph_pso_block = 60, RuleWorkgraph_pso_definition = 61, RuleRtx_pass_stat = 62, 
    RuleRtx_pass_block = 63, RuleRtx_pass_definition = 64, RuleRtx_raygen_stat = 65, 
    RuleRtx_raygen_block = 66, RuleRtx_raygen_definition = 67, RuleView_declaration = 68, 
    RuleView_stat = 69, RuleView_block = 70, RuleView_definition = 71, RulePass_definition = 72, 
    RulePipeline_stat = 73, RulePipeline_block = 74, RulePipeline_definition = 75, 
    RuleEnum_value_declaration = 76, RuleEnum_stat = 77, RuleEnum_block = 78, 
    RuleEnum_definition = 79, RuleShader_type = 80, RulePso_param_id = 81, 
    RuleBool_type = 82
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
  class PointerContext;
  class Pso_paramContext;
  class Class_no_templateContext;
  class Type_with_templateContext;
  class Inherit_idContext;
  class Name_idContext;
  class Option_idContext;
  class Owner_idContext;
  class Template_idContext;
  class Function_idContext;
  class Value_idContext;
  class Value_id_ignoreContext;
  class Type_idContext;
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
  class Node_param_idContext;
  class Node_paramContext;
  class Node_output_declContext;
  class Node_statContext;
  class Node_blockContext;
  class Node_definitionContext;
  class Workgraph_pso_statContext;
  class Workgraph_pso_blockContext;
  class Workgraph_pso_definitionContext;
  class Rtx_pass_statContext;
  class Rtx_pass_blockContext;
  class Rtx_pass_definitionContext;
  class Rtx_raygen_statContext;
  class Rtx_raygen_blockContext;
  class Rtx_raygen_definitionContext;
  class View_declarationContext;
  class View_statContext;
  class View_blockContext;
  class View_definitionContext;
  class Pass_definitionContext;
  class Pipeline_statContext;
  class Pipeline_blockContext;
  class Pipeline_definitionContext;
  class Enum_value_declarationContext;
  class Enum_statContext;
  class Enum_blockContext;
  class Enum_definitionContext;
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
    std::vector<Workgraph_pso_definitionContext *> workgraph_pso_definition();
    Workgraph_pso_definitionContext* workgraph_pso_definition(size_t i);
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
    std::vector<Pass_definitionContext *> pass_definition();
    Pass_definitionContext* pass_definition(size_t i);
    std::vector<View_definitionContext *> view_definition();
    View_definitionContext* view_definition(size_t i);
    std::vector<Pipeline_definitionContext *> pipeline_definition();
    Pipeline_definitionContext* pipeline_definition(size_t i);
    std::vector<Enum_definitionContext *> enum_definition();
    Enum_definitionContext* enum_definition(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMENT();
    antlr4::tree::TerminalNode* COMMENT(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Option_blockContext* option_block();

  class  Array_count_idContext : public antlr4::ParserRuleContext {
  public:
    Array_count_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INT_SCALAR();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Blends_declarationContext* blends_declaration();

  class  PointerContext : public antlr4::ParserRuleContext {
  public:
    PointerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *POINTER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PointerContext* pointer();

  class  Pso_paramContext : public antlr4::ParserRuleContext {
  public:
    Pso_paramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Pso_param_idContext *pso_param_id();
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pso_paramContext* pso_param();

  class  Class_no_templateContext : public antlr4::ParserRuleContext {
  public:
    Class_no_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Class_no_templateContext* class_no_template();

  class  Type_with_templateContext : public antlr4::ParserRuleContext {
  public:
    Type_with_templateContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Class_no_templateContext *class_no_template();
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *GT();
    PointerContext *pointer();
    std::vector<Template_idContext *> template_id();
    Template_idContext* template_id(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Type_with_templateContext* type_with_template();

  class  Inherit_idContext : public antlr4::ParserRuleContext {
  public:
    Inherit_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Inherit_idContext* inherit_id();

  class  Name_idContext : public antlr4::ParserRuleContext {
  public:
    Name_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Name_idContext* name_id();

  class  Option_idContext : public antlr4::ParserRuleContext {
  public:
    Option_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Option_idContext* option_id();

  class  Owner_idContext : public antlr4::ParserRuleContext {
  public:
    Owner_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Owner_idContext* owner_id();

  class  Template_idContext : public antlr4::ParserRuleContext {
  public:
    Template_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Template_idContext* template_id();

  class  Function_idContext : public antlr4::ParserRuleContext {
  public:
    Function_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *OPAR();
    antlr4::tree::TerminalNode *CPAR();
    std::vector<Value_id_ignoreContext *> value_id_ignore();
    Value_id_ignoreContext* value_id_ignore(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Function_idContext* function_id();

  class  Value_idContext : public antlr4::ParserRuleContext {
  public:
    Value_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Shader_typeContext *shader_type();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *INT_SCALAR();
    antlr4::tree::TerminalNode *FLOAT_SCALAR();
    Bool_typeContext *bool_type();
    Function_idContext *function_id();
    Array_value_idsContext *array_value_ids();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Value_idContext* value_id();

  class  Value_id_ignoreContext : public antlr4::ParserRuleContext {
  public:
    Value_id_ignoreContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *INT_SCALAR();
    antlr4::tree::TerminalNode *FLOAT_SCALAR();
    Bool_typeContext *bool_type();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Value_id_ignoreContext* value_id_ignore();

  class  Type_idContext : public antlr4::ParserRuleContext {
  public:
    Type_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_with_templateContext *type_with_template();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Type_idContext* type_id();

  class  Insert_blockContext : public antlr4::ParserRuleContext {
  public:
    Insert_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *INSERT_BLOCK();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Rt_definitionContext* rt_definition();

  class  Array_value_holderContext : public antlr4::ParserRuleContext {
  public:
    Array_value_holderContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Value_idContext *value_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Rtx_pso_definitionContext* rtx_pso_definition();

  class  Node_param_idContext : public antlr4::ParserRuleContext {
  public:
    Node_param_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_param_idContext* node_param_id();

  class  Node_paramContext : public antlr4::ParserRuleContext {
  public:
    Node_paramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Node_param_idContext *node_param_id();
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();
    antlr4::tree::TerminalNode *SCOL();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_paramContext* node_param();

  class  Node_output_declContext : public antlr4::ParserRuleContext {
  public:
    Node_output_declContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NODE_OUTPUT();
    Type_idContext *type_id();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_output_declContext* node_output_decl();

  class  Node_statContext : public antlr4::ParserRuleContext {
  public:
    Node_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Node_paramContext *node_param();
    Node_output_declContext *node_output_decl();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_statContext* node_stat();

  class  Node_blockContext : public antlr4::ParserRuleContext {
  public:
    Node_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Node_statContext *> node_stat();
    Node_statContext* node_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_blockContext* node_block();

  class  Node_definitionContext : public antlr4::ParserRuleContext {
  public:
    Node_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NODE();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Node_blockContext *node_block();
    antlr4::tree::TerminalNode *CBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Node_definitionContext* node_definition();

  class  Workgraph_pso_statContext : public antlr4::ParserRuleContext {
  public:
    Workgraph_pso_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Root_sigContext *root_sig();
    ShaderContext *shader();
    Define_declarationContext *define_declaration();
    Node_definitionContext *node_definition();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Workgraph_pso_statContext* workgraph_pso_stat();

  class  Workgraph_pso_blockContext : public antlr4::ParserRuleContext {
  public:
    Workgraph_pso_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Workgraph_pso_statContext *> workgraph_pso_stat();
    Workgraph_pso_statContext* workgraph_pso_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Workgraph_pso_blockContext* workgraph_pso_block();

  class  Workgraph_pso_definitionContext : public antlr4::ParserRuleContext {
  public:
    Workgraph_pso_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *WORKGRAPH_PSO();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Workgraph_pso_blockContext *workgraph_pso_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Workgraph_pso_definitionContext* workgraph_pso_definition();

  class  Rtx_pass_statContext : public antlr4::ParserRuleContext {
  public:
    Rtx_pass_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ShaderContext *shader();
    antlr4::tree::TerminalNode *COMMENT();
    Pso_paramContext *pso_param();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Rtx_raygen_definitionContext* rtx_raygen_definition();

  class  View_declarationContext : public antlr4::ParserRuleContext {
  public:
    View_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Type_idContext *type_id();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  View_declarationContext* view_declaration();

  class  View_statContext : public antlr4::ParserRuleContext {
  public:
    View_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    View_declarationContext *view_declaration();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  View_statContext* view_stat();

  class  View_blockContext : public antlr4::ParserRuleContext {
  public:
    View_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<View_statContext *> view_stat();
    View_statContext* view_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  View_blockContext* view_block();

  class  View_definitionContext : public antlr4::ParserRuleContext {
  public:
    View_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VIEW();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    View_blockContext *view_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  View_definitionContext* view_definition();

  class  Pass_definitionContext : public antlr4::ParserRuleContext {
  public:
    Pass_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PASS();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    View_blockContext *view_block();
    antlr4::tree::TerminalNode *CBRACE();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    InheritContext *inherit();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pass_definitionContext* pass_definition();

  class  Pipeline_statContext : public antlr4::ParserRuleContext {
  public:
    Pipeline_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    std::vector<Option_blockContext *> option_block();
    Option_blockContext* option_block(size_t i);
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pipeline_statContext* pipeline_stat();

  class  Pipeline_blockContext : public antlr4::ParserRuleContext {
  public:
    Pipeline_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Pipeline_statContext *> pipeline_stat();
    Pipeline_statContext* pipeline_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pipeline_blockContext* pipeline_block();

  class  Pipeline_definitionContext : public antlr4::ParserRuleContext {
  public:
    Pipeline_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PIPELINE();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Pipeline_blockContext *pipeline_block();
    antlr4::tree::TerminalNode *CBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Pipeline_definitionContext* pipeline_definition();

  class  Enum_value_declarationContext : public antlr4::ParserRuleContext {
  public:
    Enum_value_declarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *SCOL();
    antlr4::tree::TerminalNode *ASSIGN();
    Value_idContext *value_id();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Enum_value_declarationContext* enum_value_declaration();

  class  Enum_statContext : public antlr4::ParserRuleContext {
  public:
    Enum_statContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Enum_value_declarationContext *enum_value_declaration();
    antlr4::tree::TerminalNode *COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Enum_statContext* enum_stat();

  class  Enum_blockContext : public antlr4::ParserRuleContext {
  public:
    Enum_blockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<Enum_statContext *> enum_stat();
    Enum_statContext* enum_stat(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Enum_blockContext* enum_block();

  class  Enum_definitionContext : public antlr4::ParserRuleContext {
  public:
    Enum_definitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ENUM();
    Name_idContext *name_id();
    antlr4::tree::TerminalNode *OBRACE();
    Enum_blockContext *enum_block();
    antlr4::tree::TerminalNode *CBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Enum_definitionContext* enum_definition();

  class  Shader_typeContext : public antlr4::ParserRuleContext {
  public:
    Shader_typeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Shader_typeContext* shader_type();

  class  Pso_param_idContext : public antlr4::ParserRuleContext {
  public:
    Pso_param_idContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
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

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Bool_typeContext* bool_type();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

