## Tic Tac Toe

project using the [FTXUI library](https://github.com/ArthurSonzogni/ftxui)

simply Tic Tac Toe game, nothing else

## Build Requirements

- CMake (version 3.11)
- GNU Make
- C++ compiler with support for C++17
- Optional: ECMAScript (JavaScript), Python

# Build instructions:

```bash
mkdir build
cd build
cmake ..
make -j
./tic-tac-toe
```

## Webassembly build:

```bash
mkdir build_emscripten && cd build_emscripten
emcmake cmake ..
make -j
./run_webassembly.py
(visit localhost:8000)
```

## Cool Gameplay

[![asciicast](https://asciinema.org/a/CUkL31hJIMfxwGOg5grujzdGk.png)](https://asciinema.org/a/CUkL31hJIMfxwGOg5grujzdGk)
