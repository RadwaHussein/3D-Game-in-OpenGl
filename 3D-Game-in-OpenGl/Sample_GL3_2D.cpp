#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <SFML/Audio.hpp>
using namespace std;

#define PI 3.14159

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

sf::SoundBuffer buffer1;
sf::SoundBuffer buffer2;
sf::SoundBuffer buffer3;
sf::SoundBuffer buffer4;
sf::SoundBuffer buffer5;
sf::SoundBuffer buffer6;
sf::SoundBuffer buffer7;
sf::Sound sound1;
sf::Sound sound2;
sf::Sound sound3;
sf::Sound sound4;
sf::Sound sound5;
sf::Sound sound6;
sf::Sound sound7;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  1,                  // attribute 1. Color
						  3,                  // size (r,g,b)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}


/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = -1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int j;
float j2=1,j4=-2,j5=2.2,j6=-2;
int m=15;
int num[15],num1[10];
float t=0,ct1=0;
bool flagjump= false;
float hx,hz,ox,oz,hz1,hx1;
float i1=-1,i2=3,i3=3,i4=0,i5=0,i6=0,i7=0,i8=1,i9=0;
glm::mat4 rotateRectangle;
float u_xn = -10.0,u_xp = 10.0,u_yn = -10.0,u_yp = 10.0;
double xpos,ypos,yoffset,xoffset,yoffset1;
float wy=-2,obsy=0.0;
int flagwater=0,cflag1=0,cflag2=0,cflag=1,flagobs=0,flagobs1=0;
GLfloat cameraSpeed = 0.05f;
glm::vec3 cameraPos;
glm::vec3 cameraFront;
glm::vec3 cameraUp;
float h1=-1,h2=3,h3=3;
int score=0;
float hj;
int levelflag=1;
int textflag=1;
int speedflag=4;
int life=4;
bool camfollow=false;
//char cflag='v';

glm::vec3 eye (-1, 3, 3);
glm::vec3 target (0, 0, 0);
glm::vec3 up (0, 1, 0);
    

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

void updateProjection()
{
 Matrices.projection = glm::ortho(u_xn, u_xp, u_yn, u_yp, 0.1f, 500.0f);
}

void zoomin()
{
    u_xn *= 0.8;
    u_xp *= 0.8;
    u_yn *= 0.8;
    u_yp *= 0.8;
    updateProjection();
}

void zoomout()
{
    u_xn *= 1.25;
    u_xp *= 1.25;
    u_yn *= 1.25;
    u_yp *= 1.25;
    updateProjection();
}

void panleft()
{
    if(u_xn!=-10 && u_xp!=10 )
    {
        u_xn -= 2.0;
        u_xp -= 2.0;
        //u_yn += 1.5;
        //u_yp -= 1.5;
        updateProjection();
    }

}


void panright()
{
    if(u_xp!=-10 && u_xn!=10)
    {
        u_xn += 2.0;
        u_xp += 2.0;
 //   u_yn -= 1.5;
   // u_yp += 1.5;
    updateProjection();
    }
}



