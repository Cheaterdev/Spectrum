
// Generated from SIG.g4 by ANTLR 4.9.2


#include "SIGListener.h"

#include "SIGParser.h"


using namespace antlrcpp;
using namespace antlr4;

SIGParser::SIGParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SIGParser::~SIGParser() {
  delete _interpreter;
}

std::string SIGParser::getGrammarFileName() const {
  return "SIG.g4";
}

const std::vector<std::string>& SIGParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SIGParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- ParseContext ------------------------------------------------------------------

SIGParser::ParseContext::ParseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::ParseContext::EOF() {
  return getToken(SIGParser::EOF, 0);
}

std::vector<SIGParser::Layout_definitionContext *> SIGParser::ParseContext::layout_definition() {
  return getRuleContexts<SIGParser::Layout_definitionContext>();
}

SIGParser::Layout_definitionContext* SIGParser::ParseContext::layout_definition(size_t i) {
  return getRuleContext<SIGParser::Layout_definitionContext>(i);
}

std::vector<SIGParser::Table_definitionContext *> SIGParser::ParseContext::table_definition() {
  return getRuleContexts<SIGParser::Table_definitionContext>();
}

SIGParser::Table_definitionContext* SIGParser::ParseContext::table_definition(size_t i) {
  return getRuleContext<SIGParser::Table_definitionContext>(i);
}

std::vector<SIGParser::Rt_definitionContext *> SIGParser::ParseContext::rt_definition() {
  return getRuleContexts<SIGParser::Rt_definitionContext>();
}

SIGParser::Rt_definitionContext* SIGParser::ParseContext::rt_definition(size_t i) {
  return getRuleContext<SIGParser::Rt_definitionContext>(i);
}

std::vector<SIGParser::Compute_pso_definitionContext *> SIGParser::ParseContext::compute_pso_definition() {
  return getRuleContexts<SIGParser::Compute_pso_definitionContext>();
}

SIGParser::Compute_pso_definitionContext* SIGParser::ParseContext::compute_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Compute_pso_definitionContext>(i);
}

std::vector<SIGParser::Graphics_pso_definitionContext *> SIGParser::ParseContext::graphics_pso_definition() {
  return getRuleContexts<SIGParser::Graphics_pso_definitionContext>();
}

SIGParser::Graphics_pso_definitionContext* SIGParser::ParseContext::graphics_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Graphics_pso_definitionContext>(i);
}

std::vector<SIGParser::Rtx_pso_definitionContext *> SIGParser::ParseContext::rtx_pso_definition() {
  return getRuleContexts<SIGParser::Rtx_pso_definitionContext>();
}

SIGParser::Rtx_pso_definitionContext* SIGParser::ParseContext::rtx_pso_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_pso_definitionContext>(i);
}

std::vector<SIGParser::Rtx_pass_definitionContext *> SIGParser::ParseContext::rtx_pass_definition() {
  return getRuleContexts<SIGParser::Rtx_pass_definitionContext>();
}

SIGParser::Rtx_pass_definitionContext* SIGParser::ParseContext::rtx_pass_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_pass_definitionContext>(i);
}

std::vector<SIGParser::Rtx_raygen_definitionContext *> SIGParser::ParseContext::rtx_raygen_definition() {
  return getRuleContexts<SIGParser::Rtx_raygen_definitionContext>();
}

SIGParser::Rtx_raygen_definitionContext* SIGParser::ParseContext::rtx_raygen_definition(size_t i) {
  return getRuleContext<SIGParser::Rtx_raygen_definitionContext>(i);
}

std::vector<tree::TerminalNode *> SIGParser::ParseContext::COMMENT() {
  return getTokens(SIGParser::COMMENT);
}

tree::TerminalNode* SIGParser::ParseContext::COMMENT(size_t i) {
  return getToken(SIGParser::COMMENT, i);
}


size_t SIGParser::ParseContext::getRuleIndex() const {
  return SIGParser::RuleParse;
}

void SIGParser::ParseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParse(this);
}

void SIGParser::ParseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParse(this);
}

SIGParser::ParseContext* SIGParser::parse() {
  ParseContext *_localctx = _tracker.createInstance<ParseContext>(_ctx, getState());
  enterRule(_localctx, 0, SIGParser::RuleParse);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(127);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & ((1ULL << (SIGParser::OSBRACE - 56))
      | (1ULL << (SIGParser::LAYOUT - 56))
      | (1ULL << (SIGParser::STRUCT - 56))
      | (1ULL << (SIGParser::COMPUTE_PSO - 56))
      | (1ULL << (SIGParser::GRAPHICS_PSO - 56))
      | (1ULL << (SIGParser::RAYTRACE_PSO - 56))
      | (1ULL << (SIGParser::RAYTRACE_RAYGEN - 56))
      | (1ULL << (SIGParser::RAYTRACE_PASS - 56))
      | (1ULL << (SIGParser::RT - 56))
      | (1ULL << (SIGParser::COMMENT - 56)))) != 0)) {
      setState(125);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(116);
        layout_definition();
        break;
      }

      case 2: {
        setState(117);
        table_definition();
        break;
      }

      case 3: {
        setState(118);
        rt_definition();
        break;
      }

      case 4: {
        setState(119);
        compute_pso_definition();
        break;
      }

      case 5: {
        setState(120);
        graphics_pso_definition();
        break;
      }

      case 6: {
        setState(121);
        rtx_pso_definition();
        break;
      }

      case 7: {
        setState(122);
        rtx_pass_definition();
        break;
      }

      case 8: {
        setState(123);
        rtx_raygen_definition();
        break;
      }

      case 9: {
        setState(124);
        match(SIGParser::COMMENT);
        break;
      }

      default:
        break;
      }
      setState(129);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(130);
    match(SIGParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bind_optionContext ------------------------------------------------------------------

SIGParser::Bind_optionContext::Bind_optionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_idContext* SIGParser::Bind_optionContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

SIGParser::Owner_idContext* SIGParser::Bind_optionContext::owner_id() {
  return getRuleContext<SIGParser::Owner_idContext>(0);
}


size_t SIGParser::Bind_optionContext::getRuleIndex() const {
  return SIGParser::RuleBind_option;
}

void SIGParser::Bind_optionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBind_option(this);
}

void SIGParser::Bind_optionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBind_option(this);
}

SIGParser::Bind_optionContext* SIGParser::bind_option() {
  Bind_optionContext *_localctx = _tracker.createInstance<Bind_optionContext>(_ctx, getState());
  enterRule(_localctx, 2, SIGParser::RuleBind_option);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(135);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      setState(132);
      owner_id();
      setState(133);
      match(SIGParser::T__0);
      break;
    }

    default:
      break;
    }
    setState(137);
    value_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Options_assignContext ------------------------------------------------------------------

SIGParser::Options_assignContext::Options_assignContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Options_assignContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Bind_optionContext* SIGParser::Options_assignContext::bind_option() {
  return getRuleContext<SIGParser::Bind_optionContext>(0);
}


size_t SIGParser::Options_assignContext::getRuleIndex() const {
  return SIGParser::RuleOptions_assign;
}

void SIGParser::Options_assignContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOptions_assign(this);
}

void SIGParser::Options_assignContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOptions_assign(this);
}

SIGParser::Options_assignContext* SIGParser::options_assign() {
  Options_assignContext *_localctx = _tracker.createInstance<Options_assignContext>(_ctx, getState());
  enterRule(_localctx, 4, SIGParser::RuleOptions_assign);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(139);
    match(SIGParser::ASSIGN);
    setState(140);
    bind_option();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OptionContext ------------------------------------------------------------------

SIGParser::OptionContext::OptionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::OptionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

SIGParser::Options_assignContext* SIGParser::OptionContext::options_assign() {
  return getRuleContext<SIGParser::Options_assignContext>(0);
}


size_t SIGParser::OptionContext::getRuleIndex() const {
  return SIGParser::RuleOption;
}

void SIGParser::OptionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption(this);
}

void SIGParser::OptionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption(this);
}

SIGParser::OptionContext* SIGParser::option() {
  OptionContext *_localctx = _tracker.createInstance<OptionContext>(_ctx, getState());
  enterRule(_localctx, 6, SIGParser::RuleOption);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(142);
    name_id();
    setState(144);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(143);
      options_assign();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Option_blockContext ------------------------------------------------------------------

SIGParser::Option_blockContext::Option_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Option_blockContext::OSBRACE() {
  return getToken(SIGParser::OSBRACE, 0);
}

std::vector<SIGParser::OptionContext *> SIGParser::Option_blockContext::option() {
  return getRuleContexts<SIGParser::OptionContext>();
}

SIGParser::OptionContext* SIGParser::Option_blockContext::option(size_t i) {
  return getRuleContext<SIGParser::OptionContext>(i);
}

tree::TerminalNode* SIGParser::Option_blockContext::CSBRACE() {
  return getToken(SIGParser::CSBRACE, 0);
}


size_t SIGParser::Option_blockContext::getRuleIndex() const {
  return SIGParser::RuleOption_block;
}

void SIGParser::Option_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_block(this);
}

void SIGParser::Option_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_block(this);
}

SIGParser::Option_blockContext* SIGParser::option_block() {
  Option_blockContext *_localctx = _tracker.createInstance<Option_blockContext>(_ctx, getState());
  enterRule(_localctx, 8, SIGParser::RuleOption_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(146);
    match(SIGParser::OSBRACE);
    setState(147);
    option();
    setState(152);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__1) {
      setState(148);
      match(SIGParser::T__1);
      setState(149);
      option();
      setState(154);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(155);
    match(SIGParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_count_idContext ------------------------------------------------------------------

SIGParser::Array_count_idContext::Array_count_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Array_count_idContext::INT_SCALAR() {
  return getToken(SIGParser::INT_SCALAR, 0);
}


size_t SIGParser::Array_count_idContext::getRuleIndex() const {
  return SIGParser::RuleArray_count_id;
}

void SIGParser::Array_count_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_count_id(this);
}

void SIGParser::Array_count_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_count_id(this);
}

SIGParser::Array_count_idContext* SIGParser::array_count_id() {
  Array_count_idContext *_localctx = _tracker.createInstance<Array_count_idContext>(_ctx, getState());
  enterRule(_localctx, 10, SIGParser::RuleArray_count_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(157);
    match(SIGParser::INT_SCALAR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayContext ------------------------------------------------------------------

SIGParser::ArrayContext::ArrayContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::ArrayContext::OSBRACE() {
  return getToken(SIGParser::OSBRACE, 0);
}

tree::TerminalNode* SIGParser::ArrayContext::CSBRACE() {
  return getToken(SIGParser::CSBRACE, 0);
}

SIGParser::Array_count_idContext* SIGParser::ArrayContext::array_count_id() {
  return getRuleContext<SIGParser::Array_count_idContext>(0);
}


size_t SIGParser::ArrayContext::getRuleIndex() const {
  return SIGParser::RuleArray;
}

void SIGParser::ArrayContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray(this);
}

void SIGParser::ArrayContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray(this);
}

SIGParser::ArrayContext* SIGParser::array() {
  ArrayContext *_localctx = _tracker.createInstance<ArrayContext>(_ctx, getState());
  enterRule(_localctx, 12, SIGParser::RuleArray);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(159);
    match(SIGParser::OSBRACE);
    setState(161);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::INT_SCALAR) {
      setState(160);
      array_count_id();
    }
    setState(163);
    match(SIGParser::CSBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_declarationContext ------------------------------------------------------------------

SIGParser::Value_declarationContext::Value_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_idContext* SIGParser::Value_declarationContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::Value_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Value_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Value_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Value_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::ArrayContext* SIGParser::Value_declarationContext::array() {
  return getRuleContext<SIGParser::ArrayContext>(0);
}


size_t SIGParser::Value_declarationContext::getRuleIndex() const {
  return SIGParser::RuleValue_declaration;
}

void SIGParser::Value_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_declaration(this);
}

void SIGParser::Value_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_declaration(this);
}

SIGParser::Value_declarationContext* SIGParser::value_declaration() {
  Value_declarationContext *_localctx = _tracker.createInstance<Value_declarationContext>(_ctx, getState());
  enterRule(_localctx, 14, SIGParser::RuleValue_declaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(168);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(165);
        option_block(); 
      }
      setState(170);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx);
    }
    setState(171);
    type_id();
    setState(172);
    name_id();
    setState(174);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::OSBRACE) {
      setState(173);
      array();
    }
    setState(176);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Slot_declarationContext ------------------------------------------------------------------

SIGParser::Slot_declarationContext::Slot_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Slot_declarationContext::SLOT() {
  return getToken(SIGParser::SLOT, 0);
}

SIGParser::Name_idContext* SIGParser::Slot_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Slot_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Slot_declarationContext::getRuleIndex() const {
  return SIGParser::RuleSlot_declaration;
}

void SIGParser::Slot_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSlot_declaration(this);
}

void SIGParser::Slot_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSlot_declaration(this);
}

SIGParser::Slot_declarationContext* SIGParser::slot_declaration() {
  Slot_declarationContext *_localctx = _tracker.createInstance<Slot_declarationContext>(_ctx, getState());
  enterRule(_localctx, 16, SIGParser::RuleSlot_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    match(SIGParser::SLOT);
    setState(179);
    name_id();
    setState(180);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Sampler_declarationContext ------------------------------------------------------------------

SIGParser::Sampler_declarationContext::Sampler_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Sampler_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Sampler_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Sampler_declarationContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

tree::TerminalNode* SIGParser::Sampler_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Sampler_declarationContext::getRuleIndex() const {
  return SIGParser::RuleSampler_declaration;
}

void SIGParser::Sampler_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSampler_declaration(this);
}

void SIGParser::Sampler_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSampler_declaration(this);
}

SIGParser::Sampler_declarationContext* SIGParser::sampler_declaration() {
  Sampler_declarationContext *_localctx = _tracker.createInstance<Sampler_declarationContext>(_ctx, getState());
  enterRule(_localctx, 18, SIGParser::RuleSampler_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(182);
    match(SIGParser::T__2);
    setState(183);
    name_id();
    setState(184);
    match(SIGParser::ASSIGN);
    setState(185);
    value_id();
    setState(186);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Define_declarationContext ------------------------------------------------------------------

SIGParser::Define_declarationContext::Define_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Name_idContext* SIGParser::Define_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Define_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Define_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Define_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

tree::TerminalNode* SIGParser::Define_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Define_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}


size_t SIGParser::Define_declarationContext::getRuleIndex() const {
  return SIGParser::RuleDefine_declaration;
}

void SIGParser::Define_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefine_declaration(this);
}

void SIGParser::Define_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefine_declaration(this);
}

SIGParser::Define_declarationContext* SIGParser::define_declaration() {
  Define_declarationContext *_localctx = _tracker.createInstance<Define_declarationContext>(_ctx, getState());
  enterRule(_localctx, 20, SIGParser::RuleDefine_declaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(191);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(188);
        option_block(); 
      }
      setState(193);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    }
    setState(194);
    match(SIGParser::T__3);
    setState(195);
    name_id();
    setState(198);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::ASSIGN) {
      setState(196);
      match(SIGParser::ASSIGN);
      setState(197);
      array_value_ids();
    }
    setState(200);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtv_formats_declarationContext ------------------------------------------------------------------

SIGParser::Rtv_formats_declarationContext::Rtv_formats_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtv_formats_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Rtv_formats_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}

