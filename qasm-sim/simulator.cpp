#include <print>
#include <quantum_state.h>
#include <expected>


int main()
{
	QuantumState qs(3, 0b011);
	qs.apply_toffoli(0, 1, 2);
	qs.print_state();
	
	return 0;
}
