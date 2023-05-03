{
  description = "A simple project";

  inputs = {
    mars-std.url = "github:mars-research/mars-std";
  };

  outputs = { self, mars-std, ... }: let
    # System types to support.
    supportedSystems = [ "aarch64-linux" ];
  in mars-std.lib.eachSystem supportedSystems (system: let
    pkgs = mars-std.legacyPackages.${system};
  in {
    devShell = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [
        clang cmake meson ninja python3
      ];
      cmakeFlags = [
        "-GNinja"
        "-DGCC_INSTALL_PREFIX=${pkgs.gcc-unwrapped}"
        "-DC_INCLUDE_DIRS=${pkgs.stdenv.cc.libc.dev}/include"
        "-DPREFIX_DIRS=${pkgs.stdenv.cc.libc}/lib"
        "-DLLVM_ENABLE_PROJECTS='clang;lld'"
        "-DLLVM_ENABLE_RUNTIMES=openmp"
        "-DCMAKE_BUILD_TYPE=MinSizeRel"
        "-DLLVM_TARGETS_TO_BUILD=X86"
      ];
    };
  });
}
