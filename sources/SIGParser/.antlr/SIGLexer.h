
// Generated from SIG.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"




class  SIGLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, OR = 26, 
    AND = 27, EQ = 28, NEQ = 29, GT = 30, LT = 31, GTEQ = 32, LTEQ = 33, 
    PLUS = 34, MINUS = 35, MULT = 36, DIV = 37, MOD = 38, POW = 39, NOT = 40, 
    SCOL = 41, ASSIGN = 42, OPAR = 43, CPAR = 44, OBRACE = 45, CBRACE = 46, 
    OSBRACE = 47, CSBRACE = 48, TRUE = 49, FALSE = 50, LOG = 51, LAYOUT = 52, 
    STRUCT = 53, COMPUTE_PSO = 54, GRAPHICS_PSO = 55, SLOT = 56, RT = 57, 
    RTV = 58, DSV = 59, ROOTSIG = 60, ID = 61, INT_SCALAR = 62, FLOAT_SCALAR = 63, 
    STRING = 64, COMMENT = 65, SPACE = 66, INSERT_START = 67, INSERT_END = 68, 
    INSERT_BLOCK = 69
  };

  SIGLexer(antlr4::CharStream *input);
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

