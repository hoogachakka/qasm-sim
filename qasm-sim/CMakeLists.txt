# CMakeList.txt : CMake project for qasm-sim, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (qasm-sim "simulator.cpp"  "lexer.cpp" "parser.cpp" "include/lexer.h" "include/gates.h" "include/quantum_state.h" "quantum_state.cpp" "include/circuit.h" "circuit.cpp")

target_include_directories(qasm-sim PRIVATE include)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET qasm-sim PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
