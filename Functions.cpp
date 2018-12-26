#include <math.h>


#include "C:\devc\c++\lib\include\GLFW\glfw3.h"
#include "Functions.hpp"

void createCylinder(int segs_theta, int segs_rho, float r, GLfloat *&verts, GLushort *&indices, GLfloat *&normals, GLfloat *&theta_coords)
{
	verts = new GLfloat[segs_theta * segs_rho * 3];
	normals = new GLfloat[segs_theta * segs_rho * 3];
	indices = new GLushort[(segs_rho - 1) * segs_theta * 4];
	theta_coords = new GLfloat[segs_rho*segs_theta];
	float thetasegs = 2 * 3.14 / float(segs_theta);
	float rhosegs = 15.0f / float(segs_rho);
	
	//float theta= 0.0f;
	//float rho
	
	for(int i = 0; i < segs_rho; i ++)
	{
		for(int j = 0; j < segs_theta; j ++)
		{
			float current_theta = -3.14 / 2.0 + float(j) * thetasegs;
			theta_coords[i * segs_theta + j] = current_theta;
			verts[i * segs_theta * 3 + j * 3 + 0] = r * cosf(current_theta);
			verts[i * segs_theta * 3 + j * 3 + 1] = r * sinf(current_theta);
			verts[i * segs_theta * 3 + j * 3 + 2] = - 75.0f + r * 	  float(i) * rhosegs;
			
			normals[i * segs_theta * 3 + j * 3 + 0] = - verts[i * segs_theta * 3 + j * 3 + 0];
			normals[i * segs_theta * 3 + j * 3 + 1] = - verts[i * segs_theta * 3 + j * 3 + 1];
			normals[i * segs_theta * 3 + j * 3 + 2] = 0;
			
			float length = sqrtf(normals[i * segs_theta * 3 + j * 3 + 0] * normals[i * segs_theta * 3 + j * 3 + 0] + normals[i * segs_theta * 3 + j * 3 + 1] * normals[i * segs_theta * 3 + j * 3 + 1]);
			normals[i * segs_theta * 3 + j * 3 + 0] = normals[i * segs_theta * 3 + j * 3 + 0] / length;
			normals[i * segs_theta * 3 + j * 3 + 1] = normals[i * segs_theta * 3 + j * 3 + 1] / length;
			
			
		}
	}
	
	for(int i = 0; i < segs_rho-1; i ++)
	{
		for(int j = 0; j < segs_theta; j ++)
		{
			if(j < segs_theta-1)
			{
				indices[i * segs_theta * 4 + j * 4 + 0] = (i + 0) * segs_theta + (j + 0);
				indices[i * segs_theta * 4 + j * 4 + 1] = (i + 1) * segs_theta + (j + 0);
				indices[i * segs_theta * 4 + j * 4 + 2] = (i + 1) * segs_theta + (j + 1);
				indices[i * segs_theta * 4 + j * 4 + 3] = (i + 0) * segs_theta + (j + 1);
			}
			else
			{
				indices[i * segs_theta * 4 + j * 4 + 0] = (i + 0) * segs_theta + (j + 0);
				indices[i * segs_theta * 4 + j * 4 + 1] = (i + 1) * segs_theta + (j + 0);
				indices[i * segs_theta * 4 + j * 4 + 2] = (i + 1) * segs_theta + (0);
				indices[i * segs_theta * 4 + j * 4 + 3] = (i + 0) * segs_theta + (0);
			}
		}
	}
}

void displaceCylinder(int segs, float r, GLfloat *&verts, GLfloat *&normals, GLfloat *&theta_coords, int octaves, float baseFrequency)
{
	
	
}



void cameraMoveTo(float &camera_x, float &camera_y, float &camera_z, float moveto_x, float moveto_y, float moveto_z, float base_speed, float t, bool speed)
{
	float x_dist_sq = (moveto_x - camera_x) * (moveto_x - camera_x);
	float y_dist_sq = (moveto_y - camera_y) * (moveto_y - camera_y);
	float z_dist_sq = (moveto_z - camera_z) * (moveto_z - camera_z);
	
	/* I'M GONNA CLAMP YOU */
	float x_speed = clamp(speed ? 1.0 : 0.0, 20.0, x_dist_sq); // YOU WANNA GET CLAMPED?
	float y_speed = clamp(speed ? 1.0 : 0.0, 20.0, y_dist_sq);
	float z_speed = clamp(speed ? 1.0 : 0.0, 20.0, z_dist_sq);
	
	camera_x += x_speed * sign(moveto_x - camera_x) * t * base_speed;
	camera_y += y_speed * sign(moveto_y - camera_y) * t * base_speed;
	camera_z += z_speed * sign(moveto_z - camera_z) * t * base_speed;
}