void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE /*|| action == GLFW_REPEAT*/) {
		switch (key) {
			//case GLFW_KEY_C:
			//	rectangle_rot_status = !rectangle_rot_status;
			//	break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
            case GLFW_KEY_LEFT:
                 sound3.play();
                 j=2;
                 break;
            case GLFW_KEY_RIGHT:
                 sound3.play();
                 j=1;       
                 break;

            case GLFW_KEY_UP:
                 sound3.play();   
                 j=3;
                 break;
            case GLFW_KEY_DOWN:
                 sound3.play();
                 j=4;       
                 break;  
            case GLFW_KEY_SPACE:
                 j=5;
                 sound2.play();
                 break;
            case GLFW_KEY_C:
                j2=j2-1;
                break;

            case GLFW_KEY_F:
                j2=j2+1;
                break;   
            case GLFW_KEY_T:
                cflag=2;
                //rotateRectangle = -rotateRectangle;
                break;
            case GLFW_KEY_G:
                cflag=3;
                break;    
           case GLFW_KEY_J:
                cflag=4;
                break;
            case GLFW_KEY_V:
                cflag=1;
            
            case GLFW_KEY_H:
                cflag=5;   
                //rotateRectangle = -rotateRectangle;
                break;
            case GLFW_KEY_A:       
            if(cflag==5)
            {
                hj=2;
            }    
            break;

            case GLFW_KEY_D:
               if(cflag==5)
              {
                hj=1;

             }
              break;
          

            case GLFW_KEY_W:
             if(cflag==5) 
                 {
                 hj=3;
                 
                }
                break;
            case GLFW_KEY_S:
                if(cflag==5)
             {
                hj=4;
             }    
                 break;  

  /*          case GLFW_KEY_M:
                 camfollow=true;
*/
            case GLFW_KEY_R:
                rotateRectangle = glm::rotate((float)(90.0f*M_PI/180.0f), glm::vec3(1,1,1));
                break;

            case GLFW_KEY_O:
                zoomout();
                break; 

            case GLFW_KEY_I:
                zoomin();
                break;   

            case GLFW_KEY_L:
                panleft();
                break; 

            case GLFW_KEY_K:
                panright();
                break;  
            case GLFW_KEY_B:
                cflag1=1;
                break;
            case GLFW_KEY_N:
                cflag2=1;
                break;
           /* case GLFW_KEY_H:
            cflag1=3; 
*/
            case GLFW_KEY_Z:
                cflag1=4;   
         
   // case GLFW_KEY_A:
     //   cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            case GLFW_KEY_M:
             cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; 



			default:
				break;
		}
	}

    else if(action==GLFW_REPEAT || action==GLFW_RELEASE)
    {
        switch (key){
            case GLFW_KEY_H:
            cflag1=3;
        }
    }
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset1>yoffset)
    {
    zoomin();
    }
    else if(yoffset1<yoffset)
    {

    zoomout();
    //power=power+5;
    }
    yoffset1=yoffset;


}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				triangle_rot_dir *= -1;
                sound2.play();
                j=5;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	 is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	 glLoadIdentity ();
	 gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-14.0f, 14.0f, -14.0f, 14.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *cube[100], *player, *obst[10];

// Creates the triangle object used in this sample code


void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle (GLuint textureID)
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-10,-10,0, // vertex 1
		10,-10,0, // vertex 2
		10, 10,0, // vertex 3

		10, 10,0, // vertex 3
		-10, 10,0, // vertex 4
		-10,-10,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	static const GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DTexturedObject( GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}


void createCube(GLuint textureID1, int n)
{
  const GLfloat vertex_buffer_data [] = {
     -0.2f,-2.0f,-0.2f, // triangle 1 : begin

     -0.2f,-2.0f, -0.2f,

     -0.2f, 2.0f, 0.2f, // triangle 1 : end

     0.2f, 2.0f,-0.2f, // triangle 2 : begin

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f,-0.2f, // triangle 2 : end

     0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f,-0.2f,

     0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f, 0.2f,

     -0.2f,-2.0f,-0.2f,

     -0.2f, 2.0f, 0.2f,

     -0.2f,-2.0f, 0.2f,

     0.2f,-2.0f, 0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f,-0.2f,

     0.2f,-2.0f,-0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f,-2.0f, 0.2f,

     0.2f, 2.0f, 0.2f,

     0.2f, 2.0f,-0.2f,

     -0.2f, 2.0f,-0.2f,

     0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f,-0.2f,

     -0.2f, 2.0f,0.2f,

     0.2f, 2.0f, 0.2f,

     -0.2f, 2.0f, 0.2f,

     0.2f,-2.0f, 0.2f
  };

 static const GLfloat texture_buffer_data[] = { 
        0.000059f, 1.0f-0.000004f, 
        0.000103f, 1.0f-0.336048f, 
        0.335973f, 1.0f-0.335903f, 
        1.000023f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.336024f, 1.0f-0.671877f, 
        0.667969f, 1.0f-0.671889f, 
        1.000023f, 1.0f-0.000013f, 
        0.668104f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.000059f, 1.0f-0.000004f, 
        0.335973f, 1.0f-0.335903f, 
        0.336098f, 1.0f-0.000071f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.336024f, 1.0f-0.671877f, 
        1.000004f, 1.0f-0.671847f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.668104f, 1.0f-0.000013f, 
        0.335973f, 1.0f-0.335903f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.668104f, 1.0f-0.000013f, 
        0.336098f, 1.0f-0.000071f, 
        0.000103f, 1.0f-0.336048f, 
        0.000004f, 1.0f-0.671870f, 
        0.336024f, 1.0f-0.671877f, 
        0.000103f, 1.0f-0.336048f, 
        0.336024f, 1.0f-0.671877f, 
        0.335973f, 1.0f-0.335903f, 
        0.667969f, 1.0f-0.671889f, 
        1.000004f, 1.0f-0.671847f, 
        0.667979f, 1.0f-0.335851f
    };
  // create3DObject creates and returns a handle to a VAO that can be used later
   cube[n]= create3DTexturedObject(GL_TRIANGLES, 36, vertex_buffer_data, texture_buffer_data, textureID1, GL_FILL);

}


