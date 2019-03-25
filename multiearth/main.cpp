/*

  Earth!

  all textures courtesy of http://www.shadedrelief.com/natural3/index.html

  adapted by Christopher Rasmussen from the following texturing tutorials:
  * http://www.lighthouse3d.com/tutorials/glsl-tutorial/shader-examples 
  * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube

  2016: updated to OpenGL 3.3
  2014: added multi-texturing, using STL now
  2012: created

*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
//#include <glfw3.h>
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>

#define _USE_MATH_DEFINES

#include <math.h>

using namespace glm;
using namespace std;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// to avoid gimbal lock issues...

#define MAX_LATITUDE_DEGS     89.0
#define MIN_LATITUDE_DEGS    -89.0

#define MIN_ORBIT_CAM_RADIUS    (4.0)
#define MAX_ORBIT_CAM_RADIUS    (25.0)

#define DEFAULT_ORBIT_CAM_RADIUS            8.0
#define DEFAULT_ORBIT_CAM_LATITUDE_DEGS     0.0   
#define DEFAULT_ORBIT_CAM_LONGITUDE_DEGS    0.0   

//----------------------------------------------------------------------------

// some convenient globals 

GLFWwindow* window;

GLuint programID;
GLuint MatrixID;

GLuint SphereVertexArrayID;

// these along with Model matrix make MVP transform

glm::mat4 Projection;
glm::mat4 View;

double orbit_cam_radius = DEFAULT_ORBIT_CAM_RADIUS;
double orbit_cam_delta_radius = 0.1;

double orbit_cam_latitude_degs = DEFAULT_ORBIT_CAM_LATITUDE_DEGS;
double orbit_cam_longitude_degs = DEFAULT_ORBIT_CAM_LONGITUDE_DEGS;
double orbit_cam_delta_theta_degs = 1.0;

int win_scale_factor = 60;
int win_w = win_scale_factor * 10.0;
int win_h = win_scale_factor * 10.0;

GLfloat *sphere_vertex_buffer_data;
GLfloat *sphere_uv_buffer_data;
GLuint sphere_vertexbuffer;
GLuint sphere_uvbuffer;
int sphere_num_vertices;

vector <string> texfilename, texvarname;
vector <GLuint> teximage;
vector <GLuint> texID;

//----------------------------------------------------------------------------

// sphere generation code adapted from here: http://www.cs.cmu.edu/afs/andrew/scs/cs/15-463/2001/pub/src/a3/raymain/sphtri1.c

static int nsides = 20; 

static void output_vertex(int index, int lat, int lon) 
{
  float v[3];
  double la, lo;
  
  la = 2.0 * M_PI * lat / nsides;
  lo = 2.0 * M_PI * lon / nsides;
  // this is unoptimized 
  v[0] = cos(lo)*sin(la);
  v[1] = sin(lo)*sin(la);
  v[2] = cos(la);

  sphere_vertex_buffer_data[3 * index] = v[0];
  sphere_vertex_buffer_data[3 * index + 1] = v[1];
  sphere_vertex_buffer_data[3 * index + 2] = v[2];

  sphere_uv_buffer_data[2 * index] = -1.0 * ((float) lon / (float) nsides);
  sphere_uv_buffer_data[2 * index + 1] = 2.0 * ((float) lat / (float) nsides);
}

//----------------------------------------------------------------------------

void generate_sphere() 
{
  int lat, lon;

  sphere_num_vertices = nsides * (nsides + 1);
  int index = 0;

  glGenVertexArrays(1, &SphereVertexArrayID);
  glBindVertexArray(SphereVertexArrayID);

  sphere_vertex_buffer_data = (GLfloat *) malloc(3 * sphere_num_vertices * sizeof(GLfloat));
  sphere_uv_buffer_data = (GLfloat *) malloc(2 * sphere_num_vertices * sizeof(GLfloat));

  // this is probably doing almost exactly the same thing as gluSphere 
  // meant to be GL_TRIANGLE_STRIP

  for (lat = 0; lat < nsides/2; lat++) 
    for (lon = 0; lon <= nsides; lon++) {
      output_vertex(index++, lat, lon);
      output_vertex(index++, lat+1, lon);
    }

  glGenBuffers(1, &sphere_vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * sphere_num_vertices, sphere_vertex_buffer_data, GL_STATIC_DRAW);

  glGenBuffers(1, &sphere_uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * sphere_num_vertices, sphere_uv_buffer_data, GL_STATIC_DRAW);
}

//----------------------------------------------------------------------------

void draw_sphere()
{    
  glPointSize(5);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);
  glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
  
  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_uvbuffer);
  glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

  // Draw!

  glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere_num_vertices); 

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);

}

//----------------------------------------------------------------------------

// place the camera here

void setup_camera()
{
  // Model matrix : an identity matrix (model will be at the origin)
  //  glm::mat4 Model      = glm::mat4(1.0f);
  // a hack to fix an annoying mismatch between expected and observed orientation of sphere
  glm::mat4 Model      = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  Projection = glm::perspective(50.0f, (float) win_w / (float) win_h, 0.1f, 35.0f);

  double orbit_cam_longitude = glm::radians(orbit_cam_longitude_degs);
  double orbit_cam_latitude = glm::radians(orbit_cam_latitude_degs);

  double orbit_cam_azimuth = glm::radians(180.0 + orbit_cam_longitude_degs);
  double orbit_cam_inclination = glm::radians(90.0 - orbit_cam_latitude_degs);
   
  double x_cam = orbit_cam_radius * sin(orbit_cam_inclination) * cos(orbit_cam_azimuth); 
  double z_cam = orbit_cam_radius * sin(orbit_cam_inclination) * sin(orbit_cam_azimuth); 
  double y_cam = orbit_cam_radius * cos(orbit_cam_inclination); 
  
  View = glm::lookAt(glm::vec3(x_cam, y_cam, z_cam),   // Camera location in World Space
		     glm::vec3(0,0,0),                 // and looks at the origin
		     glm::vec3(0,1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
		     );
  
  // Our ModelViewProjection : multiplication of our 3 matrices
  
  glm::mat4 MVP = Projection * View * Model;
  
  // make this transform available to shaders  
  
  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
  
}

//----------------------------------------------------------------------------

// handle key presses

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // orbit rotate

  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
    orbit_cam_longitude_degs += orbit_cam_delta_theta_degs;
  else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    orbit_cam_longitude_degs -= orbit_cam_delta_theta_degs;
  else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_latitude_degs + orbit_cam_delta_theta_degs <= MAX_LATITUDE_DEGS)
      orbit_cam_latitude_degs += orbit_cam_delta_theta_degs;
  }
  else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_latitude_degs - orbit_cam_delta_theta_degs >= MIN_LATITUDE_DEGS)
      orbit_cam_latitude_degs -= orbit_cam_delta_theta_degs;
  }

  // orbit zoom in/out

  else if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_radius + orbit_cam_delta_radius <= MAX_ORBIT_CAM_RADIUS)
      orbit_cam_radius += orbit_cam_delta_radius;
  }
  else if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (orbit_cam_radius - orbit_cam_delta_radius >= MIN_ORBIT_CAM_RADIUS)
      orbit_cam_radius -= orbit_cam_delta_radius;
  }

  // orbit pose reset

  else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    orbit_cam_radius = DEFAULT_ORBIT_CAM_RADIUS;
    orbit_cam_latitude_degs = DEFAULT_ORBIT_CAM_LATITUDE_DEGS;
    orbit_cam_longitude_degs = DEFAULT_ORBIT_CAM_LONGITUDE_DEGS;
  }

  // draw mode

  else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && action == GLFW_PRESS) {
    int keyval = (int) (key - GLFW_KEY_0);
    GLuint drawloc = glGetUniformLocation(programID, "draw_mode");
    glUniform1i(drawloc, keyval);
  }
}

//----------------------------------------------------------------------------

int main( void )
{
  // Initialise GLFW

  if ( !glfwInit() ) {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      return -1;
    }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  // Open a window and create its OpenGL context

  window = glfwCreateWindow( win_w, win_h, "multiearth", NULL, NULL);
  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  
  // Initialize GLEW

  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }
  
  // Ensure we can capture the escape key being pressed below

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  
  // register all callbacks

  glfwSetKeyCallback(window, key_callback);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
  // Enable depth test

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); 
    
  // Create and compile our GLSL program from the shaders

  programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );
  
  // Get a handle for our "MVP" uniform

  MatrixID = glGetUniformLocation(programID, "MVP");
  printf("MatrixID %i\n", MatrixID);

  GLuint drawloc = glGetUniformLocation(programID, "draw_mode");
  printf("initial drawloc %i\n", drawloc);
  glUniform1i(drawloc, 0);

  // Load the textures 

  texfilename.push_back("big_elevation.bmp"); texvarname.push_back("tex_elevation");
  texfilename.push_back("big_water.bmp");     texvarname.push_back("tex_water");
  texfilename.push_back("big_night.bmp");     texvarname.push_back("tex_night");
  texfilename.push_back("big_clouds.bmp");    texvarname.push_back("tex_clouds");
  texfilename.push_back("big_earth.bmp");     texvarname.push_back("tex_color");

  teximage.resize(texfilename.size());
  texID.resize(texfilename.size());

  for (int i = 0; i < texfilename.size(); i++) {
    
    texID[i] = glGetUniformLocation(programID, texvarname[i].c_str());
    teximage[i] = loadBMP_custom(texfilename[i].c_str());
  }

  // compute sphere vertices

  generate_sphere();

  // drawing loop

  do {
    
    // Clear the screen

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use our shader

    glUseProgram(programID);
    
    // update camera location

    setup_camera();

    // rebind all textures

    for (int i = 0; i < texfilename.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, teximage[i]);
      glUniform1i(texID[i], i);
    }

    // draw object

    draw_sphere();
    
    // Swap buffers

    glfwSwapBuffers(window);
    glfwPollEvents();
    
  } // Check if the ESC key was pressed or the window was closed
  while ( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );
  
  // Cleanup 

  glDeleteBuffers(1, &sphere_vertexbuffer);
  glDeleteBuffers(1, &sphere_uvbuffer);
  glDeleteProgram(programID);
  for (int i = 0; i < texID.size(); i++)
    glDeleteTextures(1, &(texID[i]));

  glDeleteVertexArrays(1, &SphereVertexArrayID);
  
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  
  return 0;
}

