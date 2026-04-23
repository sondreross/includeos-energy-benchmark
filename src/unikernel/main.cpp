#include <os>
#include <service>
#include <delegate>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <arch/x86/cpu.hpp>
#include <kernel/rtc.hpp>

// Include all benchmark headers
extern "C" {
#include "../benchmarks/crc32/crc32.h"
#include "../benchmarks/cubic/cubic.h"
#include "../benchmarks/dijkstra/dijkstra.h"
#include "../benchmarks/fdct/fdct.h"
#include "../benchmarks/fir/fir.h"
#include "../benchmarks/matmult-float/matmult_float.h"
#include "../benchmarks/matmult-int/matmult_int.h"
#include "../benchmarks/nettle-sha256/nettle_sha256.h"
#include "../benchmarks/rijndael/rijndael.h"
}

// Array of benchmark functions
typedef int (*benchmark_func_t)(void);
typedef void (*init_func_t)(void);
typedef void (*void_func_t)(void);

// Wrapper functions to convert int-returning benchmarks to void
void crc32_wrapper() { (void)crc32(); }
void cubic_wrapper() { (void)cubic(); }
void dijkstra_wrapper() { (void)dijkstra_bench(); }
void fdct_wrapper() { (void)fdct_bench(); }
void fir_wrapper() { (void)fir(); }
void matmult_float_wrapper() { (void)matmult_float(); }
void matmult_int_wrapper() { (void)matmult_int(); }
void nettle_sha256_wrapper() { (void)nettle_sha256_bench(); }
void rijndael_wrapper() { (void)rijndael(); }

struct Benchmark {
    const char* name;
    init_func_t init;
    void_func_t func;
};

Benchmark benchmarks[] = {
    {"crc32", initialise_benchmark_crc32, crc32_wrapper},
    {"cubic", initialise_benchmark_cubic, cubic_wrapper},
    {"dijkstra", initialise_benchmark_dijkstra, dijkstra_wrapper},
    {"fdct", initialise_benchmark_fdct, fdct_wrapper},
    {"fir", initialise_benchmark_fir, fir_wrapper},
    {"matmult-float", initialise_benchmark_matmult_float, matmult_float_wrapper},
    {"matmult-int", initialise_benchmark_matmult_int, matmult_int_wrapper},
    {"nettle-sha256", initialise_benchmark_nettle_sha256, nettle_sha256_wrapper},
    {"rijndael", initialise_benchmark_rijndael, rijndael_wrapper}
};

