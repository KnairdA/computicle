with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "env";
  env = buildEnv { name = name; paths = buildInputs; };
  buildInputs = [
    git cmake gcc gdb cgdb
    glew
    glfw3
    glm
  ];
}
