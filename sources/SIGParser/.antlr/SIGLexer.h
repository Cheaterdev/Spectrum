
// Generated from SIG.g4 by ANTLR 4.9.2

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
    LT = 40, GTEQ = 41, LTEQ = 42, PLUS = 43, MINUS = 44, MULT = 45, DIV = 46, 
    MOD = 47, POW = 48, NOT = 49, SCOL = 50, ASSIGN = 51, OPAR = 52, CPAR = 53, 
    OBRACE = 54, CBRACE = 55, OSBRACE = 56, CSBRACE = 57, TRUE = 58, FALSE = 59, 
    LOG = 60, LAYOUT = 61, STRUCT = 62, COMPUTE_PSO = 63, GRAPHICS_PSO = 64, 
    RAYTRACE_PSO = 65, RAYTRACE_RAYGEN = 66, RAYTRACE_PASS = 67, SLOT = 68, 
    RT = 69, RTV = 70, DSV = 71, ROOTSIG = 72, ID = 73, INT_SCALAR = 74, 
    FLOAT_SCALAR = 75, STRING = 76, COMMENT = 77, SPACE = 78, INSERT_START = 79, 
    INSERT_END = 80, INSERT_BLOCK = 81
  };

  explicit SIGLexer(antlr4::CharStream *input);
  ~SIGLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

