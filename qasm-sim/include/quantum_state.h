#pragma once

#include <complex>
#include <vector>
#include <random>
#include <algorithm>

using Complex = std::complex<double>;

struct QuantumState {
	size_t n;
	std::vector<Complex> psi;
	std::mt19937 rng;

	QuantumState(size_t num_qubits, size_t init_state);

	void init(size_t num_qubits, size_t init_state);

	// measure a single qubit and collapse the wavefunction
	// returns the measurement result
	size_t measure(size_t qubit);

	// measure the entire system and collapse the wavefunction
	// returns the measurement result
	size_t measure_all();

	// GATES
	void apply_hadamard(size_t qubit);
	void apply_s(size_t qubit);
	void apply_cnot(size_t cntrl, size_t qubit);

	// sanity check function to ensure total probability is 1
	double total_probability() const;

	// DEBUG: print current state
	void print_state() const;
};
