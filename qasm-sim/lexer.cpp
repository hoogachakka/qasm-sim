#include "lexer.h"
#include <cctype>
#include <cstdint>
#include <fstream>
#include <print>

struct KwEntry {
  std::string_view text;
  TokenKind kind;
};

struct SymEntry {
  unsigned char sym[4];
  TokenKind kind;
};

static constexpr KwEntry kw_lut[] = {
#define DEF_TOK(name, text) {text, TokenKind::name},
#include "keywords.inc"
};

static constexpr SymEntry sym_lut[] = {
#include "symbols.inc"
#undef DEF_TOK
};

static constexpr size_t num_kw = std::size(kw_lut);
static constexpr size_t num_sym = std::size(sym_lut);


Lexer::Lexer() {}
Lexer::Lexer(const std::string &str) : file_contents(str) {}

std::expected<Lexer, IoError> Lexer::from_file(const std::string &path) {
  Lexer lex;
  
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return std::unexpected(IoError{IoError::Code::open_failed, path});

  file.seekg(0, std::ios::end);
  std::streampos end = file.tellg();
  if (end < 0)
    return std::unexpected(IoError{IoError::Code::tell_failed, path});

  lex.file_contents.resize(static_cast<size_t>(end));

  file.seekg(0, std::ios::beg);
  if (!lex.file_contents.empty() &&
      !file.read(lex.file_contents.data(),
                 static_cast<std::streamsize>(lex.file_contents.size())))
    return std::unexpected(IoError{IoError::Code::read_failed, path});

  return lex;
}

bool Lexer::skip_ws() {
  while (true) {
    unsigned char c = peek();
    if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
      break;
    ++pos;
  }
  return (peek() != '\0');
}

std::expected<bool, LexError> Lexer::skip_ws_and_comments() {
  size_t start = pos;
  while (true) {
    unsigned char c = peek();
    if (c == '\0') return false;

    // skip whitespace
    if (std::isspace(c)) {
      ++pos;
      continue;
    }

    // check for comments
    if (c == '/') {
      unsigned char c2 = peek(1);

      // line comment, skip to end of line
      if (c2 == '/') {
        pos += 2;
        while (true) {
          unsigned char d = peek();
          if (d == '\0' || d == '\n') break;
	  ++pos;
        }
        continue;
      }

      // block comment, skip until */
      if (c2 == '*') {
        pos += 2;
        while (true) {
          unsigned char d = peek();
          if (d == '\0') {
	    return std::unexpected(get_err(start, LexError::Code::unterminated_block_comment));
          }
          if (d == '*' && peek(1) == '/') {
            pos += 2;
            break;
          }
          ++pos;
        }
        continue;
      }
    }

    return (peek() != '\0');
  }
}



unsigned char Lexer::peek(size_t lookahead) const {
  size_t i = pos + lookahead;
  return (i < file_contents.size()) ? file_contents[i] : '\0';
}

unsigned char Lexer::peek_back(size_t lookback) const {
  size_t i = pos - lookback;
  return (i >= 0) ? file_contents[i] : '\0';
}

enum num_lex_mode {
  mode_dec = 0,
  mode_hex = 1,
  mode_oct = 2,
  mode_bin = 3,
  mode_float = 4,
  mode_exp = 5
};

static constexpr TokenKind mode_tbl[] = {
  TokenKind::DEC_LIT,
  TokenKind::HEX_LIT,
  TokenKind::OCT_LIT,
  TokenKind::BIN_LIT,
  TokenKind::FLOAT_LIT,
  TokenKind::FLOAT_LIT
};    

static inline bool is_valid_digit(num_lex_mode base, uint8_t ch) {
  switch (base) {
  case mode_float:
  case mode_exp:
  case mode_dec: return std::isdigit(ch);
  case mode_hex: return std::isxdigit(ch);
  case mode_oct: return ('0' <= ch && ch <= '7');
  case mode_bin: return (ch == '0' || ch == '1');
  }
  return false; // unreachable
}

static inline num_lex_mode detect_prefix(unsigned char c0, unsigned char c1) {
  if (c0 == '.') {
    return mode_float;
  }
  
  if (c0 == '0') {
    switch (c1) {
    case 'x':
    case 'X':
      return mode_hex;
    case 'b':
    case 'B':
      return mode_bin;
    case 'o':
      return mode_oct;
    default:
      break;
    }
  }
  
  return mode_dec;
}

std::expected<bool, LexError> Lexer::lex_version_id() {
  if (!skip_ws()) {
    return false;
  }
  size_t start = pos;
  if (!std::isdigit(peek())) {
    return std::unexpected(get_err(start, LexError::Code::bad_version_id));
  }
    
  do {
    ++pos;
  } while (std::isdigit(peek()));
    
  if (peek() == '.') {
    ++pos;
    if (!std::isdigit(peek())) {
      return std::unexpected(get_err(start, LexError::Code::bad_version_id));
    }
    
    do {
      ++pos;
    } while (std::isdigit(peek()));
  }

  toks.push_back({TokenKind::VERSION_ID, {start, pos - start}});
  mode_stack.pop_back();
  return (peek() != '\0');
}

