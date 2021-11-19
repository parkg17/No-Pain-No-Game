#pragma once
#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__

struct background_t
{
	float	left = -2.0f;
	float	right = 2.0f;
	float	bottom = -1.0f;
	float	top = 1.0f;
};

inline std::vector<background_t> create_backgrounds()
{
	int num_cnt = 1;
	std::vector<background_t> bg;
	std::vector<background_t> b(1);

	/* Declare 9 Planets */
	b[0] = { -2.0f, 2.0f, -1.1f, 1.1f };
	for (int idx = 0; idx < num_cnt; ++idx) {
		bg.emplace_back(b[idx]);
	}
	return bg;
}
#endif