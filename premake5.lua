workspace("romanesco")
configurations({"Debug", "Release"})

project("romanesco")
kind("ConsoleApp")
language("C")
targetdir(".")
files({"source/**.c", "include/**.h"})

includedirs({"include", "/usr/include"})
defines()
buildoptions({"-Wall", "-Werror"})
links({"m", "pthread", "GLEW", "SDL2", "GLU", "GL", "glut"})