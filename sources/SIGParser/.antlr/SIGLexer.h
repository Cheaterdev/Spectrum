
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
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, OR = 31, AND = 32, EQ = 33, 
    NEQ = 34, GT = 35, LT = 36, GTEQ = 37, LTEQ = 38, PLUS = 39, MINUS = 40, 
    MULT = 41, DIV = 42, MOD = 43, POW = 44, NOT = 45, SCOL = 46, ASSIGN = 47, 
    OPAR = 48, CPAR = 49, OBRACE = 50, CBRACE = 51, OSBRACE = 52, CSBRACE = 53, 
    TRUE = 54, FALSE = 55, LOG = 56, LAYOUT = 57, STRUCT = 58, COMPUTE_PSO = 59, 
    GRAPHICS_PSO = 60, RAYTRACE_PSO = 61, RAYTRACE_RAYGEN = 62, RAYTRACE_PASS = 63, 
    SLOT = 64, RT = 65, RTV = 66, DSV = 67, ROOTSIG = 68, ROOTSIG_LOCAL = 69, 
    ID = 70, INT_SCALAR = 71, FLOAT_SCALAR = 72, STRING = 73, COMMENT = 74, 
    SPACE = 75, INSERT_START = 76, INSERT_END = 77, INSERT_BLOCK = 78
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

