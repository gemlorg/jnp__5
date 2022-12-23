cmake_minimum_required(VERSION 3.16)
project(jnp__5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wfatal-errors -Wall -Wextra -Wpedantic -O2"
        )

include_directories(.)

add_executable(jnp__5
        #        organism_example.cc
        przyklad.cc
        kvfifo.h
        )


