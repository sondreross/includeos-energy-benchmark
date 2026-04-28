{
  withCcache ? false, # Enable ccache support
  smp ? false, # Enable SMP support
  workload ? "std_workload", # Workload to compile (default: std_workload)
  shortBench ? false, # Enable SHORT_BENCH for benchmark compilation
  measurePkg ? true, # Measure PKG RAPL domain
  measureDram ? false, # Measure DRAM RAPL domain
  measurePp0 ? true, # Measure PP0 RAPL domain
  measurePp1 ? true, # Measure PP1 RAPL domain

  includeos ? import (builtins.fetchGit {
        url = "https://github.com/sondreross/IncludeOS.git"; # TODO: Change to official repo when possible
        ref = "bench_func";
      }) { inherit smp; inherit withCcache; },
}:
let
  stdenv = includeos.stdenv;
  pkgs = includeos.pkgs;
  benchmarks = import ./benchmarks.nix {
    inherit pkgs shortBench;
  };
in
stdenv.mkDerivation {
  name = "FrequencyScalingTool";
  version = "dev";

  src = ./src;
  sourceRoot = "src/unikernel";

  nativeBuildInputs = includeos.nativeBuildInputs ++ [ pkgs.grub2 pkgs.xorriso ];

  buildInputs = [
    includeos
    benchmarks
  ];

  cmakeFlags = [
    "-DWORKLOAD=${workload}"
    "-DBENCHMARK_STATIC_LIB=${benchmarks}/lib/libenergy_benchmarks.a"
    "-DMEASURE_DOMAIN_PKG=${if measurePkg then "ON" else "OFF"}"
    "-DMEASURE_DOMAIN_DRAM=${if measureDram then "ON" else "OFF"}"
    "-DMEASURE_DOMAIN_PP0=${if measurePp0 then "ON" else "OFF"}"
    "-DMEASURE_DOMAIN_PP1=${if measurePp1 then "ON" else "OFF"}"
  ];

  postInstall = ''
    bash ${./create-iso.sh}
    cp frequency_scaling_tool.iso $out/
  '';

}
