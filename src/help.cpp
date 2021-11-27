#include "cgmath.h"			// slee's simple math library
#include "cgut2.h"			// slee's OpenGL utility

#define STB_TRUETYPE_IMPLEMENTATION
// Obstacle


//*************************************
// global constants - image path
static const char* help_image_path_1 = "images/light.jpg";
static const char* help_image_path_2 = "images/dark.jpg";
static const char* win_lose_image_path_1 = "images/example.jpg";


GLuint	program_help = 0;		// ID holder for GPU program
GLuint  vertex_array_help = 0;	// ID holder for vertex array object

GLuint	help_img_1 = 0;
GLuint	help_img_2 = 0;
GLuint	win_lose_img_1 = 0;

float dpi_scale = cg_get_dpi_scale();
float blinking = 0;

static const char* vert_help_shader_path = "shaders/texture.vert";
static const char* frag_help_shader_path = "shaders/texture.frag";

void render_text(std::string text, GLint x, GLint y, GLfloat scale, vec4 color, GLfloat dpi_scale = 1.0f);
void render_help_img();
void render_win_lose_img();

void render_win(int level)
{	
	std::string text = std::to_string(level + 1);
	if (level == 1) {
		text = " Clear!";
	}
	else {		
		std::string next_ = "Level ";
		text = next_ + text;
	}
	
	render_text(text, 470, 360, 1.3f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
	render_win_lose_img();
}
void render_lose()
{	
	render_text("Play Again!", 430, 140, 1.3f, vec4(1.0f, 0.3f, 0.2f, 1.0f), dpi_scale);
	
}

void render_help()
{	
	
	render_text("No Pain No Game!", 100, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
	render_text("W to Jump A to move left D to move right", 100, 200, 0.6f, vec4(1.0f, 0.8f, 0.6f, 0.7f),
		dpi_scale);
	render_text("First time, You should remember the obstacles on the map! ", 100, 300, 0.6f, vec4(1.0f, 0.8f, 0.6f, 0.7f),
		dpi_scale);
	render_text("Second time, You will pass through the map to avoid invisible obstacles.", 100, 400, 0.6f, vec4(1.0f, 0.8f, 0.6f, 0.7f),
		dpi_scale);
	render_text("Press F1 to title", 850, 100, 0.6f, vec4(0.5f, 0.7f, 0.7f, 1.0f), dpi_scale);
	render_help_img();
	
}

void render_title()
{
	blinking = abs(sin(float(glfwGetTime()) * 2.5f));
	render_text("No Pain No Game!", 100, 100, 1.0f, vec4(0.5f, 0.8f, 0.2f, 1.0f), dpi_scale);
	render_text("Press F1 to show help", 100, 200, 0.6f, vec4(1.0f, 0.8f, 0.1f, 0.4f), dpi_scale);
	render_text("Press Enter to start game", 100, 600, 0.6f, vec4(0.5f, 0.7f, 0.7f, blinking), dpi_scale);
}

void render_help_img()
{
	// clear screen (with background color) and clear depth buffer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind program
	glUseProgram(program_help);

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, help_img_1);
	glUniform1i(glGetUniformLocation(program_help, "MODE"), 0);
	glUniform1i(glGetUniformLocation(program_help, "TEX0"), 0);
	// bind vertex array object
	
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, help_img_2);
	glUniform1i(glGetUniformLocation(program_help, "MODE"), 1);
	glUniform1i(glGetUniformLocation(program_help, "TEX1"), 0);
	
	// bind vertex array object
	glBindVertexArray(vertex_array_help);

	// render quad vertices
	glDrawArrays(GL_TRIANGLES, 0, 12);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	
}
void render_win_lose_img()
{
	glUseProgram(program_help);

	// bind textures

	glActiveTexture(GL_TEXTURE0);	
	glBindTexture(GL_TEXTURE_2D, win_lose_img_1);
	glUniform1i(glGetUniformLocation(program_help, "MODE"), 0);
	glUniform1i(glGetUniformLocation(program_help, "TEX0"), 0);	

	glActiveTexture(GL_TEXTURE1);	
	glBindTexture(GL_TEXTURE_2D, win_lose_img_1);
	glUniform1i(glGetUniformLocation(program_help, "MODE"), 1);
	glUniform1i(glGetUniformLocation(program_help, "TEX1"), 1);
	
	glBindVertexArray(vertex_array_help);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	// render quad vertices
	
}

