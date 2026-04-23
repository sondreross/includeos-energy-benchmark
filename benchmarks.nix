{
	pkgs ? import <nixpkgs> {}, # gets this from IncludeOS nixpkgs in default
	shortBench ? false,
}:

pkgs.stdenv.mkDerivation {
	pname = "energy-benchmarks";
	version = "dev";

	src = ./src/benchmarks;

	nativeBuildInputs = [
		pkgs.cmake
		pkgs.clang
	];

	cmakeFlags = pkgs.lib.optionals shortBench [
		"-DSHORT_BENCH=ON"
	];
}
