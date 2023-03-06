# Animated-Presentation
A C application for creating animated presentations

## Build instructions
This project uses [premake5](https://premake.github.io/) for build configuration.
```
git clone https://github.com/Magicalbat/Animated-Presentation.git
cd Animated-Presentation
```
**NOTE: By default, clang will be used on both linux and windows. To avoid this, you can use the `--no-clang` option when running premake.**
### Windows
You can change the version if you do not have [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) installed.
```
premake5 vs2022
```
Open the Visual Studio Solution to build the project.
### Linux
You made need to install some packages. The project needs to link with X11, GL, and GLX.
```
premake5 gmake2
make
```
### Emscripten
On windows, premake5 automatically adds .exe to the output files. You can either rename the files after building, or edit the Makefile.
```
premake5 gmake2 --wasm
emmake make
```


## Inspired by
- [Mr. 4th Programming](https://www.youtube.com/c/Mr4thProgramming)
- [olc Pixel Game Engine](https://github.com/OneLoneCoder/olcPixelGameEngine)
- [Manim](https://github.com/3b1b/manim)
