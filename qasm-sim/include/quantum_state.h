#pragma once

#include <complex>
#include <vector>
#include <random>
#include <array>

using Complex = std::complex<double>;

struct SampleResult {
  size_t results[2] = { 0, 0 };

  void log_results();
};


struct QuantumState {
  size_t n;
  std::vector<Complex> psi;
  std::uniform_real_distribution<double> uni;
  std::mt19937 rng;

  QuantumState(size_t num_qubits = 1, size_t init_state = 0);

  void init(size_t num_qubits, size_t init_state);

  // gets measurement probabilities for a single qubit, normalized
  std::array<double, 2> measurement_probs(size_t qubit) const;
	
  // samples a single qubit measurement without collapsing the wavefunction
  inline size_t sample_measurement_once(double p1);

  // samples a qubit measurement num_samples times as if measuring many identical copies prepared in the same state
  SampleResult sample_measurement(size_t qubit, size_t num_samples);

  // measures a single qubit and collapses the wavefunction
  // returns the measurement result
  size_t measure(size_t qubit);

  // measures the entire system and collapses the wavefunction
  // returns the measurement result
  size_t measure_all();

  // arbitrary unitary operation on a single qubit
  void apply_unitary_1q(size_t qubit, Complex u00, Complex u01, Complex u10, Complex u11);

  // stabilizer gates
  void apply_hadamard(size_t qubit);
  void apply_s(size_t qubit);
  void apply_cnot(size_t cntrl, size_t qubit);

  // pauli gates
  void apply_x(size_t qubit);
  void apply_y(size_t qubit);
  void apply_z(size_t qubit);

  // controlled-controlled not, aka AND gate
  void apply_toffoli(size_t cntrl1, size_t cntrl2, size_t qubit);

  // sanity check function to ensure total probability is 1
  double total_probability() const;

  // DEBUG: print current state
  void print_state() const;
};
