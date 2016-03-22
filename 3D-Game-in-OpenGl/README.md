Find your path
DEPENDENCIES

Linux/Windows/ Mac OSX - Dependencies: (Recommended) GLFW GLAD GLM

Linux - Dependencies: (alternative) FreeGLUT GLEW GLM
INSTALLATION
GLFW:

    Install CMake
    Obtain & Extract the GLFW source code from
    https://github.com/glfw/glfw/archive/master.zip
    Compile with below commands
    $ cd glfw-master
    $ mkdir build
    $ cd build
    $ cmake -DBUILD_SHARED_LIBS=ON ..
    $ make && sudo make install

GLAD:

    Go to http://glad.dav1d.de
    Language: C/C++
    Specification: OpenGL
    gl: Version 4.5
    gles1: Version 1.0
    gles2: Version 3.2
    Profile: Core
    Select 'Add All' under extensions and click Generate.
    Download the zip file generated.
    Copy contents of include/ folder in the downloaded directory
    to /usr/local/include/
    src/glad.c should be always compiled along with your OpenGL
    code

GLM:

    Download the zip file from
    https://github.com/g-truc/glm/releases/tag/0.9.7.2.

    Unzip it and copy the folder glm/glm/ to /usr/local/include

    Ubuntu users can also install these libraries using apt-get.

SFML:
			sudo apt-get install libsfml-dev

Execution
To execute the code run the makefile ####

$> make
$> ./sample3D



Controls

    CONTROLS
    1.Press UP and DOWN arrow keys to move the player up and down the grid block.
    2.Press RIGHT and LEFT arrow keys to move the player right and left in the
		 	grid block.
    3.ZoomIN and ZoomOUT in current view by "i" and "o".
    4.Press SPACE or left Arrow of mouse to jump, you can move the player when
			it in in between the air while jumping.
		5.Press key 'l' and 'k' for pan left or pan right.
		6.Mouse Scroll for Zoom in or Zoom out.
		7.Press key 'f' or 's' to change the speed of the player fast or slow.

    To change the view of the game(i.e the looking way).
    1.For Top View: 't'
    2.For Tower View : 'v'
    3.For Helicopter View: 'h'
			key w,s - y direction movement
			key a,d - x direction movement
    4.For Follow View: 'g'
    5.For Adventures View: 'j'
		6.For Continuous Rotation view: 'r'
		7.For Player movement Rotation View: 'b'
		8.For Opposite View: n

Information related to Game

		1.Random static obstacles are being generated.
		2.Randoms pits are being generated.
		3.Some random Cubes are moving up and down.
		4.Total Life is initialized to 4.
		5.Once the number of lives exhausted or the Player Reaches to the last grid
		 	game ends.
		6.Sound implemented.
		7.Water moving, text rendering, image rendering, texture implemented.

Motive

	  1.WIN - Go to opposite corner level changes and win at level 5.
		2.Life - 4 lives implemented.