std::expected<bool, LexError> Lexer::lex_arbitrary_str() {
  if (!skip_ws()) {
    return false;
  }
  size_t start = pos;
  unsigned char c = peek();
  if (c != '"' && c != '\'') {
    return std::unexpected(get_err(start, LexError::Code::not_str));
  }
  ++pos;
  unsigned char quotetype = c;
  unsigned char s0 = peek();
  if (s0 == quotetype || s0 == '\r' || s0 == '\t' || s0 == '\n' || s0 == '\0') {
    return std::unexpected(get_err(start, LexError::Code::bad_str));
  }
  ++pos;
  while (true) {
    unsigned char n = peek();
    if (n == '\r' || n == '\t' || n == '\n' || n == '\0') {
      return std::unexpected(get_err(start, LexError::Code::bad_str));
    }
    ++pos;
    if (n == quotetype)
      break;
  }
  toks.push_back({TokenKind::STR_LIT, {start, pos - start}});
  mode_stack.pop_back();
  return (peek() != '\0');
}

std::expected<bool, LexError> Lexer::lex_kw(size_t start) {
  ++pos;
  while (true) {
    unsigned char d = peek();
    if (!std::isalnum(d) && d != '_')
      break;
    ++pos;
  }
  size_t tok_len = pos - start;
  std::string_view word(file_contents.data() + start, tok_len);
  Token tok = {TokenKind::IDENT, {start, tok_len}};

  // check if we found a bool lit
  if (word == "true" || word == "false") {
    tok.kind = TokenKind::BOOL_LIT;
    toks.push_back(tok);
    return (peek() != '\0');
  }

  // look up the extracted word: if it's in the table, it's a keyword, otherwise it's an identifier
  for (size_t i = 0; i < num_kw; i++) {
    if (word == kw_lut[i].text) {
      tok.kind = kw_lut[i].kind; // keyword found
      break;
    }
  }
    
  switch (tok.kind) {
  case TokenKind::OPENQASM:
    mode_stack.push_back(LexMode::VERSION_ID);
    break;
  case TokenKind::INCLUDE:
  case TokenKind::DEFCALGRAMMAR:
    mode_stack.push_back(LexMode::ARBITRARY_STR);
    break;
  case TokenKind::CAL:
    mode_stack.push_back(LexMode::CAL_PRELUDE);
    break;
  case TokenKind::DEFCAL:
    mode_stack.push_back(LexMode::DEFCAL_PRELUDE);
    break;
  default:
    break;
  }

  toks.push_back(tok);
  return (peek() != '\0');
}

std::expected<bool, LexError> Lexer::lex_num_lit(size_t start, unsigned char c) {
  auto mode = detect_prefix(c, peek(1));

  // leading '.' without digit after should be a dot token
  if (mode == mode_float && !is_valid_digit(mode, peek(1))) {
    Token tok = {TokenKind::DOT, {start, 1}};
    toks.push_back(tok);
    return (peek() != '\0');
  }

  ++pos;
  if (mode != mode_dec) {
    ++pos;
    if (mode != mode_float && !is_valid_digit(mode, peek())) {
      return std::unexpected(get_err(start, LexError::Code::bad_literal));
    }
  }
    
  while (true) {
    unsigned char n = peek();
    if (is_valid_digit(mode, n)) {
      ++pos;
      continue;
    }

    // must be in mode_dec to proceed
    if (n == '.') {
      if (mode != mode_dec) {
	return std::unexpected(get_err(start, LexError::Code::bad_literal));
      }
      mode = mode_float;
      ++pos;
      continue;
    }

    // underscore must be followed by valid digit
    if (n == '_') {
      if (!is_valid_digit(mode, peek(1))) {
	return std::unexpected(get_err(start, LexError::Code::bad_literal));
      }
      pos += 2;
      continue;
    }

    if (n == 'e' || n == 'E') {
      // mode must be dec or float to proceed
      if (mode != mode_dec && mode != mode_float) {
	return std::unexpected(get_err(start, LexError::Code::bad_literal));
      }

      mode = mode_exp;
      char n2 = peek(1);
      int skip = (n2 == '+' || n2 == '-') ? 2 : 1;
      pos += skip;

      // at least one valid digit must follow the exponent/sign
      if (!is_valid_digit(mode, peek())) {
	return std::unexpected(get_err(start, LexError::Code::bad_literal));
      }
        
      ++pos;
      continue;
    }

    break;
  }

  TokenKind tok_kind = mode_tbl[mode];

  if (mode == mode_dec || mode == mode_float || mode == mode_exp) {
    // probe for next non-ws character without advancing the pos
    size_t probe = 0;
    while (true) {
      unsigned char p = peek(probe);
      if (p != ' ' && p != '\t')
        break;
      ++probe;
    }
      
    unsigned char s0 = peek(probe);
    unsigned char s1 = peek(probe+1);

    if (s0 == 's') {
      pos += (probe + 1);
      tok_kind = TokenKind::TIME_LIT;
    }
    else if ((s1 == 's' && (s0 == 'n' || s0 == 'u' || s0 == 'm')) ||
             (s0 == 'd' && s1 == 't')) {
      pos += (probe + 2);
      tok_kind = TokenKind::TIME_LIT;
    }
    else if (peek(probe+2) == 's' &&
             (s0 == 0xC2 && s1 == 0xB5)) {
      pos += (probe + 3);
      tok_kind = TokenKind::TIME_LIT;
    }
    else if (s0 == 'i' && s1 == 'm') {
      pos += (probe + 2);
      tok_kind = (mode == mode_dec) ? TokenKind::IMAG_LIT_DEC : TokenKind::IMAG_LIT_FLOAT;
    }
  }
    
  toks.push_back({tok_kind, {start, pos-start}});
  return (peek() != '\0');
}

