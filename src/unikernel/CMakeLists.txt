cmake_minimum_required(VERSION 3.6)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(FrequencyScalingTool)

find_package(IncludeOS)

include(os)

set(BENCHMARK_STATIC_LIB "" CACHE FILEPATH "Path to libenergy_benchmarks.a")

option(MEASURE_DOMAIN_PKG "Measure PKG RAPL domain" ON)
option(MEASURE_DOMAIN_DRAM "Measure DRAM RAPL domain" OFF)
option(MEASURE_DOMAIN_PP0 "Measure PP0 RAPL domain" ON)
option(MEASURE_DOMAIN_PP1 "Measure PP1 RAPL domain" ON)

if(NOT BENCHMARK_STATIC_LIB)
    message(FATAL_ERROR "BENCHMARK_STATIC_LIB must be set to the static benchmarks library path")
endif()

set(MEASURE_DOMAIN_PKG_VALUE 0)
if(MEASURE_DOMAIN_PKG)
    set(MEASURE_DOMAIN_PKG_VALUE 1)
endif()

set(MEASURE_DOMAIN_DRAM_VALUE 0)
if(MEASURE_DOMAIN_DRAM)
    set(MEASURE_DOMAIN_DRAM_VALUE 1)
endif()

set(MEASURE_DOMAIN_PP0_VALUE 0)
if(MEASURE_DOMAIN_PP0)
    set(MEASURE_DOMAIN_PP0_VALUE 1)
endif()

set(MEASURE_DOMAIN_PP1_VALUE 0)
if(MEASURE_DOMAIN_PP1)
    set(MEASURE_DOMAIN_PP1_VALUE 1)
endif()

add_compile_definitions(
    MEASURE_DOMAIN_PKG=${MEASURE_DOMAIN_PKG_VALUE}
    MEASURE_DOMAIN_DRAM=${MEASURE_DOMAIN_DRAM_VALUE}
    MEASURE_DOMAIN_PP0=${MEASURE_DOMAIN_PP0_VALUE}
    MEASURE_DOMAIN_PP1=${MEASURE_DOMAIN_PP1_VALUE}
)

os_add_executable(frequency_scaling_tool "Frequency scaling tool" main.cpp)
os_link_libraries(frequency_scaling_tool ${BENCHMARK_STATIC_LIB})
os_add_stdout(frequency_scaling_tool default_stdout)

# This would be for application specific libs.
#os_link_libraries(frequency_scaling_tool your_custom_lib)


# TODO: remove or hide the ELF_POSTFIX.
# currently os.cmake expects the target name to be TARGET.elf.bin
# where TARGET is the first param to os_add_executable.
# I added it here becuase nix expects an install target.
set(BINARY_NAME frequency_scaling_tool)
set(ELF_TARGET ${BINARY_NAME}${ELF_POSTFIX})

install(TARGETS ${ELF_TARGET} DESTINATION bin)