GLuint create_texture(const char* image_path, bool mipmap = true, GLenum wrap = GL_CLAMP_TO_EDGE, GLenum filter = GL_LINEAR)
{
	// load image
	image* i = cg_load_image(image_path); if (!i) return 0; // return null texture; 0 is reserved as a null texture
	int		w = i->width, h = i->height, c = i->channels;

	// induce internal format and format from image
	GLint	internal_format = c == 1 ? GL_R8 : c == 2 ? GL_RG8 : c == 3 ? GL_RGB8 : GL_RGBA8;
	GLenum	format = c == 1 ? GL_RED : c == 2 ? GL_RG : c == 3 ? GL_RGB : GL_RGBA;

	// create a src texture (lena texture)
	GLuint texture;
	glGenTextures(1, &texture); if (texture == 0) { printf("%s(): failed in glGenTextures()\n", __func__); return 0; }
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, i->ptr);
	if (i) { delete i; i = nullptr; } // release image

	// build mipmap
	if (mipmap)
	{
		int mip_levels = 0; for (int k = w > h ? w : h; k; k >>= 1) mip_levels++;
		for (int l = 1; l < mip_levels; l++)
			glTexImage2D(GL_TEXTURE_2D, l, internal_format, (w >> l) == 0 ? 1 : (w >> l), (h >> l) == 0 ? 1 : (h >> l), 0, format, GL_UNSIGNED_BYTE, nullptr);
		if (glGenerateMipmap) glGenerateMipmap(GL_TEXTURE_2D);
	}

	// set up texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, !mipmap ? filter : filter == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);

	return texture;
}

bool init_help() 
{
	vertex corners[8];
	corners[0].pos = vec3(0.05f, -0.85f, 0.0f);		corners[0].tex = vec2(0.0f, 0.0f); // left down
	corners[1].pos = vec3(+0.65f, -0.85f, 0.0f);	corners[1].tex = vec2(1.0f, 0.0f); // right down
	corners[2].pos = vec3(0.65f, -0.25f, 0.0f);		corners[2].tex = vec2(1.0f, 1.0f); // right up
	corners[3].pos = vec3(0.05f, -0.25f, 0.0f);		corners[3].tex = vec2(0.0f, 1.0f); // left up // right box
	corners[4].pos = vec3(-0.75f, -0.85f, 0.0f);	corners[4].tex = vec2(0.0f, 0.0f);
	corners[5].pos = vec3(-0.15f, -0.85f, 0.0f);	corners[5].tex = vec2(1.0f, 0.0f);
	corners[6].pos = vec3(-0.15f, -0.25f, 0.0f);	corners[6].tex = vec2(1.0f, 1.0f);
	corners[7].pos = vec3(-0.75f, -0.25f, 0.0f);	corners[7].tex = vec2(0.0f, 1.0f); // left box
	vertex vertices[12] = { corners[0], corners[1], corners[2], corners[0], corners[2], corners[3], corners[4], corners[5], corners[6], corners[4], corners[6], corners[7] };
	//vertex vertices[6] = { corners[1], corners[0], corners[3] , corners[1], corners[3], corners[2] };
	
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	  
	program_help = cg_create_program(vert_help_shader_path, frag_help_shader_path);
	if (!program_help) return false;

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array_help) glDeleteVertexArrays(1, &vertex_array_help);
	vertex_array_help = cg_create_vertex_array(vertex_buffer);
	if (!vertex_array_help) { printf("%s(): failed to create vertex aray\n", __func__); return false; }

	help_img_1 = create_texture(help_image_path_1, true); if (!help_img_1) return false;
	help_img_2 = create_texture(help_image_path_2, true); if (!help_img_2) return false;
	win_lose_img_1 = create_texture(win_lose_image_path_1, true); if (!win_lose_img_1) return false;

	return true;
}