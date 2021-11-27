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
uniform vec4	light_position, light_position2, light_position3, light_position4, light_position5;
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
	vec4 light_p[5];
	vec4 lpos, iKd;
	vec3 n, p, l, v, h;
	float distance[5];
	float att[5]; // attenuation

	light_p[0] = light_position;
	light_p[1] = light_position2;
	light_p[2] = light_position3;
	light_p[3] = light_position4;
	light_p[4] = light_position5;

	for(int idx = 0; idx < light_num; ++idx) {
		distance[idx] = sqrt( pow(light_p[idx][0]-material_position[0], 2)+
						      pow(light_p[idx][1]-material_position[1], 2)+
					          pow(light_p[idx][2]-material_position[2], 2));
		att[idx] = 1.0 / (constant + linear * distance[idx] + quadratic * pow(distance[idx], 3) );

		// Shading
		lpos = view_matrix * light_p[idx];
		n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
		p = epos.xyz;			// 3D position of this fragment
		l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
		v = normalize(-p);		// eye-epos = vec3(0)-epos
		h = normalize(l+v);	// the halfway vector

		if(MODE == 0) { // Texture Doesn't exists
			if(idx == 0) fragColor = phong( l, n, h, Kd, Ia * att[idx], Id * att[idx], Is * att[idx])/light_num;
			else fragColor += phong( l, n, h, Kd, Ia * att[idx], Id * att[idx], Is * att[idx])/light_num;
		}
		else { // Texture exists
			iKd = texture( TEX, tc );
			if(idx == 0) fragColor = phong( l, n, h, iKd, Ia * att[idx], Id * att[idx], Is * att[idx])/light_num;
			else fragColor += phong( l, n, h, iKd, Ia * att[idx], Id * att[idx], Is * att[idx])/light_num;
		}
	} 
}