tree::TerminalNode* SIGParser::Rtv_formats_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtv_formats_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtv_formats_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Rtv_formats_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRtv_formats_declaration;
}

void SIGParser::Rtv_formats_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtv_formats_declaration(this);
}

void SIGParser::Rtv_formats_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtv_formats_declaration(this);
}

SIGParser::Rtv_formats_declarationContext* SIGParser::rtv_formats_declaration() {
  Rtv_formats_declarationContext *_localctx = _tracker.createInstance<Rtv_formats_declarationContext>(_ctx, getState());
  enterRule(_localctx, 22, SIGParser::RuleRtv_formats_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(205);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(202);
        option_block(); 
      }
      setState(207);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 10, _ctx);
    }
    setState(208);
    match(SIGParser::T__4);
    setState(209);
    match(SIGParser::ASSIGN);
    setState(210);
    array_value_ids();
    setState(211);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Blends_declarationContext ------------------------------------------------------------------

SIGParser::Blends_declarationContext::Blends_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Blends_declarationContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Array_value_idsContext* SIGParser::Blends_declarationContext::array_value_ids() {
  return getRuleContext<SIGParser::Array_value_idsContext>(0);
}

tree::TerminalNode* SIGParser::Blends_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Blends_declarationContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Blends_declarationContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::Blends_declarationContext::getRuleIndex() const {
  return SIGParser::RuleBlends_declaration;
}

void SIGParser::Blends_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlends_declaration(this);
}

void SIGParser::Blends_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlends_declaration(this);
}

SIGParser::Blends_declarationContext* SIGParser::blends_declaration() {
  Blends_declarationContext *_localctx = _tracker.createInstance<Blends_declarationContext>(_ctx, getState());
  enterRule(_localctx, 24, SIGParser::RuleBlends_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(216);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(213);
        option_block(); 
      }
      setState(218);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    }
    setState(219);
    match(SIGParser::T__5);
    setState(220);
    match(SIGParser::ASSIGN);
    setState(221);
    array_value_ids();
    setState(222);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pso_paramContext ------------------------------------------------------------------

SIGParser::Pso_paramContext::Pso_paramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Pso_param_idContext* SIGParser::Pso_paramContext::pso_param_id() {
  return getRuleContext<SIGParser::Pso_param_idContext>(0);
}

tree::TerminalNode* SIGParser::Pso_paramContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Value_idContext* SIGParser::Pso_paramContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}

tree::TerminalNode* SIGParser::Pso_paramContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Pso_paramContext::getRuleIndex() const {
  return SIGParser::RulePso_param;
}

void SIGParser::Pso_paramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param(this);
}

void SIGParser::Pso_paramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param(this);
}

SIGParser::Pso_paramContext* SIGParser::pso_param() {
  Pso_paramContext *_localctx = _tracker.createInstance<Pso_paramContext>(_ctx, getState());
  enterRule(_localctx, 26, SIGParser::RulePso_param);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(224);
    pso_param_id();
    setState(225);
    match(SIGParser::ASSIGN);
    setState(226);
    value_id();
    setState(227);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_with_templateContext ------------------------------------------------------------------

SIGParser::Type_with_templateContext::Type_with_templateContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Type_with_templateContext::ID() {
  return getToken(SIGParser::ID, 0);
}

tree::TerminalNode* SIGParser::Type_with_templateContext::LT() {
  return getToken(SIGParser::LT, 0);
}

SIGParser::Template_idContext* SIGParser::Type_with_templateContext::template_id() {
  return getRuleContext<SIGParser::Template_idContext>(0);
}

tree::TerminalNode* SIGParser::Type_with_templateContext::GT() {
  return getToken(SIGParser::GT, 0);
}


size_t SIGParser::Type_with_templateContext::getRuleIndex() const {
  return SIGParser::RuleType_with_template;
}

void SIGParser::Type_with_templateContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_with_template(this);
}

void SIGParser::Type_with_templateContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_with_template(this);
}

SIGParser::Type_with_templateContext* SIGParser::type_with_template() {
  Type_with_templateContext *_localctx = _tracker.createInstance<Type_with_templateContext>(_ctx, getState());
  enterRule(_localctx, 28, SIGParser::RuleType_with_template);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(229);
    match(SIGParser::ID);
    setState(234);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::LT) {
      setState(230);
      match(SIGParser::LT);
      setState(231);
      template_id();
      setState(232);
      match(SIGParser::GT);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Inherit_idContext ------------------------------------------------------------------

SIGParser::Inherit_idContext::Inherit_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Inherit_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Inherit_idContext::getRuleIndex() const {
  return SIGParser::RuleInherit_id;
}

void SIGParser::Inherit_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit_id(this);
}

void SIGParser::Inherit_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit_id(this);
}

SIGParser::Inherit_idContext* SIGParser::inherit_id() {
  Inherit_idContext *_localctx = _tracker.createInstance<Inherit_idContext>(_ctx, getState());
  enterRule(_localctx, 30, SIGParser::RuleInherit_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(236);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Name_idContext ------------------------------------------------------------------

SIGParser::Name_idContext::Name_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Name_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Name_idContext::getRuleIndex() const {
  return SIGParser::RuleName_id;
}

void SIGParser::Name_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterName_id(this);
}

void SIGParser::Name_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitName_id(this);
}

SIGParser::Name_idContext* SIGParser::name_id() {
  Name_idContext *_localctx = _tracker.createInstance<Name_idContext>(_ctx, getState());
  enterRule(_localctx, 32, SIGParser::RuleName_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(238);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_idContext ------------------------------------------------------------------

SIGParser::Type_idContext::Type_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_with_templateContext* SIGParser::Type_idContext::type_with_template() {
  return getRuleContext<SIGParser::Type_with_templateContext>(0);
}


size_t SIGParser::Type_idContext::getRuleIndex() const {
  return SIGParser::RuleType_id;
}

void SIGParser::Type_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_id(this);
}

void SIGParser::Type_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_id(this);
}

SIGParser::Type_idContext* SIGParser::type_id() {
  Type_idContext *_localctx = _tracker.createInstance<Type_idContext>(_ctx, getState());
  enterRule(_localctx, 34, SIGParser::RuleType_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(240);
    type_with_template();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Option_idContext ------------------------------------------------------------------

SIGParser::Option_idContext::Option_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Option_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Option_idContext::getRuleIndex() const {
  return SIGParser::RuleOption_id;
}

void SIGParser::Option_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOption_id(this);
}

void SIGParser::Option_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOption_id(this);
}

SIGParser::Option_idContext* SIGParser::option_id() {
  Option_idContext *_localctx = _tracker.createInstance<Option_idContext>(_ctx, getState());
  enterRule(_localctx, 36, SIGParser::RuleOption_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(242);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Owner_idContext ------------------------------------------------------------------

SIGParser::Owner_idContext::Owner_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Owner_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Owner_idContext::getRuleIndex() const {
  return SIGParser::RuleOwner_id;
}

void SIGParser::Owner_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOwner_id(this);
}

void SIGParser::Owner_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOwner_id(this);
}

SIGParser::Owner_idContext* SIGParser::owner_id() {
  Owner_idContext *_localctx = _tracker.createInstance<Owner_idContext>(_ctx, getState());
  enterRule(_localctx, 38, SIGParser::RuleOwner_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(244);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Template_idContext ------------------------------------------------------------------

SIGParser::Template_idContext::Template_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Template_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}


size_t SIGParser::Template_idContext::getRuleIndex() const {
  return SIGParser::RuleTemplate_id;
}

void SIGParser::Template_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTemplate_id(this);
}

void SIGParser::Template_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTemplate_id(this);
}

SIGParser::Template_idContext* SIGParser::template_id() {
  Template_idContext *_localctx = _tracker.createInstance<Template_idContext>(_ctx, getState());
  enterRule(_localctx, 40, SIGParser::RuleTemplate_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(246);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Value_idContext ------------------------------------------------------------------

SIGParser::Value_idContext::Value_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Value_idContext::ID() {
  return getToken(SIGParser::ID, 0);
}

tree::TerminalNode* SIGParser::Value_idContext::INT_SCALAR() {
  return getToken(SIGParser::INT_SCALAR, 0);
}

tree::TerminalNode* SIGParser::Value_idContext::FLOAT_SCALAR() {
  return getToken(SIGParser::FLOAT_SCALAR, 0);
}

SIGParser::Bool_typeContext* SIGParser::Value_idContext::bool_type() {
  return getRuleContext<SIGParser::Bool_typeContext>(0);
}


size_t SIGParser::Value_idContext::getRuleIndex() const {
  return SIGParser::RuleValue_id;
}

void SIGParser::Value_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValue_id(this);
}

void SIGParser::Value_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValue_id(this);
}

SIGParser::Value_idContext* SIGParser::value_id() {
  Value_idContext *_localctx = _tracker.createInstance<Value_idContext>(_ctx, getState());
  enterRule(_localctx, 42, SIGParser::RuleValue_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(252);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(248);
        match(SIGParser::ID);
        break;
      }

      case SIGParser::INT_SCALAR: {
        enterOuterAlt(_localctx, 2);
        setState(249);
        match(SIGParser::INT_SCALAR);
        break;
      }

      case SIGParser::FLOAT_SCALAR: {
        enterOuterAlt(_localctx, 3);
        setState(250);
        match(SIGParser::FLOAT_SCALAR);
        break;
      }

      case SIGParser::TRUE:
      case SIGParser::FALSE: {
        enterOuterAlt(_localctx, 4);
        setState(251);
        bool_type();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Insert_blockContext ------------------------------------------------------------------

SIGParser::Insert_blockContext::Insert_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Insert_blockContext::INSERT_BLOCK() {
  return getToken(SIGParser::INSERT_BLOCK, 0);
}


size_t SIGParser::Insert_blockContext::getRuleIndex() const {
  return SIGParser::RuleInsert_block;
}

void SIGParser::Insert_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInsert_block(this);
}

void SIGParser::Insert_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInsert_block(this);
}

SIGParser::Insert_blockContext* SIGParser::insert_block() {
  Insert_blockContext *_localctx = _tracker.createInstance<Insert_blockContext>(_ctx, getState());
  enterRule(_localctx, 44, SIGParser::RuleInsert_block);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(254);
    match(SIGParser::INSERT_BLOCK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Path_idContext ------------------------------------------------------------------

SIGParser::Path_idContext::Path_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SIGParser::Path_idContext::ID() {
  return getTokens(SIGParser::ID);
}

tree::TerminalNode* SIGParser::Path_idContext::ID(size_t i) {
  return getToken(SIGParser::ID, i);
}

std::vector<tree::TerminalNode *> SIGParser::Path_idContext::DIV() {
  return getTokens(SIGParser::DIV);
}

tree::TerminalNode* SIGParser::Path_idContext::DIV(size_t i) {
  return getToken(SIGParser::DIV, i);
}


size_t SIGParser::Path_idContext::getRuleIndex() const {
  return SIGParser::RulePath_id;
}

void SIGParser::Path_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPath_id(this);
}

void SIGParser::Path_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPath_id(this);
}

SIGParser::Path_idContext* SIGParser::path_id() {
  Path_idContext *_localctx = _tracker.createInstance<Path_idContext>(_ctx, getState());
  enterRule(_localctx, 46, SIGParser::RulePath_id);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(260);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(256);
        match(SIGParser::ID);
        setState(257);
        match(SIGParser::DIV); 
      }
      setState(262);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx);
    }
    setState(263);
    match(SIGParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InheritContext ------------------------------------------------------------------

SIGParser::InheritContext::InheritContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Inherit_idContext *> SIGParser::InheritContext::inherit_id() {
  return getRuleContexts<SIGParser::Inherit_idContext>();
}

SIGParser::Inherit_idContext* SIGParser::InheritContext::inherit_id(size_t i) {
  return getRuleContext<SIGParser::Inherit_idContext>(i);
}


size_t SIGParser::InheritContext::getRuleIndex() const {
  return SIGParser::RuleInherit;
}

void SIGParser::InheritContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInherit(this);
}

void SIGParser::InheritContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInherit(this);
}

SIGParser::InheritContext* SIGParser::inherit() {
  InheritContext *_localctx = _tracker.createInstance<InheritContext>(_ctx, getState());
  enterRule(_localctx, 48, SIGParser::RuleInherit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(265);
    match(SIGParser::T__6);
    setState(266);
    inherit_id();
    setState(271);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(267);
        match(SIGParser::T__1);
        setState(268);
        inherit_id(); 
      }
      setState(273);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_statContext ------------------------------------------------------------------

SIGParser::Layout_statContext::Layout_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Slot_declarationContext* SIGParser::Layout_statContext::slot_declaration() {
  return getRuleContext<SIGParser::Slot_declarationContext>(0);
}

SIGParser::Sampler_declarationContext* SIGParser::Layout_statContext::sampler_declaration() {
  return getRuleContext<SIGParser::Sampler_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Layout_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Layout_statContext::getRuleIndex() const {
  return SIGParser::RuleLayout_stat;
}

void SIGParser::Layout_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_stat(this);
}

void SIGParser::Layout_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_stat(this);
}

