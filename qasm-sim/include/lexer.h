// https://openqasm.com/ -- language spec

#pragma once

#include <regex>
#include <variant>
#include <vector>
#include <unordered_map>
#include <expected>


struct IoError {
  enum class Code { open_failed, tell_failed, read_failed };
  Code code;
  std::string path;
};

struct LexError {
  enum class Code {};
  Code code;
  size_t pos;
  size_t span;
};

enum class FixedTok {
  // language keywords
  OPENQASM,
  INCLUDE,
  DEFCALGRAMMAR,
  DEF,
  CAL,
  DEFCAL,
  GATE,
  EXTERN,
  BOX,
  LET,
  BREAK,
  CONTINUE,
  IF,
  ELSE,
  END,
  RETURN,
  FOR,
  WHILE,
  IN,
  SWITCH,
  CASE,
  DEFAULT,
  NOP,
  PRAGMA,
  ANNOTATION,

  // types
  INPUT,
  OUTPUT,
  CONST,
  READONLY,
  MUTABLE,
  QREG,
  QUBIT,
  CREG,
  BOOL,
  BIT,
  INT,
  UINT,
  FLOAT,
  ANGLE,
  COMPLEX,
  ARRAY,
  VOID,
  DURATION,
  STRETCH,
  IMAG,

  // identifiers/ops
  GPHASE,
  INV,
  POW,
  CTRL,
  NEGCTRL,
  DIM,
  DURATIONOF,
  DELAY,
  RESET,
  MEASURE,
  BARRIER,

  // symbols
  LBRACKET,
  RBRACKET,
  LBRACE,
  RBRACE,
  LPAREN,
  RPAREN,
  COLON,
  SEMICOLON,
  DOT,
  COMMA,
  EQUALS,
  ARROW,
  PLUS,
  DOUBLE_PLUS,
  MINUS,
  ASTERISK,
  DOUBLE_ASTERISK,
  SLASH,
  PERCENT,
  PIPE,
  DOUBLE_PIPE,
  AMPERSAND,
  DOUBLE_AMPERSAND,
  CARET,
  AT,
  TILDE,
  EXCLAMATION_POINT,
  EQEQ,
  NOTEQ,
  PLUSEQ,
  MINUSEQ,
  TIMESEQ,
  DIVEQ,
  ANDEQ,
  GREATERTHAN,
  LESSTHAN,
  GREATEREQ,
  LESSEQ,
  SHIFTR,
  SHIFTL
};

struct IntLit { int64_t val; };
struct FloatLit { float val; };
struct BoolLit { bool val; };
struct BitStrLit {
  uint64_t bits;
  uint32_t nbits;
};  
using Ident = std::string;

using Token = std::variant<FixedTok, IntLit, FloatLit, BoolLit, BitStrLit, Ident>;

using TokList = std::vector<Token>;

struct Lexer {
  TokList toks;
  std::string file_contents;
  size_t pos = 0;

  // consumes the next token and appends to toks, advances the string cursor
  std::expected<void, LexError> next_tok();

  char peek(size_t lookahead = 0) const;

  std::expected<void, LexError> skip_ws_and_comments();

  std::expected<void, IoError> read_file_contents(const std::string &path);
  
  static std::expected<Lexer, IoError> from_file(const std::string& filepath);
};


