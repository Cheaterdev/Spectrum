
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
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    OR = 45, AND = 46, PIPE = 47, EQ = 48, NEQ = 49, GT = 50, LT = 51, GTEQ = 52, 
    LTEQ = 53, PLUS = 54, MINUS = 55, DIV = 56, MOD = 57, POW = 58, NOT = 59, 
    SCOL = 60, ASSIGN = 61, OPAR = 62, CPAR = 63, OBRACE = 64, CBRACE = 65, 
    OSBRACE = 66, CSBRACE = 67, TRUE = 68, FALSE = 69, LOG = 70, LAYOUT = 71, 
    STRUCT = 72, COMPUTE_PSO = 73, GRAPHICS_PSO = 74, RAYTRACE_PSO = 75, 
    WORKGRAPH_PSO = 76, NODE = 77, NODE_OUTPUT = 78, RAYTRACE_RAYGEN = 79, 
    RAYTRACE_PASS = 80, PASS = 81, VIEW = 82, PIPELINE = 83, SLOT = 84, 
    RT = 85, RTV = 86, DSV = 87, ROOTSIG = 88, ENUM = 89, ID = 90, INT_SCALAR = 91, 
    FLOAT_SCALAR = 92, STRING = 93, COMMENT = 94, SPACE = 95, POINTER = 96, 
    INSERT_START = 97, INSERT_END = 98, INSERT_BLOCK = 99
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

