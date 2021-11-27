#pragma once

#include "cgmath.h"
#include "cgut.h"

#include "object.h"

#include <iostream>

class player_t : public object_t {
    static constexpr GLfloat X_MIN = -20.f, X_MAX = 150.f;
    static constexpr GLfloat Y_MIN = -10.f;

public:
    void update(const float t) {
        location.x += t * x_dir * 15;
        if (location.x < X_MIN) location.x = X_MIN;
        if (location.x > X_MAX) location.x = X_MAX;
        if (a != FLT_MIN) {
            location.y += t * a * 60;
            a -= t;
            if (location.y < Y_MIN) {
                location.y = Y_MIN;
                a = FLT_MIN;
            }
        }
        if (x_dir == -1 && on_left == false || x_dir == 1 && on_left == true) {
            on_left ^= true;
            set_rotate(vec3(0, 1, 0), PI);
        }
    }
    void jump() {
        if (a == FLT_MIN) {
            a = 0.5f;
        }
    }
    void set_x_dir(const int d) {
        x_dir = d;
    }

private:
    GLfloat a = FLT_MIN;
    int x_dir = 0;
    bool on_left = false;
};