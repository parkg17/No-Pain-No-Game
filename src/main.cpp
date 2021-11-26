#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "assimp_loader.h"
#include "background.h"
#include "block.h"    // blocks
#include "camera.h"   // camera
#include "cgmath.h"   // slee's simple math library
#include "cgut2.h"    // slee's OpenGL utility
#include "enemy.h"    // enemy
#include "goal.h"     // goal
#include "light.h"    // light
#include "material.h" // material
#include "model.h"    // model(player)
#include "object.h"
#include "obstacle.h" // Obstacle
#include "player.h"
#include "skybox.h" // skybox
#include "trackball.h"

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
// global constants - constant
constexpr float TILE_SCALE = 0.05f;
constexpr int TILE_X = 3, TILE_Y = 6;

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2 window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint program = 0;         // ID holder for GPU program
GLuint bg_vertex_array = 0; // ID holder for vertex array object
GLuint Background = 0;

// background_t backgrounds;

//*************************************
// global variables
int frame = 0;  // index of rendering frames
float t = 0.0f; // current simulation parameter
float little_time;
double last_time;           // time variable to run the program by time not frame unit.
bool b_index_buffer = true; // use index buffering?
bool is_game = false;
bool is_help = false;

bool press_shift, press_ctrl;

#ifndef GL_ES_VERSION_2_0
bool b_wireframe = false;
#endif

//*************************************
// scene objects
mesh2* pMesh_player = nullptr;
mesh2* pMesh_thorn = nullptr;
mesh2* pMesh_cube = nullptr;
mesh2* pMesh_enemy = nullptr;
mesh2* pMesh_goal = nullptr;
material_t material;
light_t light;
camera cam;
skybox sky;
object_t spike, tile, flag;
player_t player;
trackball tb;

std::vector<GLfloat> spike_loc;
GLfloat flag_loc;

int level = 0;

//*************************************
// render function
void render_bg(float t, std::vector<background_t>& background, GLuint program, GLuint Background, GLuint bg_vertex_array);
void render_obstacle(float t, std::vector<obstacle_t>& obstacles, GLuint program, mesh2* pMesh_thorn);
void render_enemy(float t, std::vector<enemy_t>& enemies, GLuint program, mesh2* pMesh_enemy);
void render_goal(float t, std::vector<goal_t>& goal, GLuint program, mesh2* pMesh_goal);
void render_block(float t, std::vector<block_t>& blocks, GLuint program, mesh2* pMesh_cube);
void render_player(float t, std::vector<background_t>& backgrounds, std::vector<enemy_t>& enemies, std::vector<obstacle_t>& obstacles, std::vector<goal_t>& goal, std::vector<model_t>& models, GLuint program, mesh2* pMesh_player);

bool init_text();
bool init_help();
void render_help();
void render_title();

//*************************************
// holder of vertices and indices of a unit circle
// std::vector<vertex> unit_background_vertices;
// auto obstacles = std::move(create_obstacles());
// auto models = std::move(create_models());
// auto enemies = std::move(create_enemies());
// auto blocks = std::move(create_blocks());
// auto backgrounds = std::move(create_backgrounds());
// auto goal = std::move(create_goal());

//*************************************
void update() {
    const auto current_time = glfwGetTime();
    const auto delta_time = static_cast<float>(current_time - std::exchange(last_time, current_time));

    player.update(delta_time);
    cam.update(player.get_x_loc());

    glUseProgram(program);

    // update global simulation parameter
    t = float(glfwGetTime()) * 0.4f;

    cam.aspect = window_size.x / float(window_size.y);
    cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);

    // update common uniform variables in vertex/fragment shaders
    glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"), 1, GL_TRUE, cam.view_matrix);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection_matrix"), 1, GL_TRUE, cam.projection_matrix);

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

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (is_game == true && is_help == false) {
        sky.render(cam.view_matrix, cam.projection_matrix);
    }
    float dpi_scale = cg_get_dpi_scale();

    // notify GL that we use our own program
    glUseProgram(program);
    if (is_game == true && is_help == false) {
        // render_bg(t, backgrounds, program, Background, bg_vertex_array);
        player.render(program);
        {
            for (const auto& x : spike_loc) {
                spike.set_location(x);
                spike.render(program);
            }
        }
        for (int x = -TILE_X; x <= TILE_X; ++x) {
            tile.set_location(vec3(1 / TILE_SCALE * x, -10, -50));
            tile.render(program);
            tile.set_location(vec3(1 / TILE_SCALE * x, -10, -30));
            tile.render(program);
        }
        flag.render(program);
    } else {
        if (is_help) render_help();
        else
            render_title();
    }

    glfwSwapBuffers(window);
}

