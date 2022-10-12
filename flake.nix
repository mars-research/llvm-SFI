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
    armNativePkgs = mars-std.legacyPackages.aarch64-linux;
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
        gcc = armNativePkgs.stdenv.cc.cc;
        linuxArch = armNativePkgs.stdenv.targetPlatform.linuxArch;
        platform = armNativePkgs.stdenv.targetPlatform.config;
        gccPath = "${gcc}/lib/gcc/${platform}/${gcc.version}";
        libc = armNativePkgs.glibc;
      in "--target=aarch64-pc-linux -I ${libc.dev}/include -L ${gccPath} -B ${gccPath} -L ${libc}/lib -B ${libc}/lib -Wl,-dynamic-linker,${libc}/lib/ld-linux-aarch64.so.1";

      nativeBuildInputs = with pkgs; [
        # Insert dev dependencies here
        armPkgs.stdenv.cc
        qemu
      ];
    };
  });
}