void createPlayer(GLuint textureID2)
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.1f,-0.2f,-0.1f, // triangle 1 : begin

     -0.1f,-0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f, // triangle 1 : end

     0.1f, 0.2f,-0.1f, // triangle 2 : begin

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f, // triangle 2 : end

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f,0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     -0.1f,-0.2f,-0.1f,

     -0.1f, 0.2f, 0.1f,

     -0.1f,-0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f,-0.1f,

     0.1f,-0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f,

     0.1f, 0.2f, 0.1f,

     0.1f, 0.2f,-0.1f,

     -0.1f, 0.2f,-0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f,-0.1f,

     -0.1f, 0.2, 0.1f,

     0.1f, 0.2f, 0.1f,

     -0.1f, 0.2f, 0.1f,

     0.1f,-0.2f, 0.1f
    

  };
 static const GLfloat texture_buffer_data[] = { 
        0.000059f, 1.0f-0.000004f, 
        0.000103f, 1.0f-0.336048f, 
        0.335973f, 1.0f-0.335903f, 
        1.000023f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.336024f, 1.0f-0.671877f, 
        0.667969f, 1.0f-0.671889f, 
        1.000023f, 1.0f-0.000013f, 
        0.668104f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.000059f, 1.0f-0.000004f, 
        0.335973f, 1.0f-0.335903f, 
        0.336098f, 1.0f-0.000071f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.336024f, 1.0f-0.671877f, 
        1.000004f, 1.0f-0.671847f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.668104f, 1.0f-0.000013f, 
        0.335973f, 1.0f-0.335903f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.668104f, 1.0f-0.000013f, 
        0.336098f, 1.0f-0.000071f, 
        0.000103f, 1.0f-0.336048f, 
        0.000004f, 1.0f-0.671870f, 
        0.336024f, 1.0f-0.671877f, 
        0.000103f, 1.0f-0.336048f, 
        0.336024f, 1.0f-0.671877f, 
        0.335973f, 1.0f-0.335903f, 
        0.667969f, 1.0f-0.671889f, 
        1.000004f, 1.0f-0.671847f, 
        0.667979f, 1.0f-0.335851f
    };
  // create3DObject creates and returns a handle to a VAO that can be used later
player= create3DTexturedObject(GL_TRIANGLES, 36, vertex_buffer_data, texture_buffer_data, textureID2, GL_FILL);
  // create3DObject creates and returns a handle to a VAO that can be used later
}



