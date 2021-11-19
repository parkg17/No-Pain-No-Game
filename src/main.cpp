#include "cgmath.h"		// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"		// slee's OpenGL utility

#include "background.h"
#include "circle.h"		// Obstacle
#include "model.h"		// model(me)
#include "assimp_loader.h"


//*************************************
// global constants
static const char* window_name = "No Pain No Game";
static const char* mesh_obj = "mesh/bunny.obj";
static const char* vert_shader_path = "shaders/trackball.vert";
static const char* frag_shader_path = "shaders/trackball.frag";

static const uint	NUM_LONGITUDE_EDGE = 72;		// Edges in longitude for making sphere
static const uint	NUM_LATITUDE_EDGE = 36;		// Edges in latitude  for making sphere
static const uint	MIN_CIRCLES = 20;		// Minimum Circles Number
static const uint	MAX_CIRCLES = 512;		// Maximum Circles Number
uint				NUM_CIRCLES = 32;		// My Initial Circles Number
uint				NUM_TESS = 36;		// My Tessellation factor
uint				index_total_size = 0;
uint				is_move = 3;

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size
//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3(0, 0, 3);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 1, 0);
	mat4	view_matrix = mat4::look_at(eye, at, up);
	
	float	fovy = PI / 4.0f; // must be in radian
	float	aspect;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};

struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 0.5f, 1.0f);   // position light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 100.0f;
};

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
double	last_time;						// time variable to run the program by time not frame unit.
bool	b_solid_color = true;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif

struct {
	bool add = false;
	bool sub = false;
	operator bool() const {
		return add || sub;
	}
} b; // flags of keys for smooth changes

//*************************************
// scene objects
mesh2*		pMesh = nullptr;
camera		cam;
light_t		light;
material_t	material;

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices
std::vector<vertex>	unit_model_vertices;	// host-side vertices

auto	circles = std::move(create_circles(NUM_CIRCLES));
auto	models = std::move(create_models());
auto	backgrounds = std::move(create_backgrounds());

//*************************************
void update()
{
	// update global simulation parameter
	t = float(glfwGetTime()) * 0.4f;

	cam.aspect = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);
	uloc = glGetUniformLocation(program, "b_solid_color");	if (uloc > -1) glUniform1i(uloc, b_solid_color);

	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

	glActiveTexture(GL_TEXTURE0);
}

void render()
{
	/* Computes movements based on time difference */
	double current_time = glfwGetTime();
	if (current_time - last_time < 0.0005) return;
	else {
		last_time = current_time;
	}

	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// notify GL that we use our own program
	glUseProgram(program);

	// bind vertex array object
	glBindVertexArray(pMesh->vertex_array);
	
	for (size_t k = 0, kn = pMesh->geometry_list.size(); k < kn; k++) {
		geometry& g = pMesh->geometry_list[k];
		
		if (g.mat->textures.diffuse) {
			glBindTexture(GL_TEXTURE_2D, g.mat->textures.diffuse->id);
			glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0
			glUniform1i(glGetUniformLocation(program, "use_texture"), true);
		}
		else {
			glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
			glUniform1i(glGetUniformLocation(program, "use_texture"), false);
		}

		// render vertices: trigger shader programs to process vertex data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh->index_buffer);
		glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
	}

	glBindVertexArray(vertex_array);
	
	// Obtacle 
	glBindVertexArray(vertex_array);
	for (auto& c : circles)
	{
		// per-circle update
		c.update(t);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, c.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, c.model_matrix);

		// per-circle draw calls
		if (b_index_buffer)	glDrawElements(GL_TRIANGLES, index_total_size, GL_UNSIGNED_INT, nullptr);
		else				glDrawArrays(GL_TRIANGLES, 0, index_total_size); // NUM_TESS = N
	}

	
	for (auto& m : models)
	{
		// per-circle update
		m.move(is_move, t);
		m.check_collision(circles, backgrounds, 0);
		m.update(t);

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, m.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);

		
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press '+/-' to increase/decrease the number of circle (min=%d, max=%d)\n", MIN_CIRCLES, MAX_CIRCLES);
#ifndef GL_ES_VERSION_2_0
	printf("- press 'w' to toggle wireframe\n");
#endif
	printf("- press 'r' to reset circles\n");
	printf("\n");
}

std::vector<vertex> create_circle_vertices2(uint N)
{
	std::vector<vertex> v = { { vec3(0), vec3(0,0, -1.0f), vec2(0.5f) } }; // origin
	for (uint k = 0; k <= N; k++)
	{
		float t = PI * 2.0f * k / float(N), c = cos(t), s = sin(t);
		v.push_back({ vec3(c,s,0), vec3(0,0,-1.0f), vec2(c,s) * 0.5f + 0.5f });
	}
	return v;
}

