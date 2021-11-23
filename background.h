#pragma once
#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

struct background_t
{
	vec3	center = vec3(0);
	float	left = -2.0f;
	float	right = 2.0f;
	float	bottom = -1.0f;
	float	top = 1.0f;
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<background_t> create_backgrounds()
{
	int num_cnt = 1;
	std::vector<background_t> bg;
	std::vector<background_t> b(1);

	/* Declare 9 Planets */
	b[0] = { vec3(-3.7f,-1.8f,-2.0f), -2.0f, 2.0f, -1.1f, 1.1f };
	for (int idx = 0; idx < num_cnt; ++idx) {
		bg.emplace_back(b[idx]);
	}
	return bg;
}

inline void background_t::update(float t)
{
	float size = 4.0f;
	mat4 scale_matrix =
	{
		size, 0, 0, 0,
		0, size, 0, 0,
		0, 0, size, 0,
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