SIGParser::Layout_statContext* SIGParser::layout_stat() {
  Layout_statContext *_localctx = _tracker.createInstance<Layout_statContext>(_ctx, getState());
  enterRule(_localctx, 50, SIGParser::RuleLayout_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(277);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::SLOT: {
        enterOuterAlt(_localctx, 1);
        setState(274);
        slot_declaration();
        break;
      }

      case SIGParser::T__2: {
        enterOuterAlt(_localctx, 2);
        setState(275);
        sampler_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(276);
        match(SIGParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_blockContext ------------------------------------------------------------------

SIGParser::Layout_blockContext::Layout_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Layout_statContext *> SIGParser::Layout_blockContext::layout_stat() {
  return getRuleContexts<SIGParser::Layout_statContext>();
}

SIGParser::Layout_statContext* SIGParser::Layout_blockContext::layout_stat(size_t i) {
  return getRuleContext<SIGParser::Layout_statContext>(i);
}


size_t SIGParser::Layout_blockContext::getRuleIndex() const {
  return SIGParser::RuleLayout_block;
}

void SIGParser::Layout_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_block(this);
}

void SIGParser::Layout_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_block(this);
}

SIGParser::Layout_blockContext* SIGParser::layout_block() {
  Layout_blockContext *_localctx = _tracker.createInstance<Layout_blockContext>(_ctx, getState());
  enterRule(_localctx, 52, SIGParser::RuleLayout_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(282);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__2 || _la == SIGParser::SLOT

    || _la == SIGParser::COMMENT) {
      setState(279);
      layout_stat();
      setState(284);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Layout_definitionContext ------------------------------------------------------------------

SIGParser::Layout_definitionContext::Layout_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Layout_definitionContext::LAYOUT() {
  return getToken(SIGParser::LAYOUT, 0);
}

SIGParser::Name_idContext* SIGParser::Layout_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Layout_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Layout_blockContext* SIGParser::Layout_definitionContext::layout_block() {
  return getRuleContext<SIGParser::Layout_blockContext>(0);
}

tree::TerminalNode* SIGParser::Layout_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Layout_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Layout_definitionContext::getRuleIndex() const {
  return SIGParser::RuleLayout_definition;
}

void SIGParser::Layout_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLayout_definition(this);
}

void SIGParser::Layout_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLayout_definition(this);
}

SIGParser::Layout_definitionContext* SIGParser::layout_definition() {
  Layout_definitionContext *_localctx = _tracker.createInstance<Layout_definitionContext>(_ctx, getState());
  enterRule(_localctx, 54, SIGParser::RuleLayout_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(285);
    match(SIGParser::LAYOUT);
    setState(286);
    name_id();
    setState(288);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(287);
      inherit();
    }
    setState(290);
    match(SIGParser::OBRACE);
    setState(291);
    layout_block();
    setState(292);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_statContext ------------------------------------------------------------------

SIGParser::Table_statContext::Table_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_declarationContext* SIGParser::Table_statContext::value_declaration() {
  return getRuleContext<SIGParser::Value_declarationContext>(0);
}

SIGParser::Insert_blockContext* SIGParser::Table_statContext::insert_block() {
  return getRuleContext<SIGParser::Insert_blockContext>(0);
}

tree::TerminalNode* SIGParser::Table_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Table_statContext::getRuleIndex() const {
  return SIGParser::RuleTable_stat;
}

void SIGParser::Table_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_stat(this);
}

void SIGParser::Table_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_stat(this);
}

SIGParser::Table_statContext* SIGParser::table_stat() {
  Table_statContext *_localctx = _tracker.createInstance<Table_statContext>(_ctx, getState());
  enterRule(_localctx, 56, SIGParser::RuleTable_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(297);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::OSBRACE:
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(294);
        value_declaration();
        break;
      }

      case SIGParser::INSERT_BLOCK: {
        enterOuterAlt(_localctx, 2);
        setState(295);
        insert_block();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(296);
        match(SIGParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_blockContext ------------------------------------------------------------------

SIGParser::Table_blockContext::Table_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Table_statContext *> SIGParser::Table_blockContext::table_stat() {
  return getRuleContexts<SIGParser::Table_statContext>();
}

SIGParser::Table_statContext* SIGParser::Table_blockContext::table_stat(size_t i) {
  return getRuleContext<SIGParser::Table_statContext>(i);
}


size_t SIGParser::Table_blockContext::getRuleIndex() const {
  return SIGParser::RuleTable_block;
}

void SIGParser::Table_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_block(this);
}

void SIGParser::Table_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_block(this);
}

SIGParser::Table_blockContext* SIGParser::table_block() {
  Table_blockContext *_localctx = _tracker.createInstance<Table_blockContext>(_ctx, getState());
  enterRule(_localctx, 58, SIGParser::RuleTable_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(302);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & ((1ULL << (SIGParser::OSBRACE - 56))
      | (1ULL << (SIGParser::ID - 56))
      | (1ULL << (SIGParser::COMMENT - 56))
      | (1ULL << (SIGParser::INSERT_BLOCK - 56)))) != 0)) {
      setState(299);
      table_stat();
      setState(304);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Table_definitionContext ------------------------------------------------------------------

SIGParser::Table_definitionContext::Table_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Table_definitionContext::STRUCT() {
  return getToken(SIGParser::STRUCT, 0);
}

SIGParser::Name_idContext* SIGParser::Table_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Table_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Table_blockContext* SIGParser::Table_definitionContext::table_block() {
  return getRuleContext<SIGParser::Table_blockContext>(0);
}

tree::TerminalNode* SIGParser::Table_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Table_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Table_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Table_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Table_definitionContext::getRuleIndex() const {
  return SIGParser::RuleTable_definition;
}

void SIGParser::Table_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTable_definition(this);
}

void SIGParser::Table_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTable_definition(this);
}

SIGParser::Table_definitionContext* SIGParser::table_definition() {
  Table_definitionContext *_localctx = _tracker.createInstance<Table_definitionContext>(_ctx, getState());
  enterRule(_localctx, 60, SIGParser::RuleTable_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(308);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(305);
        option_block(); 
      }
      setState(310);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
    setState(311);
    match(SIGParser::STRUCT);
    setState(312);
    name_id();
    setState(314);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(313);
      inherit();
    }
    setState(316);
    match(SIGParser::OBRACE);
    setState(317);
    table_block();
    setState(318);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_color_declarationContext ------------------------------------------------------------------

SIGParser::Rt_color_declarationContext::Rt_color_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Type_idContext* SIGParser::Rt_color_declarationContext::type_id() {
  return getRuleContext<SIGParser::Type_idContext>(0);
}

SIGParser::Name_idContext* SIGParser::Rt_color_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_color_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Rt_color_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRt_color_declaration;
}

void SIGParser::Rt_color_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_color_declaration(this);
}

void SIGParser::Rt_color_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_color_declaration(this);
}

SIGParser::Rt_color_declarationContext* SIGParser::rt_color_declaration() {
  Rt_color_declarationContext *_localctx = _tracker.createInstance<Rt_color_declarationContext>(_ctx, getState());
  enterRule(_localctx, 62, SIGParser::RuleRt_color_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(320);
    type_id();
    setState(321);
    name_id();
    setState(322);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_ds_declarationContext ------------------------------------------------------------------

SIGParser::Rt_ds_declarationContext::Rt_ds_declarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rt_ds_declarationContext::DSV() {
  return getToken(SIGParser::DSV, 0);
}

SIGParser::Name_idContext* SIGParser::Rt_ds_declarationContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_ds_declarationContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Rt_ds_declarationContext::getRuleIndex() const {
  return SIGParser::RuleRt_ds_declaration;
}

void SIGParser::Rt_ds_declarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_ds_declaration(this);
}

void SIGParser::Rt_ds_declarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_ds_declaration(this);
}

SIGParser::Rt_ds_declarationContext* SIGParser::rt_ds_declaration() {
  Rt_ds_declarationContext *_localctx = _tracker.createInstance<Rt_ds_declarationContext>(_ctx, getState());
  enterRule(_localctx, 64, SIGParser::RuleRt_ds_declaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(324);
    match(SIGParser::DSV);
    setState(325);
    name_id();
    setState(326);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_statContext ------------------------------------------------------------------

SIGParser::Rt_statContext::Rt_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Rt_color_declarationContext* SIGParser::Rt_statContext::rt_color_declaration() {
  return getRuleContext<SIGParser::Rt_color_declarationContext>(0);
}

SIGParser::Rt_ds_declarationContext* SIGParser::Rt_statContext::rt_ds_declaration() {
  return getRuleContext<SIGParser::Rt_ds_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Rt_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rt_statContext::getRuleIndex() const {
  return SIGParser::RuleRt_stat;
}

void SIGParser::Rt_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_stat(this);
}

void SIGParser::Rt_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_stat(this);
}

SIGParser::Rt_statContext* SIGParser::rt_stat() {
  Rt_statContext *_localctx = _tracker.createInstance<Rt_statContext>(_ctx, getState());
  enterRule(_localctx, 66, SIGParser::RuleRt_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(331);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ID: {
        enterOuterAlt(_localctx, 1);
        setState(328);
        rt_color_declaration();
        break;
      }

      case SIGParser::DSV: {
        enterOuterAlt(_localctx, 2);
        setState(329);
        rt_ds_declaration();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 3);
        setState(330);
        match(SIGParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_blockContext ------------------------------------------------------------------

SIGParser::Rt_blockContext::Rt_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rt_statContext *> SIGParser::Rt_blockContext::rt_stat() {
  return getRuleContexts<SIGParser::Rt_statContext>();
}

SIGParser::Rt_statContext* SIGParser::Rt_blockContext::rt_stat(size_t i) {
  return getRuleContext<SIGParser::Rt_statContext>(i);
}


size_t SIGParser::Rt_blockContext::getRuleIndex() const {
  return SIGParser::RuleRt_block;
}

void SIGParser::Rt_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_block(this);
}

void SIGParser::Rt_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_block(this);
}

SIGParser::Rt_blockContext* SIGParser::rt_block() {
  Rt_blockContext *_localctx = _tracker.createInstance<Rt_blockContext>(_ctx, getState());
  enterRule(_localctx, 68, SIGParser::RuleRt_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(336);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 71) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 71)) & ((1ULL << (SIGParser::DSV - 71))
      | (1ULL << (SIGParser::ID - 71))
      | (1ULL << (SIGParser::COMMENT - 71)))) != 0)) {
      setState(333);
      rt_stat();
      setState(338);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rt_definitionContext ------------------------------------------------------------------

SIGParser::Rt_definitionContext::Rt_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rt_definitionContext::RT() {
  return getToken(SIGParser::RT, 0);
}

SIGParser::Name_idContext* SIGParser::Rt_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rt_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rt_blockContext* SIGParser::Rt_definitionContext::rt_block() {
  return getRuleContext<SIGParser::Rt_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rt_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Rt_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRt_definition;
}

void SIGParser::Rt_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRt_definition(this);
}

void SIGParser::Rt_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRt_definition(this);
}

SIGParser::Rt_definitionContext* SIGParser::rt_definition() {
  Rt_definitionContext *_localctx = _tracker.createInstance<Rt_definitionContext>(_ctx, getState());
  enterRule(_localctx, 70, SIGParser::RuleRt_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(339);
    match(SIGParser::RT);
    setState(340);
    name_id();
    setState(341);
    match(SIGParser::OBRACE);
    setState(342);
    rt_block();
    setState(343);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_value_holderContext ------------------------------------------------------------------

SIGParser::Array_value_holderContext::Array_value_holderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Value_idContext* SIGParser::Array_value_holderContext::value_id() {
  return getRuleContext<SIGParser::Value_idContext>(0);
}


size_t SIGParser::Array_value_holderContext::getRuleIndex() const {
  return SIGParser::RuleArray_value_holder;
}

void SIGParser::Array_value_holderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_holder(this);
}

void SIGParser::Array_value_holderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_holder(this);
}

SIGParser::Array_value_holderContext* SIGParser::array_value_holder() {
  Array_value_holderContext *_localctx = _tracker.createInstance<Array_value_holderContext>(_ctx, getState());
  enterRule(_localctx, 72, SIGParser::RuleArray_value_holder);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(345);
    value_id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Array_value_idsContext ------------------------------------------------------------------

SIGParser::Array_value_idsContext::Array_value_idsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Array_value_idsContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

std::vector<SIGParser::Array_value_holderContext *> SIGParser::Array_value_idsContext::array_value_holder() {
  return getRuleContexts<SIGParser::Array_value_holderContext>();
}

SIGParser::Array_value_holderContext* SIGParser::Array_value_idsContext::array_value_holder(size_t i) {
  return getRuleContext<SIGParser::Array_value_holderContext>(i);
}

tree::TerminalNode* SIGParser::Array_value_idsContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}


size_t SIGParser::Array_value_idsContext::getRuleIndex() const {
  return SIGParser::RuleArray_value_ids;
}

void SIGParser::Array_value_idsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArray_value_ids(this);
}

void SIGParser::Array_value_idsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArray_value_ids(this);
}

SIGParser::Array_value_idsContext* SIGParser::array_value_ids() {
  Array_value_idsContext *_localctx = _tracker.createInstance<Array_value_idsContext>(_ctx, getState());
  enterRule(_localctx, 74, SIGParser::RuleArray_value_ids);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(347);
    match(SIGParser::OBRACE);
    setState(348);
    array_value_holder();
    setState(353);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::T__1) {
      setState(349);
      match(SIGParser::T__1);
      setState(350);
      array_value_holder();
      setState(355);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(356);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Root_sigContext ------------------------------------------------------------------

SIGParser::Root_sigContext::Root_sigContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Root_sigContext::ROOTSIG() {
  return getToken(SIGParser::ROOTSIG, 0);
}

tree::TerminalNode* SIGParser::Root_sigContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Name_idContext* SIGParser::Root_sigContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Root_sigContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}


size_t SIGParser::Root_sigContext::getRuleIndex() const {
  return SIGParser::RuleRoot_sig;
}

void SIGParser::Root_sigContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRoot_sig(this);
}

void SIGParser::Root_sigContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRoot_sig(this);
}

SIGParser::Root_sigContext* SIGParser::root_sig() {
  Root_sigContext *_localctx = _tracker.createInstance<Root_sigContext>(_ctx, getState());
  enterRule(_localctx, 76, SIGParser::RuleRoot_sig);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(358);
    match(SIGParser::ROOTSIG);
    setState(359);
    match(SIGParser::ASSIGN);
    setState(360);
    name_id();
    setState(361);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShaderContext ------------------------------------------------------------------

SIGParser::ShaderContext::ShaderContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Shader_typeContext* SIGParser::ShaderContext::shader_type() {
  return getRuleContext<SIGParser::Shader_typeContext>(0);
}

tree::TerminalNode* SIGParser::ShaderContext::ASSIGN() {
  return getToken(SIGParser::ASSIGN, 0);
}

SIGParser::Path_idContext* SIGParser::ShaderContext::path_id() {
  return getRuleContext<SIGParser::Path_idContext>(0);
}

tree::TerminalNode* SIGParser::ShaderContext::SCOL() {
  return getToken(SIGParser::SCOL, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::ShaderContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::ShaderContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}


size_t SIGParser::ShaderContext::getRuleIndex() const {
  return SIGParser::RuleShader;
}

void SIGParser::ShaderContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader(this);
}

void SIGParser::ShaderContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader(this);
}

SIGParser::ShaderContext* SIGParser::shader() {
  ShaderContext *_localctx = _tracker.createInstance<ShaderContext>(_ctx, getState());
  enterRule(_localctx, 78, SIGParser::RuleShader);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(366);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(363);
        option_block(); 
      }
      setState(368);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
    }
    setState(369);
    shader_type();
    setState(370);
    match(SIGParser::ASSIGN);
    setState(371);
    path_id();
    setState(372);
    match(SIGParser::SCOL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_statContext ------------------------------------------------------------------

SIGParser::Compute_pso_statContext::Compute_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Compute_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

SIGParser::ShaderContext* SIGParser::Compute_pso_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

SIGParser::Define_declarationContext* SIGParser::Compute_pso_statContext::define_declaration() {
  return getRuleContext<SIGParser::Define_declarationContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Compute_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_stat;
}

void SIGParser::Compute_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_stat(this);
}

void SIGParser::Compute_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_stat(this);
}

SIGParser::Compute_pso_statContext* SIGParser::compute_pso_stat() {
  Compute_pso_statContext *_localctx = _tracker.createInstance<Compute_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 80, SIGParser::RuleCompute_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(378);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(374);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(375);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(376);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(377);
      match(SIGParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_blockContext ------------------------------------------------------------------

SIGParser::Compute_pso_blockContext::Compute_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Compute_pso_statContext *> SIGParser::Compute_pso_blockContext::compute_pso_stat() {
  return getRuleContexts<SIGParser::Compute_pso_statContext>();
}

SIGParser::Compute_pso_statContext* SIGParser::Compute_pso_blockContext::compute_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Compute_pso_statContext>(i);
}


size_t SIGParser::Compute_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_block;
}

void SIGParser::Compute_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_block(this);
}

void SIGParser::Compute_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_block(this);
}

SIGParser::Compute_pso_blockContext* SIGParser::compute_pso_block() {
  Compute_pso_blockContext *_localctx = _tracker.createInstance<Compute_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 82, SIGParser::RuleCompute_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(383);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__3)
      | (1ULL << SIGParser::T__7)
      | (1ULL << SIGParser::T__8)
      | (1ULL << SIGParser::T__9)
      | (1ULL << SIGParser::T__10)
      | (1ULL << SIGParser::T__11)
      | (1ULL << SIGParser::T__12)
      | (1ULL << SIGParser::T__13)
      | (1ULL << SIGParser::T__14)
      | (1ULL << SIGParser::T__15)
      | (1ULL << SIGParser::T__16)
      | (1ULL << SIGParser::T__17)
      | (1ULL << SIGParser::OSBRACE))) != 0) || _la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(380);
      compute_pso_stat();
      setState(385);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Compute_pso_definitionContext ------------------------------------------------------------------