void restart_level() {
    player.set_location(vec3(-20, -10, -25));
    switch (level) {
    case 0: spike_loc = {-10, 0, 30}; break;
    case 1: spike_loc = {-10, 0, 20, 35}; break;
    case 2: spike_loc = {}; break;
    default: assert(false);
    }
}

void reshape(GLFWwindow* window, int width, int height) {
    window_size = ivec2(width, height);
    glViewport(0, 0, width, height);
}

void print_help() {
    printf("[help]\n");
    printf("- press ESC or 'q' to terminate the program\n");
    printf("- press F1 or 'h' to see help\n");
    printf("- press 'r' to reset circles\n");
    printf("\n");
}

std::vector<vertex> create_background() {
    float bg_size = 1.0f;
    std::vector<vertex> v = {{vec3(0), vec3(0, 0, 1.0f), vec2(0.5f)}}; // origin
    v.push_back({vec3(bg_size * 2, 0, 0), vec3(0, 0, 1.0f), vec2(0.5f)});
    v.push_back({vec3(bg_size * 2, bg_size, 0), vec3(0, 0, 1.0f), vec2(0.5f)});
    v.push_back({vec3(0, bg_size, 0), vec3(0, 0, 1.0f), vec2(0.5f)});
    return v;
}

void bg_update_vertex_buffer(const std::vector<vertex>& vertices) {
    static GLuint vertex_buffer = 0; // ID holder for vertex buffer
    static GLuint index_buffer = 0;  // ID holder for index buffer

    // clear and create new buffers
    if (vertex_buffer)
        glDeleteBuffers(1, &vertex_buffer);
    vertex_buffer = 0;
    if (index_buffer)
        glDeleteBuffers(1, &index_buffer);
    index_buffer = 0;

    // check exceptions
    if (vertices.empty()) {
        printf("[error] vertices is empty.\n");
        return;
    }

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
    if (bg_vertex_array)
        glDeleteVertexArrays(1, &bg_vertex_array);
    bg_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
    if (!bg_vertex_array) {
        printf("%s(): failed to create vertex aray\n", __func__);
        return;
    }
}

void reset_game() {
    // for (auto& m : models) {
    //     m.center = vec3(-2.0f, -1.0f, 0);
    //     m.is_stop = false;
    // }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)
            glfwSetWindowShouldClose(window, GL_TRUE);
        else if (key == GLFW_KEY_R) { // Added key with Reset fucntion
            // reset_game();
            restart_level();
            printf("> reset game\n");
        }
#ifndef GL_ES_VERSION_2_0
        else if (key == GLFW_KEY_P) {
            b_wireframe = !b_wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
            printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
        }
#endif
        else if (key == GLFW_KEY_W) { // Upward
            player.jump();
        } else if (key == GLFW_KEY_A) { // Left
            player.set_x_dir(-1);
        } else if (key == GLFW_KEY_D) { // Right
            player.set_x_dir(1);
        } else if (key == GLFW_KEY_ENTER && is_help == false) { // start
            is_game = true;
        } else if (key == GLFW_KEY_F1) { // help
            is_help = !is_help;
        } else if (key == GLFW_KEY_HOME) {
            cam = camera();
        } else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            press_shift = true;
        } else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
            press_ctrl = true;
        }
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_A) { // Left
            player.set_x_dir(0);
        } else if (key == GLFW_KEY_D) { // Right
            player.set_x_dir(0);
        } else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            press_shift = false;
        } else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
            press_ctrl = false;
        }
    }
}

