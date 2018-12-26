#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glu.h>
#include <glfw3.h>
#include "Shader.hpp"
#include "Functions.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;

/* Globala variabler för shaders och (V/I)BOs */
GLuint program;
GLint attribute_coord2d;
GLuint vbo_triangle;
GLuint vbo_tex;
GLuint theta_buffer;
GLuint cave_vertex_buffer;
GLuint cave_index_buffer;
GLuint cave_normal_buffer;

GLFWwindow* window;

float M_PI = 3.14f;

/* Globala variabler för bl a fps och kamera */
float r = 10.0f;
int frames = 0;
double time_elapsed = 0.0f;
double time_last = 0.0f;
int segs = 150;
float oct_seeds = 0.0;
bool step = true;
bool start = true;
float x_offset = 0.0f;
float y_offset = 0.0f;
float z_offset = 1.0f;
float sin_coord = 0.0f;
float x_position = 15.0f;
float y_position = -6.0f;
float z_position = 0.0f;
float scrot_time = 0.0f;

float x_position_moveto = 15.0f;
float y_position_moveto = 0.0f;
float z_position_moveto = 0.0f;

float viewer_fi = M_PI / 2.0;
float viewer_theta = M_PI/2.0;
float z_walk = 0.0f;
float z_walk_texture = 0.0f;

/* Fixa kameran, delvis tagen från något stegu-exempel */
void setupCamera(Shader& sh) 
{

    int width, height;
    
    // Get window size. It may start out different from the requested
    // size, and will change if the user resizes the window.
    glfwGetWindowSize(window, &width, &height );
    if(height<=0) height=1; // Safeguard against iconified/closed window

    // Set viewport. This is the pixel rectangle we want to draw into.
    glViewport( 0, 0, width, height ); // The entire window

        glm::mat4 modelview_matrix;       
    if(y_position > -5.99999 && start)
		modelview_matrix = glm::lookAt(glm::vec3(x_position, y_position, z_position), glm::vec3(15.0 + x_offset * 5.0, -6.0 + y_offset * 5.0, 0.0 + z_offset * 5.0), glm::vec3(0.0, 1.0, 0.0));
	else
	{
		start = false;
		modelview_matrix = glm::lookAt(glm::vec3(x_position, y_position, z_position), glm::vec3(x_position + x_offset, y_position + y_offset, z_position + z_offset), glm::vec3(0.0, 1.0, 0.0));
	}
    glm::mat4 projection_matrix = glm::perspective(45.0f, 1.0f*float(width)/float(height), 0.1f, 100.0f);
    GLint uniform_modelview;
    GLint uniform_projection;
    uniform_modelview = glGetUniformLocation(sh.getProgram(), "modelview_matrix");
    uniform_projection = glGetUniformLocation(sh.getProgram(), "projection_matrix");
    
    glUniformMatrix4fv(uniform_modelview, 1, GL_FALSE, glm::value_ptr(modelview_matrix));
    glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
}

/* Skriv ut fps i terminalen */
void printFps()
{
	if(time_last == 0.0f)
	{
		time_last = glfwGetTime();
		return;
	}
	
	double delta_time = glfwGetTime() - time_last;
	time_last = glfwGetTime();
	time_elapsed += delta_time;
	frames++;
	if(time_elapsed >= 1.0f)
	{
		printf("FPS: %f\n", float(frames) / float(time_elapsed));
		time_elapsed = 0.0f;
		frames = 0;
	}
}

/* Ta en screenshot, slipper screen tearing */
void print_image(const char* filename, int width, int height)
{
	unsigned char* pixels = new unsigned char[width * height * 4];
	
	glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
	int r,g,b;
	FILE *f;
	if(!(f = fopen(filename, "w")))
		return;
	fprintf(f, "P3\n%d %d\n%d\n", width, height, 255); 
	
	/* Loopa igenom alla pixlar */
	for (unsigned short i=height - 1; i >= 1; i--) 
	{
		for(unsigned short j = 0; j < width; j++)
		{
			//float weight
			r = pixels[i * width * 4 + j * 4 + 0];
			g = pixels[i * width * 4 + j * 4 + 1];
			b = pixels[i * width * 4 + j * 4 + 2];
			
			/* Skriv subpixlarna till filen */
			fprintf(f,"%d %d %d ", r, g, b);
		}
	}
	
	/* Stäng filen */
	fclose(f);
	
	delete [] pixels;
}




