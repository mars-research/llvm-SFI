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
    devShells.default = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [
        # Insert dev dependencies here
        llvmPackages_14.clang
        cmake
        meson
        ninja
        gdb
      ];
    };
  });
}
