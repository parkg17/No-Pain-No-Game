#pragma once
#ifndef __GOAL_H__
#define __GOAL_H__

float gsf = 0.1f; // goal_size_factor

struct goal_t
{
	vec3	center = vec3(0);	// 2D position for translation
	float	radius = 1.0f;		// radius
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<goal_t> create_goal()
{
	int num_cnt = 0;
	std::vector<goal_t> goal;
	std::vector<goal_t> g(1);

	g[num_cnt++] = { vec3(2.0f, -1.0f, 0), gsf };
	for (int idx = 0; idx < num_cnt; ++idx) {
		goal.emplace_back(g[idx]);
	}

	return goal;
}

inline void goal_t::update(float t)
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