int hole(int h)
{
    
    hx=-2+(((h%10)-1)*0.4);
    hz= -2 + (h/10)*0.4;
    hx1=hx;
    hz1=hz;
   /* if(h%10==0)
    {
    cout<<h<<"\n";
    cout << hx <<"\t" << hz<<"\n";
}*/
}
int obstacle(int o)
{
    ox=-2+(((o%10)-1)*0.4);
    oz= -2 + (o/10)*0.4;

}
void check1 ()
{
                                       
  if(j4<-2 || j6<-2 || j4>1.6 || j6>1.6)
  {
    j4=-2;
    j6=-2;
    j=0;
    score++;
    life--;
    sound6.play();
  } 
for(int c=0;c<15;c++)
{
    hole(num[c]);
    if(j4>hx-0.2 && j4<hx+0.2 && j6>hz-0.2 && j6<hz+0.2  )
    {
        j4=-2;
        j6=-2;
        j=0;
        life--; 
       score++;
       sound6.play();
    }
 
}
for(int c=0;c<10;c++)
{
    obstacle(num1[c]);
     if(j4>ox-0.2 && j4<ox+0.2 && j6>oz-0.2 && j6<oz+0.2 && j5<=3 && j5>obsy+3 && flagobs1==0)
    {
        j4=-2;
        j6=-2;
        j=0;
        life--;
        score++;
        sound7.play();
        flagobs1=1;
    }

}
if(j6>1.4 && j4>1.4 )
{
    cout<<"You win \n";
  //  if(levelflag==4)
    {
    levelflag++;
    cout<<" Second level \n";
    
    }
     j4=-2;
        j6=-2;
        j=0;
        sound5.play();
    
}
}
void check2 ()
{
                                       
  if(j4<-2 || j6<-2 || j4>1.6 || j6>1.6)
  {
    j4=-2;
    j6=-2;
    j=0;
    score++;
    life--;
    sound6.play();
  } 
for(int c=0;c<15;c++)
{
    hole(num[c]);
    if(j4>hx-0.2 && j4<hx+0.2 && j6>hz-0.2 && j6<hz+0.2  )
    {
        j4=-2;
        j6=-2;
        j=0;
        life--; 
       score++;
       sound6.play();
    }

 
}

for(int c=0;c<10;c++)
{
    obstacle(num1[c]);
         if(j4>ox-0.2 && j4<ox+0.2 && j6>oz-0.2 && j6<oz+0.2 && j5<=3 && j5>obsy+3 && flagobs1==0)
    {
        j4=-2;
        j6=-2;
        j=0;
        life--;
        score++;
        sound7.play();
        flagobs1=1;
    }
    

}
if(j6>1.4 && j4>1.4 )
{
    cout<<"You win \n";

    levelflag++;
    cout<<"Nextlevel \n";
     j4=-2;
        j6=-2;
        j=0;
    sound5.play();
    
    
}
}
void check3 ()
{
                                       
  if(j4<-2 || j6<-2 || j4>1.6 || j6>1.6)
  {
    j4=-2;
    j6=-2;
    j=0;
    score++;
    life--;
    sound6.play();
  } 
for(int c=0;c<15;c++)
{
    hole(num[c]);
    if(j4>hx-0.2 && j4<hx+0.2 && j6>hz-0.2 && j6<hz+0.2  )
    {
        j4=-2;
        j6=-2;
        j=0;
        life--; 
       score++;
       sound6.play();
    }
 
}
for(int c=0;c<10;c++)
{
    obstacle(num1[c]);
        if(j4>ox-0.2 && j4<ox+0.2 && j6>oz-0.2 && j6<oz+0.2 && j5<=3 && j5>obsy+3 && flagobs1==0)
    {
        j4=-2;
        j6=-2;
        j=0;
        life--;
        score++;
        sound7.play();
        flagobs1=1;
    }
    /*else if(j5==1)
    {

    }*/

}
if(j6>1.4 && j4>1.4 )
{
    cout<<"You win \n";
   
    {
    levelflag++;
    cout<<"Nextlevel \n";
    
    }
     j4=-2;
        j6=-2;
        j=0;
        sound5.play();
    
}
}