std::expected<bool, LexError> Lexer::lex_symbol(size_t start, unsigned char c) {
  Token tok;
  tok.span.pos = start;

  unsigned char c2 = peek(1);
  unsigned char c3 = peek(2);

  // symbol table is sorted by length descending:
  // maximally correct symbol will always be emitted
  for (size_t i = 0; i < num_sym; i++) {
    auto& e = sym_lut[i];
    if (c == e.sym[0]) { // first char match
      if (e.sym[1] != '\0') { // is multi-char symbol
        if (c2 == e.sym[1]) { // second char matches
          if (e.sym[2] != '\0') { // is three-char symbol
            if (c3 == e.sym[2]) { // third char matches
              pos += 3;
              tok.kind = e.kind;
              tok.span.len = 3;
              toks.push_back(tok);
              return (peek() != '\0');
            }
            continue;
          }
          pos += 2;
          tok.kind = e.kind;
          tok.span.len = 2;
          toks.push_back(tok);
          return (peek() != '\0');
        }
        continue;
      }
      ++pos;
      tok.kind = e.kind;
      tok.span.len = 1;
      toks.push_back(tok);
      return (peek() != '\0');
    }
  }

  // no symbol was found.
  // this function runs last in the lexing process, so there must be an unknown
  // character
  return std::unexpected(get_err(start, LexError::Code::unknown_char));
}

std::expected<bool, LexError> Lexer::next_tok() {
  switch (mode_stack.back()) {
  case LexMode::VERSION_ID: return lex_version_id();
  case LexMode::ARBITRARY_STR: return lex_arbitrary_str();
  default:
    break;
  }
  // always do this at the start to ensure the next character matters
  if (auto ok = skip_ws_and_comments(); !ok)
    return std::unexpected(ok.error());
  
  size_t start = pos;
  unsigned char c = peek();
  if (c == '\0')
    return false;

  // first check for keywords/idents/bool literals
  // the spec allows for unicode but i am not opening that can of worms just yet
  if (std::isalpha(c) || c == '_') {
    return lex_kw(start);
  }
  
  if (std::isdigit(c) || c == '.') {
    return lex_num_lit(start, c);
  }

  // bitstring literal
  if (c == '"') {
    ++pos; // consume opening quote
    while (true) {
      unsigned char d = peek();
      if (d != '0' && d != '1') {
	return std::unexpected(get_err(start, LexError::Code::bad_bit_str));
      }
      ++pos;
      if (peek() == '_') {
        ++pos;
        continue;
      }
      
      if (peek() == '"')
        break;
    }
    ++pos; // consume closing quote
    toks.push_back({TokenKind::BITSTR_LIT, start, pos - start});
    return (peek() != '\0');
  }

  return lex_symbol(start, c);
}


std::string_view Lexer::str_from_span(Span span) {
  return std::string_view(file_contents.data() + span.pos, span.len);
}

LexError Lexer::get_err(size_t start, LexError::Code code) {
  LexError err;
  err.code = code;
  err.span.pos = start;
  err.span.len = (++pos - start);
  err.contents = str_from_span(err.span);
  return err;
}

// DEBUG

void Lexer::print_latest_tok() {
  const auto &tok = toks.back();
  std::print("kind: {}, ", to_string(tok.kind));
  std::println("contents: '{}'", str_from_span(tok.span));
}

void Lexer::print_toks() {
  for (auto &tok : toks) {
    std::print("kind: {}, ", to_string(tok.kind));
    std::println("contents: '{}'", str_from_span(tok.span));
  }
}
