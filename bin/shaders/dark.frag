#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec4 epos;
in vec2 tc;
in vec3 norm;

// the only output variable
out vec4 fragColor;

// uniform variables
uniform mat4	view_matrix;
uniform float	shininess;
uniform float	constant, linear, quadratic;
uniform vec4	Ia, Id, Is;	// light
uniform vec4	light_position, light_position2;
uniform vec4	material_position, Ka, Kd, Ks;					// material properties
uniform int		light_num;


// texture sampler
uniform sampler2D TEX;
uniform sampler2D NORMS;
uniform int MODE;
uniform float alpha;

vec4 phong( vec3 l, vec3 n, vec3 h, vec4 Kd, vec4 Ia_att, vec4 Id_att, vec4 Is_att )
{
	vec4 Ira = Ka*Ia_att;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id_att,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is_att,0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	
	
	float distance = sqrt((light_position[0]-material_position[0])*(light_position[0]-material_position[0]) + 
			(light_position[1]-material_position[1])*(light_position[1]-material_position[1]) + 
			(light_position[2]-material_position[2])*(light_position[2]-material_position[2]));
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance * distance));


	float distance2 = sqrt((light_position2[0]-material_position[0])*(light_position2[0]-material_position[0]) + 
			(light_position2[1]-material_position[1])*(light_position2[1]-material_position[1]) + 
			(light_position2[2]-material_position[2])*(light_position2[2]-material_position[2]));
	float attenuation2 = 1.0 / (constant + linear * distance2 + quadratic * (distance2 * distance2 * distance2));

	// light position in the eye space
	vec4 lpos = view_matrix*light_position;
	vec4 lpos2 = view_matrix*light_position2;

	// Shading
	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 l2 = normalize(lpos2.xyz-(lpos2.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l+v);	// the halfway vector
	vec3 h2 = normalize(l2+v);	// the halfway vector

	if(MODE == 0) { // Texture Doesn't exists
		fragColor = phong( l, n, h, Kd, Ia * attenuation, Id * attenuation, Is * attenuation)/light_num;
		fragColor += phong( l2, n, h2, Kd, Ia * attenuation2, Id * attenuation2, Is * attenuation2)/light_num;
	}
	else { // Texture exists
		vec4 iKd = texture( TEX, tc );
		fragColor = iKd;
		fragColor = phong( l, n, h, iKd, Ia * attenuation, Id * attenuation, Is * attenuation)/light_num;
		fragColor += phong( l2, n, h2, iKd, Ia * attenuation2, Id * attenuation2, Is * attenuation2)/light_num;
	}
	
}