SIGParser::Compute_pso_definitionContext::Compute_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::COMPUTE_PSO() {
  return getToken(SIGParser::COMPUTE_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Compute_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Compute_pso_blockContext* SIGParser::Compute_pso_definitionContext::compute_pso_block() {
  return getRuleContext<SIGParser::Compute_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Compute_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Compute_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Compute_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleCompute_pso_definition;
}

void SIGParser::Compute_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompute_pso_definition(this);
}

void SIGParser::Compute_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompute_pso_definition(this);
}

SIGParser::Compute_pso_definitionContext* SIGParser::compute_pso_definition() {
  Compute_pso_definitionContext *_localctx = _tracker.createInstance<Compute_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 84, SIGParser::RuleCompute_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(386);
    match(SIGParser::COMPUTE_PSO);
    setState(387);
    name_id();
    setState(389);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(388);
      inherit();
    }
    setState(391);
    match(SIGParser::OBRACE);
    setState(392);
    compute_pso_block();
    setState(393);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_statContext ------------------------------------------------------------------

SIGParser::Graphics_pso_statContext::Graphics_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Graphics_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

SIGParser::ShaderContext* SIGParser::Graphics_pso_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

SIGParser::Define_declarationContext* SIGParser::Graphics_pso_statContext::define_declaration() {
  return getRuleContext<SIGParser::Define_declarationContext>(0);
}

SIGParser::Rtv_formats_declarationContext* SIGParser::Graphics_pso_statContext::rtv_formats_declaration() {
  return getRuleContext<SIGParser::Rtv_formats_declarationContext>(0);
}

SIGParser::Blends_declarationContext* SIGParser::Graphics_pso_statContext::blends_declaration() {
  return getRuleContext<SIGParser::Blends_declarationContext>(0);
}

SIGParser::Pso_paramContext* SIGParser::Graphics_pso_statContext::pso_param() {
  return getRuleContext<SIGParser::Pso_paramContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Graphics_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_stat;
}

void SIGParser::Graphics_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_stat(this);
}

void SIGParser::Graphics_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_stat(this);
}

SIGParser::Graphics_pso_statContext* SIGParser::graphics_pso_stat() {
  Graphics_pso_statContext *_localctx = _tracker.createInstance<Graphics_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 86, SIGParser::RuleGraphics_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(402);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(395);
      root_sig();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(396);
      shader();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(397);
      define_declaration();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(398);
      rtv_formats_declaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(399);
      blends_declaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(400);
      pso_param();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(401);
      match(SIGParser::COMMENT);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_blockContext ------------------------------------------------------------------

SIGParser::Graphics_pso_blockContext::Graphics_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Graphics_pso_statContext *> SIGParser::Graphics_pso_blockContext::graphics_pso_stat() {
  return getRuleContexts<SIGParser::Graphics_pso_statContext>();
}

SIGParser::Graphics_pso_statContext* SIGParser::Graphics_pso_blockContext::graphics_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Graphics_pso_statContext>(i);
}


size_t SIGParser::Graphics_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_block;
}

void SIGParser::Graphics_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_block(this);
}

void SIGParser::Graphics_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_block(this);
}

SIGParser::Graphics_pso_blockContext* SIGParser::graphics_pso_block() {
  Graphics_pso_blockContext *_localctx = _tracker.createInstance<Graphics_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 88, SIGParser::RuleGraphics_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(407);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__3)
      | (1ULL << SIGParser::T__4)
      | (1ULL << SIGParser::T__5)
      | (1ULL << SIGParser::T__7)
      | (1ULL << SIGParser::T__8)
      | (1ULL << SIGParser::T__9)
      | (1ULL << SIGParser::T__10)
      | (1ULL << SIGParser::T__11)
      | (1ULL << SIGParser::T__12)
      | (1ULL << SIGParser::T__13)
      | (1ULL << SIGParser::T__14)
      | (1ULL << SIGParser::T__15)
      | (1ULL << SIGParser::T__16)
      | (1ULL << SIGParser::T__17)
      | (1ULL << SIGParser::T__18)
      | (1ULL << SIGParser::T__19)
      | (1ULL << SIGParser::T__20)
      | (1ULL << SIGParser::T__21)
      | (1ULL << SIGParser::T__22)
      | (1ULL << SIGParser::T__23)
      | (1ULL << SIGParser::T__24)
      | (1ULL << SIGParser::T__25)
      | (1ULL << SIGParser::T__26)
      | (1ULL << SIGParser::T__27)
      | (1ULL << SIGParser::T__28)
      | (1ULL << SIGParser::T__29)
      | (1ULL << SIGParser::T__30)
      | (1ULL << SIGParser::T__31)
      | (1ULL << SIGParser::T__32)
      | (1ULL << SIGParser::T__33)
      | (1ULL << SIGParser::OSBRACE))) != 0) || _la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(404);
      graphics_pso_stat();
      setState(409);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Graphics_pso_definitionContext ------------------------------------------------------------------

SIGParser::Graphics_pso_definitionContext::Graphics_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::GRAPHICS_PSO() {
  return getToken(SIGParser::GRAPHICS_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Graphics_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Graphics_pso_blockContext* SIGParser::Graphics_pso_definitionContext::graphics_pso_block() {
  return getRuleContext<SIGParser::Graphics_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Graphics_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Graphics_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Graphics_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleGraphics_pso_definition;
}

void SIGParser::Graphics_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGraphics_pso_definition(this);
}

void SIGParser::Graphics_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGraphics_pso_definition(this);
}

SIGParser::Graphics_pso_definitionContext* SIGParser::graphics_pso_definition() {
  Graphics_pso_definitionContext *_localctx = _tracker.createInstance<Graphics_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 90, SIGParser::RuleGraphics_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(410);
    match(SIGParser::GRAPHICS_PSO);
    setState(411);
    name_id();
    setState(413);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(412);
      inherit();
    }
    setState(415);
    match(SIGParser::OBRACE);
    setState(416);
    graphics_pso_block();
    setState(417);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_statContext ------------------------------------------------------------------

SIGParser::Rtx_pso_statContext::Rtx_pso_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::Root_sigContext* SIGParser::Rtx_pso_statContext::root_sig() {
  return getRuleContext<SIGParser::Root_sigContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rtx_pso_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_stat;
}

void SIGParser::Rtx_pso_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_stat(this);
}

void SIGParser::Rtx_pso_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_stat(this);
}

SIGParser::Rtx_pso_statContext* SIGParser::rtx_pso_stat() {
  Rtx_pso_statContext *_localctx = _tracker.createInstance<Rtx_pso_statContext>(_ctx, getState());
  enterRule(_localctx, 92, SIGParser::RuleRtx_pso_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(421);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::ROOTSIG: {
        enterOuterAlt(_localctx, 1);
        setState(419);
        root_sig();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(420);
        match(SIGParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_blockContext ------------------------------------------------------------------

SIGParser::Rtx_pso_blockContext::Rtx_pso_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_pso_statContext *> SIGParser::Rtx_pso_blockContext::rtx_pso_stat() {
  return getRuleContexts<SIGParser::Rtx_pso_statContext>();
}

SIGParser::Rtx_pso_statContext* SIGParser::Rtx_pso_blockContext::rtx_pso_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_pso_statContext>(i);
}


size_t SIGParser::Rtx_pso_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_block;
}

void SIGParser::Rtx_pso_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_block(this);
}

void SIGParser::Rtx_pso_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_block(this);
}

SIGParser::Rtx_pso_blockContext* SIGParser::rtx_pso_block() {
  Rtx_pso_blockContext *_localctx = _tracker.createInstance<Rtx_pso_blockContext>(_ctx, getState());
  enterRule(_localctx, 94, SIGParser::RuleRtx_pso_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(426);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SIGParser::ROOTSIG

    || _la == SIGParser::COMMENT) {
      setState(423);
      rtx_pso_stat();
      setState(428);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pso_definitionContext ------------------------------------------------------------------

SIGParser::Rtx_pso_definitionContext::Rtx_pso_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::RAYTRACE_PSO() {
  return getToken(SIGParser::RAYTRACE_PSO, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_pso_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_pso_blockContext* SIGParser::Rtx_pso_definitionContext::rtx_pso_block() {
  return getRuleContext<SIGParser::Rtx_pso_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pso_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

SIGParser::InheritContext* SIGParser::Rtx_pso_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_pso_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pso_definition;
}

void SIGParser::Rtx_pso_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pso_definition(this);
}

void SIGParser::Rtx_pso_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pso_definition(this);
}

SIGParser::Rtx_pso_definitionContext* SIGParser::rtx_pso_definition() {
  Rtx_pso_definitionContext *_localctx = _tracker.createInstance<Rtx_pso_definitionContext>(_ctx, getState());
  enterRule(_localctx, 96, SIGParser::RuleRtx_pso_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(429);
    match(SIGParser::RAYTRACE_PSO);
    setState(430);
    name_id();
    setState(432);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(431);
      inherit();
    }
    setState(434);
    match(SIGParser::OBRACE);
    setState(435);
    rtx_pso_block();
    setState(436);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_statContext ------------------------------------------------------------------

SIGParser::Rtx_pass_statContext::Rtx_pass_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::ShaderContext* SIGParser::Rtx_pass_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}

SIGParser::Pso_paramContext* SIGParser::Rtx_pass_statContext::pso_param() {
  return getRuleContext<SIGParser::Pso_paramContext>(0);
}


size_t SIGParser::Rtx_pass_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_stat;
}

void SIGParser::Rtx_pass_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_stat(this);
}

void SIGParser::Rtx_pass_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_stat(this);
}

SIGParser::Rtx_pass_statContext* SIGParser::rtx_pass_stat() {
  Rtx_pass_statContext *_localctx = _tracker.createInstance<Rtx_pass_statContext>(_ctx, getState());
  enterRule(_localctx, 98, SIGParser::RuleRtx_pass_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(441);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::T__7:
      case SIGParser::T__8:
      case SIGParser::T__9:
      case SIGParser::T__10:
      case SIGParser::T__11:
      case SIGParser::T__12:
      case SIGParser::T__13:
      case SIGParser::T__14:
      case SIGParser::T__15:
      case SIGParser::T__16:
      case SIGParser::T__17:
      case SIGParser::OSBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(438);
        shader();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(439);
        match(SIGParser::COMMENT);
        break;
      }

      case SIGParser::T__18:
      case SIGParser::T__19:
      case SIGParser::T__20:
      case SIGParser::T__21:
      case SIGParser::T__22:
      case SIGParser::T__23:
      case SIGParser::T__24:
      case SIGParser::T__25:
      case SIGParser::T__26:
      case SIGParser::T__27:
      case SIGParser::T__28:
      case SIGParser::T__29:
      case SIGParser::T__30:
      case SIGParser::T__31:
      case SIGParser::T__32:
      case SIGParser::T__33: {
        enterOuterAlt(_localctx, 3);
        setState(440);
        pso_param();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_blockContext ------------------------------------------------------------------

SIGParser::Rtx_pass_blockContext::Rtx_pass_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_pass_statContext *> SIGParser::Rtx_pass_blockContext::rtx_pass_stat() {
  return getRuleContexts<SIGParser::Rtx_pass_statContext>();
}

SIGParser::Rtx_pass_statContext* SIGParser::Rtx_pass_blockContext::rtx_pass_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_pass_statContext>(i);
}


size_t SIGParser::Rtx_pass_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_block;
}

void SIGParser::Rtx_pass_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_block(this);
}

void SIGParser::Rtx_pass_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_block(this);
}

SIGParser::Rtx_pass_blockContext* SIGParser::rtx_pass_block() {
  Rtx_pass_blockContext *_localctx = _tracker.createInstance<Rtx_pass_blockContext>(_ctx, getState());
  enterRule(_localctx, 100, SIGParser::RuleRtx_pass_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(446);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__7)
      | (1ULL << SIGParser::T__8)
      | (1ULL << SIGParser::T__9)
      | (1ULL << SIGParser::T__10)
      | (1ULL << SIGParser::T__11)
      | (1ULL << SIGParser::T__12)
      | (1ULL << SIGParser::T__13)
      | (1ULL << SIGParser::T__14)
      | (1ULL << SIGParser::T__15)
      | (1ULL << SIGParser::T__16)
      | (1ULL << SIGParser::T__17)
      | (1ULL << SIGParser::T__18)
      | (1ULL << SIGParser::T__19)
      | (1ULL << SIGParser::T__20)
      | (1ULL << SIGParser::T__21)
      | (1ULL << SIGParser::T__22)
      | (1ULL << SIGParser::T__23)
      | (1ULL << SIGParser::T__24)
      | (1ULL << SIGParser::T__25)
      | (1ULL << SIGParser::T__26)
      | (1ULL << SIGParser::T__27)
      | (1ULL << SIGParser::T__28)
      | (1ULL << SIGParser::T__29)
      | (1ULL << SIGParser::T__30)
      | (1ULL << SIGParser::T__31)
      | (1ULL << SIGParser::T__32)
      | (1ULL << SIGParser::T__33)
      | (1ULL << SIGParser::OSBRACE))) != 0) || _la == SIGParser::COMMENT) {
      setState(443);
      rtx_pass_stat();
      setState(448);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_pass_definitionContext ------------------------------------------------------------------

SIGParser::Rtx_pass_definitionContext::Rtx_pass_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::RAYTRACE_PASS() {
  return getToken(SIGParser::RAYTRACE_PASS, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_pass_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_pass_blockContext* SIGParser::Rtx_pass_definitionContext::rtx_pass_block() {
  return getRuleContext<SIGParser::Rtx_pass_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_pass_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtx_pass_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtx_pass_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Rtx_pass_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_pass_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_pass_definition;
}

void SIGParser::Rtx_pass_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_pass_definition(this);
}

void SIGParser::Rtx_pass_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_pass_definition(this);
}

SIGParser::Rtx_pass_definitionContext* SIGParser::rtx_pass_definition() {
  Rtx_pass_definitionContext *_localctx = _tracker.createInstance<Rtx_pass_definitionContext>(_ctx, getState());
  enterRule(_localctx, 102, SIGParser::RuleRtx_pass_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(452);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(449);
        option_block(); 
      }
      setState(454);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx);
    }
    setState(455);
    match(SIGParser::RAYTRACE_PASS);
    setState(456);
    name_id();
    setState(458);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(457);
      inherit();
    }
    setState(460);
    match(SIGParser::OBRACE);
    setState(461);
    rtx_pass_block();
    setState(462);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_statContext ------------------------------------------------------------------

SIGParser::Rtx_raygen_statContext::Rtx_raygen_statContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SIGParser::ShaderContext* SIGParser::Rtx_raygen_statContext::shader() {
  return getRuleContext<SIGParser::ShaderContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_statContext::COMMENT() {
  return getToken(SIGParser::COMMENT, 0);
}


size_t SIGParser::Rtx_raygen_statContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_stat;
}

void SIGParser::Rtx_raygen_statContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_stat(this);
}

void SIGParser::Rtx_raygen_statContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_stat(this);
}

