#include <GL/glew.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "Shader.hpp"

const char* stegus_noise = "stegu_noise.glsl";
const char* version_string = "#version 330\n";
using namespace std;

/* Använd shadern */
bool Shader::use()
{
	if(compiled)
	{
		glUseProgram(program);
		return true;
	}
	return false;
}

/* Sluta använda shadern */
void Shader::stopUsing()
{
	glUseProgram(0);
	//glDeleteProgram(0);
}

void Shader::Delete()
{
	glDeleteProgram(program);
}

/* Läs sträng från fil, från wikibooks "opengl" */
char* Shader::readFile(const char* filename, int& size)
{
  FILE* in = fopen(filename, "rb");
  if (in == NULL) return NULL;
 
  int res_size = BUFSIZ;
  char* res = (char*)malloc(res_size);
  int nb_read_total = 0;
 
  while (!feof(in) && !ferror(in)) {
	if (nb_read_total + BUFSIZ > res_size) {
	  if (res_size > 10*1024*1024) break;
	  res_size = res_size * 2;
	  res = (char*)realloc(res, res_size);
	}
	char* p_res = res + nb_read_total;
	nb_read_total += fread(p_res, 1, BUFSIZ, in);
  }
 
  fclose(in);
  res = (char*)realloc(res, nb_read_total + 1);
  size = nb_read_total + 1;
  res[nb_read_total] = '\0';
  return res;
}

/* Skriv ut eventuella kompilerings- eller länkningsfel, taget från wikibooks opengl */
void Shader::printLog(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else {
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}
	 
	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);
	 
	fprintf(stderr, "%s", log);
	free(log);
}

/* Skapa en shader med två filnamn, delvis tagen från wikibooks opengl */
bool Shader::createShader(const char* vertexSource, const char* fragmentSource)
{
	int stegu_size = 0;
	const char* stegu_source = readFile(stegus_noise, stegu_size);
	
	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;
	
	/* Kompilera vertexshadern */
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	int vertex_size = 0;
	const char *vs_source_file = readFile(vertexSource, vertex_size);
	/*
	char* vs_source2 = new char[sizeof(version_string)];
	strcpy(vs_source2, version_string);
	strcat(vs_source2, stegu_source);
	strcat(vs_source2, vs_source_file);
	*/
	//const char* vs_source = vs_source2;
	//string vs_source_string(vs_source2);
	//const char* vs_source = vs_source_string.c_str();
	
	const char** vs_sources = new const char*[3];
	vs_sources[0] = version_string;
	vs_sources[1] = stegu_source;
	vs_sources[2] = vs_source_file;
	
	glShaderSource(vs, 3, (vs_sources), NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	printLog(vs);
	
	/* Blev det något fel? */
	if (!compile_ok) 
	{
		printf("Error, vertex shader: ");
		printLog(vs);
		return 0;
	}
	
	/* Kompilera fragmentshadern */
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	int fragment_size = 0;
	const char *fs_source_file = readFile(fragmentSource,fragment_size);
	
	const char** fs_sources = new const char*[3];
	fs_sources[0] = version_string;
	fs_sources[1] = stegu_source;
	fs_sources[2] = fs_source_file;
	
	glShaderSource(fs, 3, fs_sources, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
	printLog(fs);
	/* Blev något fel? */
	if (!compile_ok) 
	{
		printf("Error, fragment shader: ");
		printLog(fs);
		return 0;
	}
	
	/* Länka programmet */
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	printLog(program);
	/* Skriv ut eventuella fel */
	if (!link_ok) 
	{
		printf("Link error: ");
		printLog(program);
		return false;
	}
	glBindFragDataLocation(program, 0, "MyFragColor");
	/* Allt gick bra */
	compiled = true;
	//delete [] vs_source;
	delete [] fs_sources;
	delete [] stegu_source;
	return true;
}

Shader::~Shader()
{
	
}

GLuint Shader::getProgram()
{
	return program;
}

/* Lägg till en uniform float i shadern */
bool Shader::createUniformFloat(const char* name, float value)
{
	/* Använd shadern */
	if(!use())
	{
		return false;
	}
	
	
	GLint location = glGetUniformLocation(program, name);
	if(location != -1)
		glUniform1fv( location, 1, &value );
	else
		return false;
		
	names.push_back(string(name));
	locations.push_back(location);
	return true;
}

bool Shader::updateUniformFloat(const char* name, float value)
{
	/* Använd shadern */
	if(!use())
	{
		return false;
	}
	
	GLint location = -1;
	
	for(int i = 0; i < names.size(); i ++)
	{
		if(names[i].compare(string(name)) == 0)
		{
			location = locations[i];
		}
	}
	
	if(location == -1)
		return false;
	glUniform1fv(location, 1, &value );
	return true;
	
}
