#pragma once
#ifndef __OBSTACLE_H__
#define __OBSTACLE_H__

float size_factor = 0.002f;

struct obstacle_t
{
	vec3	center = vec3(0);	// 2D position for translation
	float	radius = 1.0f;		// radius
	float	theta = 0.0f;		// rotation angle
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<obstacle_t> create_obstacles()
{
	int num_cnt = 0;
	std::vector<obstacle_t> obstacles;
	std::vector<obstacle_t> o(4);

	o[num_cnt++] = { vec3(-1.7f, -1.0f, 0), 0.1f * size_factor};
	o[num_cnt++] = { vec3(-0.5f, -0.4f, 0), 0.1f * size_factor};
	o[num_cnt++] = { vec3(0.3f, -1.0f, 0), 0.2f * size_factor};
	o[num_cnt++] = { vec3(0.4f, 0.6f, 0), 0.1f * size_factor};
	for (int idx = 0; idx < num_cnt; ++idx) {
		obstacles.emplace_back(o[idx]);
	}

	return obstacles;
}

inline void obstacle_t::update(float t)
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