namespace energy_bench {

static inline uint64_t rdtsc_start() {
    uint32_t lo, hi;
    asm volatile (
        "cpuid\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        :: "%rbx", "%rcx"
    );
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t rdtsc_end() {
    uint32_t lo, hi;
    asm volatile (
        "rdtscp\n\t"
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"
        : "=r"(hi), "=r"(lo)
        :: "%rax", "%rbx", "%rcx", "%rdx"
    );
    return ((uint64_t)hi << 32) | lo;
}

double get_rapl_units() {
    uint64_t power_unit_msr = x86::CPU::read_msr(MSR_RAPL_POWER_UNIT);
    uint8_t energy_unit_bits = (power_unit_msr >> 8) & 0x1F;  // Bits 12:8

    // Energy unit: 1/2^ESU joules per count
    double energy_unit_joules = 1.0 / (1 << energy_unit_bits);

    return energy_unit_joules;
}


uint64_t bench_function_single(delegate<void()> func)
{
    double energy_unit = get_rapl_units();
    uint32_t raw_before = x86::CPU::read_msr(MSR_PKG_ENERGY_STATUS) & 0xFFFFFFFF;

    func();

    uint32_t raw_after = x86::CPU::read_msr(MSR_PKG_ENERGY_STATUS) & 0xFFFFFFFF;

    // Calculate difference first, then convert to microjoules
    uint64_t raw_diff = raw_after - raw_before;
    //TODO: Handle wrap-around
    uint64_t energy_microjoules = static_cast<uint64_t>(raw_diff * energy_unit * 1000000.0);
    return energy_microjoules;
}

energy_result bench_function(void (*func)(), uint32_t domains)
{
    double energy_unit = get_rapl_units();
    energy_result result;

    // Store raw "before" values (lower 32 bits only; upper 32 are reserved)
    uint32_t pkg_before = 0, dram_before = 0, pp0_before = 0, pp1_before = 0;

    // Read IA32_TEMPERATURE_TARGET (MSR 0x1A2) — bits 23:16 contain TjMax (temperature target)
    uint64_t temp_target_msr = x86::CPU::read_msr(MSR_TEMPERATURE_TARGET);
    uint8_t therm_max = static_cast<uint8_t>((temp_target_msr >> 16) & 0xFF);
    uint32_t therm_start = static_cast<uint32_t>((x86::CPU::read_msr(0x19C) >> 16) & 0x7F); // IA32_THERM_STATUS bits 22:16
    uint32_t pkg_therm_start = static_cast<uint32_t>((x86::CPU::read_msr(0x1B1) >> 16) & 0x7F); // IA32_PACKAGE_THERM_STATUS bits 22:16
    uint64_t start_ns = RTC::nanos_now();
    if (domains & PKG) {
        pkg_before = x86::CPU::read_msr(MSR_PKG_ENERGY_STATUS) & 0xFFFFFFFF;
        result.measured_domains |= PKG;
    }
    if (domains & DRAM) {
        dram_before = x86::CPU::read_msr(MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF;
        result.measured_domains |= DRAM;
    }
    if (domains & PP0) {
        pp0_before = x86::CPU::read_msr(MSR_PP0_ENERGY_STATUS) & 0xFFFFFFFF;
        result.measured_domains |= PP0;
    }
    if (domains & PP1) {
        pp1_before = x86::CPU::read_msr(MSR_PP1_ENERGY_STATUS) & 0xFFFFFFFF;
        result.measured_domains |= PP1;
    }

    // Capture start time (CPU cycles) using serialized RDTSC
    volatile uint64_t start_cycles = rdtsc_start();

    func();

    // Capture end time (CPU cycles) using serialized RDTSCP
    volatile uint64_t end_cycles = rdtsc_end();


    // Calculate differences and convert to microjoules
    if (domains & PKG) {
        uint32_t pkg_after = x86::CPU::read_msr(MSR_PKG_ENERGY_STATUS) & 0xFFFFFFFF;
        uint32_t raw_diff = pkg_after - pkg_before;
        result.pkg_microjoules = static_cast<uint64_t>(raw_diff * energy_unit * 1000000.0);
    }
    if (domains & DRAM) {
        uint32_t dram_after = x86::CPU::read_msr(MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF;
        uint32_t raw_diff = dram_after - dram_before;
        result.dram_microjoules = static_cast<uint64_t>(raw_diff * energy_unit * 1000000.0);
    }
    if (domains & PP0) {
        uint32_t pp0_after = x86::CPU::read_msr(MSR_PP0_ENERGY_STATUS) & 0xFFFFFFFF;
        uint32_t raw_diff = pp0_after - pp0_before;
        result.pp0_microjoules = static_cast<uint64_t>(raw_diff * energy_unit * 1000000.0);
    }
    if (domains & PP1) {
        uint32_t pp1_after = x86::CPU::read_msr(MSR_PP1_ENERGY_STATUS) & 0xFFFFFFFF;
        uint32_t raw_diff = pp1_after - pp1_before;
        result.pp1_microjoules = static_cast<uint64_t>(raw_diff * energy_unit * 1000000.0);
    }
    uint64_t end_ns = RTC::nanos_now();

    uint32_t therm_end = static_cast<uint32_t>((x86::CPU::read_msr(0x19C) >> 16) & 0x7F); // IA32_THERM_STATUS bits 22:16
    uint32_t pkg_therm_end = static_cast<uint32_t>((x86::CPU::read_msr(0x1B1) >> 16) & 0x7F); // IA32_PACKAGE_THERM_STATUS bits 22:16

    result.therm_tcc = static_cast<uint8_t>(therm_max);
    result.therm_start = static_cast<uint8_t>(therm_start);
    result.therm_end = static_cast<uint8_t>(therm_end);
    result.pkg_therm_start = static_cast<uint8_t>(pkg_therm_start);
    result.pkg_therm_end = static_cast<uint8_t>(pkg_therm_end);
    result.cycles_start = start_cycles;
    result.cycles_end = end_cycles;
    result.cycles_elapsed = result.cycles_end - result.cycles_start;
    result.nanos_elapsed = end_ns - start_ns;

    return result;
}

} // namespace energy_bench

// Check if RAPL is available
bool check_rapl_support() {
  // Try reading RAPL power unit MSR
  // If this causes an exception, RAPL is not supported
  try {
    uint64_t test = x86::CPU::read_msr(MSR_RAPL_POWER_UNIT);
    (void)test; // Suppress unused warning
    return true;
  } catch (...) {
    return false;
  }
}

// Wait for package temperature to cool down
// Returns the time waited in milliseconds
double wait_for_cooldown(double target_temp) {
    auto start_time = std::chrono::steady_clock::now();
    
    double current_temp;
    do {
        // Read package thermal status
        uint64_t temp_target = x86::CPU::read_msr(MSR_TEMPERATURE_TARGET);
        uint32_t tj_max = (temp_target >> 16) & 0xFF;
        uint64_t pkg_therm_status = x86::CPU::read_msr(IA32_PACKAGE_THERM_STATUS);
        uint32_t digital_readout = (pkg_therm_status >> 16) & 0x7F;
        current_temp = tj_max - digital_readout;
        
        if (current_temp > target_temp) {
            // Sleep for 10 milliseconds to allow CPU to cool
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } while (current_temp > target_temp);
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    return static_cast<double>(elapsed.count());
}

void Service::start(const std::string&) {
    const int repetitions = 50;
    std::vector<std::string> results;
    
    // Warmup: Run first benchmark until CPU reaches target temperature (45°C)
    benchmarks[2].init();
    double current_temp = 0.0;
    while (current_temp < 45.0) {
        benchmarks[2].func();
        
        // Read current package temperature
        uint64_t temp_target = x86::CPU::read_msr(MSR_TEMPERATURE_TARGET);
        uint32_t tj_max = (temp_target >> 16) & 0xFF;
        uint64_t pkg_therm_status = x86::CPU::read_msr(IA32_PACKAGE_THERM_STATUS);
        uint32_t digital_readout = (pkg_therm_status >> 16) & 0x7F;
        current_temp = tj_max - digital_readout;
        printf("Warmup: Current package temperature: %.2f °C\n", current_temp);
    }
  
    // Run each benchmark
    for (size_t b = 0; b < sizeof(benchmarks) / sizeof(benchmarks[0]); b++) {
        for (int i = 0; i < repetitions; ++i) {
            // Send 0x1b marker with benchmark name and repetition number
            printf("\x1b%s,%d\n", benchmarks[b].name, i);
            
            // Wait for package temperature to be under 45 degrees
            double cooldown_ms = wait_for_cooldown(45.0);
      
            benchmarks[b].init();
            auto result = energy_bench::bench_function(
                benchmarks[b].func,
                energy_bench::PKG | energy_bench::PP0 | energy_bench::PP1
            );

            double temp_before = result.therm_tcc - result.therm_start;
            double temp_after = result.therm_tcc - result.therm_end;
            double pkg_temp_before = result.therm_tcc - result.pkg_therm_start;
            double pkg_temp_after = result.therm_tcc - result.pkg_therm_end;
            double time_ns = result.nanos_elapsed;
            double time_ms = time_ns / 1e6;
            double pkg_joules = result.pkg_joules();
            double pp0_joules = (result.measured_domains & energy_bench::PP0) ? result.pp0_joules() : 0.0;
            double pp1_joules = (result.measured_domains & energy_bench::PP1) ? result.pp1_joules() : 0.0;
            double total_joules = result.total_joules();

            // Format and store CSV row
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), 
                     "%s,%lu,%lu,%lu,%.3f,%.6f,%.3f,%.2f,%.2f,%.2f,%.2f,%.6f,%.3f,%.6f,%.3f,%.6f,%.3f,,,%.6f,%.3f",
                     benchmarks[b].name,
                     result.cycles_elapsed,
                     result.cycles_start,
                     result.cycles_end,
                     time_ns,
                     time_ms,
                     cooldown_ms,
                     temp_before,
                     temp_after,
                     pkg_temp_before,
                     pkg_temp_after,
                     pkg_joules,
                     pkg_joules * 1000,
                     pp0_joules,
                     pp0_joules * 1000,
                     pp1_joules,
                     pp1_joules * 1000,
                     total_joules,
                     total_joules * 1000);
            results.push_back(std::string(buffer));
            
            // Send 0x1b marker at the end
            printf("\x1b%s,%d\n", benchmarks[b].name, i);
        }
    }
    
    // Print CSV header
    printf("benchmark,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,cooldown_ms,temp_before,temp_after,pkg_temp_before,pkg_temp_after,pkg_joules,pkg_mJ,pp0_joules,pp0_mJ,pp1_joules,pp1_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");
    
    // Print all results
    for (const auto& row : results) {
        printf("%s\n", row.c_str());
    }
    fflush(stdout);
  
    os::shutdown();
}
