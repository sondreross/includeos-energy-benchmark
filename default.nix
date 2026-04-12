{
  withCcache ? false, # Enable ccache support
  smp ? false, # Enable SMP support
  workload ? "std_workload", # Workload to compile (default: std_workload)
  shortBench ? false, # Enable SHORT_BENCH for benchmark compilation

  includeos ? import (builtins.fetchGit {
        url = "https://github.com/includeos/IncludeOS.git"; # TODO: Change to official repo when possible
        rev = "d39cb456b92925e7baf2505578099788518b0043";
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

  nativeBuildInputs = includeos.nativeBuildInputs ++ [ pkgs.grub2 pkgs.xorriso ];

  buildInputs = [
    includeos
    benchmarks
  ];

  cmakeFlags = [
    "-DWORKLOAD=${workload}"
    "-DBENCHMARK_STATIC_LIB=${benchmarks}/lib/libenergy_benchmarks.a"
  ];

  postInstall = ''
    bash ${./create-iso.sh}
    cp freq_profiling.iso $out/
  '';

}