SIGParser::Rtx_raygen_statContext* SIGParser::rtx_raygen_stat() {
  Rtx_raygen_statContext *_localctx = _tracker.createInstance<Rtx_raygen_statContext>(_ctx, getState());
  enterRule(_localctx, 104, SIGParser::RuleRtx_raygen_stat);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(466);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SIGParser::T__7:
      case SIGParser::T__8:
      case SIGParser::T__9:
      case SIGParser::T__10:
      case SIGParser::T__11:
      case SIGParser::T__12:
      case SIGParser::T__13:
      case SIGParser::T__14:
      case SIGParser::T__15:
      case SIGParser::T__16:
      case SIGParser::T__17:
      case SIGParser::OSBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(464);
        shader();
        break;
      }

      case SIGParser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(465);
        match(SIGParser::COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_blockContext ------------------------------------------------------------------

SIGParser::Rtx_raygen_blockContext::Rtx_raygen_blockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SIGParser::Rtx_raygen_statContext *> SIGParser::Rtx_raygen_blockContext::rtx_raygen_stat() {
  return getRuleContexts<SIGParser::Rtx_raygen_statContext>();
}

SIGParser::Rtx_raygen_statContext* SIGParser::Rtx_raygen_blockContext::rtx_raygen_stat(size_t i) {
  return getRuleContext<SIGParser::Rtx_raygen_statContext>(i);
}


size_t SIGParser::Rtx_raygen_blockContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_block;
}

void SIGParser::Rtx_raygen_blockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_block(this);
}

void SIGParser::Rtx_raygen_blockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_block(this);
}

SIGParser::Rtx_raygen_blockContext* SIGParser::rtx_raygen_block() {
  Rtx_raygen_blockContext *_localctx = _tracker.createInstance<Rtx_raygen_blockContext>(_ctx, getState());
  enterRule(_localctx, 106, SIGParser::RuleRtx_raygen_block);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(471);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__7)
      | (1ULL << SIGParser::T__8)
      | (1ULL << SIGParser::T__9)
      | (1ULL << SIGParser::T__10)
      | (1ULL << SIGParser::T__11)
      | (1ULL << SIGParser::T__12)
      | (1ULL << SIGParser::T__13)
      | (1ULL << SIGParser::T__14)
      | (1ULL << SIGParser::T__15)
      | (1ULL << SIGParser::T__16)
      | (1ULL << SIGParser::T__17)
      | (1ULL << SIGParser::OSBRACE))) != 0) || _la == SIGParser::COMMENT) {
      setState(468);
      rtx_raygen_stat();
      setState(473);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Rtx_raygen_definitionContext ------------------------------------------------------------------

SIGParser::Rtx_raygen_definitionContext::Rtx_raygen_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::RAYTRACE_RAYGEN() {
  return getToken(SIGParser::RAYTRACE_RAYGEN, 0);
}

SIGParser::Name_idContext* SIGParser::Rtx_raygen_definitionContext::name_id() {
  return getRuleContext<SIGParser::Name_idContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::OBRACE() {
  return getToken(SIGParser::OBRACE, 0);
}

SIGParser::Rtx_raygen_blockContext* SIGParser::Rtx_raygen_definitionContext::rtx_raygen_block() {
  return getRuleContext<SIGParser::Rtx_raygen_blockContext>(0);
}

tree::TerminalNode* SIGParser::Rtx_raygen_definitionContext::CBRACE() {
  return getToken(SIGParser::CBRACE, 0);
}

std::vector<SIGParser::Option_blockContext *> SIGParser::Rtx_raygen_definitionContext::option_block() {
  return getRuleContexts<SIGParser::Option_blockContext>();
}

SIGParser::Option_blockContext* SIGParser::Rtx_raygen_definitionContext::option_block(size_t i) {
  return getRuleContext<SIGParser::Option_blockContext>(i);
}

SIGParser::InheritContext* SIGParser::Rtx_raygen_definitionContext::inherit() {
  return getRuleContext<SIGParser::InheritContext>(0);
}


size_t SIGParser::Rtx_raygen_definitionContext::getRuleIndex() const {
  return SIGParser::RuleRtx_raygen_definition;
}

void SIGParser::Rtx_raygen_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRtx_raygen_definition(this);
}

void SIGParser::Rtx_raygen_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRtx_raygen_definition(this);
}

SIGParser::Rtx_raygen_definitionContext* SIGParser::rtx_raygen_definition() {
  Rtx_raygen_definitionContext *_localctx = _tracker.createInstance<Rtx_raygen_definitionContext>(_ctx, getState());
  enterRule(_localctx, 108, SIGParser::RuleRtx_raygen_definition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(477);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(474);
        option_block(); 
      }
      setState(479);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    }
    setState(480);
    match(SIGParser::RAYTRACE_RAYGEN);
    setState(481);
    name_id();
    setState(483);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SIGParser::T__6) {
      setState(482);
      inherit();
    }
    setState(485);
    match(SIGParser::OBRACE);
    setState(486);
    rtx_raygen_block();
    setState(487);
    match(SIGParser::CBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Shader_typeContext ------------------------------------------------------------------

SIGParser::Shader_typeContext::Shader_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SIGParser::Shader_typeContext::getRuleIndex() const {
  return SIGParser::RuleShader_type;
}

void SIGParser::Shader_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterShader_type(this);
}

void SIGParser::Shader_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitShader_type(this);
}

SIGParser::Shader_typeContext* SIGParser::shader_type() {
  Shader_typeContext *_localctx = _tracker.createInstance<Shader_typeContext>(_ctx, getState());
  enterRule(_localctx, 110, SIGParser::RuleShader_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(489);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__7)
      | (1ULL << SIGParser::T__8)
      | (1ULL << SIGParser::T__9)
      | (1ULL << SIGParser::T__10)
      | (1ULL << SIGParser::T__11)
      | (1ULL << SIGParser::T__12)
      | (1ULL << SIGParser::T__13)
      | (1ULL << SIGParser::T__14)
      | (1ULL << SIGParser::T__15)
      | (1ULL << SIGParser::T__16)
      | (1ULL << SIGParser::T__17))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Pso_param_idContext ------------------------------------------------------------------

SIGParser::Pso_param_idContext::Pso_param_idContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SIGParser::Pso_param_idContext::getRuleIndex() const {
  return SIGParser::RulePso_param_id;
}

void SIGParser::Pso_param_idContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPso_param_id(this);
}

void SIGParser::Pso_param_idContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPso_param_id(this);
}

SIGParser::Pso_param_idContext* SIGParser::pso_param_id() {
  Pso_param_idContext *_localctx = _tracker.createInstance<Pso_param_idContext>(_ctx, getState());
  enterRule(_localctx, 112, SIGParser::RulePso_param_id);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(491);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SIGParser::T__18)
      | (1ULL << SIGParser::T__19)
      | (1ULL << SIGParser::T__20)
      | (1ULL << SIGParser::T__21)
      | (1ULL << SIGParser::T__22)
      | (1ULL << SIGParser::T__23)
      | (1ULL << SIGParser::T__24)
      | (1ULL << SIGParser::T__25)
      | (1ULL << SIGParser::T__26)
      | (1ULL << SIGParser::T__27)
      | (1ULL << SIGParser::T__28)
      | (1ULL << SIGParser::T__29)
      | (1ULL << SIGParser::T__30)
      | (1ULL << SIGParser::T__31)
      | (1ULL << SIGParser::T__32)
      | (1ULL << SIGParser::T__33))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bool_typeContext ------------------------------------------------------------------

SIGParser::Bool_typeContext::Bool_typeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SIGParser::Bool_typeContext::TRUE() {
  return getToken(SIGParser::TRUE, 0);
}

tree::TerminalNode* SIGParser::Bool_typeContext::FALSE() {
  return getToken(SIGParser::FALSE, 0);
}


size_t SIGParser::Bool_typeContext::getRuleIndex() const {
  return SIGParser::RuleBool_type;
}

void SIGParser::Bool_typeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBool_type(this);
}

void SIGParser::Bool_typeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SIGListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBool_type(this);
}

