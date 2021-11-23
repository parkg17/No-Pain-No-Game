#pragma once
#ifndef __ENEMY_H__
#define __ENEMY_H__



struct enemy_t
{
	vec3	center = vec3(0);	// 2D position for translation
	float	radius = 1.0f;		// radius
	float	theta = 0.0f;		// rotation angle
	vec4	color;				// RGBA color in [0,1]
	vec2	velocity;			// circle's velocity 
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

inline std::vector<enemy_t> create_enemies()
{
	float esf = 0.01f; // enemy_size_factor
	int num_cnt = 0;
	std::vector<enemy_t> enemies;
	std::vector<enemy_t> c(2);

	c[num_cnt++] = { vec3(-1.2f, -1.0f, 0), esf};
	c[num_cnt++] = { vec3(0.8f, -1.0f, 0), esf};
	for (int idx = 0; idx < num_cnt; ++idx) {
		enemies.emplace_back(c[idx]);
	}

	return enemies;
}

inline void enemy_t::update(float t)
{
	float t_x = 0.2f * cos(2 * t);
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x + t_x,
		0, 1, 0, center.y,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * scale_matrix;
}
#endif
