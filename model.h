#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

struct model_t
{
	vec3	center = vec3(0);		// 2D position for translation
	vec3	velocity = vec3(0);		// 2D position for translation
	float	radius = 1.0f;			// Radius
	float	theta = 0.0f;			// Individual rotation angle
	float	orbit = 1.0f;			// Distance between the sun and the planet
	uint	is_move = 3;
	bool	is_jumping = false;
	bool	is_stop = false;
	float	acc = 0.0f;
	vec4	color;					// RGBA color in [0,1]
	mat4	model_matrix;			// modeling transformation

	// public functions
	void	check_collision(std::vector<obstacle_t>& circles, std::vector<enemy_t>& enemies,
							std::vector<goal_t>& goal, std::vector<background_t>& bg, int stage);
	void	move(int direction, float t);
	uint	get_move();
	void	set_move(uint n);
	void	update(float t);

};

inline std::vector<model_t> create_models() // Actually, Shphere not circle
{
	std::vector<model_t> models;
	int num_model = 1;
	std::vector<model_t> c(num_model);

	c[0] = { vec3(-2.0f, -1.0f, 0), vec3(0), 0.1f, 0.0f, 0.0f }; 

	for (int idx = 0; idx < num_model; ++idx) {
		models.emplace_back(c[idx]);
	}

	return models;
}

inline void model_t::check_collision(std::vector<obstacle_t>& obstacles, std::vector<enemy_t>& enemies,
									 std::vector<goal_t>& goal, std::vector<background_t>& bg, int stage)
{
	float distance;
	obstacle_t o1;
	enemy_t e1;
	background_t bg1 = bg[stage];
	uint num_obstacles = obstacles.size();
	uint num_enemies = enemies.size();

	// Obstacle
	for (uint idx = 0; idx < num_obstacles; ++idx) {
		o1 = obstacles[idx];
		distance = float(sqrt(pow(o1.center.x - center.x, 2) + pow(o1.center.y - center.y, 2)));

		if((o1.radius + radius) > distance) {	// Overlap Case
			printf("Game Over! Press 'R' key\n");
			velocity.x = 0;
			velocity.y = 0;
			is_stop = true;
			return;
		}
	}

	// Enemies
	for (uint idx = 0; idx < num_enemies; ++idx) {
		e1 = enemies[idx];
		distance = float(sqrt(pow(e1.center.x - center.x, 2) + pow(e1.center.y - center.y, 2)));

		if ((e1.radius + radius) > distance) {	// Overlap Case
			printf("Game Over! Press 'R' key\n");
			velocity.x = 0;
			velocity.y = 0;
			is_stop = true;
			return;
		}
	}

	// Goal
	distance = float(sqrt(pow(goal[0].center.x - center.x, 2) + pow(goal[0].center.y - center.y, 2)));
	if ((goal[0].radius + radius) > distance) {	// Overlap Case
		printf("Game Clear! You Win!\n");
		velocity.x = 0;
		velocity.y = 0;
		is_stop = true;
		return;
	}
	
	// Background
	if ((bg1.left > center.x) && (velocity.x < 0)) {
		velocity.x = 0;
	}
	if ((bg1.right < center.x) && (velocity.x > 0)) {
		velocity.x = 0;
	}
	if ((bg1.top < center.y ) && (velocity.y > 0)) {
		center.y = bg1.top;
	}
	if ((bg1.bottom > center.y) && (velocity.y < 0 )) {
		center.y = bg1.bottom;
		velocity.y = 0;
		is_jumping = false;
	}
}

inline void model_t::move(int direction, float t) {
	if (is_stop) return;
	if (direction == 0) {
		if (!is_jumping) {
			velocity.y = 2.0f;
			is_jumping = true;
		}
	}
	else if (direction == 1) {
		velocity.x = -1.0f;
	}
	else if (direction == 2) {
		velocity.x = 1.0f;
	}	
	else if (direction == 3){
		
	}
	else if (direction == 4) {
		velocity.x = 0;
	}
	
}

inline uint model_t::get_move()
{
	return is_move;
}

inline void model_t::set_move(uint n)
{
	is_move = n;
}


inline void model_t::update(float t)
{
	float c = cos(t);	// rotation
	float s = sin(t);	// rotation
	float gravity = 1.8f;
	velocity.y -= gravity * t;
	center.x += velocity.x*t;
	center.y += velocity.y*t;	

	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
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
