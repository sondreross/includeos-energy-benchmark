# Energy Tool

Proof-of-concept tool for measuring energy consumption across the full frequency range of an Intel x86 CPU. Built on IncludeOS, packaged with Nix. Part of the master's thesis *Energy Measurement of Programs Using Unikernels*
(UiO, 2026).


## Build
Build by running `nix build`. This will results in a results direcory, inside this is the bootable ```frequency_scaling_tool.iso```. 
You can add arguments using the `--arg` flag:
- `withCcache` (default: `false`) — Enable ccache support
- `smp` (default: `false`) — Enable SMP support
- `workload` (default: `"std_workload"`) — Workload to compile (currently only std_workload, this is the BEEBS suite.)
- `shortBench` (default: `false`) — Enable SHORT_BENCH for benchmark compilation
- `measurePkg` (default: `true`) — Measure PKG RAPL domain
- `measureDram` (default: `false`) — Measure DRAM RAPL domain
- `measurePp0` (default: `true`) — Measure PP0 RAPL domain
- `measurePp1` (default: `true`) — Measure PP1 RAPL domain

### Example

Build with short benchmarks:
```bash
nix build --arg shortBench true
```

## Running
1. Write the `.iso` to a USB stick and boot the DUT from it.
2. Connect a serial cable from the DUT to an external machine.
3. Read the serial port (e.g. `screen /dev/ttyUSB0 115200`).

