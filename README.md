# Romanesco
#### A simple GPU Julia sets and Mandelbrot set plotter

This program is a simple GPU, double-floating-point precision plotter for Julia sets and Mandlebrot sets. It made with OpenGL compute-shaders written in C.

#### Features

- Julia sets with live constant changing
- Mandlebrot set
- Live Panning and zooming
- Integer exponents
- Real exponents (Moivre's formula)
- Image exporting
- Uses -Wall and -Werror

#### Todo (Not ordered)

- Clean code
  - Separate fractal renderers in different files
  - Clean CPU side code
- Multiple coloring methods
- Multiple coloring palettes
- Burning ship fractal
- Newton's fractals
- Better windows support (POSIX for Unix, windows.h for windows)
- Save locations
- Add some sort of configuration (Mostly to configure OpenGL workgroup sizes)

#### Dependencies

- OpenGL 3.3
- SDL2 for window and GL context
- GLEW for OpenGL
- Nuklear for UI
- Currently working with POSIX

#### Building 

For Linux (and MacOSX, not tested yet)):
Install SDL2 and glew through your package manager
examble on Debian based systems:\
`# apt install libsdl2-dev libglew-dev`\
(Package name differ depending on distro)\

`$ premake5 gmake2`\
`$ make`\
`$ ./romanesco`

For Windows:
You must use Visual Studio / Visual C compiler :
The necessary dependencies (include, libs and dlls) are included in the windows folder.

`> premake5 vs2019` (Or the installed vs version)\
`> msbuild`\
The exe should the be built.
Don't forget to include the `SDL2.dll` file new to the executable. The dll is included in `windows\SDL2\lib\x64\SDL2.dll`.

