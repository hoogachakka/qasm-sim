#include "quantum_state.h"
#include <print>
#include <bitset>

static constexpr double EPS = 1e-12;

void SampleResult::log_results() {
  std::println("0 measured {} times\n1 measured {} times", results[0], results[1]);
}

QuantumState::QuantumState(size_t num_qubits, size_t init_state) {
  std::random_device rd;
  rng = std::mt19937(rd());
  uni = std::uniform_real_distribution(0.0, 1.0);
  init(num_qubits, init_state);
}

// sets state to |00...0>
void QuantumState::init(size_t num_qubits, size_t init_state) {
  n = num_qubits;
  psi.assign(1ULL << n, Complex(0.0, 0.0));
  psi[init_state] = Complex(1.0, 0.0);
}

std::array<double, 2> QuantumState::measurement_probs(size_t qubit) const {
  size_t bit = 1ULL << qubit;
  std::array<double, 2> prob = { 0.0, 0.0 };

  for (size_t i = 0; i < psi.size(); i++) {
    prob[((i & bit) != 0)] += std::norm(psi[i]);
  }

  if (prob[0] < EPS && prob[1] < EPS) {
    throw std::runtime_error("At least one probability must be non-zero");
  }
  else if (prob[0] < EPS) {
    prob[0] = 0.0;
    prob[1] = 1.0;
  }
  else if (prob[1] < EPS) {
    prob[0] = 1.0;
    prob[1] = 0.0;
  }
  else {
    const double norm = 1 / (prob[0] + prob[1]);
    prob[0] *= norm;
    prob[1] *= norm;
  }

  return prob;
}

inline size_t QuantumState::sample_measurement_once(double p1) {
  double u = uni(rng);
  return (u < p1) ? 1 : 0;
}

SampleResult QuantumState::sample_measurement(size_t qubit, size_t num_samples) {
  auto prob = measurement_probs(qubit);
  SampleResult res;

  for (size_t i = 0; i < num_samples; i++) {
    res.results[sample_measurement_once(prob[1])]++;
  }

  return res;
}

size_t QuantumState::measure(size_t qubit) {
  auto prob = measurement_probs(qubit);
  size_t res = sample_measurement_once(prob[1]);

  size_t bit = 1ULL << qubit;
  double scl = 1.0 / std::sqrt(prob[res]);

  for (size_t i = 0; i < psi.size(); i++) {
    if (((i & bit) != 0) == res) {
      psi[i] *= scl;
    }
    else {
      psi[i] = Complex(0.0, 0.0);
    }
  }

  return res;
}

size_t QuantumState::measure_all() {
  std::vector<size_t> outcomes;
  std::vector<double> weights;
  outcomes.reserve(psi.size());
  weights.reserve(psi.size());

  double total = 0.0;
  for (size_t i = 0; i < psi.size(); i++) {
    double p = std::norm(psi[i]);
    if (p > EPS) {
      outcomes.push_back(i);
      weights.push_back(p);
      total += p;
    }
  }

  if (outcomes.empty() || total < EPS) {
    // TODO: something went wrong
    return 0;
  }

  std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
  size_t res = outcomes[dist(rng)];

  std::fill(psi.begin(), psi.end(), Complex(0.0, 0.0));
  psi[res] = Complex(1.0, 0.0);
  return res;
}

void QuantumState::apply_unitary_1q(size_t qubit, Complex u00, Complex u01, Complex u10, Complex u11) {
  size_t bit = 1ULL << qubit;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) == 0) {
      size_t j = i | bit;
      Complex a = psi[i];
      Complex b = psi[j];
      psi[i] = (u00 * a) + (u01 * b);
      psi[j] = (u10 * a) + (u11 * b);
    }
  }
}

void QuantumState::apply_hadamard(size_t qubit) {
  size_t bit = 1ULL << qubit;
  const double scl = 1.0 / std::sqrt(2);

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) == 0) {
      size_t j = i | bit;
      Complex a = psi[i];
      Complex b = psi[j];
      psi[i] = (a + b) * scl;
      psi[j] = (a - b) * scl;
    }
  }
}

void QuantumState::apply_s(size_t qubit) {
  size_t bit = 1ULL << qubit;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) != 0) {
      psi[i] *= Complex(0.0, 1.0);
    }
  }
}

void QuantumState::apply_cnot(size_t cntrl, size_t qubit) {
  size_t bit = 1ULL << qubit;
  size_t control_bit = 1ULL << cntrl;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & control_bit) != 0 && (i & bit) == 0) {
      Complex tmp = psi[i];
      psi[i] = psi[i | bit];
      psi[i | bit] = tmp;
    }
  }
}

void QuantumState::apply_x(size_t qubit) {
  size_t bit = 1ULL << qubit;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) == 0) {
      Complex tmp = psi[i];
      psi[i] = psi[i | bit];
      psi[i | bit] = tmp;
    }
  }
}

void QuantumState::apply_y(size_t qubit) {
  size_t bit = 1ULL << qubit;
  const Complex I(0.0, 1.0);

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) == 0) {
      Complex tmp = psi[i];
      psi[i] = -I * psi[i | bit];
      psi[i | bit] = I * tmp;
    }
  }
}

void QuantumState::apply_z(size_t qubit) {
  size_t bit = 1ULL << qubit;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & bit) != 0) {
      psi[i] *= -1;
    }
  }
}

void QuantumState::apply_toffoli(size_t cntrl1, size_t cntrl2, size_t qubit) {
  size_t bit = 1ULL << qubit;
  size_t control_bit1 = 1ULL << cntrl1;
  size_t control_bit2 = 1ULL << cntrl2;

  for (size_t i = 0; i < psi.size(); i++) {
    if ((i & control_bit1) != 0 && (i & control_bit2) != 0 && (i & bit) == 0) {
      Complex tmp = psi[i];
      psi[i] = psi[i | bit];
      psi[i | bit] = tmp;
    }
  }
}

double QuantumState::total_probability() const {
  double total = 0.0;
  for (const auto& c : psi) {
    total += std::norm(c);
  }
  return total;
}

static std::string complex_to_string(Complex c) {
  if (std::fabs(c.imag()) <= EPS && std::fabs(c.real()) <= EPS) {
    return "";
  }
  else if (std::fabs(c.imag()) <= EPS) {
    return std::format("{:.4}", c.real());
  }
  else if (std::fabs(c.real()) <= EPS) {
    return std::format("{:.4}i", c.imag());
  }
  else {
    return std::format("{:.4} + {:.4}i", c.real(), c.imag());
  }
}

void QuantumState::print_state() const {
  for (int i = 0; i < psi.size(); i++) {
    auto s = std::bitset<32>(i).to_string().substr(32 - n);
    if (std::norm(psi[i]) > EPS) {
      std::println("({:.4} + {:.4}i)|{}>", psi[i].real(), psi[i].imag(), s);
    }
  }
}
