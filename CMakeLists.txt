cmake_minimum_required (VERSION 2.6)
project (lispy)
SET( CMAKE_CXX_COMPILER, "clang++" )
add_executable(lispy.tsk src/lispy_elems.cpp src/main.cpp src/mpc.c src/operator_eval.cpp )

add_definitions(--std=c++11)
target_link_libraries( lispy.tsk readline )