void check4 ()
{
                                       
  if(j4<-2 || j6<-2 || j4>1.6 || j6>1.6)
  {
    j4=-2;
    j6=-2;
    j=0;
    score++;
    life--;
    sound6.play();
  } 
for(int c=0;c<15;c++)
{
    hole(num[c]);
    if(j4>hx-0.2 && j4<hx+0.2 && j6>hz-0.2 && j6<hz+0.2  )
    {
        j4=-2;
        j6=-2;
        j=0;
        life--; 
       score++;
       sound6.play();
    }
 
}
for(int c=0;c<10;c++)
{
    obstacle(num1[c]);
     if(j4>ox-0.2 && j4<ox+0.2 && j6>oz-0.2 && j6<oz+0.2 && j5<=3 && j5>obsy+3 && flagobs1==0)
    {
        j4=-2;
        j6=-2;
        j=0;
        life--;
        score++;
        sound7.play();
        flagobs1=1;
    }
    /*else if(j5==1)
    {

    }*/

}
if(j6>1.4 && j4>1.4 )
{
    cout<<"You win \n";
    //if(levelflag==4)
    {
    levelflag++;
    cout<<"Nextlevel \n";
    
    }
     j4=-2;
        j6=-2;
        j=0;
        sound5.play();
    
}
}
void check5 ()
{
                                       
  if(j4<-2 || j6<-2 || j4>1.6 || j6>1.6)
  {
    j4=-2;
    j6=-2;
    j=0;
    score++;
    life--;
    sound6.play();
  } 
for(int c=0;c<15;c++)
{
    hole(num[c]);
    if(j4>hx-0.2 && j4<hx+0.2 && j6>hz-0.2 && j6<hz+0.2  )
    {
        j4=-2;
        j6=-2;
        j=0;
        life--; 
       score++;
       sound6.play();
    }
 
}
for(int c=0;c<10;c++)
{
    obstacle(num1[c]);
     if(j4>ox-0.2 && j4<ox+0.2 && j6>oz-0.2 && j6<oz+0.2 && j5<=3 && j5>obsy+3 && flagobs1==0)
    {
        j4=-2;
        j6=-2;
        j=0;
        life--;
        score++;
        sound7.play();
        flagobs1=1;
    }
    /*else if(j5==1)
    {

    }*/

}
if(j6>1.4 && j4>1.4 )
{
    cout<<"You win \n";
  //  if(levelflag==4)
    {
    levelflag++;
    sound4.play();
   // cout<<"Nextlevel \n";
     j4=-2;
        j6=-2;
        j=0;
    
    }
    
}
}





float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	static float c = 0;
	//c++;
	//Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(sinf(c*M_PI/180.0),3*cosf(c*M_PI/180.0),0)); // Fixed camera for 2D (ortho) in XY plane
    if(cflag==1)
    {
        //tower view
         i1=-1,i2=3,i3=3,i4=0,i5=0,i6=0,i7=0,i8=1,i9=0;
          Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    }
    else if(cflag==2)
    {
        //top view
        i1=0,i2=20,i3=1,i4=0,i5=0,i6=0,i7=0,i8=1,i9=0;
         Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    }
    else if(cflag==3)
    {
        i1=j4;
        i2=8;
        i3=j6;
        i4=j4+2;
        i5=2;
        i6=j6-2;
        Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    }
    else if(cflag==4)
    {
        i1=j4+1;
        i2=j5+4;
        i3=j6-0.6;
        i4=j4+1;
        i5=j5;
        i6=j6-1;
        Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    
    }
    else if(cflag==5)
    {
        if(hj==1)
        {
            i1=i1+0.5;

        }
        else if(hj==2)
        {
            i1=i1-0.5;
        }
        else if(hj==3)
        {
            i2=i2-0.5;
        }
        else if(hj==4)
        {
            i2=i2+0.5;
        }

        Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));

    }
    if(cflag1==1)
    {
        GLfloat radius = 2.0f;
    GLfloat camX = sin(ct1) * radius;
    GLfloat camZ = cos(ct1) * radius;
    glm::mat4 view;
    Matrices.view = glm::lookAt(glm::vec3(camX, 1.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
ct1=ct1+0.002;

    }
    else if(cflag2==1)
    {
        i1=j4;
        i2=j5;
        i3=j6;
        /*  i4=j4+2;
        j5= i2;
        i6=j6+2;*/
        Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));

    }
    else if(cflag1==3)
    {

        h1=h1+0.2;
        //h3=h3+0.5;
        Matrices.view = glm::lookAt(glm::vec3(h1,h2,h3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));

    }
    /*else if(cflag1==4)
    {
        Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    }*/
    else
    {
    Matrices.view = glm::lookAt(glm::vec3(i1,i2,i3), glm::vec3(i4,i5,i6), glm::vec3(i7,i8,i9));
    cflag1=0;
    cflag2=0;
    }
	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */
	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform;
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	// Copy MVP to normal shaders
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);



	// Render with texture shaders now
	glUseProgram(textureProgramID);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();

