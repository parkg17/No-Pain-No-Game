#pragma once

struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 1.5f, 1.0f);   // position light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};