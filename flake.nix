{
  description = "NaCl on ARM";

  inputs = {
    mars-std.url = "github:mars-research/mars-std";
  };

  outputs = { self, mars-std, ... }: let
    # System types to support.
    supportedSystems = [ "x86_64-linux" ];
  in mars-std.lib.eachSystem supportedSystems (system: let
    pkgs = mars-std.legacyPackages.${system};
    armPkgs = pkgs.pkgsCross.aarch64-multiplatform;

    llvmSelected = pkgs.llvmPackages_14;

    mkShell = pkgs.mkShell.override {
      stdenv = llvmSelected.stdenv;
    };
  in {
    devShell = mkShell {
      inputsFrom = [ llvmSelected.llvm ];

      # cmake -B build $cmakeFlags
      cmakeFlags = [
        "-Sllvm"
        "-DLLVM_ENABLE_PROJECTS='clang;lld'"
        "-DCMAKE_BUILD_TYPE=Debug"
        "-DLLVM_ENABLE_LIBCXX=TRUE"
        "-GNinja"
      ];

      ARM_CFLAGS = let
        gcc = armPkgs.stdenv.cc.cc;
        platform = armPkgs.stdenv.targetPlatform.config;
        gccPath = "${gcc}/lib/gcc/${platform}/${gcc.version}";
        libc = armPkgs.glibc;
      in "-Wl,-L,${gccPath} -B ${gccPath} -B ${libc}/lib";

      nativeBuildInputs = with pkgs; [
        # Insert dev dependencies here
        armPkgs.stdenv.cc
        
      ];
    };
  });
}
