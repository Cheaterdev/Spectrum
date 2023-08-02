
// Generated from SIG.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  SIGLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, OR = 35, AND = 36, EQ = 37, NEQ = 38, GT = 39, 
    LT = 40, GTEQ = 41, LTEQ = 42, PLUS = 43, MINUS = 44, DIV = 45, MOD = 46, 
    POW = 47, NOT = 48, SCOL = 49, ASSIGN = 50, OPAR = 51, CPAR = 52, OBRACE = 53, 
    CBRACE = 54, OSBRACE = 55, CSBRACE = 56, TRUE = 57, FALSE = 58, LOG = 59, 
    LAYOUT = 60, STRUCT = 61, COMPUTE_PSO = 62, GRAPHICS_PSO = 63, RAYTRACE_PSO = 64, 
    RAYTRACE_RAYGEN = 65, RAYTRACE_PASS = 66, SLOT = 67, RT = 68, RTV = 69, 
    DSV = 70, ROOTSIG = 71, ID = 72, INT_SCALAR = 73, FLOAT_SCALAR = 74, 
    STRING = 75, COMMENT = 76, SPACE = 77, POINTER = 78, INSERT_START = 79, 
    INSERT_END = 80, INSERT_BLOCK = 81
  };

  explicit SIGLexer(antlr4::CharStream *input);

  ~SIGLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

