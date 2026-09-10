
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
    T__44 = 45, OR = 46, AND = 47, PIPE = 48, EQ = 49, NEQ = 50, GT = 51, 
    LT = 52, GTEQ = 53, LTEQ = 54, PLUS = 55, MINUS = 56, DIV = 57, MOD = 58, 
    POW = 59, NOT = 60, SCOL = 61, ASSIGN = 62, OPAR = 63, CPAR = 64, OBRACE = 65, 
    CBRACE = 66, OSBRACE = 67, CSBRACE = 68, TRUE = 69, FALSE = 70, LOG = 71, 
    LAYOUT = 72, STRUCT = 73, COMPUTE_PSO = 74, GRAPHICS_PSO = 75, RAYTRACE_PSO = 76, 
    WORKGRAPH_PSO = 77, NODE = 78, NODE_OUTPUT = 79, RAYTRACE_RAYGEN = 80, 
    RAYTRACE_PASS = 81, PASS = 82, VIEW = 83, PIPELINE = 84, SLOT = 85, 
    RT = 86, RTV = 87, DSV = 88, ROOTSIG = 89, ENUM = 90, ID = 91, INT_SCALAR = 92, 
    FLOAT_SCALAR = 93, STRING = 94, RAWEXPR = 95, COMMENT = 96, SPACE = 97, 
    POINTER = 98, INSERT_START = 99, INSERT_END = 100, INSERT_BLOCK = 101
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

