cmake_minimum_required(VERSION 3.0.0)
project(KnightPack VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0072 NEW)

add_executable(KnightPack 
	src/main.cpp
)

target_include_directories(KnightPack
	PRIVATE
	dependencies
)
