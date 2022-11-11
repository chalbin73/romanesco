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

For Linux:
`$ premake5 gmake2`
`$ make`
`$ ./romanesco`

For Windows:
Comming soon&trade;

