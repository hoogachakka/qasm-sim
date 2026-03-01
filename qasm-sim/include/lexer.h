// https://openqasm.com/ -- language spec

#pragma once

#include <vector>
#include <expected>
#include <string>
#include <print>

struct Span {
  size_t pos;
  size_t len;
};

struct IoError {
  enum class Code { open_failed, tell_failed, read_failed };
  Code code;
  std::string path;

  void print() {
    switch (code) {
    case Code::open_failed:
      std::println(stderr, "Error opening file {}", path);
      break;
    default:
      std::println("unimplemented");
    }
  }    
};

struct LexError {
  enum class Code { unterminated_block_comment, bad_literal, bad_version_id, not_str, bad_str, bad_bit_str, unknown_char };
  Code code;
  Span span;
  std::string_view contents;

  std::string_view err_str() {
    switch (code) {
    case Code::unterminated_block_comment:
      return "Unterminated Block Comment";
    case Code::bad_literal:
      return "Bad numeric literal";
    case Code::bad_version_id:
      return "Bad version ID";
    case Code::bad_str:
      return "Bad string";
    case Code::bad_bit_str:
      return "Bad bit string";
    case Code::unknown_char:
      return "Unknown char";
    default:
      return "Unreachable";
    }
  }
  
  void print() {
    std::println(stderr, "Error: {} at pos {}: {}", err_str(), span.pos, contents);
  }    
};

enum class TokenKind {
#define DEF_TOK(name, text) name,
#include "tokens.inc"
#undef DEF_TOK
};

constexpr std::string_view to_string(TokenKind k) {
#define DEF_TOK(name, str) case TokenKind::name: return #name;
switch (k) {
#include "tokens.inc"
}
return "UNKNOWN";
#undef DEF_TOK
}



// struct IntLit { int64_t val; };
// struct FloatLit { float val; };
// struct BoolLit { bool val; };
// struct StrLit { std::string_view str; };
// struct Ident {
//   size_t id;
//   std::string_view name;
// };



struct Token {
  TokenKind kind;
  Span span;
};  

// struct BitStrLit {
//   uint64_t bits;
//   uint32_t nbits;
// };  

// using Token = std::variant<FixedTok, IntLit, FloatLit, BoolLit, StrLit, Ident>;

enum class LexMode {
  NORMAL,
  VERSION_ID,
  ARBITRARY_STR,
  CAL_PRELUDE,
  DEFCAL_PRELUDE,
  EAT_TO_LINE_END
};

struct Lexer {
  std::vector<Token> toks;
  std::string file_contents;
  size_t pos = 0;
  std::vector<LexMode> mode_stack = {LexMode::NORMAL};

  Lexer();
  Lexer(const std::string& str);

  std::expected<bool, LexError> lex_version_id();
  std::expected<bool, LexError> lex_arbitrary_str();
  std::expected<bool, LexError> lex_kw(size_t start);
  std::expected<bool, LexError> lex_num_lit(size_t start, unsigned char c);
  std::expected<bool, LexError> lex_symbol(size_t start, unsigned char c);
  
  // consumes the next token and appends to toks, advances the string cursor
  std::expected<bool, LexError> next_tok();

  unsigned char peek(size_t lookahead = 0) const;
  unsigned char peek_back(size_t lookback = 1) const;

  bool skip_ws();
  std::expected<bool, LexError> skip_ws_and_comments();

  std::string_view str_from_span(Span span);
  LexError get_err(size_t start, LexError::Code code);

  void print_latest_tok();
  void print_toks();
  static std::expected<Lexer, IoError> from_file(const std::string &filepath);
};


