//#version 330

/** Grottans fragmentshader, bump-mapping, färgtextur, dimma och is */

uniform float time;
uniform float z_walk;
uniform float z_walk_texture;
uniform float octaves;
uniform vec3 fire_position;

out vec4 MyFragColor;
in vec4 position;
in vec3 normal_in;
in float theta_in;

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

/* Fraktal med simplex noise */
float snoise(vec3 val, float octaves, vec3 basefreq)
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
	/* Ska noise användas? Om false blir det grottan utan dimma och noise, bara belysning från facklan */
	bool noise_calc = true;
	vec3 lightdirection = (fire_position-position.xyz);
	float light_length_sq = (dot(lightdirection,lightdirection));
	lightdirection = lightdirection /sqrt(light_length_sq);
	float light_atten_weight = clamp(200.0 / light_length_sq, 0.0, 1.0);
	
	/* Om bidraget ändå blir väldigt väldigt litet, avbryt tidigt */
	if(light_atten_weight < 0.01 && position.y > -7.0)
		discard;
	
	/* Räkna ut hur många oktaver som ska användas */
	float oct_extras = 0.0;//6.0 - 6.0 * smoothstep( 2.0, 10.0,sqrt(light_length_sq));
	float oct_extras2 = 3.0 - 3.0 * smoothstep( 0.0, 20.0,sqrt(light_length_sq));
	float noise = 1.0;
	float bump_octaves = clamp(4.0 + oct_extras + 0.5 * snoise(vec2(position.z + z_walk, 1.0) * 0.0001) , 0.0, 5.0) + oct_extras2;
	
	/* Gör ytan olika mycket skrovlig beroende på vart man är, kom in i koden väldigt sent så tonade ned det lite
	 *  */
	float bump_strength = 0.5 * (snoise(vec2(position.z + z_walk + z_walk_texture, 1.0) * 0.0001) + 1.0);
	float theta_base = 10.0 / 3.14;
	float rho_base = 0.5;
	
	/* Noise med fasta frekvenser */
	noise = sqrt(clamp(0.5*(
		1.0 * 
		(
			0.75 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base) * 2.0) + 
			0.25 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base)  * 4.0) +
			0.25 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base) * 9.0)  +
			0.125 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base) * 19.0) + 
			0.125 * 0.5 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base) * 32.0) +
			0.725 * 0.25 * snoise(vec2(theta_in * theta_base, (position.z + z_walk) * rho_base) * 64.0)
		)
		 + 1.0), 0.0, 1.0));
	
	/* Extra-oktaver med noise, när man närmar sig ytan */
	noise += 0.125 * snoise(vec2(theta_in, position.z + z_walk), clamp(bump_octaves - 5.0, 0.0, 10.0), vec2(1.0, 1.0 / 3.14) * 16.0 * 4.0);


	vec2 cell = cellular((position.xyz + vec3(0,0,z_walk)) * 0.5) * 2.0;
	vec3 normal_skewed = normal_in;
	float cell_percentage = 0.7 + 0.3 * snoise(vec2(z_walk + position.z, 1.0) * 0.001);
	cell_percentage = 1.0 - cell_percentage;
	
	/* "wetness" räknar ut om det är is eller inte */
	float wetness = 0.6*(1.0 + snoise((vec2(position.z + z_walk, theta_in / 3.14))*0.1));
	wetness = smoothstep(0.5,1.0,wetness)*wetness;
	wetness = smoothstep(0.0,-2.0,position.y)*wetness;
	wetness = smoothstep(-9.0,-7.0,position.y)*wetness;
	
	/* Räkna inte ut bumpmappen om punkten är under dimman */
	if(position.y > -9.0)
	{
		float bumpfreq = 0.6;
		vec3 tan1 = normalize(cross(normal_skewed, vec3(1.0,1.0,1.0)));
		vec3 tan2 = normalize(cross(normal_skewed, tan1));
		vec3 position_tan1 = position.xyz + vec3(0.0,0.0,z_walk) + tan1 * 0.001;
		vec3 position_tan2 = position.xyz + vec3(0.0,0.0,z_walk) + tan2 * 0.001;
		
		/* Öka koeffecienterna för mer skrovlighet (* 2.0 för samma utseende som den sista resultatbilden i rapporten)
		 *  */
		float noise_1 = z_walk_texture * (bump_strength * 0.1 + 0.9) * 0.08 * snoise(position.xyz + vec3(0.0,0.0,z_walk), bump_octaves, vec3(1.0,1.0,1.0) * bumpfreq);
		float noise_2 = z_walk_texture * (bump_strength * 0.1 + 0.9) * 0.08 * snoise(position_tan1, bump_octaves, vec3(1.0,1.0,1.0) * bumpfreq);
		float noise_3 = z_walk_texture * (bump_strength * 0.1 + 0.9) * 0.08 * snoise(position_tan2, bump_octaves, vec3(1.0,1.0,1.0) * bumpfreq);
		
		vec3 position_1 = position.xyz + vec3(0.0,0.0,z_walk) + normal_skewed * noise_1;
		position_tan1 += normal_skewed * noise_2;
		position_tan2 += normal_skewed * noise_3;
		
		normal_skewed = normalize(cross(position_tan1 - position_1, position_tan2 - position_1));
		
	}

	/* Räkna bara ut den speukulära delen om den faktiskt ska användas */
	float spec = 0.0;
	if(noise_calc && wetness > 0.001)
		spec = 1.0 * pow(abs(dot(lightdirection, -normal_skewed)), 40.0);
	else if(wetness > 0.001)
		spec = 1.0 * pow(abs(dot(lightdirection, -normal_in)), 40.0);
	
	float f = 0.0;
	if(noise_calc)	
		/* Blanda cellular noise med fraktalen samt lägg in ljusberäkningar enligt phong-modellen */
		f = ( (cell.x  - cell.y*0.1 + 0.5) * abs(noise)) * abs(dot(lightdirection, (normal_skewed)) * clamp(0.8 + 0.2*snoise(vec2(1.0, time * 1.5)), 0.0, 1.0) * light_atten_weight);// * noise;
	else
		f = abs(dot(lightdirection, (normal_in)) * clamp(0.8 + 0.2*snoise(vec2(1.0, time * 1.5)), 0.0, 1.0) * light_atten_weight);// * noise;
	
	/* Rendera en mystisk dimma för att dölja det faktum att noise-funktionen inte är periodisk i theta-riktningen */
	if(position.y < -7.0 && noise_calc)
	{
		
		float weight = smoothstep(-7.0, -9.0, position.y);
		float light2 = light_atten_weight;
		/* Dimman avtar med med avseende på avståndet också, men inte lika snabbt (det var för tydligt att dimman var blåfärgad mark innan) */
		light_atten_weight = clamp(light_atten_weight, 0.0, 0.01) * 50.0;
		light_atten_weight = sqrt(light_atten_weight);
		float distance_weight = smoothstep(50.0, 75.0, sqrt(light_length_sq));
		MyFragColor[0] = (light_atten_weight * 0.5 * weight + (1.0 - weight) * (1.0 - wetness * 0.6) * f) * (1.0 - distance_weight) +       abs(spec) * 2.0 * light2 * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness;
		MyFragColor[1] = (light_atten_weight * 0.5 * weight + (1.0 - weight) * (1.0 - wetness * 0.6) * f * 0.8) * (1.0 - distance_weight) + abs(spec) * 2.0 * light2 * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness * 0.8;
		MyFragColor[2] = (light_atten_weight * 0.7 * weight + (1.0 - weight) * (1.0 - wetness * 0.5) * f * 0.7) * (1.0 - distance_weight) + abs(spec) * 2.0 * light2 * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness * 0.7;
		MyFragColor[3] = 1.0;
		
	}
	else
	{
		if(noise_calc)
		{
			MyFragColor[0] = (1.0 - wetness * 0.6) * f + abs(spec) * 2.0 * light_atten_weight * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness;
			MyFragColor[1] = (1.0 - wetness * 0.6) * f * 0.8 + abs(spec) * 2.0 * light_atten_weight * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness * 0.8;
			MyFragColor[2] = (1.0 - wetness * 0.5) * f * 0.7 + abs(spec) * 2.0 * light_atten_weight * (smoothstep(0.0, -10.0, position.y) + 0.1) * wetness * 0.7;
			MyFragColor[3] = 1.0;
		}
		else
		{
			MyFragColor[0] = f;
			MyFragColor[1] = f * 0.8;
			MyFragColor[2] = f * 0.7;
			MyFragColor[3] = 1.0;
		}
	}

	//MyFragColor[0] =  z_walk_texture;

}
