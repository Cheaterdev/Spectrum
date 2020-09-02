
// Generated from SIG.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"




class  SIGLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, OR = 5, AND = 6, EQ = 7, NEQ = 8, 
    GT = 9, LT = 10, GTEQ = 11, LTEQ = 12, PLUS = 13, MINUS = 14, MULT = 15, 
    DIV = 16, MOD = 17, POW = 18, NOT = 19, SCOL = 20, ASSIGN = 21, OPAR = 22, 
    CPAR = 23, OBRACE = 24, CBRACE = 25, OSBRACE = 26, CSBRACE = 27, TRUE = 28, 
    FALSE = 29, LOG = 30, LAYOUT = 31, STRUCT = 32, SLOT = 33, RT = 34, 
    RTV = 35, DSV = 36, ID = 37, INT_SCALAR = 38, FLOAT_SCALAR = 39, STRING = 40, 
    COMMENT = 41, SPACE = 42, INSERT_START = 43, INSERT_END = 44, INSERT_BLOCK = 45
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

