with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "computicle-env";
  env = buildEnv { name = name; paths = buildInputs; };

  buildInputs = [
    fish
    git cmake gcc gdb cgdb
    glfw3
    glew
    glm
  ];

  shellHook = ''
    export NIX_SHELL_NAME="${name}"
  '';
}
