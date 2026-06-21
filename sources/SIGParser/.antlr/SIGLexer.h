
// Generated from sources/SIGParser/SIG.g4 by ANTLR 4.11.1

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
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, OR = 41, AND = 42, EQ = 43, NEQ = 44, GT = 45, 
    LT = 46, GTEQ = 47, LTEQ = 48, PLUS = 49, MINUS = 50, DIV = 51, MOD = 52, 
    POW = 53, NOT = 54, SCOL = 55, ASSIGN = 56, OPAR = 57, CPAR = 58, OBRACE = 59, 
    CBRACE = 60, OSBRACE = 61, CSBRACE = 62, TRUE = 63, FALSE = 64, LOG = 65, 
    LAYOUT = 66, STRUCT = 67, COMPUTE_PSO = 68, GRAPHICS_PSO = 69, RAYTRACE_PSO = 70, 
    WORKGRAPH_PSO = 71, NODE = 72, NODE_OUTPUT = 73, RAYTRACE_RAYGEN = 74, 
    RAYTRACE_PASS = 75, PASS = 76, VIEW = 77, PIPELINE = 78, SLOT = 79, 
    RT = 80, RTV = 81, DSV = 82, ROOTSIG = 83, ID = 84, INT_SCALAR = 85, 
    FLOAT_SCALAR = 86, STRING = 87, COMMENT = 88, SPACE = 89, POINTER = 90, 
    INSERT_START = 91, INSERT_END = 92, INSERT_BLOCK = 93
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

