#include "quantum_state.h"
#include <print>
#include <bitset>

static constexpr double EPS = 1e-12;

QuantumState::QuantumState(size_t num_qubits = 1, size_t init_state = 0) {
	std::random_device rd;
	rng = std::mt19937(rd());
	init(num_qubits, init_state);
}

// sets state to |00...0>
void QuantumState::init(size_t num_qubits, size_t init_state) {
	n = num_qubits;
	psi.assign(1ULL << n, Complex(0.0, 0.0));
	psi[init_state] = Complex(1.0, 0.0);
}

size_t QuantumState::measure(size_t qubit) {
	size_t bit = 1ULL << qubit;
	double prob[2] = { 0.0, 0.0 };

	for (size_t i = 0; i < psi.size(); i++) {
		prob[((i & bit) != 0)] += std::norm(psi[i]);
	}

	size_t res;
	if (prob[0] < EPS && prob[1] < EPS) {
		// something went wrong, will see later
		return 0;
	}
	else if (prob[0] < EPS) {
		res = 1;
	}
	else if (prob[1] < EPS) {
		res = 0;
	}
	else {
		std::discrete_distribution<size_t> dist({prob[0], prob[1]});
		res = dist(rng);
	}

	double scale = 1.0 / std::sqrt(prob[res]);
	for (size_t i = 0; i < psi.size(); i++) {
		if (((i & bit) != 0) == res) {
			psi[i] *= scale;
		}
		else {
			psi[i] = Complex(0.0, 0.0);
		}
	}

	return res;
}

// measure the system and collapse the wavefunction
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

void QuantumState::apply_hadamard(size_t qubit) {
	const double scl = 1.0 / std::sqrt(2);
	size_t bit = 1ULL << qubit;

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
	size_t control_bit = 1ULL << cntrl;
	size_t q_bit = 1ULL << qubit;

	for (size_t i = 0; i < psi.size(); i++) {
		if ((i & control_bit) != 0 && (i & q_bit) == 0) {
			Complex tmp = psi[i];
			psi[i] = psi[i | q_bit];
			psi[i | q_bit] = tmp;
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