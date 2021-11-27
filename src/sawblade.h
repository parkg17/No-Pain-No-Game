#pragma once

#include "cgmath.h"
#include "cgut.h"

#include "object.h"
#include "player.h"

#include <chrono>
#include <random>

class sawblade_t : public object_t {
    static constexpr GLfloat X_MIN = -20.f, X_MAX = 60.f;
    static constexpr GLfloat Y_MIN = -10.f;

public:
    void set_player(player_t* player) {
        this->player = player;
    }
    void update(const float t) {
        static std::mt19937 rng;
        static std::uniform_real_distribution<float> random_range(0.5f, 1.5f);

        set_rotate(vec3(0, 0, 1), t);
        location.x += x_dir * t * (a < mx_a / 2 ? a : mx_a - a) * 100.f / mx_a;
        a += t;
        if (a > mx_a) {
            a = 0.f, mx_a = random_range(rng);
            x_dir = player->get_x_loc() < location.x ? -1 : 1;
        }
    }
    void set_x_dir(const int d) {
        x_dir = d;
    }

private:
    player_t* player {};
    GLfloat a {}, mx_a = 2.f;
    GLint x_dir {};
};