std::vector<vertex> create_circle_vertices(uint M, uint N)
{
	std::vector<vertex> v;
	for (uint k = 0; k <= M; ++k) {
		float t1 = PI * 2.0f * k / float(M);
		float cos_phi = cos(t1), sin_phi = sin(t1);

		for (uint j = 0; j <= N; ++j) {
			float t2 = PI * 1.0f * j / float(N);
			float cos_theta = cos(t2), sin_theta = sin(t2);
			v.push_back({ vec3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta),	// Position Vector
				vec3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta),				// Normal Vector
				vec2(t1 / (PI * 2.0f), 1.0f - t2 / (PI * 1.0f)) });						// Texture Coordinates
		}
	}
	return v;
}

void update_vertex_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	//if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	//if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	/* Reuse My Sphere Code */
	// Making Sphere using index buffer
	uint v1, v2;
	std::vector<uint> indices;
	for (uint j = 0; j < NUM_LONGITUDE_EDGE; ++j) {
		v1 = j * (NUM_LATITUDE_EDGE + 1);
		v2 = v1 + (NUM_LATITUDE_EDGE + 1);
		for (uint i = 0; i < NUM_LATITUDE_EDGE; ++i) {
			/* the Vertices connected in the Counter-Clockwise Order */
			if (i != 0) { // Excluding top point
				indices.push_back(v1);
				indices.push_back(v1 + 1);
				indices.push_back(v2);
			}
			if (i != (NUM_LATITUDE_EDGE - 1)) { // Excluding bottom point
				indices.push_back(v2);
				indices.push_back(v1 + 1);
				indices.push_back(v2 + 1);
			}
			++v1;
			++v2;
		}
	}

	index_total_size = indices.size();

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);


	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

/*
void update_vertex_buffer(const std::vector<vertex>& vertices, uint N)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if (b_index_buffer)
	{
		std::vector<uint> indices;
		for (uint k = 0; k < N; k++)
		{
			indices.push_back(0);	// the origin
			indices.push_back(k + 1);
			indices.push_back(k + 2);
		}

		// generation of vertex buffer: use vertices as it is
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);
	}
	else
	{
		std::vector<vertex> v; // triangle vertices
		for (uint k = 0; k < N; k++)
		{
			v.push_back(vertices.front());	// the origin
			v.push_back(vertices[k + 1]);
			v.push_back(vertices[k + 2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertices
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * v.size(), &v[0], GL_STATIC_DRAW);
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}*/

/* New Function that Changing the nunber of Circles */
void update_circles()
{
	uint n = NUM_CIRCLES;
	if (b.add) {
		++n;
		b.add = false;
	}
	else if (b.sub) {
		--n;
		b.sub = false;
	}
	if (n == NUM_CIRCLES || n<MIN_CIRCLES || n>MAX_CIRCLES) return;

	circles.clear();
	circles = std::move(create_circles(NUM_CIRCLES = n));
	printf("> number of circles = %-4d\r", NUM_CIRCLES);
	return;
}

void reset_circles()
{
	circles.clear();
	circles = std::move(create_circles(NUM_CIRCLES));
	return;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)))	b.add = true;
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) b.sub = true;
		else if (key == GLFW_KEY_I)
		{
			b_index_buffer = !b_index_buffer;
			update_vertex_buffer(unit_circle_vertices);
			printf("> using %s buffering\n", b_index_buffer ? "index" : "vertex");
		}

		else if (key == GLFW_KEY_R) // Added key with Reset fucntion
		{
			//reset_circles();
			printf("> reset circles\n");
		}
#ifndef GL_ES_VERSION_2_0
		else if (key == GLFW_KEY_P)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
#endif
		else if (key == GLFW_KEY_W) { // Upward
			is_move = 0;
			//move_model(0);
		}
		else if (key == GLFW_KEY_A) { // Left
			is_move = 1;
			//move_model(1);
		}
		else if (key == GLFW_KEY_D) { // Right
			is_move = 2;
			//move_model(2);
		}
		else if (key == GLFW_KEY_S) { // Right
			is_move = 4;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		is_move = 3;
		if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)))	b.add = false;
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) b.sub = false;
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		printf("> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y));
	}
}

void motion(GLFWwindow* window, double x, double y)
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth(1.0f);
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_CULL_FACE);								// turn on backface culling
	glEnable(GL_DEPTH_TEST);								// turn on depth tests
	glActiveTexture(GL_TEXTURE0);

	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices(NUM_LONGITUDE_EDGE, NUM_LATITUDE_EDGE));
	//unit_model_vertices = std::move(create_circle_vertices(NUM_LONGITUDE_EDGE, NUM_LATITUDE_EDGE));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer(unit_circle_vertices);
	//update_vertex_buffer(unit_model_vertices);

	// load the mesh
	pMesh = load_model(mesh_obj);
	if (pMesh == nullptr) { printf("Unable to load mesh\n"); return false; }

	last_time = glfwGetTime();

	return true;
}

void user_finalize()
{
	delete_texture_cache();
	delete pMesh;
}

int main(int argc, char* argv[])
{
	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }	// create and compile shaders/program
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
	glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
	glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
	glfwSetCursorPosCallback(window, motion);		// callback for mouse movements

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}