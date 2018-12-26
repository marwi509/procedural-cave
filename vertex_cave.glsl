//#version 330

/** Grottans vertex shader, fixar displacement och normaler */

uniform float time;
uniform float oct_seed;
uniform float z_walk;

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

uniform vec3 fire_position;
in vec3 vertex;
in vec2 texcoords;
in float theta;
in vec3 normal;
out vec4 position;
out vec2 texes;
out vec3 normal_in;
out float theta_in;

/* Fraktal med simplex noise */
float snoise(vec2 val, float octaves, vec2 basefreq)
{
	float f = 0.0;
	float i = 0;
	float power = 0.0;
	float power_inv = 0.0;
	for(i = 0; i < floor(octaves); i ++)
	{
		power = pow(2.0, i);
		power_inv = pow(0.5, i);
		f += snoise(val * basefreq * power) * power_inv;
	}
	power = pow(2.0, i);
	power_inv = pow(0.5, i);
	f += (octaves - floor(octaves)) * snoise(val * basefreq * power) * power_inv;
	return f;
}

void main(void) 
{ 
	bool displace = true;
	
	
	if(displace)
	{
		// Octaves for the high frequency displacement 
		float oct_seed_extra;
		if(oct_seed == 0.0)
			oct_seed_extra = 0.0;
		else 
			oct_seed_extra = sin(oct_seed);
		
		float octaves = clamp(2.0 +  3.0 * snoise(vec2(vertex.z + z_walk, 1.0) * 0.01) + 2.0 * oct_seed_extra, 1.0, 8.0);
		
		// Octaves for the low frequency displacement (the one making the cave turn in different directions)
		float octaves_large = 2.0 + snoise(vec2(vertex.z + z_walk, 1.0) * 0.00001);
		vec3 vertex_used = vec3(vertex.x, vertex.y, vertex.z) + vec3(0,0,z_walk);
		vec2 two_dim_position = vec2(vertex_used.z * 0.1, theta);
		vec3 three_dim_position = vertex_used.xyz;
		
		float start_offs = -10.0 * 0.5;
		//float noise_larger = snoise(vec2(start_offs + vertex_used.z * 0.01, 1.0)) * 50.0 + snoise(vec2(start_offs * 0.5 + vertex_used.z * 0.005, 1.0)) * 100.0;
		float noise_larger = 100.0 * snoise(vec2(start_offs + vertex_used.z * 0.005, 1.0), octaves_large, vec2(1.0,1.0));
		
		//float noise_larger2 = snoise(vec2(start_offs + vertex_used.z * 0.01 + 0.0001, 1.0)) * 50.0 + snoise(vec2(start_offs * 0.5 + vertex_used.z * 0.005 + 0.00005, 1.0)) * 100.0;
		float noise_larger2 = 100.0 * snoise(vec2(start_offs + vertex_used.z * 0.005 + 0.00005, 1.0), octaves_large, vec2(1.0,1.0));
		
		// Low frequency displacement in y-direction, not used right now since it messes up the fog
		float noise_larger3 = 0.0;

		//float noisex = 3.0 * (snoise(two_dim_position) + snoise(two_dim_position * 2.0)*0.5 + snoise(two_dim_position * 4.0) * 0.25 + snoise(two_dim_position * 8.0) * 0.125);
		float noisex = 3.0 * snoise(two_dim_position, octaves, vec2(1.0,1.0));
		vec3 tangent_position_theta = vec3(10.0 * cos(theta + 0.01), 10.0 * sin(theta + 0.01), vertex_used.z);
		vec2 two_dim_tan_theta = vec2(vertex_used.z  * 0.1, theta + 0.01);
		
		vec3 tangent_position_rho = vec3(10.0 * cos(theta), 10.0 * sin(theta), vertex_used.z + 0.01);
		vec2 two_dim_tan_rho = vec2(vertex_used.z   * 0.1 + 0.001, theta);
		
		//float noisey = 3.0*(snoise(two_dim_tan_theta) + snoise(two_dim_tan_theta * 2.0)*0.5 + snoise(two_dim_tan_theta * 4.0) * 0.25 + snoise(two_dim_tan_theta * 8.0) * 0.125);
		float noisey = 3.0 * snoise(two_dim_tan_theta, octaves, vec2(1.0,1.0));
		float noisez = 3.0 * snoise(two_dim_tan_rho, octaves, vec2(1.0,1.0));
		//float noisez = 3.0*(snoise(two_dim_tan_rho) + snoise(two_dim_tan_rho * 2.0)*0.5 + snoise(two_dim_tan_rho * 4.0) * 0.25 + snoise(two_dim_tan_rho * 8.0) * 0.125);
		noisex = (smoothstep(-10.0,-5.0, vertex_used.y)*0.5 + 0.5) * noisex;
		noisey = (smoothstep(-10.0,-5.0, vertex_used.y)*0.5 + 0.5) * noisey;
		noisez = (smoothstep(-10.0,-5.0, vertex_used.y)*0.5 + 0.5) * noisez;
		
		vec3 new_position0 = vec3(vertex_used.xy*(1.0 - noisex*0.05), vertex_used.z) + vec3(noise_larger,-noise_larger3,0);
		vec3 new_position1 = vec3(tangent_position_theta.xy*(1.0 - noisey*0.05), vertex_used.z) + vec3(noise_larger,-noise_larger3,0);
		vec3 new_position2 = vec3(tangent_position_rho.xy*(1.0 - noisez*0.05), vertex_used.z + 0.01) + vec3(noise_larger2,-noise_larger3,0);
		
		
		vec3 new_normal = normalize(cross(new_position1 - new_position0, new_position2 - new_position0));
		//*/
		gl_Position = projection_matrix * modelview_matrix * vec4((new_position0 - vec3(0,0,z_walk)), 1.0);
		position = vec4((new_position0 - vec3(0,0,z_walk)), 1.0);
		normal_in = new_normal;
		theta_in = theta;
		//texes = texcoords;
	}
	
	else
	{
		gl_Position = projection_matrix * modelview_matrix * vec4(vertex, 1.0);
		position = vec4(vertex, 1.0);
		normal_in = normal;
		theta_in = theta;
	}
}
