// input attributes of vertices
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

out vec3 tc;

// uniform variables
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
	tc = position;
	
	vec4 pos = vec4(position,1);
	gl_Position = projection_matrix * mat4(mat3(view_matrix)) * pos;
}
