workspace("romanesco")
configurations({"Debug", "Release"})

project("romanesco")
kind("ConsoleApp")
language("C")
targetdir(".")
files({"source/**.c", "include/**.h"})

if os.istarget("linux") or os.istarget("macosx") then
	print("Os: LINUX or MACOSX")
	architecture("x86_64")
    -- Glew is only compiled from source on windows because its easier
    removefiles({"source/glew.c"})
	includedirs({"include", "/usr/include"})
	buildoptions({"-Wall", "-Werror"})
	links({"m" , "GLEW", "SDL2", "GLU", "GL", "glut"})
end

if os.istarget("windows") then
	print("Os: WINDOWS")
	system("Windows")
	architecture("x86_64")
	includedirs({"%(ProjectDir)include", "%(ProjectDir)windows/SDL2/include", "%(ProjectDir)windows/GLEW/include"})
	libdirs("%(ProjectDir)windows/SDL2/lib/x64/");
	links({"SDL2", "SDL2main", "opengl32"});
end