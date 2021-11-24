/*
	init(), load()는 user_init()에서 한 번만 해주면 됩니다.
	render()는 역시 main.cpp의 render()에서 계속 실행해주면 되는데, 아래의 순서만 지키면 됩니다.
	먼저 Clear한 뒤 skybox를 render하고 그 다음에 다른 program을 사용하면 됩니다.

	1. glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	2. skybox.render();
	3. glUseProgram(program);
*/

//    7----6
//   /|   /|
//  4----5 |
//  | 3--|-2
//  |/   |/
//  0----1

// 0 vec3(-1.0f, -1.0f, +1.0f)
// 1 vec3(+1.0f, -1.0f, +1.0f)
// 2 vec3(+1.0f, -1.0f, -1.0f)
// 3 vec3(-1.0f, -1.0f, -1.0f)
// 4 vec3(-1.0f, +1.0f, +1.0f)
// 5 vec3(+1.0f, +1.0f, +1.0f)
// 6 vec3(+1.0f, +1.0f, -1.0f)
// 7 vec3(-1.0f, +1.0f, -1.0f)

#pragma once

#include "cgmath.h"
#include "cgut.h"

class skybox {
	// right, left, top, bot, back, front
	const std::array<std::string, 6> skybox_texture_path = {
		"../bin/textures/skybox/side2.jpg",
		"../bin/textures/skybox/side3.jpg",
		"../bin/textures/skybox/top.jpg",
		"../bin/textures/skybox/bot.jpg",
		"../bin/textures/skybox/side0.jpg",
		"../bin/textures/skybox/side1.jpg" };

	static constexpr char* vert_shader_path = "../bin/shaders/skybox.vert";
	static constexpr char* frag_shader_path = "../bin/shaders/skybox.frag";

public:
	void init() {
		program = cg_create_program(vert_shader_path, frag_shader_path);

		float sv[] = {
			// positions
			+1.0f, +1.0f, -1.0f, // 6
			-1.0f, +1.0f, -1.0f, // 7
			-1.0f, -1.0f, -1.0f, // 3
			-1.0f, -1.0f, -1.0f, // 3
			+1.0f, -1.0f, -1.0f, // 2
			+1.0f, +1.0f, -1.0f, // 6

			-1.0f, -1.0f, +1.0f, // 0
			-1.0f, -1.0f, -1.0f, // 3
			-1.0f, +1.0f, -1.0f, // 7
			-1.0f, +1.0f, -1.0f, // 7
			-1.0f, +1.0f, +1.0f, // 4
			-1.0f, -1.0f, +1.0f, // 0

			+1.0f, -1.0f, -1.0f, // 2
			+1.0f, -1.0f, +1.0f, // 1
			+1.0f, +1.0f, +1.0f, // 5
			+1.0f, +1.0f, +1.0f, // 5
			+1.0f, +1.0f, -1.0f, // 6
			+1.0f, -1.0f, -1.0f, // 2

			-1.0f, -1.0f, +1.0f, // 0
			-1.0f, +1.0f, +1.0f, // 4
			+1.0f, +1.0f, +1.0f, // 5
			+1.0f, +1.0f, +1.0f, // 5
			+1.0f, -1.0f, +1.0f, // 1
			-1.0f, -1.0f, +1.0f, // 0

			-1.0f, +1.0f, -1.0f, // 7
			+1.0f, +1.0f, -1.0f, // 6
			+1.0f, +1.0f, +1.0f, // 5
			+1.0f, +1.0f, +1.0f, // 5
			-1.0f, +1.0f, +1.0f, // 4
			-1.0f, +1.0f, -1.0f, // 7

			-1.0f, -1.0f, -1.0f,  // 3
			-1.0f, -1.0f, +1.0f,  // 0
			+1.0f, -1.0f, -1.0f,  // 2
			+1.0f, -1.0f, -1.0f,  // 2
			-1.0f, -1.0f, +1.0f,  // 0
			+1.0f, -1.0f, +1.0f }; // 1

		std::array<vertex, sizeof(sv) / sizeof(sv[0]) / 3> vertices;
		for (size_t i = 0; i != vertices.size(); ++i) {
			vertices[i].pos = vec3(sv[i * 3], sv[3 * i + 1], sv[3 * i + 2]);
		}

		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		vertex_array = cg_create_vertex_array(vertex_buffer);
	}

	void load() {
		glGenTextures(1, &cube_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);

		for (int i = 0; i != 6; ++i) {
			auto img = cg_load_image(skybox_texture_path[i].c_str());
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->ptr);
			delete img;
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	void render(mat4& viewMatrix, mat4& projMatrix) {
		glDepthMask(GL_FALSE);

		glUseProgram(program);

		glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"), 1, GL_TRUE, viewMatrix);
		glUniformMatrix4fv(glGetUniformLocation(program, "projection_matrix"), 1, GL_TRUE, projMatrix);

		glBindVertexArray(vertex_array);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
		glUniform1i(glGetUniformLocation(program, "skybox"), 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glDepthMask(GL_TRUE);
	}

private:
	GLuint vertex_buffer{};
	GLuint vertex_array{};
	GLuint program = {};
	GLuint cube_map = {};
};