if(wy<1 && flagwater==0)
    {
    //cout << "hekko"<<"\n";
    wy=wy+0.01;

    }
    else if(wy>=1)
    { 
    flagwater=1;
    wy=wy-0.01;
    }
    else if(flagwater==1 && wy>-3)
    {
    wy=wy-0.01;
    }
    else if(flagwater==1 && wy<=-3)
    {
    //cout << "hekko"<<"\n";   
    wy=wy+0.01;
    flagwater=0;
    }        



	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(0, wy, -4));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;

	// Copy MVP to texture shaders
	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DTexturedObject(rectangle);

int i;
float xc=-2;
float zc=-2;
int q=0;

for(int c=0;c<m;c++)
{
    if(num[c]==1)
    {
        num[c]=23;
    }
    else if(num1[c]==1)
    {
        num1[c]=55;
    }
}

for(i=0;i<100;i++)
{
	Matrices.model = glm::mat4(1.0f);
	if(i!=0 && i%10==0)
    {
        xc=-2;
        zc=zc+0.4;
    }		  
    q++;
  glm::mat4 translateCube ;
  if(q==num1[0]|| q==num1[1]||q==num1[2]||q==num1[3]||q==num1[4]||q==num1[5]||q==num1[6]||q==num1[7]||q==num1[8] || q==num1[9] )
  {
    if(obsy<1 && flagobs==0)
    {
    //cout << "hekko"<<"\n";
    obsy=obsy+0.005;

    }
    else if(obsy>=1)
    { 
    flagobs=1;
    obsy=obsy-0.005;
    }
    else if(flagobs==1 && obsy>-3)
    {
    obsy=obsy-0.005;
    }
    else if(flagobs==1 && obsy<=-3)
    {
    //cout << "hekko"<<"\n";   
    obsy=obsy+0.005;
    flagobs=0;
    }        

   translateCube = glm::translate (glm::vec3(1.f*xc, obsy,1.f*zc));

  }
  else
    translateCube = glm::translate (glm::vec3(1.f*xc, 0.0f,1.f*zc));

  Matrices.model *=(translateCube * rotateRectangle) ; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

  // Set the texture sampler to access Texture0 memory
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

    // draw3DObject draws the VAO given to it using current MVP matrix
    //for(int c=0;c<m;c++)
    {
       // cout<<num[c]<<"\n";
    if(q!=num[0]&&q!=num[1]&&q!=num[2]&&q!=num[3]&&q!=num[4]&&q!=num[5]&&q!=num[6]&&q!=num[7]&&q!=num[8]&&q!=num[9]&&q!=num[10]&&q!=num[11]&&q!=num[12]&&q!=num[13]&&q!=num[14])
    {//cout<<num[c]<<"\n";
    draw3DTexturedObject(cube[i]);
    }
    /*if(q==num1[0]&& q==num1[1]&&q==num1[2]&&q==num1[3]&&q==num1[4]&&q==num1[5]&&q==num1[6]&&q==num1[7]&&q==num1[8] && q==num1[9] )
    {
        translateCube = glm::translate (glm::vec3(1.f*xc, 2.0f,1.f*zc));
        Matrices.model *=(translateCube * rotateRectangle) ; 
         MVP = VP * Matrices.model;
         glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
         glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
         draw3DTexturedObject(cube[i]);
    */
    
}
    xc=xc+0.4;


}

