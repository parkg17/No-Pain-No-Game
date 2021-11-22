#include "cgmath.h"			// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut2.h"			// slee's OpenGL utility
#include "background.h"
#include "obstacle.h"		// Obstacle
#include "block.h"			// blocks
#include "enemy.h"			// enemy
#include "goal.h"			// goal
#include "model.h"			// model(player)
#include "assimp_loader.h"
#include "light.h"			// light
#include "material.h"		// material
#include "camera.h"			// camera
#include "skybox.h"

//*************************************
// global constants - shader
static const char* vert_shader_path = "shaders/trackball.vert";
static const char* frag_shader_path = "shaders/trackball.frag";
static const char* window_name = "No Pain No Game";

//*************************************
// global constants - object path
static const char* background_path = "images/background.jpg";
static const char* thorn_obj = "mesh/scrubPine.obj";
static const char* player_obj = "mesh/player/bunny.obj";
static const char* cube_obj = "mesh/block/cube.obj";
static const char* enemy_obj = "mesh/enemy/re-optimized sphere.obj";
static const char* goal_obj = "mesh/goal/sphere-cubecoords.obj";

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	bg_vertex_array = 0;	// ID holder for vertex array object
GLuint	Background = 0;

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
double	last_time;						// time variable to run the program by time not frame unit.
bool	b_index_buffer = true;			// use index buffering?
uint	is_move = 3;
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif

//*************************************
// scene objects
mesh2* pMesh_player = nullptr;
mesh2* pMesh_thorn = nullptr;
mesh2* pMesh_cube = nullptr;
mesh2* pMesh_enemy = nullptr;
mesh2* pMesh_goal = nullptr;
material_t	material;
light_t		light;
camera		cam;
skybox		sky;

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_background_vertices;
auto	obstacles = std::move(create_obstacles());
auto	models = std::move(create_models());
auto	enemies = std::move(create_enemies());
auto	blocks = std::move(create_blocks());
auto	backgrounds = std::move(create_backgrounds());
auto	goal = std::move(create_goal());

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

	sky.render(cam.view_matrix, cam.projection_matrix);

	// notify GL that we use our own program
	glUseProgram(program);

	// Background
	for (auto& bg : backgrounds)
	{
		bg.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, bg.model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Background);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glUniform1i(glGetUniformLocation(program, "TEX"), 0);
		glBindVertexArray(bg_vertex_array);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	// Obtacle 
	for (auto& o : obstacles)
	{
		o.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, o.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, o.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glBindVertexArray(pMesh_thorn->vertex_array);
		for (size_t k = 0, kn = pMesh_thorn->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_thorn->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_thorn->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

	// enemies
	for (auto& en : enemies)
	{
		en.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, en.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, en.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glBindVertexArray(pMesh_enemy->vertex_array);
		for (size_t k = 0, kn = pMesh_enemy->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_enemy->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_enemy->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

	// goal
	for (auto& g : goal)
	{
		g.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, g.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glBindVertexArray(pMesh_goal->vertex_array);
		for (size_t k = 0, kn = pMesh_goal->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_goal->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_goal->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

	// blocks
	for (auto& b : blocks)
	{
		b.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, b.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, b.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glBindVertexArray(pMesh_cube->vertex_array);
		for (size_t k = 0, kn = pMesh_cube->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_cube->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_cube->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

	// Player
	for (auto& m : models)
	{
		if (!m.is_stop) {
			m.move(is_move, t);
			m.check_collision(obstacles, enemies, goal, backgrounds, 0);
			m.update(t);
		}
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, m.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 0);
		glBindVertexArray(pMesh_player->vertex_array);
		for (size_t k = 0, kn = pMesh_player->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_player->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_player->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
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
	printf("- press 'r' to reset circles\n");
	printf("\n");
}

std::vector<vertex> create_background()
{
	return {};
	float bg_size = 1.0f;
	std::vector<vertex> v = { { vec3(0), vec3(0, 0, 1.0f), vec2(0.5f) } }; // origin
	v.push_back({ vec3(bg_size * 2,0,0), vec3(0, 0,1.0f), vec2(0.5f) });
	v.push_back({ vec3(bg_size * 2,bg_size,0), vec3(0, 0,1.0f), vec2(0.5f) });
	v.push_back({ vec3(0,bg_size,0), vec3(0, 0, 1.0f), vec2(0.5f) });
	return v;
}

void bg_update_vertex_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	/* Reuse My Sphere Code */
	// Making Sphere using index buffer
	std::vector<uint> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);


	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);


	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (bg_vertex_array) glDeleteVertexArrays(1, &bg_vertex_array);
	bg_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!bg_vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void reset_game()
{
	for (auto& m : models) {
		m.center = vec3(-2.0f, -1.0f, 0);
		m.is_stop = false;
	}
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_R) // Added key with Reset fucntion
		{
			reset_game();
			printf("> reset game\n");
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
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	// modeling background backbone
	Background = cg_create_texture(background_path, true); if (!Background) return false;
	unit_background_vertices = std::move(create_background());
	bg_update_vertex_buffer(unit_background_vertices);

	sky.init();
	sky.load();

	// load the mesh
	pMesh_player = load_model(player_obj);
	if (pMesh_player == nullptr) { printf("Unable to load mesh\n"); return false; }
	pMesh_thorn = load_model(thorn_obj);
	if (pMesh_thorn == nullptr) { printf("Unable to load mesh\n"); return false; }
	pMesh_cube = load_model(cube_obj);
	if (pMesh_cube == nullptr) { printf("Unable to load mesh\n"); return false; }
	pMesh_enemy = load_model(enemy_obj);
	if (pMesh_enemy == nullptr) { printf("Unable to load mesh\n"); return false; }
	pMesh_goal = load_model(goal_obj);
	if (pMesh_goal == nullptr) { printf("Unable to load mesh\n"); return false; }

	last_time = glfwGetTime();

	return true;
}

void user_finalize()
{
	delete_texture_cache();
	delete pMesh_player;
	delete pMesh_thorn;
	delete pMesh_cube;
	delete pMesh_enemy;
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