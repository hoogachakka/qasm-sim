#pragma once
#include "quantum_state.h"

struct Circuit {
	size_t num_qubits;
	size_t num_regs;
	QuantumState qs;
	bool is_stable = true;
};