void mouse(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        dvec2 pos;
        glfwGetCursorPos(window, &pos.x, &pos.y);
        vec2 npos = cursor_to_ndc(pos, window_size);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (press_shift) {
                tb.begin_zooming(cam.view_matrix, npos);
            } else if (press_ctrl) {
                tb.begin_panning(cam.view_matrix, npos);
            } else {
                tb.begin_tracking(cam.view_matrix, npos);
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            tb.begin_zooming(cam.view_matrix, npos);
        } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            tb.begin_panning(cam.view_matrix, npos);
        }
    } else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
            tb.end();
        }
    }
}

void motion(GLFWwindow* window, double x, double y) {
    if (!tb.is_tracking()) return;
    vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
    cam.view_matrix = tb.update(npos);
}

bool win() {
    return abs(player.get_x_loc() - flag.get_x_loc()) < 1.f;
}

bool lose() {
    if (player.get_location().y > -8.f) return false;
    for (const auto& x : spike_loc) {
        if (fabs(player.get_x_loc() - x) < 3.f) return true;
    }
    return false;
}

bool user_init() {
    // log hotkeys
    print_help();

    // init GL states
    glLineWidth(1.0f);
    glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f); // set clear color
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);  // turn on backface culling
    glEnable(GL_DEPTH_TEST); // turn on depth tests
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // modeling background backbone
    Background = cg_create_texture(background_path, true);
    if (!Background) return false;
    // unit_background_vertices = std::move(create_background());
    // bg_update_vertex_buffer(unit_background_vertices);

    if (!init_text()) return false;
    if (!init_help()) return false;

    sky.init();

    spike.init("../bin/mesh/spike/spike.obj", "../bin/mesh/spike/spike.png");
    spike.set_scale(0.1f);
    spike.set_location(vec3(0, -10, -25));

    tile.init("../bin/mesh/tile/tile.obj", "../bin/mesh/tile/tile.png");
    tile.set_scale(TILE_SCALE);

    flag.init("../bin/mesh/flag/flag.obj", "../bin/mesh/flag/flag.png");
    flag.set_scale(4.f);
    flag.set_rotate(vec3(1, 0, 0), -PI / 2);
    flag.set_location(vec3(60, -10, -25));

    player.init("../bin/mesh/player/player.obj", "../bin/mesh/player/player.jpeg");
    player.set_scale(3.f);
    player.set_rotate(vec3(0, 1, 0), PI / 2);
    player.set_location(vec3(-20, -10, -25));

    last_time = glfwGetTime();

    restart_level();

    return true;
}

void user_finalize() {
    delete_texture_cache();
    delete pMesh_player;
    delete pMesh_thorn;
    delete pMesh_cube;
    delete pMesh_enemy;
}

int main(int argc, char* argv[]) {
    // create window and initialize OpenGL extensions
    if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) {
        glfwTerminate();
        return 1;
    }
    if (!cg_init_extensions(window)) {
        glfwTerminate();
        return 1;
    } // init OpenGL extensions

    // initializations and validations of GLSL program
    if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) {
        glfwTerminate();
        return 1;
    } // create and compile shaders/program
    if (!user_init()) {
        printf("Failed to user_init()\n");
        glfwTerminate();
        return 1;
    } // user initialization

    // register event callbacks
    glfwSetWindowSizeCallback(window, reshape); // callback for window resizing events
    glfwSetKeyCallback(window, keyboard);       // callback for keyboard events
    glfwSetMouseButtonCallback(window, mouse);  // callback for mouse click inputs
    glfwSetCursorPosCallback(window, motion);   // callback for mouse movements

    // enters rendering/event loop
    for (frame = 0; !glfwWindowShouldClose(window); frame++) {
        glfwPollEvents(); // polling and processing of events
        update();         // per-frame update
        render();         // per-frame render

        if (win()) {
            ++level;
            if (level == 3) return 0;
            restart_level();
        } else if (lose()) {
            restart_level();
        }
    }

    // normal termination
    user_finalize();
    cg_destroy_window(window);

    return 0;
}