int main(int argc, char** argv)
{
	/* Starta GLFW och GLEW */
	
	glfwInit();
	window = glfwCreateWindow(1024, 600, "My Title", NULL, NULL);
	if( !window)
    {
		std::cout << "EXITING" << std::endl;
		glfwTerminate();
		return 0;
    }
    
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwMakeContextCurrent(window);
	
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) 
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}
	
	/* Kompilera shadern som gör elden */
	Shader sh;
	sh.createShader("vertex.glsl", "fragment.glsl");
	sh.use();
	
	//Shader sh2;
	//sh2.createShader("vertex_cave.glsl", "fragment_cave.glsl");

	glEnable(GL_BLEND);
	glEnable( GL_DEPTH_TEST );
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//int n = 10;
	float g = 16.0/9.0;
	GLfloat triangle_vertices1[] = 
	{
		 -g,  g, 1,
		g,  g, 1,
		g, -g - 0.3, 1,
		-g, -g - 0.3, 1,
	};
	
	GLfloat vertex_texcoords[] = 
	{
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0,
	};
	
	
	glGenBuffers(1, &vbo_triangle);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, triangle_vertices1, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &vbo_tex);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertex_texcoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	bool running = true;
	
	
	
	if(!sh.createUniformFloat("time", glfwGetTime()))
		printf("Directno");
	sh.createUniformFloat("octaves", 2.0);
	
	GLint attribute_coords = glGetAttribLocation(sh.getProgram(), "vertex");
	GLint tex_coords = glGetAttribLocation(sh.getProgram(), "texcoords");
	
	
	
	float octaves = 2.0;
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	
	//glEnableVertexAttribArray(attribute_coords);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	
	glVertexAttribPointer(
		attribute_coords, // attribute
		3,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	
	
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	GLfloat* cave_vertices;
	GLfloat* cave_normals;
	GLfloat* theta_coords;
	GLushort* cave_indices;
	
	//cout << cave_vertices[10] << endl;
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
	//glEnableVertexAttribArray(tex_coords);
	glVertexAttribPointer(
		tex_coords, // attribute
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	//glDisableVertexAttribArray(tex_coords);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	sh.stopUsing();
	int segs_theta = 150;
	int segs_rho = 150;
	
	/* Skapa cylindern (funktionen är rätt buggig, så håll båda segs = 150 */
	createCylinder(segs_theta, segs_rho, r, cave_vertices, cave_indices, cave_normals, theta_coords);
	
	/* Kompilera shadern som renderar grottan */
	Shader sh2;
	sh2.createShader("vertex_cave.glsl", "fragment_cave.glsl");
	GLint cave_vertex_attribute = glGetAttribLocation(sh2.getProgram(), "vertex");
	GLint cave_normal_attribute = glGetAttribLocation(sh2.getProgram(), "normal");
	sh2.createUniformFloat("time", glfwGetTime());
	sh2.createUniformFloat("z_walk", z_walk);
	sh2.createUniformFloat("oct_seed", oct_seeds);
	sh2.createUniformFloat("z_walk_texture", z_walk_texture);
	GLint fire_position_coords = glGetUniformLocation(sh2.getProgram(), "fire_position");
	if(fire_position_coords == -1)
		cout << "Fire position uniform not found" << endl;
	
	/* Generera alla buffrar */
	glGenBuffers(1, &cave_vertex_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, cave_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * segs_theta * segs_rho * 3, cave_vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(
		cave_vertex_attribute, // attribute
		3,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &cave_normal_buffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, cave_normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * segs_theta * segs_rho * 3, cave_normals, GL_STATIC_DRAW);
	
	glVertexAttribPointer(
		cave_normal_attribute, // attribute
		3,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &cave_index_buffer);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cave_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * segs_theta * (segs_rho - 1) * 4, cave_indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	GLint theta_location = glGetAttribLocation(sh2.getProgram(), "theta");
	GLuint theta_buffer;
	glGenBuffers(1, &theta_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, theta_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * segs * segs, theta_coords, GL_STATIC_DRAW);
	glVertexAttribPointer(
		theta_location, // attribute
		1,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	
	/* VSync */
	glfwSwapInterval(1);
	
	/** Renderingsloopen */
	while(running)
	{
		/* Cleara fönstret samt uppdatera uniforms */
		sh.use();
		glClearColor(0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		setupCamera(sh);
		float time_now = -time_last + glfwGetTime();
		scrot_time += time_now;
		
		sh.updateUniformFloat("time", glfwGetTime());
		sh.updateUniformFloat("octaves", octaves);
		printFps();
		
		sh2.use();
		sh2.updateUniformFloat("time", glfwGetTime());
		sh2.updateUniformFloat("z_walk", z_walk);
		sh2.updateUniformFloat("oct_seed", oct_seeds);
		
		sh2.updateUniformFloat("z_walk_texture", z_walk_texture);
		std::cout << z_walk_texture << std::endl;
		
		/* Sätt eldens position till samma som kamerans */
		float fire[] = {x_position, y_position, z_position};
		glUniform3fv(fire_position_coords, 1, fire);
		setupCamera(sh2);
		
		/* Rita grottan först, eftersom den inte är genomskinnlig */
		glEnableVertexAttribArray(cave_vertex_attribute);
		glBindBuffer(GL_ARRAY_BUFFER, cave_vertex_buffer);
		glVertexAttribPointer(
			cave_vertex_attribute, // attribute
			3,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		
		glEnableVertexAttribArray(cave_normal_attribute);
		glBindBuffer(GL_ARRAY_BUFFER, cave_normal_buffer);
		glVertexAttribPointer(
			cave_normal_attribute, // attribute
			3,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		
		glEnableVertexAttribArray(theta_location);
		glBindBuffer(GL_ARRAY_BUFFER, theta_buffer);
		glVertexAttribPointer(
			theta_location, // attribute
			1,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		//glEnableVertexAttribArray(attribute_coords);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cave_index_buffer);
		int size;  
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glDrawElements(GL_QUADS, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
		glDisableVertexAttribArray(cave_vertex_attribute);
		glDisableVertexAttribArray(cave_normal_attribute);
		glDisableVertexAttribArray(theta_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		/* Rita elden */
		sh.use();
		
		glEnableVertexAttribArray(attribute_coords);
		glEnableVertexAttribArray(tex_coords);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
		glVertexAttribPointer(
			tex_coords, // attribute
			2,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
		glVertexAttribPointer(
			attribute_coords, // attribute
			3,                 // number of elements per vertex, here (x,y)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableVertexAttribArray(tex_coords);
		glDisableVertexAttribArray(attribute_coords);

	
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		
		/** Hantera tangentbordsinput
		 * 	Väldigt väldigt VÄLDIGT ful kod
		 * Styr med WASD och se dig omkring med TFGH */
		if(glfwGetKey(window, GLFW_KEY_ESCAPE))
			running = false;
		if(glfwGetKey(window, GLFW_KEY_RIGHT))
		{
			float weight = 1.0f;
			if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
				weight = 2.0f;
			viewer_fi += weight * 2.0 * time_now;
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT))
		{
			float weight = 1.0f;
			if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
				weight = 2.0f;
			viewer_fi -= weight * 2.0 * time_now;
		}
		if(glfwGetKey(window, GLFW_KEY_UP))
		{
			float weight = 1.0f;
			if(glfwGetKey(window,  GLFW_KEY_LEFT_SHIFT))
				weight = 2.0f;
			if(viewer_theta > 0.1)
			viewer_theta -= weight * 2.0 * time_now;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN))
		{
			float weight = 1.0f;
			if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
				weight = 2.0f;
			if(viewer_theta < M_PI - 0.1)
			viewer_theta += weight * 2.0 * time_now;
		}
		
		y_offset = cosf(viewer_theta);
		z_offset = sinf(viewer_fi) * sinf(viewer_theta);
		x_offset = cosf(viewer_fi) * sinf(viewer_theta);
		float length = sqrt(x_offset * x_offset + z_offset * z_offset);
		y_position_moveto = -6.0;
		if(glfwGetKey(window, 'W'))
		{
			float weight = 1.0f;
			if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
				weight = 3.0f;
			//z_walk += time_now;
			x_position += weight * 10.0 * x_offset * time_now / length;
			x_position_moveto += weight * 10.0 * x_offset * time_now / length;
			y_position += weight * 10.0 * y_offset * time_now;
			y_position_moveto += weight * 10.0 * y_offset * time_now;
			z_position += weight * 10.0 * z_offset * time_now / length;
			z_position_moveto += weight * 10.0 * z_offset * time_now / length;
			y_position = -6.0 + 0.3 * sinf(sin_coord += (time_now * 10.0f));
			
		}
		
		if(glfwGetKey(window, 'S'))
		{
			//z_walk -= time_now;
			x_position -= 10.0 * x_offset * time_now / length;
			x_position_moveto -= 10.0 * x_offset * time_now / length;
			y_position -= 10.0 * y_offset * time_now;
			y_position_moveto -= 10.0 * y_offset * time_now;
			z_position -= 10.0 * z_offset * time_now / length;
			z_position_moveto -= 10.0 * z_offset * time_now / length;
			y_position = -6.0 + 0.3 * sinf(sin_coord -= (time_now * 10.0f));
		}
		
		if(glfwGetKey(window, 'A'))
		{
			x_position += 10.0 * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta))) * time_now;
			x_position_moveto += 10.0 * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta))) * time_now;
			z_position += 10.0 * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta))) * time_now;
			z_position_moveto += 10.0 * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi - M_PI / 2.0) * sinf(viewer_theta))) * time_now;
		}
		
		if(glfwGetKey(window, 'D'))
		{
			x_position += 10.0 * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta))) * time_now;
			x_position_moveto += 10.0 * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta))) * time_now;
			z_position += 10.0 * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)))* time_now;
			z_position_moveto += 10.0 * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) / sqrt((cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (cosf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) + (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)) * (sinf(viewer_fi + M_PI / 2.0) * sinf(viewer_theta)))* time_now;
		}
		
		if(glfwGetKey(window, 'J'))
		{
			x_position_moveto += 20.0* time_now;
		}
		if(glfwGetKey(window, '+'))
		{
			x_position_moveto += 20.0* time_now;
		}
		if(glfwGetKey(window, '-'))
		{
			x_position_moveto += 20.0* time_now;
		}
		if(glfwGetKey(window, GLFW_KEY_SPACE))
		{
			//oct_seeds += time_now;
			z_walk_texture += time_now * 10.0f;
		}

		if (glfwGetKey(window, GLFW_KEY_BACKSPACE))
		{
			//oct_seeds += time_now;
			z_walk_texture -= time_now * 10.0f;
			if (z_walk_texture <= 0.0f) {
				z_walk_texture = 0.0f;
			}
		}
		
		/* Ta screenshot med P */
		if(glfwGetKey(window, 'P'))
		{
			if(scrot_time > 2.0)
			{
				print_image("bild.ppm", 2560, 1440);
				cout << "Screenshot saved to bild.ppm" << endl;
				
			}
			scrot_time = 0.0f;
		}
		
		cameraMoveTo(x_position, y_position, z_position, x_position_moveto, y_position_moveto, z_position_moveto, 1.0f, time_now, start);
		//z_walk = 0.0f;
		while(z_position > 1.0f || z_position < -1.0f)
		{
			if(z_position > 1.0f)
			{
				z_walk += 1.0f;
				z_position -= 1.0f;
				z_position_moveto -= 1.0f;
			}
			
			if(z_position < -1.0f)
			{
				z_walk -= 1.0f;
				z_position += 1.0f;
				z_position_moveto += 1.0f;
			}
		}
		sh.stopUsing();
	}
	
	
	/* Städa lite grann */
	sh.Delete();
	sh2.Delete();
	glDeleteBuffers(1, &vbo_triangle);
	glDeleteBuffers(1, &cave_index_buffer);
	glDeleteBuffers(1, &cave_vertex_buffer);
	glDeleteBuffers(1, &cave_normal_buffer);
	glDeleteBuffers(1, &theta_buffer);
	glfwTerminate();
	return 0;
}
