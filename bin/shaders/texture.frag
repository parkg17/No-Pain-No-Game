#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec2 tc;

// the only output variable
out vec4 fragColor;

uniform sampler2D	TEX0;
uniform sampler2D	TEX1;
uniform int			mode;



void main()
{	
	if(mode == 0) fragColor = texture( TEX0, tc );
	else if (mode == 1) fragColor = texture( TEX1, tc );
	
}
