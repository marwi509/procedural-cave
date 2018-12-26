/** Vertex shader för elden, gör typ ingenting */

uniform float time;


uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

in vec3 vertex;
in vec2 texcoords;
out vec4 position;
out vec2 texes;
void main(void) 
{ 
	
	gl_Position = vec4(vertex, 2.0);//projection_matrix * modelview_matrix * vec4(vertex, 1.0);
	position = gl_Position;
	texes = texcoords;
}
