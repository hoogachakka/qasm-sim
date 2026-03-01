#include "lexer.h"
#include <optional>
#include <variant>
#include <unordered_map>

using ExprId = uint32_t;
using StmtId = uint32_t;
using NameId = uint32_t;
using BlockId = uint32_t;
using SymbolId = uint32_t;

enum class ExprKind {


};

enum class StmtKind {

};  

struct Expr {
  enum class kind {
    PAREN,
    INDEX,
    POWER,
    UNARY,
    MULT,
    ADD,
    BITSHIFT,
    COMP,
    EQUALITY,
    BITWISE_AND,
    BITWISE_XOR,
    BITWISE_OR,
    LOGICAL_AND,
    LOGICAL_OR,
    CAST,
    DURATIONOF,
    CALL,
    LITERAL,
    IDENT
  };

  enum class type {


  };    

  std::vector<Expr> exprs;
  Span span;
};

struct Stmt {
  enum class kind {
    INCLUDE,
    BREAK,
    CONTINUE,
    END,
    FOR,
    IF,
    RETURN,
    WHILE,
    SWITCH,
    BARRIER,
    BOX,
    DELAY,
    NOP,
    GATECALL,
    MEASURE_ARROW_ASSIGN,
    RESET,
    ALIAS_DECL,
    CLASSICAL_DECL,
    CONST_DECL,
    IO_DECL,
    OLDSTYLE_DECL,
    QUANTUM_DECL,
    DEF,
    EXTERN,
    GATE,
    ASSIGNMENT,
    EXPRESSION,
    CAL,
    DEFCAL,
    PRAGMA,
    ANNOTATION
  };

  Span span;
};

struct Scope {
  std::vector<Stmt> stmts;
  Span span;
};

struct IfStmnt {
  Expr cond;
  Scope if_body;
  std::optional<Scope> else_body;
};

struct IdentExpr {
  NameId name;
};

struct ScalarType {
  enum class kind {
    Bit,
    Int,
    UInt,
    Float,
    Angle,
    Bool,
    Duration,
    Stretch,
    Complex
  };
  kind k;
  Span span;
  std::optional<ExprId> designator;
};  



struct IncludeStmt {
  std::string_view path;
};
struct BreakStmt {};
struct ContinueStmt {};
struct EndStmt {};
struct ForStmnt {
  ScalarType type;
  ExprId ident;
  ExprId rng;
  Scope body;
};

// // Inclusion statements.
// calibrationGrammarStatement: DEFCALGRAMMAR StringLiteral SEMICOLON;
// includeStatement: INCLUDE StringLiteral SEMICOLON;

// // Control-flow statements.

// forStatement: FOR scalarType Identifier IN (setExpression | LBRACKET rangeExpression RBRACKET | expression) body=statementOrScope;
// ifStatement: IF LPAREN expression RPAREN if_body=statementOrScope (ELSE else_body=statementOrScope)?;
// returnStatement: RETURN (expression | measureExpression | quantumCallExpression)? SEMICOLON;
// whileStatement: WHILE LPAREN expression RPAREN body=statementOrScope;
// switchStatement: SWITCH LPAREN expression RPAREN LBRACE switchCaseItem* RBRACE;
// switchCaseItem:
//     CASE expressionList scope
//     | DEFAULT scope
// ;

// // Quantum directive statements.
// barrierStatement: BARRIER gateOperandList? SEMICOLON;
// boxStatement: BOX designator? scope;
// delayStatement: DELAY designator gateOperandList? SEMICOLON;
// nopStatement: NOP gateOperandList? SEMICOLON;
// /* `gateCallStatement`  is split in two to avoid a potential ambiguity with an
//  * `expressionStatement` that consists of a single function call.  The only
//  * "gate" that can have no operands is `gphase` with no control modifiers, and
//  * `gphase(pi);` looks grammatically identical to `fn(pi);`.  We disambiguate by
//  * having `gphase` be its own token, and requiring that all other gate calls
//  * grammatically have at least one qubit.  Strictly, as long as `gphase` is a
//  * separate token, ANTLR can disambiguate the statements by the definition
//  * order, but this is more robust. */
// gateCallStatement:
//     gateModifier* Identifier (LPAREN expressionList? RPAREN)? designator? gateOperandList SEMICOLON
//     | gateModifier* GPHASE (LPAREN expressionList? RPAREN)? designator? gateOperandList? SEMICOLON
// ;
// // measureArrowAssignmentStatement also permits the case of not assigning the
// // result to any classical value too.
// measureArrowAssignmentStatement: (measureExpression | quantumCallExpression) (ARROW indexedIdentifier)? SEMICOLON;
// resetStatement: RESET gateOperand SEMICOLON;

// // Primitive declaration statements.
// aliasDeclarationStatement: LET Identifier EQUALS aliasExpression SEMICOLON;
// classicalDeclarationStatement: (scalarType | arrayType) Identifier (EQUALS declarationExpression)? SEMICOLON;
// constDeclarationStatement: CONST scalarType Identifier EQUALS declarationExpression SEMICOLON;
// ioDeclarationStatement: (INPUT | OUTPUT) (scalarType | arrayType) Identifier SEMICOLON;
// oldStyleDeclarationStatement: (CREG | QREG) Identifier designator? SEMICOLON;
// quantumDeclarationStatement: qubitType Identifier SEMICOLON;

// // Declarations and definitions of higher-order objects.
// defStatement: DEF Identifier LPAREN argumentDefinitionList? RPAREN returnSignature? scope;
// externStatement: EXTERN Identifier LPAREN externArgumentList? RPAREN returnSignature? SEMICOLON;
// gateStatement: GATE Identifier (LPAREN params=identifierList? RPAREN)? qubits=identifierList scope;

// // Non-declaration assignments and calculations.
// assignmentStatement: indexedIdentifier op=(EQUALS | CompoundAssignmentOperator) (expression | measureExpression | quantumCallExpression) SEMICOLON;
// expressionStatement: expression SEMICOLON;

// // Statements where the bulk is in the calibration language.
// calStatement: CAL LBRACE CalibrationBlock? RBRACE;
// defcalStatement: DEFCAL defcalTarget (LPAREN defcalArgumentDefinitionList? RPAREN)? defcalOperandList returnSignature? LBRACE CalibrationBlock? RBRACE;



struct Program {
  int version = -1; // default val if version not specified
};

struct NameTable {
  std::vector<std::string_view> id_to_text;
  std::unordered_map<std::string_view, NameId> text_to_id;
  NameId next_id = 0;

  NameId get_id(std::string_view s) {
    auto it = text_to_id.find(s);
    if (it != text_to_id.end())
      return it->second;
    // id not found, it must be new
    NameId id = next_id++;
    id_to_text.push_back(s);
    text_to_id.emplace(s, id);
    return id;
  }

  std::string_view get_name(NameId id) {
    return id_to_text[id];
  }
};  

struct ParseContext {
  std::vector<Stmt> stmt_list;
  std::vector<Expr> expr_list;
  NameTable identifiers;
  std::string text_buf;

};  

struct Parser {
  std::vector<Token> toks;
  size_t pos = 0;
  ParseContext& ctx;

  Parser(Lexer& lex);
  
  Program prog;
};
