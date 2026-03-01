#include <print>
#include "quantum_state.h"
#include "lexer.h"

int main()
{
  auto l = Lexer::from_file("/home/etai/source/qasm-sim/qasm-sim/examples/test.qasm");
  if (!l) {
    l.error().print();
    return 1;
  }
  
  auto& lex = l.value();

  while (true) {
    auto ok = lex.next_tok();
    if (!ok) {
      ok.error().print();
      return 1;
    }
    else if (!ok.value())
      break;
    // lex.print_latest_tok();
  }

  lex.print_toks();
  // QuantumState qs(3, 0b011);
  // qs.apply_toffoli(0, 1, 2);
  // qs.print_state();
	
  return 0;
}
