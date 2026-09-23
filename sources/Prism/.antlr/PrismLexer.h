
// Generated from sources/Prism/Prism.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  PrismLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    OR = 45, AND = 46, PIPE = 47, EQ = 48, NEQ = 49, GT = 50, LT = 51, GTEQ = 52, 
    LTEQ = 53, PLUS = 54, MINUS = 55, DIV = 56, MOD = 57, POW = 58, NOT = 59, 
    SCOL = 60, COLON = 61, DOT = 62, ASSIGN = 63, OPAR = 64, CPAR = 65, 
    OBRACE = 66, CBRACE = 67, OSBRACE = 68, CSBRACE = 69, TRUE = 70, FALSE = 71, 
    LOG = 72, LAYOUT = 73, STRUCT = 74, COMPUTE_PSO = 75, GRAPHICS_PSO = 76, 
    RAYTRACE_PSO = 77, WORKGRAPH_PSO = 78, NODE = 79, NODE_OUTPUT = 80, 
    RAYTRACE_RAYGEN = 81, RAYTRACE_PASS = 82, PASS = 83, VIEW = 84, PIPELINE = 85, 
    SLOT = 86, RT = 87, RTV = 88, DSV = 89, ROOTSIG = 90, ENUM = 91, ID = 92, 
    INT_SCALAR = 93, FLOAT_SCALAR = 94, STRING = 95, RAWEXPR = 96, COMMENT = 97, 
    SPACE = 98, POINTER = 99, FUNC_BODY = 100, INSERT_START = 101, INSERT_END = 102, 
    INSERT_BLOCK = 103
  };

  explicit PrismLexer(antlr4::CharStream *input);

  ~PrismLexer() override;


  	size_t last_types[3] = { 0, 0, 0 };

  	bool at_function_body() const
  	{
  		return last_types[0] == CPAR
  		    || (last_types[0] == ID && last_types[1] == COLON && last_types[2] == CPAR);
  	}

  	std::unique_ptr<antlr4::Token> nextToken() override
  	{
  		auto token = antlr4::Lexer::nextToken();
  		if (token->getChannel() == antlr4::Token::DEFAULT_CHANNEL)
  		{
  			last_types[2] = last_types[1];
  			last_types[1] = last_types[0];
  			last_types[0] = token->getType();
  		}
  		return token;
  	}


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.
  bool FUNC_BODYSempred(antlr4::RuleContext *_localctx, size_t predicateIndex);

};

