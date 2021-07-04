
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
    OR = 33, AND = 34, EQ = 35, NEQ = 36, GT = 37, LT = 38, GTEQ = 39, LTEQ = 40, 
    PLUS = 41, MINUS = 42, MULT = 43, DIV = 44, MOD = 45, POW = 46, NOT = 47, 
    SCOL = 48, ASSIGN = 49, OPAR = 50, CPAR = 51, OBRACE = 52, CBRACE = 53, 
    OSBRACE = 54, CSBRACE = 55, TRUE = 56, FALSE = 57, LOG = 58, LAYOUT = 59, 
    STRUCT = 60, COMPUTE_PSO = 61, GRAPHICS_PSO = 62, RAYTRACE_PSO = 63, 
    RAYTRACE_RAYGEN = 64, RAYTRACE_PASS = 65, SLOT = 66, RT = 67, RTV = 68, 
    DSV = 69, ROOTSIG = 70, ID = 71, INT_SCALAR = 72, FLOAT_SCALAR = 73, 
    STRING = 74, COMMENT = 75, SPACE = 76, INSERT_START = 77, INSERT_END = 78, 
    INSERT_BLOCK = 79
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