Matrices.model = glm::mat4(1.0f);
 glm::mat4 translatePlayer = glm::translate (glm::vec3(j4, j5, j6));        // glTranslatef
    //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatePlayer * rotateRectangle);
    MVP = VP * Matrices.model;

    // Copy MVP to texture shaders
    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Set the texture sampler to access Texture0 memory
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

    // draw3DObject draws the VAO given to it using current MVP matrix


 if(obsy<1 && flagobs==0 && flagobs1==1)
    {
    //cout << "hekko"<<"\n";
    obsy=obsy+0.005;
    glm::mat4 translatePlayer = glm::translate (glm::vec3(j4, obsy, j6)); 

    }
    else if(obsy>=1 && flagobs1==1)
    { 
    flagobs=1;
    obsy=obsy-0.005;
        glm::mat4 translatePlayer = glm::translate (glm::vec3(j4, obsy, j6)); 

    }
    else if(flagobs==1 && obsy>-3 && flagobs1==1)
    {
    obsy=obsy-0.005;
        glm::mat4 translatePlayer = glm::translate (glm::vec3(j4, obsy, j6)); 

    }
    else if(flagobs==1 && obsy<=-3 && flagobs1==1)
    {
    //cout << "hekko"<<"\n";   
    obsy=obsy+0.005;
    flagobs=0;
        glm::mat4 translatePlayer = glm::translate (glm::vec3(j4, obsy, j6)); 

    }        

  /*   if(flagobs1==1)
            translateRectangle = glm::translate (glm::vec3(j4, obsy, j6));

*/

    if(levelflag==1)
    {
    
    check1();
}
    draw3DTexturedObject(player);


    int k;
for(k=0;k<j2 && j!=0;k++)   
    {

    Matrices.model = glm::mat4(1.0f);
    if(j==1)
    {
        if(flagjump==true)
            j4=j4+0.8;
        else
      j4=j4+0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==2)
    {
        if(flagjump==true)
            j4=j4-0.8;
        else
      j4=j4-0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==3)
    {
        if(flagjump==true)
            j6=j6-0.8;
        else
      j6=j6-0.4;
translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }
    else if(j==4)
    {
        if(flagjump==true)
            j6=j6+0.8;
        else
      j6=j6+0.4;

translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
    }

    else if(j==5 && j5>2.2)
    {
        flagjump=true;
        
    }
  //  glm::mat4 translatePlayer = glm::translate (glm::vec3(-2, 2.2, -1));        // glTranslatef
    //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatePlayer * rotateRectangle );
    MVP = VP * Matrices.model;

    // Copy MVP to texture shaders
    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Set the texture sampler to access Texture0 memory
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DTexturedObject(player);
}
if(flagjump==true)
{
    if(j5>2.2)
    {
    float uy=(5*sin(90*(PI/180)));
        j5=  2.2 + (uy*t-5*t*t);
        t=t+0.1;
        translateRectangle = glm::translate (glm::vec3(j4, j5, j6));
}

else if(j4==ox && j6==oz)
{
    translateRectangle = glm::translate (glm::vec3(j4, 3.2f, j6));
}

else 
{
    t=0;
    j5=2.2;
    flagjump=false;
}
}

