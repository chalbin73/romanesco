# Romanesco
#### A simple GPU Julia sets and Mandelbrot set plotter

This program is a simple GPU, double-floating-point precision plotter for Julia sets and Mandlebrot sets. It is made with OpenGL compute-shaders and it is written in C.

#### Features

- [x] Julia sets with live constant changing
- [x] Mandlebrot set
- [x] Live Panning and zooming
- [x] Integer exponents
- [x] Real exponents (Moivre's formula)
- [x] Image exporting
- [x] Uses -Wall and -Werror
- [x] Works on both linux and windows

#### Todo (Not ordered)

- [x] Multiple coloring methods
- [x] Multiple coloring palettes
- [ ] Visualize the different values taken by the serie
- [ ] Buddhabrot fractal (probably another backend)
- [ ] Burning ship fractal
- [ ] Newton's fractals (another backend)
- [ ] Save locations
- [ ] Add some sort of configuration (Mostly to configure OpenGL workgroup sizes)

#### Dependencies

- OpenGL 3.3
- SDL2 for window and GL context
- GLEW for OpenGL
- Nuklear for UI
- Posix for file I/O
- tinydir.h (included) to scan directories
- stb_image.h (included) to read images
- stb_include.h (included) to use include directive in GLSL

## Building 

##### For Linux (and MacOSX but not tested):
Install SDL2 and glew through your package manager
examble on Debian based systems:\
`# apt install libsdl2-dev libglew-dev`\
(Package name differ depending on distro)\

`$ premake5 gmake2`\
`$ make`\
`$ ./romanesco`

##### For Windows:
You must use Visual Studio / Visual C compiler :
The necessary dependencies (include, libs and dlls) are included in the windows folder.

/!\ Current source generates alot of warnings on windows.

`> premake5 vs2019` (Or the installed vs version)\
`> msbuild`\
The exe should the be built.
Don't forget to include the `SDL2.dll` file next to the executable. The dll is included in `windows\SDL2\lib\x64\SDL2.dll`.

