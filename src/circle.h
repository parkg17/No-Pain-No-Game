#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

float check_overlap_array[512][512];

/* Added a new argument velocity */
struct circle_t
{
	vec3	center = vec3(0);		// 2D position for translation
	float	radius = 1.0f;		// radius
	float	theta = 0.0f;			// rotation angle
	vec4	color;				// RGBA color in [0,1]
	vec2	velocity;			// circle's velocity 
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

/* Create n Circles with Random Position */
inline std::vector<circle_t> create_circles(uint n)
{
	int num_cnt = 0;
	std::vector<circle_t> circles;
	std::vector<circle_t> c(4);

	/* Declare 9 Planets */
	c[num_cnt++] = { vec3(-1.7f, -1.0f, 0), 0.1f, 0.0f};
	c[num_cnt++] = { vec3(-0.5f, -0.4f, 0), 0.1f, 0.1f };
	c[num_cnt++] = { vec3(0.3f, -1.0f, 0), 0.2f, 0.1f };
	c[num_cnt++] = { vec3(0.4f, 1.0f, 0), 0.1f, 0.1f };
	for (int idx = 0; idx < num_cnt; ++idx) {
		circles.emplace_back(c[idx]);
	}

	return circles;
}

inline void circle_t::update(float t)
{
	const float limit_x = (float)1.777; // Window Width length
	const float limit_y = (float)1.000; // Window Height length

	/* Changing Position by each velocity */
	center[0] += velocity[0];
	center[1] += velocity[1];

	/* Window Collision Case */
	if (center[0] + radius >= limit_x) {
		velocity[0] = -abs(velocity[0]);
	}
	else if (center[0] - radius <= -limit_x) {
		velocity[0] = abs(velocity[0]);
	}
	if (center[1] + radius >= limit_y) {
		velocity[1] = -abs(velocity[1]);
	}
	else if (center[1] - radius <= -limit_y) {
		velocity[1] = abs(velocity[1]);
	}

	// these transformations will be explained in later transformation lecture
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
