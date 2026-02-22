#include "lexer.h"
#include <fstream>
#include <string_view>

std::expected<Lexer, IoError> Lexer::from_file(const std::string &filepath) {
  Lexer lex;

  if (auto ok = lex.read_file_contents(filepath); !ok)
    return std::unexpected(ok.error());

  return lex;
}

std::expected<void, IoError> Lexer::read_file_contents(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return std::unexpected(IoError{IoError::Code::open_failed, path});

  file.seekg(0, std::ios::end);
  std::streampos end = file.tellg();
  if (end < 0)
    return std::unexpected(IoError{IoError::Code::tell_failed, path});

  file_contents.resize(static_cast<size_t>(end));

  file.seekg(0, std::ios::beg);
  if (!file_contents.empty() &&
      !file.read(file_contents.data(),
                 static_cast<std::streamsize>(file_contents.size())))
    return std::unexpected(IoError{IoError::Code::read_failed, path});

  return {};
}

std::expected<void, LexError> Lexer::skip_ws_and_comments() {
  while (true) {
    uint8_t c = static_cast<uint8_t>(peek());
    if (c == '\0') return {};

    // skip whitespace
    if (std::isspace(c)) {
      ++pos;
      continue;
    }

    // check for comments
    if (c == '/') {
      char c2 = peek(1);

      // line comment, skip to end of line
      if (c2 == '/') {
        pos += 2;
        while (true) {
          char d = peek();
          if (d == '\0' || d == '\n') break;
	  ++pos;
        }
        continue;
      }

      // block comment, skip until */
      if (c2 == '*') {
        pos += 2;
        while (true) {
          char d = peek();
          if (d == '\0') {
            // TODO: unterminated block comment
            return {};
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

    return {};
  }
}

char Lexer::peek(size_t lookahead) const {
  size_t i = pos + lookahead;
  return (i < file_contents.size()) ? file_contents[i] : '\0';
}

std::expected<void, LexError> Lexer::next_tok() {
  //always do this at the start to ensure the next character matters
  auto ok = skip_ws_and_comments();

  size_t start = pos;
  uint8_t c = static_cast<uint8_t>(peek());

  // first we check for kwds/idents
  if (std::isalpha(c) || c == '_') {
    ++pos;
    while (true) {
      uint8_t d = static_cast<uint8_t>(peek());
      if (d == '\0' || (!std::isalnum(d) && d != '_'))
        break;
      ++pos;
    }
    
    std::string_view text(file_contents.data() + start, pos-start);
    
    
  }
  
  return {}; 
}

