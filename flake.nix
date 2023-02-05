{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devenv.url = "github:cachix/devenv";
  };

  outputs = { self, nixpkgs, flake-utils, devenv, ... } @ inputs:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.default = devenv.lib.mkShell {
        inherit inputs pkgs;
        modules = [
          {
            languages.python.enable = true;

            packages = [
               pkgs.python3Packages.pycryptodome
            ];

            pre-commit.hooks.clang-format = {
              enable = true;
              excludes = [
                "components/RPi_SPI_TPM/3rdParty"
              ];
            };
          }
        ];
      };
    });
}
