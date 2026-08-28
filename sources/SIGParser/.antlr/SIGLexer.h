
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
    T__38 = 39, T__39 = 40, T__40 = 41, OR = 42, AND = 43, EQ = 44, NEQ = 45, 
    GT = 46, LT = 47, GTEQ = 48, LTEQ = 49, PLUS = 50, MINUS = 51, DIV = 52, 
    MOD = 53, POW = 54, NOT = 55, SCOL = 56, ASSIGN = 57, OPAR = 58, CPAR = 59, 
    OBRACE = 60, CBRACE = 61, OSBRACE = 62, CSBRACE = 63, TRUE = 64, FALSE = 65, 
    LOG = 66, LAYOUT = 67, STRUCT = 68, COMPUTE_PSO = 69, GRAPHICS_PSO = 70, 
    RAYTRACE_PSO = 71, WORKGRAPH_PSO = 72, NODE = 73, NODE_OUTPUT = 74, 
    RAYTRACE_RAYGEN = 75, RAYTRACE_PASS = 76, PASS = 77, VIEW = 78, PIPELINE = 79, 
    SLOT = 80, RT = 81, RTV = 82, DSV = 83, ROOTSIG = 84, ID = 85, INT_SCALAR = 86, 
    FLOAT_SCALAR = 87, STRING = 88, COMMENT = 89, SPACE = 90, POINTER = 91, 
    INSERT_START = 92, INSERT_END = 93, INSERT_BLOCK = 94
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

