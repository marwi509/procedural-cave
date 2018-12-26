/** Fragmentshader för elden, räknar ut i stort sett allt med hjälp av simplex noise och avstånd från eldens centrum (vilket tyvärr är hårdkodat just nu) */

uniform float time;
uniform float octaves;
out vec4 MyFragColor;
in vec4 position;
in vec2 texes;

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
	vec2 position_final = vec2(texes.x*2.0  - 1.0, texes.y*2.0 - 1.0);
	float firesizex = 4.0 ;
	float firesizey = 1.2 ;
	vec2 fireposition = vec2(0.6, -0.4);
	
	float checkx = 1.0 - abs(fireposition.x - position_final.x) * firesizex;
	checkx = clamp(checkx, 0.0,0.9);
	
	float checky = 1.0 - abs(fireposition.y - position_final.y + 0.2) * firesizey;
	checky = clamp(checky, 0.0, 0.9);
	
	float checky2 = 1.0 - abs(fireposition.y - position_final.y) * firesizey;
	
	/* Är punkten tillräckligt nära eldens centrum? */
	if(checkx > 0.1 / firesizex && checky > 0.1 / firesizey && (1.0 - checkx)*(1.0 - checkx) + (1.0 - checky2)*(1.0 - checky2) > 0.003)
	{
		
		float noise = 0.0;
		float base = 2.5;
		float limit = 2.5;
		float sinfreq = 1.5;
		float sq = pow(2.0, ceil(limit));
		
		/* Fraktal-noise */
		noise = snoise(vec2(position_final.x, position_final.y - time * 1.8), floor(limit + 1.0), vec2(base));
		noise = noise + step(0.0, limit) * (1/sq) * (limit - floor(limit)) * snoise(vec2(position_final.x, position_final.y - time * .8) * base * sq);
		
		noise = 0.5*(1.0 + clamp(noise, -1.0, 1.0));
		float flamma = (1.0 + 0.2*snoise(vec2(1.0, time * sinfreq)));
		noise = flamma * 0.8*(noise + 1.2) * checkx * checky;
		
		noise = 1.5*noise + 0.05*snoise(vec2(position_final.x + 0.1*time, position_final.y - 0.1*time)*2.0);
		
		/* Ge elden en ganska skarp gräns mellan gult och ingenting */
		noise = smoothstep(0.6, 1.0, noise) * noise ;
		float alpha = noise;
		float red = 1.0;
		float green = noise*noise*noise;
		float blue = smoothstep(0.6,1.0,noise * 0.5) * noise * 0.5;
	
		MyFragColor[0] = red;
		MyFragColor[1] = green;
		MyFragColor[2] = blue;
		MyFragColor[3] = alpha;
			
		/* Ligger punkten på facklans skaft? */
		if((1.0 - abs(fireposition.x - position_final.x) > 0.97 && position_final.y < fireposition.y + 0.0))
		{
			/* noise */
			float shaft = 0.5 * (1.0 + snoise(vec2(5.0 * smoothstep(0.97, 1.03,1.0 - (fireposition.x - position_final.x)), position_final.y * 20.0)));
			/* få det att se ut som om elden är en ljuskälla */
			float weight = flamma * (smoothstep(0.97, 1.0,1.0 - abs(fireposition.x - position_final.x)) + 0.1) * (0.1 + 0.9 * clamp(1.0 /  (10.0 * abs(fireposition.y - position_final.y) * abs(fireposition.y - position_final.y)), 0.0, 1.0));;
			MyFragColor[0] = (shaft*0.7 + 0.3) * 0.5 * weight + red * alpha;
			MyFragColor[1] = (shaft*0.7 + 0.3)* 0.3 * weight + green * alpha;
			MyFragColor[2] = (shaft*0.7 + 0.3)* 0.2 * weight + blue * alpha;
			MyFragColor[3] = 1.0;
		
		}
	}
	/* Annars avbryt */
	else
	{
		discard;
	}	

}
