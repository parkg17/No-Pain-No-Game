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
uniform vec4	light_position, Ia, Id, Is;	// light
uniform vec4	Ka, Kd, Ks;					// material properties

// texture sampler
uniform sampler2D TEX;
uniform sampler2D NORMS;
uniform int MODE;
uniform float alpha;

vec4 phong( vec3 l, vec3 n, vec3 h, vec4 Kd )
{
	vec4 Ira = Ka*Ia;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is,0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	// light position in the eye space
	vec4 lpos = view_matrix*light_position;

	// Shading
	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l+v);	// the halfway vector

	if(MODE == 0) { // Texture Doesn't exists
		fragColor = phong( l, n, h, Kd );
	}
	else { // Texture exists
		vec4 iKd = texture( TEX, tc );
		fragColor = phong( l, n, h, iKd );
	}

}
