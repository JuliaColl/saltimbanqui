# saltimbanqui
 
Saltimbanqui is a 3D game developed by Júlia Coll and Veronica Bruno for the TJE course at Universitat Pompeu Fabra. 
 
The game is based in different levels. The goal of each level is to collect all the coins before reaching the end flag. During each level there are some obstacles that the player needs to overcome otherwise they will lose lives. If the player loses all of their lives, the game is over.

In the following link there is a video demo of the game:
https://youtu.be/sBo6iRdxABA
 
# TJE Framework #

TJE Framework is a C++ layer on top of SDL and OpenGL to help create games or visual applications.
It only provides the basic GPU abstraction (Meshes, Textures, Shaders, Application).
It has been used for many years in the Videogame's development courses Javi Agenjo gives at the Universitat Pompeu Fabra.

It contains the basics to do something nice:
- Mesh, Texture, Shader and FBO classes
- Vector2,Vector3,Vector4,Matrix44 and Quaternion classes
- Meshes have a ray-mesh and sphere-mesh collision method (thanks to library Coldet)
- Parser to load OBJ, PNG and TGA. Re-stores meshes in binary for faster load.
- Supports skinned animated meshes using own format (you must use a web tool to convert them).

Enough to make gamejam games pretty easily.


## Installation ##

It comes with a Visual Studio solution, but the code should compile in any platform that supports SDL.


## Compiling in Linux

Remember to install the libraries:

sudo apt-get install libsdl2-dev freeglut3 freeglut3-dev

and then just use the makefile:

make


