cmake_minimum_required(VERSION 3.10)
project(Benchmarks C)

option(SHORT_BENCH "Use short benchmark repeat count" OFF)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "-Wall -O2")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

# CRC32
add_library(crc32_obj OBJECT crc32/crc_32.c)

# Cubic
add_library(cubic_obj OBJECT cubic/libcubic.c cubic/basicmath_small.c cubic/snipmath.h)

# Dijkstra
add_library(dijkstra_obj OBJECT dijkstra/dijkstra_small.c)

# FDCT
add_library(fdct_obj OBJECT fdct/libfdct.c)

# FIR
add_library(fir_obj OBJECT fir/libfir.c)

# Matmult-float
add_library(matmult_float_obj OBJECT matmult-float/matmult.c)
target_compile_definitions(matmult_float_obj PRIVATE MATMULT_FLOAT)

# Matmult-int
add_library(matmult_int_obj OBJECT matmult-int/matmult.c)
target_compile_definitions(matmult_int_obj PRIVATE MATMULT_INT)

# Nettle-SHA256
add_library(nettle_sha256_obj OBJECT nettle-sha256/nettle-sha256.c)

# Rijndael
add_library(rijndael_obj OBJECT rijndael/aesxam.c rijndael/aes.c)

if(SHORT_BENCH)
    foreach(target crc32_obj cubic_obj dijkstra_obj fdct_obj fir_obj
                   matmult_float_obj matmult_int_obj nettle_sha256_obj rijndael_obj)
        target_compile_definitions(${target} PRIVATE SHORT_BENCH)
    endforeach()
endif()

add_library(energy_benchmarks STATIC
    $<TARGET_OBJECTS:crc32_obj>
    $<TARGET_OBJECTS:cubic_obj>
    $<TARGET_OBJECTS:dijkstra_obj>
    $<TARGET_OBJECTS:fdct_obj>
    $<TARGET_OBJECTS:fir_obj>
    $<TARGET_OBJECTS:matmult_float_obj>
    $<TARGET_OBJECTS:matmult_int_obj>
    $<TARGET_OBJECTS:nettle_sha256_obj>
    $<TARGET_OBJECTS:rijndael_obj>
)

install(TARGETS energy_benchmarks ARCHIVE DESTINATION lib)
