// all stabilizer circuits from the normalizer of the qubit Pauli group/Clifford group can be perfectly simulated in polynomial time.
// Clifford group can be generated with CNOT, Hadamard, and phase gates - stabilizer circuits can be constructed using only these gates.
// in our sim, we check if a circuit is clifford circuit - if it is, simulation is polynomial time. we will get to how to simulate other gates later.

// a stabilizer is a subgroup Gx of G consisting of all g in G s.t. g(x) = x -- Gx is the stabilizer of x
// furthermore, a stabilizer of a subset fixes the subset