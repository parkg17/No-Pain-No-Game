#pragma once
#ifndef __LIGHT_H__
#define __LIGHT_H__

struct light_t {
	vec4	position[5] = { vec4(0.0f, 0.0f, 1.5f, 1.0f), vec4(0.0f, 0.0f, 1.5f, 1.0f), vec4(0.0f, 0.0f, 1.5f, 1.0f),
							vec4(0.0f, 0.0f, 1.5f, 1.0f), vec4(0.0f, 0.0f, 1.5f, 1.0f) };
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	float constant = 0.1f;
	float linear = 0.01f;
	float quadratic = 1.10f;
};

#endif