SIGParser::Bool_typeContext* SIGParser::bool_type() {
  Bool_typeContext *_localctx = _tracker.createInstance<Bool_typeContext>(_ctx, getState());
  enterRule(_localctx, 114, SIGParser::RuleBool_type);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(493);
    _la = _input->LA(1);
    if (!(_la == SIGParser::TRUE

    || _la == SIGParser::FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> SIGParser::_decisionToDFA;
atn::PredictionContextCache SIGParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SIGParser::_atn;
std::vector<uint16_t> SIGParser::_serializedATN;

std::vector<std::string> SIGParser::_ruleNames = {
  "parse", "bind_option", "options_assign", "option", "option_block", "array_count_id", 
  "array", "value_declaration", "slot_declaration", "sampler_declaration", 
  "define_declaration", "rtv_formats_declaration", "blends_declaration", 
  "pso_param", "type_with_template", "inherit_id", "name_id", "type_id", 
  "option_id", "owner_id", "template_id", "value_id", "insert_block", "path_id", 
  "inherit", "layout_stat", "layout_block", "layout_definition", "table_stat", 
  "table_block", "table_definition", "rt_color_declaration", "rt_ds_declaration", 
  "rt_stat", "rt_block", "rt_definition", "array_value_holder", "array_value_ids", 
  "root_sig", "shader", "compute_pso_stat", "compute_pso_block", "compute_pso_definition", 
  "graphics_pso_stat", "graphics_pso_block", "graphics_pso_definition", 
  "rtx_pso_stat", "rtx_pso_block", "rtx_pso_definition", "rtx_pass_stat", 
  "rtx_pass_block", "rtx_pass_definition", "rtx_raygen_stat", "rtx_raygen_block", 
  "rtx_raygen_definition", "shader_type", "pso_param_id", "bool_type"
};

std::vector<std::string> SIGParser::_literalNames = {
  "", "'::'", "','", "'Sampler'", "'define'", "'rtv'", "'blend'", "':'", 
  "'compute'", "'vertex'", "'pixel'", "'domain'", "'hull'", "'geometry'", 
  "'miss'", "'closest_hit'", "'raygen'", "'amplification'", "'mesh'", "'ds'", 
  "'cull'", "'depth_func'", "'depth_write'", "'conservative'", "'enable_depth'", 
  "'topology'", "'enable_stencil'", "'stencil_func'", "'stencil_pass_op'", 
  "'stencil_read_mask'", "'stencil_write_mask'", "'recursion_depth'", "'payload'", 
  "'per_material'", "'local'", "'||'", "'&&'", "'=='", "'!='", "'>'", "'<'", 
  "'>='", "'<='", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "'!'", "';'", 
  "'='", "'('", "')'", "'{'", "'}'", "'['", "']'", "'true'", "'false'", 
  "'log'", "'layout'", "'struct'", "'ComputePSO'", "'GraphicsPSO'", "'RaytracePSO'", 
  "'RaytraceRaygen'", "'RaytracePass'", "'slot'", "'rt'", "'RTV'", "'DSV'", 
  "'root'", "", "", "", "", "", "", "'%{'", "'}%'"
};

std::vector<std::string> SIGParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "OR", 
  "AND", "EQ", "NEQ", "GT", "LT", "GTEQ", "LTEQ", "PLUS", "MINUS", "MULT", 
  "DIV", "MOD", "POW", "NOT", "SCOL", "ASSIGN", "OPAR", "CPAR", "OBRACE", 
  "CBRACE", "OSBRACE", "CSBRACE", "TRUE", "FALSE", "LOG", "LAYOUT", "STRUCT", 
  "COMPUTE_PSO", "GRAPHICS_PSO", "RAYTRACE_PSO", "RAYTRACE_RAYGEN", "RAYTRACE_PASS", 
  "SLOT", "RT", "RTV", "DSV", "ROOTSIG", "ID", "INT_SCALAR", "FLOAT_SCALAR", 
  "STRING", "COMMENT", "SPACE", "INSERT_START", "INSERT_END", "INSERT_BLOCK"
};

dfa::Vocabulary SIGParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SIGParser::_tokenNames;

SIGParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  static const uint16_t serializedATNSegment0[] = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
       0x3, 0x53, 0x1f2, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
       0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 
       0x7, 0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 
       0x4, 0xb, 0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 
       0xe, 0x9, 0xe, 0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 
       0x9, 0x11, 0x4, 0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 
       0x9, 0x14, 0x4, 0x15, 0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 
       0x9, 0x17, 0x4, 0x18, 0x9, 0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 
       0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 
       0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 0x1f, 0x9, 0x1f, 0x4, 0x20, 
       0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 0x9, 0x22, 0x4, 0x23, 
       0x9, 0x23, 0x4, 0x24, 0x9, 0x24, 0x4, 0x25, 0x9, 0x25, 0x4, 0x26, 
       0x9, 0x26, 0x4, 0x27, 0x9, 0x27, 0x4, 0x28, 0x9, 0x28, 0x4, 0x29, 
       0x9, 0x29, 0x4, 0x2a, 0x9, 0x2a, 0x4, 0x2b, 0x9, 0x2b, 0x4, 0x2c, 
       0x9, 0x2c, 0x4, 0x2d, 0x9, 0x2d, 0x4, 0x2e, 0x9, 0x2e, 0x4, 0x2f, 
       0x9, 0x2f, 0x4, 0x30, 0x9, 0x30, 0x4, 0x31, 0x9, 0x31, 0x4, 0x32, 
       0x9, 0x32, 0x4, 0x33, 0x9, 0x33, 0x4, 0x34, 0x9, 0x34, 0x4, 0x35, 
       0x9, 0x35, 0x4, 0x36, 0x9, 0x36, 0x4, 0x37, 0x9, 0x37, 0x4, 0x38, 
       0x9, 0x38, 0x4, 0x39, 0x9, 0x39, 0x4, 0x3a, 0x9, 0x3a, 0x4, 0x3b, 
       0x9, 0x3b, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 
       0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x7, 0x2, 0x80, 0xa, 0x2, 0xc, 
       0x2, 0xe, 0x2, 0x83, 0xb, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 
       0x3, 0x3, 0x3, 0x5, 0x3, 0x8a, 0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 
       0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 0x3, 0x5, 0x5, 0x5, 0x93, 0xa, 
       0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x7, 0x6, 0x99, 0xa, 
       0x6, 0xc, 0x6, 0xe, 0x6, 0x9c, 0xb, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 
       0x7, 0x3, 0x7, 0x3, 0x8, 0x3, 0x8, 0x5, 0x8, 0xa4, 0xa, 0x8, 0x3, 
       0x8, 0x3, 0x8, 0x3, 0x9, 0x7, 0x9, 0xa9, 0xa, 0x9, 0xc, 0x9, 0xe, 
       0x9, 0xac, 0xb, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0xb1, 
       0xa, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 
       0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 
       0x3, 0xc, 0x7, 0xc, 0xc0, 0xa, 0xc, 0xc, 0xc, 0xe, 0xc, 0xc3, 0xb, 
       0xc, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 0x5, 0xc, 0xc9, 0xa, 
       0xc, 0x3, 0xc, 0x3, 0xc, 0x3, 0xd, 0x7, 0xd, 0xce, 0xa, 0xd, 0xc, 
       0xd, 0xe, 0xd, 0xd1, 0xb, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 
       0xd, 0x3, 0xd, 0x3, 0xe, 0x7, 0xe, 0xd9, 0xa, 0xe, 0xc, 0xe, 0xe, 
       0xe, 0xdc, 0xb, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 
       0xe, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0x10, 
       0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x5, 0x10, 0xed, 0xa, 
       0x10, 0x3, 0x11, 0x3, 0x11, 0x3, 0x12, 0x3, 0x12, 0x3, 0x13, 0x3, 
       0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x15, 0x3, 0x15, 0x3, 0x16, 0x3, 
       0x16, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0xff, 
       0xa, 0x17, 0x3, 0x18, 0x3, 0x18, 0x3, 0x19, 0x3, 0x19, 0x7, 0x19, 
       0x105, 0xa, 0x19, 0xc, 0x19, 0xe, 0x19, 0x108, 0xb, 0x19, 0x3, 0x19, 
       0x3, 0x19, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x7, 0x1a, 
       0x110, 0xa, 0x1a, 0xc, 0x1a, 0xe, 0x1a, 0x113, 0xb, 0x1a, 0x3, 0x1b, 
       0x3, 0x1b, 0x3, 0x1b, 0x5, 0x1b, 0x118, 0xa, 0x1b, 0x3, 0x1c, 0x7, 
       0x1c, 0x11b, 0xa, 0x1c, 0xc, 0x1c, 0xe, 0x1c, 0x11e, 0xb, 0x1c, 0x3, 
       0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x5, 0x1d, 0x123, 0xa, 0x1d, 0x3, 0x1d, 
       0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 
       0x5, 0x1e, 0x12c, 0xa, 0x1e, 0x3, 0x1f, 0x7, 0x1f, 0x12f, 0xa, 0x1f, 
       0xc, 0x1f, 0xe, 0x1f, 0x132, 0xb, 0x1f, 0x3, 0x20, 0x7, 0x20, 0x135, 
       0xa, 0x20, 0xc, 0x20, 0xe, 0x20, 0x138, 0xb, 0x20, 0x3, 0x20, 0x3, 
       0x20, 0x3, 0x20, 0x5, 0x20, 0x13d, 0xa, 0x20, 0x3, 0x20, 0x3, 0x20, 
       0x3, 0x20, 0x3, 0x20, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 
       0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x3, 0x23, 0x3, 0x23, 
       0x3, 0x23, 0x5, 0x23, 0x14e, 0xa, 0x23, 0x3, 0x24, 0x7, 0x24, 0x151, 
       0xa, 0x24, 0xc, 0x24, 0xe, 0x24, 0x154, 0xb, 0x24, 0x3, 0x25, 0x3, 
       0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x26, 0x3, 
       0x26, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x7, 0x27, 0x162, 
       0xa, 0x27, 0xc, 0x27, 0xe, 0x27, 0x165, 0xb, 0x27, 0x3, 0x27, 0x3, 
       0x27, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 
       0x29, 0x7, 0x29, 0x16f, 0xa, 0x29, 0xc, 0x29, 0xe, 0x29, 0x172, 0xb, 
       0x29, 0x3, 0x29, 0x3, 0x29, 0x3, 0x29, 0x3, 0x29, 0x3, 0x29, 0x3, 
       0x2a, 0x3, 0x2a, 0x3, 0x2a, 0x3, 0x2a, 0x5, 0x2a, 0x17d, 0xa, 0x2a, 
       0x3, 0x2b, 0x7, 0x2b, 0x180, 0xa, 0x2b, 0xc, 0x2b, 0xe, 0x2b, 0x183, 
       0xb, 0x2b, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x188, 0xa, 
       0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2d, 0x3, 
       0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x5, 
       0x2d, 0x195, 0xa, 0x2d, 0x3, 0x2e, 0x7, 0x2e, 0x198, 0xa, 0x2e, 0xc, 
       0x2e, 0xe, 0x2e, 0x19b, 0xb, 0x2e, 0x3, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 
       0x5, 0x2f, 0x1a0, 0xa, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 0x3, 
       0x2f, 0x3, 0x30, 0x3, 0x30, 0x5, 0x30, 0x1a8, 0xa, 0x30, 0x3, 0x31, 
       0x7, 0x31, 0x1ab, 0xa, 0x31, 0xc, 0x31, 0xe, 0x31, 0x1ae, 0xb, 0x31, 
       0x3, 0x32, 0x3, 0x32, 0x3, 0x32, 0x5, 0x32, 0x1b3, 0xa, 0x32, 0x3, 
       0x32, 0x3, 0x32, 0x3, 0x32, 0x3, 0x32, 0x3, 0x33, 0x3, 0x33, 0x3, 
       0x33, 0x5, 0x33, 0x1bc, 0xa, 0x33, 0x3, 0x34, 0x7, 0x34, 0x1bf, 0xa, 
       0x34, 0xc, 0x34, 0xe, 0x34, 0x1c2, 0xb, 0x34, 0x3, 0x35, 0x7, 0x35, 
       0x1c5, 0xa, 0x35, 0xc, 0x35, 0xe, 0x35, 0x1c8, 0xb, 0x35, 0x3, 0x35, 
       0x3, 0x35, 0x3, 0x35, 0x5, 0x35, 0x1cd, 0xa, 0x35, 0x3, 0x35, 0x3, 
       0x35, 0x3, 0x35, 0x3, 0x35, 0x3, 0x36, 0x3, 0x36, 0x5, 0x36, 0x1d5, 
       0xa, 0x36, 0x3, 0x37, 0x7, 0x37, 0x1d8, 0xa, 0x37, 0xc, 0x37, 0xe, 
       0x37, 0x1db, 0xb, 0x37, 0x3, 0x38, 0x7, 0x38, 0x1de, 0xa, 0x38, 0xc, 
       0x38, 0xe, 0x38, 0x1e1, 0xb, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 
       0x5, 0x38, 0x1e6, 0xa, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 
       0x38, 0x3, 0x39, 0x3, 0x39, 0x3, 0x3a, 0x3, 0x3a, 0x3, 0x3b, 0x3, 
       0x3b, 0x3, 0x3b, 0xc, 0xaa, 0xc1, 0xcf, 0xda, 0x106, 0x111, 0x136, 
       0x170, 0x1c6, 0x1df, 0x2, 0x3c, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 
       0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 
       0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 
       0x3c, 0x3e, 0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 
       0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 
       0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x2, 0x5, 0x3, 0x2, 0xa, 
       0x14, 0x3, 0x2, 0x15, 0x24, 0x3, 0x2, 0x3c, 0x3d, 0x2, 0x1f7, 0x2, 
       0x81, 0x3, 0x2, 0x2, 0x2, 0x4, 0x89, 0x3, 0x2, 0x2, 0x2, 0x6, 0x8d, 
       0x3, 0x2, 0x2, 0x2, 0x8, 0x90, 0x3, 0x2, 0x2, 0x2, 0xa, 0x94, 0x3, 
       0x2, 0x2, 0x2, 0xc, 0x9f, 0x3, 0x2, 0x2, 0x2, 0xe, 0xa1, 0x3, 0x2, 
       0x2, 0x2, 0x10, 0xaa, 0x3, 0x2, 0x2, 0x2, 0x12, 0xb4, 0x3, 0x2, 0x2, 
       0x2, 0x14, 0xb8, 0x3, 0x2, 0x2, 0x2, 0x16, 0xc1, 0x3, 0x2, 0x2, 0x2, 
       0x18, 0xcf, 0x3, 0x2, 0x2, 0x2, 0x1a, 0xda, 0x3, 0x2, 0x2, 0x2, 0x1c, 
       0xe2, 0x3, 0x2, 0x2, 0x2, 0x1e, 0xe7, 0x3, 0x2, 0x2, 0x2, 0x20, 0xee, 
       0x3, 0x2, 0x2, 0x2, 0x22, 0xf0, 0x3, 0x2, 0x2, 0x2, 0x24, 0xf2, 0x3, 
       0x2, 0x2, 0x2, 0x26, 0xf4, 0x3, 0x2, 0x2, 0x2, 0x28, 0xf6, 0x3, 0x2, 
       0x2, 0x2, 0x2a, 0xf8, 0x3, 0x2, 0x2, 0x2, 0x2c, 0xfe, 0x3, 0x2, 0x2, 
       0x2, 0x2e, 0x100, 0x3, 0x2, 0x2, 0x2, 0x30, 0x106, 0x3, 0x2, 0x2, 
       0x2, 0x32, 0x10b, 0x3, 0x2, 0x2, 0x2, 0x34, 0x117, 0x3, 0x2, 0x2, 
       0x2, 0x36, 0x11c, 0x3, 0x2, 0x2, 0x2, 0x38, 0x11f, 0x3, 0x2, 0x2, 
       0x2, 0x3a, 0x12b, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x130, 0x3, 0x2, 0x2, 
       0x2, 0x3e, 0x136, 0x3, 0x2, 0x2, 0x2, 0x40, 0x142, 0x3, 0x2, 0x2, 
       0x2, 0x42, 0x146, 0x3, 0x2, 0x2, 0x2, 0x44, 0x14d, 0x3, 0x2, 0x2, 
       0x2, 0x46, 0x152, 0x3, 0x2, 0x2, 0x2, 0x48, 0x155, 0x3, 0x2, 0x2, 
       0x2, 0x4a, 0x15b, 0x3, 0x2, 0x2, 0x2, 0x4c, 0x15d, 0x3, 0x2, 0x2, 
       0x2, 0x4e, 0x168, 0x3, 0x2, 0x2, 0x2, 0x50, 0x170, 0x3, 0x2, 0x2, 
       0x2, 0x52, 0x17c, 0x3, 0x2, 0x2, 0x2, 0x54, 0x181, 0x3, 0x2, 0x2, 
       0x2, 0x56, 0x184, 0x3, 0x2, 0x2, 0x2, 0x58, 0x194, 0x3, 0x2, 0x2, 
       0x2, 0x5a, 0x199, 0x3, 0x2, 0x2, 0x2, 0x5c, 0x19c, 0x3, 0x2, 0x2, 
       0x2, 0x5e, 0x1a7, 0x3, 0x2, 0x2, 0x2, 0x60, 0x1ac, 0x3, 0x2, 0x2, 
       0x2, 0x62, 0x1af, 0x3, 0x2, 0x2, 0x2, 0x64, 0x1bb, 0x3, 0x2, 0x2, 
       0x2, 0x66, 0x1c0, 0x3, 0x2, 0x2, 0x2, 0x68, 0x1c6, 0x3, 0x2, 0x2, 
       0x2, 0x6a, 0x1d4, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x1d9, 0x3, 0x2, 0x2, 
       0x2, 0x6e, 0x1df, 0x3, 0x2, 0x2, 0x2, 0x70, 0x1eb, 0x3, 0x2, 0x2, 
       0x2, 0x72, 0x1ed, 0x3, 0x2, 0x2, 0x2, 0x74, 0x1ef, 0x3, 0x2, 0x2, 
       0x2, 0x76, 0x80, 0x5, 0x38, 0x1d, 0x2, 0x77, 0x80, 0x5, 0x3e, 0x20, 
       0x2, 0x78, 0x80, 0x5, 0x48, 0x25, 0x2, 0x79, 0x80, 0x5, 0x56, 0x2c, 
       0x2, 0x7a, 0x80, 0x5, 0x5c, 0x2f, 0x2, 0x7b, 0x80, 0x5, 0x62, 0x32, 
       0x2, 0x7c, 0x80, 0x5, 0x68, 0x35, 0x2, 0x7d, 0x80, 0x5, 0x6e, 0x38, 
       0x2, 0x7e, 0x80, 0x7, 0x4f, 0x2, 0x2, 0x7f, 0x76, 0x3, 0x2, 0x2, 
       0x2, 0x7f, 0x77, 0x3, 0x2, 0x2, 0x2, 0x7f, 0x78, 0x3, 0x2, 0x2, 0x2, 
       0x7f, 0x79, 0x3, 0x2, 0x2, 0x2, 0x7f, 0x7a, 0x3, 0x2, 0x2, 0x2, 0x7f, 
       0x7b, 0x3, 0x2, 0x2, 0x2, 0x7f, 0x7c, 0x3, 0x2, 0x2, 0x2, 0x7f, 0x7d, 
       0x3, 0x2, 0x2, 0x2, 0x7f, 0x7e, 0x3, 0x2, 0x2, 0x2, 0x80, 0x83, 0x3, 
       0x2, 0x2, 0x2, 0x81, 0x7f, 0x3, 0x2, 0x2, 0x2, 0x81, 0x82, 0x3, 0x2, 
       0x2, 0x2, 0x82, 0x84, 0x3, 0x2, 0x2, 0x2, 0x83, 0x81, 0x3, 0x2, 0x2, 
       0x2, 0x84, 0x85, 0x7, 0x2, 0x2, 0x3, 0x85, 0x3, 0x3, 0x2, 0x2, 0x2, 
       0x86, 0x87, 0x5, 0x28, 0x15, 0x2, 0x87, 0x88, 0x7, 0x3, 0x2, 0x2, 
       0x88, 0x8a, 0x3, 0x2, 0x2, 0x2, 0x89, 0x86, 0x3, 0x2, 0x2, 0x2, 0x89, 
       0x8a, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x8b, 0x3, 0x2, 0x2, 0x2, 0x8b, 0x8c, 
       0x5, 0x2c, 0x17, 0x2, 0x8c, 0x5, 0x3, 0x2, 0x2, 0x2, 0x8d, 0x8e, 
       0x7, 0x35, 0x2, 0x2, 0x8e, 0x8f, 0x5, 0x4, 0x3, 0x2, 0x8f, 0x7, 0x3, 
       0x2, 0x2, 0x2, 0x90, 0x92, 0x5, 0x22, 0x12, 0x2, 0x91, 0x93, 0x5, 
       0x6, 0x4, 0x2, 0x92, 0x91, 0x3, 0x2, 0x2, 0x2, 0x92, 0x93, 0x3, 0x2, 
       0x2, 0x2, 0x93, 0x9, 0x3, 0x2, 0x2, 0x2, 0x94, 0x95, 0x7, 0x3a, 0x2, 
       0x2, 0x95, 0x9a, 0x5, 0x8, 0x5, 0x2, 0x96, 0x97, 0x7, 0x4, 0x2, 0x2, 
       0x97, 0x99, 0x5, 0x8, 0x5, 0x2, 0x98, 0x96, 0x3, 0x2, 0x2, 0x2, 0x99, 
       0x9c, 0x3, 0x2, 0x2, 0x2, 0x9a, 0x98, 0x3, 0x2, 0x2, 0x2, 0x9a, 0x9b, 
       0x3, 0x2, 0x2, 0x2, 0x9b, 0x9d, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x9a, 0x3, 
       0x2, 0x2, 0x2, 0x9d, 0x9e, 0x7, 0x3b, 0x2, 0x2, 0x9e, 0xb, 0x3, 0x2, 
       0x2, 0x2, 0x9f, 0xa0, 0x7, 0x4c, 0x2, 0x2, 0xa0, 0xd, 0x3, 0x2, 0x2, 
       0x2, 0xa1, 0xa3, 0x7, 0x3a, 0x2, 0x2, 0xa2, 0xa4, 0x5, 0xc, 0x7, 
       0x2, 0xa3, 0xa2, 0x3, 0x2, 0x2, 0x2, 0xa3, 0xa4, 0x3, 0x2, 0x2, 0x2, 
       0xa4, 0xa5, 0x3, 0x2, 0x2, 0x2, 0xa5, 0xa6, 0x7, 0x3b, 0x2, 0x2, 
       0xa6, 0xf, 0x3, 0x2, 0x2, 0x2, 0xa7, 0xa9, 0x5, 0xa, 0x6, 0x2, 0xa8, 
       0xa7, 0x3, 0x2, 0x2, 0x2, 0xa9, 0xac, 0x3, 0x2, 0x2, 0x2, 0xaa, 0xab, 
       0x3, 0x2, 0x2, 0x2, 0xaa, 0xa8, 0x3, 0x2, 0x2, 0x2, 0xab, 0xad, 0x3, 
       0x2, 0x2, 0x2, 0xac, 0xaa, 0x3, 0x2, 0x2, 0x2, 0xad, 0xae, 0x5, 0x24, 
       0x13, 0x2, 0xae, 0xb0, 0x5, 0x22, 0x12, 0x2, 0xaf, 0xb1, 0x5, 0xe, 
       0x8, 0x2, 0xb0, 0xaf, 0x3, 0x2, 0x2, 0x2, 0xb0, 0xb1, 0x3, 0x2, 0x2, 
       0x2, 0xb1, 0xb2, 0x3, 0x2, 0x2, 0x2, 0xb2, 0xb3, 0x7, 0x34, 0x2, 
       0x2, 0xb3, 0x11, 0x3, 0x2, 0x2, 0x2, 0xb4, 0xb5, 0x7, 0x46, 0x2, 
       0x2, 0xb5, 0xb6, 0x5, 0x22, 0x12, 0x2, 0xb6, 0xb7, 0x7, 0x34, 0x2, 
       0x2, 0xb7, 0x13, 0x3, 0x2, 0x2, 0x2, 0xb8, 0xb9, 0x7, 0x5, 0x2, 0x2, 
       0xb9, 0xba, 0x5, 0x22, 0x12, 0x2, 0xba, 0xbb, 0x7, 0x35, 0x2, 0x2, 
       0xbb, 0xbc, 0x5, 0x2c, 0x17, 0x2, 0xbc, 0xbd, 0x7, 0x34, 0x2, 0x2, 
       0xbd, 0x15, 0x3, 0x2, 0x2, 0x2, 0xbe, 0xc0, 0x5, 0xa, 0x6, 0x2, 0xbf, 
       0xbe, 0x3, 0x2, 0x2, 0x2, 0xc0, 0xc3, 0x3, 0x2, 0x2, 0x2, 0xc1, 0xc2, 
       0x3, 0x2, 0x2, 0x2, 0xc1, 0xbf, 0x3, 0x2, 0x2, 0x2, 0xc2, 0xc4, 0x3, 
       0x2, 0x2, 0x2, 0xc3, 0xc1, 0x3, 0x2, 0x2, 0x2, 0xc4, 0xc5, 0x7, 0x6, 
       0x2, 0x2, 0xc5, 0xc8, 0x5, 0x22, 0x12, 0x2, 0xc6, 0xc7, 0x7, 0x35, 
       0x2, 0x2, 0xc7, 0xc9, 0x5, 0x4c, 0x27, 0x2, 0xc8, 0xc6, 0x3, 0x2, 
       0x2, 0x2, 0xc8, 0xc9, 0x3, 0x2, 0x2, 0x2, 0xc9, 0xca, 0x3, 0x2, 0x2, 
       0x2, 0xca, 0xcb, 0x7, 0x34, 0x2, 0x2, 0xcb, 0x17, 0x3, 0x2, 0x2, 
       0x2, 0xcc, 0xce, 0x5, 0xa, 0x6, 0x2, 0xcd, 0xcc, 0x3, 0x2, 0x2, 0x2, 
       0xce, 0xd1, 0x3, 0x2, 0x2, 0x2, 0xcf, 0xd0, 0x3, 0x2, 0x2, 0x2, 0xcf, 
       0xcd, 0x3, 0x2, 0x2, 0x2, 0xd0, 0xd2, 0x3, 0x2, 0x2, 0x2, 0xd1, 0xcf, 
       0x3, 0x2, 0x2, 0x2, 0xd2, 0xd3, 0x7, 0x7, 0x2, 0x2, 0xd3, 0xd4, 0x7, 
       0x35, 0x2, 0x2, 0xd4, 0xd5, 0x5, 0x4c, 0x27, 0x2, 0xd5, 0xd6, 0x7, 
       0x34, 0x2, 0x2, 0xd6, 0x19, 0x3, 0x2, 0x2, 0x2, 0xd7, 0xd9, 0x5, 
       0xa, 0x6, 0x2, 0xd8, 0xd7, 0x3, 0x2, 0x2, 0x2, 0xd9, 0xdc, 0x3, 0x2, 
       0x2, 0x2, 0xda, 0xdb, 0x3, 0x2, 0x2, 0x2, 0xda, 0xd8, 0x3, 0x2, 0x2, 
       0x2, 0xdb, 0xdd, 0x3, 0x2, 0x2, 0x2, 0xdc, 0xda, 0x3, 0x2, 0x2, 0x2, 
       0xdd, 0xde, 0x7, 0x8, 0x2, 0x2, 0xde, 0xdf, 0x7, 0x35, 0x2, 0x2, 
       0xdf, 0xe0, 0x5, 0x4c, 0x27, 0x2, 0xe0, 0xe1, 0x7, 0x34, 0x2, 0x2, 
       0xe1, 0x1b, 0x3, 0x2, 0x2, 0x2, 0xe2, 0xe3, 0x5, 0x72, 0x3a, 0x2, 
       0xe3, 0xe4, 0x7, 0x35, 0x2, 0x2, 0xe4, 0xe5, 0x5, 0x2c, 0x17, 0x2, 
       0xe5, 0xe6, 0x7, 0x34, 0x2, 0x2, 0xe6, 0x1d, 0x3, 0x2, 0x2, 0x2, 
       0xe7, 0xec, 0x7, 0x4b, 0x2, 0x2, 0xe8, 0xe9, 0x7, 0x2a, 0x2, 0x2, 
       0xe9, 0xea, 0x5, 0x2a, 0x16, 0x2, 0xea, 0xeb, 0x7, 0x29, 0x2, 0x2, 
       0xeb, 0xed, 0x3, 0x2, 0x2, 0x2, 0xec, 0xe8, 0x3, 0x2, 0x2, 0x2, 0xec, 
       0xed, 0x3, 0x2, 0x2, 0x2, 0xed, 0x1f, 0x3, 0x2, 0x2, 0x2, 0xee, 0xef, 
       0x7, 0x4b, 0x2, 0x2, 0xef, 0x21, 0x3, 0x2, 0x2, 0x2, 0xf0, 0xf1, 
       0x7, 0x4b, 0x2, 0x2, 0xf1, 0x23, 0x3, 0x2, 0x2, 0x2, 0xf2, 0xf3, 
       0x5, 0x1e, 0x10, 0x2, 0xf3, 0x25, 0x3, 0x2, 0x2, 0x2, 0xf4, 0xf5, 
       0x7, 0x4b, 0x2, 0x2, 0xf5, 0x27, 0x3, 0x2, 0x2, 0x2, 0xf6, 0xf7, 
       0x7, 0x4b, 0x2, 0x2, 0xf7, 0x29, 0x3, 0x2, 0x2, 0x2, 0xf8, 0xf9, 
       0x7, 0x4b, 0x2, 0x2, 0xf9, 0x2b, 0x3, 0x2, 0x2, 0x2, 0xfa, 0xff, 
       0x7, 0x4b, 0x2, 0x2, 0xfb, 0xff, 0x7, 0x4c, 0x2, 0x2, 0xfc, 0xff, 
       0x7, 0x4d, 0x2, 0x2, 0xfd, 0xff, 0x5, 0x74, 0x3b, 0x2, 0xfe, 0xfa, 
       0x3, 0x2, 0x2, 0x2, 0xfe, 0xfb, 0x3, 0x2, 0x2, 0x2, 0xfe, 0xfc, 0x3, 
       0x2, 0x2, 0x2, 0xfe, 0xfd, 0x3, 0x2, 0x2, 0x2, 0xff, 0x2d, 0x3, 0x2, 
       0x2, 0x2, 0x100, 0x101, 0x7, 0x53, 0x2, 0x2, 0x101, 0x2f, 0x3, 0x2, 
       0x2, 0x2, 0x102, 0x103, 0x7, 0x4b, 0x2, 0x2, 0x103, 0x105, 0x7, 0x30, 
       0x2, 0x2, 0x104, 0x102, 0x3, 0x2, 0x2, 0x2, 0x105, 0x108, 0x3, 0x2, 
       0x2, 0x2, 0x106, 0x107, 0x3, 0x2, 0x2, 0x2, 0x106, 0x104, 0x3, 0x2, 
       0x2, 0x2, 0x107, 0x109, 0x3, 0x2, 0x2, 0x2, 0x108, 0x106, 0x3, 0x2, 
       0x2, 0x2, 0x109, 0x10a, 0x7, 0x4b, 0x2, 0x2, 0x10a, 0x31, 0x3, 0x2, 
       0x2, 0x2, 0x10b, 0x10c, 0x7, 0x9, 0x2, 0x2, 0x10c, 0x111, 0x5, 0x20, 
       0x11, 0x2, 0x10d, 0x10e, 0x7, 0x4, 0x2, 0x2, 0x10e, 0x110, 0x5, 0x20, 
       0x11, 0x2, 0x10f, 0x10d, 0x3, 0x2, 0x2, 0x2, 0x110, 0x113, 0x3, 0x2, 
       0x2, 0x2, 0x111, 0x112, 0x3, 0x2, 0x2, 0x2, 0x111, 0x10f, 0x3, 0x2, 
       0x2, 0x2, 0x112, 0x33, 0x3, 0x2, 0x2, 0x2, 0x113, 0x111, 0x3, 0x2, 
       0x2, 0x2, 0x114, 0x118, 0x5, 0x12, 0xa, 0x2, 0x115, 0x118, 0x5, 0x14, 
       0xb, 0x2, 0x116, 0x118, 0x7, 0x4f, 0x2, 0x2, 0x117, 0x114, 0x3, 0x2, 
       0x2, 0x2, 0x117, 0x115, 0x3, 0x2, 0x2, 0x2, 0x117, 0x116, 0x3, 0x2, 
       0x2, 0x2, 0x118, 0x35, 0x3, 0x2, 0x2, 0x2, 0x119, 0x11b, 0x5, 0x34, 
       0x1b, 0x2, 0x11a, 0x119, 0x3, 0x2, 0x2, 0x2, 0x11b, 0x11e, 0x3, 0x2, 
       0x2, 0x2, 0x11c, 0x11a, 0x3, 0x2, 0x2, 0x2, 0x11c, 0x11d, 0x3, 0x2, 
       0x2, 0x2, 0x11d, 0x37, 0x3, 0x2, 0x2, 0x2, 0x11e, 0x11c, 0x3, 0x2, 
       0x2, 0x2, 0x11f, 0x120, 0x7, 0x3f, 0x2, 0x2, 0x120, 0x122, 0x5, 0x22, 
       0x12, 0x2, 0x121, 0x123, 0x5, 0x32, 0x1a, 0x2, 0x122, 0x121, 0x3, 
       0x2, 0x2, 0x2, 0x122, 0x123, 0x3, 0x2, 0x2, 0x2, 0x123, 0x124, 0x3, 
       0x2, 0x2, 0x2, 0x124, 0x125, 0x7, 0x38, 0x2, 0x2, 0x125, 0x126, 0x5, 
       0x36, 0x1c, 0x2, 0x126, 0x127, 0x7, 0x39, 0x2, 0x2, 0x127, 0x39, 
       0x3, 0x2, 0x2, 0x2, 0x128, 0x12c, 0x5, 0x10, 0x9, 0x2, 0x129, 0x12c, 
       0x5, 0x2e, 0x18, 0x2, 0x12a, 0x12c, 0x7, 0x4f, 0x2, 0x2, 0x12b, 0x128, 
       0x3, 0x2, 0x2, 0x2, 0x12b, 0x129, 0x3, 0x2, 0x2, 0x2, 0x12b, 0x12a, 
       0x3, 0x2, 0x2, 0x2, 0x12c, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x12d, 0x12f, 
       0x5, 0x3a, 0x1e, 0x2, 0x12e, 0x12d, 0x3, 0x2, 0x2, 0x2, 0x12f, 0x132, 
       0x3, 0x2, 0x2, 0x2, 0x130, 0x12e, 0x3, 0x2, 0x2, 0x2, 0x130, 0x131, 
       0x3, 0x2, 0x2, 0x2, 0x131, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x132, 0x130, 
       0x3, 0x2, 0x2, 0x2, 0x133, 0x135, 0x5, 0xa, 0x6, 0x2, 0x134, 0x133, 
       0x3, 0x2, 0x2, 0x2, 0x135, 0x138, 0x3, 0x2, 0x2, 0x2, 0x136, 0x137, 
       0x3, 0x2, 0x2, 0x2, 0x136, 0x134, 0x3, 0x2, 0x2, 0x2, 0x137, 0x139, 
       0x3, 0x2, 0x2, 0x2, 0x138, 0x136, 0x3, 0x2, 0x2, 0x2, 0x139, 0x13a, 
       0x7, 0x40, 0x2, 0x2, 0x13a, 0x13c, 0x5, 0x22, 0x12, 0x2, 0x13b, 0x13d, 
       0x5, 0x32, 0x1a, 0x2, 0x13c, 0x13b, 0x3, 0x2, 0x2, 0x2, 0x13c, 0x13d, 
       0x3, 0x2, 0x2, 0x2, 0x13d, 0x13e, 0x3, 0x2, 0x2, 0x2, 0x13e, 0x13f, 
       0x7, 0x38, 0x2, 0x2, 0x13f, 0x140, 0x5, 0x3c, 0x1f, 0x2, 0x140, 0x141, 
       0x7, 0x39, 0x2, 0x2, 0x141, 0x3f, 0x3, 0x2, 0x2, 0x2, 0x142, 0x143, 
       0x5, 0x24, 0x13, 0x2, 0x143, 0x144, 0x5, 0x22, 0x12, 0x2, 0x144, 
       0x145, 0x7, 0x34, 0x2, 0x2, 0x145, 0x41, 0x3, 0x2, 0x2, 0x2, 0x146, 
       0x147, 0x7, 0x49, 0x2, 0x2, 0x147, 0x148, 0x5, 0x22, 0x12, 0x2, 0x148, 
       0x149, 0x7, 0x34, 0x2, 0x2, 0x149, 0x43, 0x3, 0x2, 0x2, 0x2, 0x14a, 
       0x14e, 0x5, 0x40, 0x21, 0x2, 0x14b, 0x14e, 0x5, 0x42, 0x22, 0x2, 
       0x14c, 0x14e, 0x7, 0x4f, 0x2, 0x2, 0x14d, 0x14a, 0x3, 0x2, 0x2, 0x2, 
       0x14d, 0x14b, 0x3, 0x2, 0x2, 0x2, 0x14d, 0x14c, 0x3, 0x2, 0x2, 0x2, 
       0x14e, 0x45, 0x3, 0x2, 0x2, 0x2, 0x14f, 0x151, 0x5, 0x44, 0x23, 0x2, 
       0x150, 0x14f, 0x3, 0x2, 0x2, 0x2, 0x151, 0x154, 0x3, 0x2, 0x2, 0x2, 
       0x152, 0x150, 0x3, 0x2, 0x2, 0x2, 0x152, 0x153, 0x3, 0x2, 0x2, 0x2, 
       0x153, 0x47, 0x3, 0x2, 0x2, 0x2, 0x154, 0x152, 0x3, 0x2, 0x2, 0x2, 
       0x155, 0x156, 0x7, 0x47, 0x2, 0x2, 0x156, 0x157, 0x5, 0x22, 0x12, 
       0x2, 0x157, 0x158, 0x7, 0x38, 0x2, 0x2, 0x158, 0x159, 0x5, 0x46, 
       0x24, 0x2, 0x159, 0x15a, 0x7, 0x39, 0x2, 0x2, 0x15a, 0x49, 0x3, 0x2, 
       0x2, 0x2, 0x15b, 0x15c, 0x5, 0x2c, 0x17, 0x2, 0x15c, 0x4b, 0x3, 0x2, 
       0x2, 0x2, 0x15d, 0x15e, 0x7, 0x38, 0x2, 0x2, 0x15e, 0x163, 0x5, 0x4a, 
       0x26, 0x2, 0x15f, 0x160, 0x7, 0x4, 0x2, 0x2, 0x160, 0x162, 0x5, 0x4a, 
       0x26, 0x2, 0x161, 0x15f, 0x3, 0x2, 0x2, 0x2, 0x162, 0x165, 0x3, 0x2, 
       0x2, 0x2, 0x163, 0x161, 0x3, 0x2, 0x2, 0x2, 0x163, 0x164, 0x3, 0x2, 
       0x2, 0x2, 0x164, 0x166, 0x3, 0x2, 0x2, 0x2, 0x165, 0x163, 0x3, 0x2, 
       0x2, 0x2, 0x166, 0x167, 0x7, 0x39, 0x2, 0x2, 0x167, 0x4d, 0x3, 0x2, 
       0x2, 0x2, 0x168, 0x169, 0x7, 0x4a, 0x2, 0x2, 0x169, 0x16a, 0x7, 0x35, 
       0x2, 0x2, 0x16a, 0x16b, 0x5, 0x22, 0x12, 0x2, 0x16b, 0x16c, 0x7, 
       0x34, 0x2, 0x2, 0x16c, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x16d, 0x16f, 0x5, 
       0xa, 0x6, 0x2, 0x16e, 0x16d, 0x3, 0x2, 0x2, 0x2, 0x16f, 0x172, 0x3, 
       0x2, 0x2, 0x2, 0x170, 0x171, 0x3, 0x2, 0x2, 0x2, 0x170, 0x16e, 0x3, 
       0x2, 0x2, 0x2, 0x171, 0x173, 0x3, 0x2, 0x2, 0x2, 0x172, 0x170, 0x3, 
       0x2, 0x2, 0x2, 0x173, 0x174, 0x5, 0x70, 0x39, 0x2, 0x174, 0x175, 
       0x7, 0x35, 0x2, 0x2, 0x175, 0x176, 0x5, 0x30, 0x19, 0x2, 0x176, 0x177, 
       0x7, 0x34, 0x2, 0x2, 0x177, 0x51, 0x3, 0x2, 0x2, 0x2, 0x178, 0x17d, 
       0x5, 0x4e, 0x28, 0x2, 0x179, 0x17d, 0x5, 0x50, 0x29, 0x2, 0x17a, 
       0x17d, 0x5, 0x16, 0xc, 0x2, 0x17b, 0x17d, 0x7, 0x4f, 0x2, 0x2, 0x17c, 
       0x178, 0x3, 0x2, 0x2, 0x2, 0x17c, 0x179, 0x3, 0x2, 0x2, 0x2, 0x17c, 
       0x17a, 0x3, 0x2, 0x2, 0x2, 0x17c, 0x17b, 0x3, 0x2, 0x2, 0x2, 0x17d, 
       0x53, 0x3, 0x2, 0x2, 0x2, 0x17e, 0x180, 0x5, 0x52, 0x2a, 0x2, 0x17f, 
       0x17e, 0x3, 0x2, 0x2, 0x2, 0x180, 0x183, 0x3, 0x2, 0x2, 0x2, 0x181, 
       0x17f, 0x3, 0x2, 0x2, 0x2, 0x181, 0x182, 0x3, 0x2, 0x2, 0x2, 0x182, 
       0x55, 0x3, 0x2, 0x2, 0x2, 0x183, 0x181, 0x3, 0x2, 0x2, 0x2, 0x184, 
       0x185, 0x7, 0x41, 0x2, 0x2, 0x185, 0x187, 0x5, 0x22, 0x12, 0x2, 0x186, 
       0x188, 0x5, 0x32, 0x1a, 0x2, 0x187, 0x186, 0x3, 0x2, 0x2, 0x2, 0x187, 
       0x188, 0x3, 0x2, 0x2, 0x2, 0x188, 0x189, 0x3, 0x2, 0x2, 0x2, 0x189, 
       0x18a, 0x7, 0x38, 0x2, 0x2, 0x18a, 0x18b, 0x5, 0x54, 0x2b, 0x2, 0x18b, 
       0x18c, 0x7, 0x39, 0x2, 0x2, 0x18c, 0x57, 0x3, 0x2, 0x2, 0x2, 0x18d, 
       0x195, 0x5, 0x4e, 0x28, 0x2, 0x18e, 0x195, 0x5, 0x50, 0x29, 0x2, 
       0x18f, 0x195, 0x5, 0x16, 0xc, 0x2, 0x190, 0x195, 0x5, 0x18, 0xd, 
       0x2, 0x191, 0x195, 0x5, 0x1a, 0xe, 0x2, 0x192, 0x195, 0x5, 0x1c, 
       0xf, 0x2, 0x193, 0x195, 0x7, 0x4f, 0x2, 0x2, 0x194, 0x18d, 0x3, 0x2, 
       0x2, 0x2, 0x194, 0x18e, 0x3, 0x2, 0x2, 0x2, 0x194, 0x18f, 0x3, 0x2, 
       0x2, 0x2, 0x194, 0x190, 0x3, 0x2, 0x2, 0x2, 0x194, 0x191, 0x3, 0x2, 
       0x2, 0x2, 0x194, 0x192, 0x3, 0x2, 0x2, 0x2, 0x194, 0x193, 0x3, 0x2, 
       0x2, 0x2, 0x195, 0x59, 0x3, 0x2, 0x2, 0x2, 0x196, 0x198, 0x5, 0x58, 
       0x2d, 0x2, 0x197, 0x196, 0x3, 0x2, 0x2, 0x2, 0x198, 0x19b, 0x3, 0x2, 
       0x2, 0x2, 0x199, 0x197, 0x3, 0x2, 0x2, 0x2, 0x199, 0x19a, 0x3, 0x2, 
       0x2, 0x2, 0x19a, 0x5b, 0x3, 0x2, 0x2, 0x2, 0x19b, 0x199, 0x3, 0x2, 
       0x2, 0x2, 0x19c, 0x19d, 0x7, 0x42, 0x2, 0x2, 0x19d, 0x19f, 0x5, 0x22, 
       0x12, 0x2, 0x19e, 0x1a0, 0x5, 0x32, 0x1a, 0x2, 0x19f, 0x19e, 0x3, 
       0x2, 0x2, 0x2, 0x19f, 0x1a0, 0x3, 0x2, 0x2, 0x2, 0x1a0, 0x1a1, 0x3, 
       0x2, 0x2, 0x2, 0x1a1, 0x1a2, 0x7, 0x38, 0x2, 0x2, 0x1a2, 0x1a3, 0x5, 
       0x5a, 0x2e, 0x2, 0x1a3, 0x1a4, 0x7, 0x39, 0x2, 0x2, 0x1a4, 0x5d, 
       0x3, 0x2, 0x2, 0x2, 0x1a5, 0x1a8, 0x5, 0x4e, 0x28, 0x2, 0x1a6, 0x1a8, 
       0x7, 0x4f, 0x2, 0x2, 0x1a7, 0x1a5, 0x3, 0x2, 0x2, 0x2, 0x1a7, 0x1a6, 
       0x3, 0x2, 0x2, 0x2, 0x1a8, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x1a9, 0x1ab, 
       0x5, 0x5e, 0x30, 0x2, 0x1aa, 0x1a9, 0x3, 0x2, 0x2, 0x2, 0x1ab, 0x1ae, 
       0x3, 0x2, 0x2, 0x2, 0x1ac, 0x1aa, 0x3, 0x2, 0x2, 0x2, 0x1ac, 0x1ad, 
       0x3, 0x2, 0x2, 0x2, 0x1ad, 0x61, 0x3, 0x2, 0x2, 0x2, 0x1ae, 0x1ac, 
       0x3, 0x2, 0x2, 0x2, 0x1af, 0x1b0, 0x7, 0x43, 0x2, 0x2, 0x1b0, 0x1b2, 
       0x5, 0x22, 0x12, 0x2, 0x1b1, 0x1b3, 0x5, 0x32, 0x1a, 0x2, 0x1b2, 
       0x1b1, 0x3, 0x2, 0x2, 0x2, 0x1b2, 0x1b3, 0x3, 0x2, 0x2, 0x2, 0x1b3, 
       0x1b4, 0x3, 0x2, 0x2, 0x2, 0x1b4, 0x1b5, 0x7, 0x38, 0x2, 0x2, 0x1b5, 
       0x1b6, 0x5, 0x60, 0x31, 0x2, 0x1b6, 0x1b7, 0x7, 0x39, 0x2, 0x2, 0x1b7, 
       0x63, 0x3, 0x2, 0x2, 0x2, 0x1b8, 0x1bc, 0x5, 0x50, 0x29, 0x2, 0x1b9, 
       0x1bc, 0x7, 0x4f, 0x2, 0x2, 0x1ba, 0x1bc, 0x5, 0x1c, 0xf, 0x2, 0x1bb, 
       0x1b8, 0x3, 0x2, 0x2, 0x2, 0x1bb, 0x1b9, 0x3, 0x2, 0x2, 0x2, 0x1bb, 
       0x1ba, 0x3, 0x2, 0x2, 0x2, 0x1bc, 0x65, 0x3, 0x2, 0x2, 0x2, 0x1bd, 
       0x1bf, 0x5, 0x64, 0x33, 0x2, 0x1be, 0x1bd, 0x3, 0x2, 0x2, 0x2, 0x1bf, 
       0x1c2, 0x3, 0x2, 0x2, 0x2, 0x1c0, 0x1be, 0x3, 0x2, 0x2, 0x2, 0x1c0, 
       0x1c1, 0x3, 0x2, 0x2, 0x2, 0x1c1, 0x67, 0x3, 0x2, 0x2, 0x2, 0x1c2, 
       0x1c0, 0x3, 0x2, 0x2, 0x2, 0x1c3, 0x1c5, 0x5, 0xa, 0x6, 0x2, 0x1c4, 
       0x1c3, 0x3, 0x2, 0x2, 0x2, 0x1c5, 0x1c8, 0x3, 0x2, 0x2, 0x2, 0x1c6, 
       0x1c7, 0x3, 0x2, 0x2, 0x2, 0x1c6, 0x1c4, 0x3, 0x2, 0x2, 0x2, 0x1c7, 
       0x1c9, 0x3, 0x2, 0x2, 0x2, 0x1c8, 0x1c6, 0x3, 0x2, 0x2, 0x2, 0x1c9, 
       0x1ca, 0x7, 0x45, 0x2, 0x2, 0x1ca, 0x1cc, 0x5, 0x22, 0x12, 0x2, 0x1cb, 
       0x1cd, 0x5, 0x32, 0x1a, 0x2, 0x1cc, 0x1cb, 0x3, 0x2, 0x2, 0x2, 0x1cc, 
       0x1cd, 0x3, 0x2, 0x2, 0x2, 0x1cd, 0x1ce, 0x3, 0x2, 0x2, 0x2, 0x1ce, 
       0x1cf, 0x7, 0x38, 0x2, 0x2, 0x1cf, 0x1d0, 0x5, 0x66, 0x34, 0x2, 0x1d0, 
       0x1d1, 0x7, 0x39, 0x2, 0x2, 0x1d1, 0x69, 0x3, 0x2, 0x2, 0x2, 0x1d2, 
       0x1d5, 0x5, 0x50, 0x29, 0x2, 0x1d3, 0x1d5, 0x7, 0x4f, 0x2, 0x2, 0x1d4, 
       0x1d2, 0x3, 0x2, 0x2, 0x2, 0x1d4, 0x1d3, 0x3, 0x2, 0x2, 0x2, 0x1d5, 
       0x6b, 0x3, 0x2, 0x2, 0x2, 0x1d6, 0x1d8, 0x5, 0x6a, 0x36, 0x2, 0x1d7, 
       0x1d6, 0x3, 0x2, 0x2, 0x2, 0x1d8, 0x1db, 0x3, 0x2, 0x2, 0x2, 0x1d9, 
       0x1d7, 0x3, 0x2, 0x2, 0x2, 0x1d9, 0x1da, 0x3, 0x2, 0x2, 0x2, 0x1da, 
       0x6d, 0x3, 0x2, 0x2, 0x2, 0x1db, 0x1d9, 0x3, 0x2, 0x2, 0x2, 0x1dc, 
       0x1de, 0x5, 0xa, 0x6, 0x2, 0x1dd, 0x1dc, 0x3, 0x2, 0x2, 0x2, 0x1de, 
       0x1e1, 0x3, 0x2, 0x2, 0x2, 0x1df, 0x1e0, 0x3, 0x2, 0x2, 0x2, 0x1df, 
       0x1dd, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1e2, 0x3, 0x2, 0x2, 0x2, 0x1e1, 
       0x1df, 0x3, 0x2, 0x2, 0x2, 0x1e2, 0x1e3, 0x7, 0x44, 0x2, 0x2, 0x1e3, 
       0x1e5, 0x5, 0x22, 0x12, 0x2, 0x1e4, 0x1e6, 0x5, 0x32, 0x1a, 0x2, 
       0x1e5, 0x1e4, 0x3, 0x2, 0x2, 0x2, 0x1e5, 0x1e6, 0x3, 0x2, 0x2, 0x2, 
       0x1e6, 0x1e7, 0x3, 0x2, 0x2, 0x2, 0x1e7, 0x1e8, 0x7, 0x38, 0x2, 0x2, 
       0x1e8, 0x1e9, 0x5, 0x6c, 0x37, 0x2, 0x1e9, 0x1ea, 0x7, 0x39, 0x2, 
       0x2, 0x1ea, 0x6f, 0x3, 0x2, 0x2, 0x2, 0x1eb, 0x1ec, 0x9, 0x2, 0x2, 
       0x2, 0x1ec, 0x71, 0x3, 0x2, 0x2, 0x2, 0x1ed, 0x1ee, 0x9, 0x3, 0x2, 
       0x2, 0x1ee, 0x73, 0x3, 0x2, 0x2, 0x2, 0x1ef, 0x1f0, 0x9, 0x4, 0x2, 
       0x2, 0x1f0, 0x75, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x7f, 0x81, 0x89, 0x92, 
       0x9a, 0xa3, 0xaa, 0xb0, 0xc1, 0xc8, 0xcf, 0xda, 0xec, 0xfe, 0x106, 
       0x111, 0x117, 0x11c, 0x122, 0x12b, 0x130, 0x136, 0x13c, 0x14d, 0x152, 
       0x163, 0x170, 0x17c, 0x181, 0x187, 0x194, 0x199, 0x19f, 0x1a7, 0x1ac, 
       0x1b2, 0x1bb, 0x1c0, 0x1c6, 0x1cc, 0x1d4, 0x1d9, 0x1df, 0x1e5, 
  };

  _serializedATN.insert(_serializedATN.end(), serializedATNSegment0,
    serializedATNSegment0 + sizeof(serializedATNSegment0) / sizeof(serializedATNSegment0[0]));


  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SIGParser::Initializer SIGParser::_init;
