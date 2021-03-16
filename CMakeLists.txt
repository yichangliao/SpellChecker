cmake_minimum_required(VERSION 3.15)
project(5_1 C)

set(CMAKE_C_STANDARD 99)

add_executable(5_1 CuTest.c CuTest.h hashMap.c hashMap.h spellChecker.c)

