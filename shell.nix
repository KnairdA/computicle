with import <nixpkgs> {};

stdenv.mkDerivation rec {
  name = "env";
  env = buildEnv { name = name; paths = buildInputs; };
  buildInputs = [
    git cmake gcc gdb cgdb
    glfw3
    glew
    glm
  ];
}
