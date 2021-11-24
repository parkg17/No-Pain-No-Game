#pragma once
#ifndef __BLOCK_H__
#define __BLOCK_H__


struct block_t
{
	vec3	center = vec3(0);		// 2D position for translation
	float	radius = 1.0f;		// radius
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

/* Create n Circles with Random Position */
inline std::vector<block_t> create_blocks()
{
	int num_cnt = 0;
	float bcf = 0.25f; // block_size_factor

	std::vector<block_t> blocks;
	std::vector<block_t> b(42);


	// Right
	//b[num_cnt++] = { vec3( 2.3f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f,  0.9f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f,  0.6f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f,  0.3f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f,  0.0f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f, -0.3f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f, -0.6f, 0), bcf };
	b[num_cnt++] = { vec3( 2.3f, -0.9f, 0), bcf };

	// Downward
	//b[num_cnt++] = { vec3( 2.3f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 2.0f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.7f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.4f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.1f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.8f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.5f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.2f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.1f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.4f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.7f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.0f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.3f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.6f, -1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.9f, -1.2f, 0), bcf };
	//b[num_cnt++] = { vec3(-2.2f, -1.2f, 0), bcf };

	// Left
	b[num_cnt++] = { vec3(-2.2f, -0.9f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f, -0.6f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f, -0.3f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f,  0.0f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f,  0.3f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f,  0.6f, 0), bcf };
	b[num_cnt++] = { vec3(-2.2f,  0.9f, 0), bcf };

	// Upward
	//b[num_cnt++] = { vec3(-2.2f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.9f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.6f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.3f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-1.0f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.7f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.4f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3(-0.1f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.2f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.5f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 0.8f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.1f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.4f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 1.7f,  1.2f, 0), bcf };
	b[num_cnt++] = { vec3( 2.0f,  1.2f, 0), bcf };

	for (int idx = 0; idx < num_cnt; ++idx) {
		blocks.emplace_back(b[idx]);
	}

	return blocks;
}

inline void block_t::update(float t)
{
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * scale_matrix;
}
#endif