j=0;
/*Matrices.model = glm::mat4(1.0f);
//glm::mat4 translatePlayer = glm::translate (glm::vec3(-2, 2.2, -1));        // glTranslatef
    //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatePlayer );
    MVP = VP * Matrices.model;

    // Copy MVP to texture shaders
    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Set the texture sampler to access Texture0 memory
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DTexturedObject(player);*/

	// Increment angles
	float increments = 1;

	// Render font on screen
	static int fontScale = 0;
	float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
	glm::vec3 fontColor = getRGBfromHue (fontScale);

	// Use font Shaders for next part of code
	glUseProgram(fontProgramID);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    
	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(-6,4,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
    /*character time_string[2]
    time_c=timer;
    int r;
    for(i=0;i<2;i++){
    r=time_c%10;
    time_c/=10;
    time_string[1-i]= (char)(r + 48);
    }*/
	// Render font
    if(life==4)
    {
	GL3Font.font->Render("     lives: 4");
}
else if(life==3)
    {
    GL3Font.font->Render("     lives: 3");
}
else if(life==2)
    {
    GL3Font.font->Render("     lives: 2");
}
else if(life==1)
{
     GL3Font.font->Render("    lives: 1");
}
else
{
            textflag=(100*(levelflag-1))-(5*score);
            if(textflag<0)
            {
                textflag=0;
            }
            cout<<"Score " << textflag<<"\n";  
   // cout<<"Score " << (100*(levelflag-1))-(5*score)<<"\n";
    exit(0);
}
    if(levelflag==1)
    {
        GL3Font.font->Render("                   level: 1");
    }

    else if(levelflag==2)
        {
            GL3Font.font->Render("                   level: 2");
            check2();
            speedflag=3;
        }
        else if(levelflag==3)
        {
            GL3Font.font->Render("                 level: 3");
            
            check3();
            speedflag=2;
        }
        else if(levelflag==4)
        {
            GL3Font.font->Render("                 level: 4");
            check4();
            speedflag=1;
        }
        else if(levelflag==5)
        {
            GL3Font.font->Render("                   level: 5");
            check5();
            textflag=(100*levelflag)-(5*score);
            if(textflag<0)
            {
                textflag=0;
            }
            cout<<"Score " << textflag<<"\n";  
            exit(0);
        }

	//camera_rotation_angle++; // Simulating camera rotation
	//triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	// font size and color changes
	//fontScale = (fontScale + 1) % 360;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	 is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	// Load Textures
	// Enable Texture0 as current texture memory
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	GLuint textureID = createTexture("bg2.jpg");
    GLuint textureID1= createTexture("cube2.jpg");
    GLuint textureID2= createTexture("cube.bmp");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle (textureID);
    createPlayer(textureID2);
    int i;
    for(i=0;i<100;i++)
	createCube(textureID1,i);


	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 1.0f, 0.2f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise FTGL stuff
	const char* fontfile = "arial.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

}

int main (int argc, char** argv)
{
	int width = 1800;
	int height = 1000;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;
    for(int c=0;c<m;c++)
            {
                num[c]=rand()%98+0;
            }

    if(!buffer1.loadFromFile("my.wav"))
    return -1;
    sound1.setBuffer(buffer1);   
     if(!buffer2.loadFromFile("jump.wav"))
    return -1;
    sound2.setBuffer(buffer2);    
    if(!buffer3.loadFromFile("walk.wav"))
    return -1;
    sound3.setBuffer(buffer3);   
    if(!buffer4.loadFromFile("final.wav"))
    return -1;
    sound4.setBuffer(buffer4);
    if(!buffer5.loadFromFile("lvl.wav"))
    return -1;
    sound5.setBuffer(buffer5);
    if(!buffer6.loadFromFile("hole.wav"))
    return -1;
    sound6.setBuffer(buffer6);
    if(!buffer7.loadFromFile("obstac.wav"))
    return -1;
    sound7.setBuffer(buffer7);
/*
    glfwGetCursorPos(window, &xpos, &ypos);
    double xpos1=xpos;
    double ypos1=ypos;  */      
       
         glfwGetCursorPos(window, &xpos, &ypos);

    double xpos1=xpos;
    double ypos1=ypos;  

	/* Draw in loop */
    sound1.play();

 
	while (!glfwWindowShouldClose(window)) {
      //  if(cflag==1)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
    glfwGetCursorPos(window, &xpos, &ypos);
    if( ypos>ypos1)
    {
     i1=i1+1;   


    }
    else if(ypos<ypos1 )
    {
       i1=i1-1;
    }
  

    xpos1=xpos;
    ypos1=ypos;
}

        glfwSetScrollCallback(window, scroll_callback);
		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

             glfwGetCursorPos(window, &xpos, &ypos);

    double xpos1=xpos;
    double ypos1=ypos; 

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= speedflag) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
            for(int c=0;c<m;c++)
            {
                num[c]=rand()%98+0;
                hole(num[c]);
                if(num[c]%10==0)
                {
                    num[c]=98;
                }
                else if(hx1==j4 && hz1==j6)
                {
                    num[c]=84;
                }
            }
            for(int c=0;c<10;c++)
            {
               
                num1[c]=rand()%98+0;
                if(num1[c]%10==0)
                {
                    num1[c]=98;
                }
                else if(hx1==j4 && hz1==j6)
                {
                    num1[c]=84;
                }
            
            }

			last_update_time = current_time;
		}
	}


	glfwTerminate();
	exit(EXIT_SUCCESS);
}
