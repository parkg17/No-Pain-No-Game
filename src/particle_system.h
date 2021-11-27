#pragma once

#include "cgmath.h"
#include "cgut.h"

#include "object.h"

#include <chrono>
#include <random>

class particle_system {
public:
    object_t rain;

private:
    static constexpr size_t NUM_PARTICLES = 512;

    struct particle_t {
        vec3 direction; // direction vector
        vec3 centre;
        float life = -1.f;

        particle_t() { init(); }
        particle_t(const vec3& d, const vec3& c) : direction(d), centre(c), life(-1.f) {}
        void update(const float t) {
            centre += direction * 3 * t;
            if (life == -1.f && centre.y < -10.f) {
                life = 0.2f;
                direction.x = -direction.x;
                direction.y *= -0.1f;
            } else if (life != -1.f) {
                life -= t;
                if (life < 0) init();
            }
        }

        void init(bool first_time = false) {
            static std::mt19937 rng;
            std::uniform_real_distribution<float> dx_range(-3.f, 3.f);     // x-velocity
            std::uniform_real_distribution<float> dy_range {-20.f, -30.f}; // y-velocity
            std::uniform_real_distribution<float> x_range {50, 60};        // x-position

            direction = {dx_range(rng), dy_range(rng), 0};
            centre = {x_range(rng), first_time ? 10.f + x_range(rng) : 15.f, (x_range(rng) - 45.f) / 5.f - 25.f};
            life = -1.f;
        }
    };

public:
    void init() {
        rain.init("../bin/mesh/rain/rain.obj", "../bin/mesh/rain/rain.png");
        for (auto& x : particles) {
            x.init(true);
        }
    }
    void update(const float t) {
        for (auto& p : particles) {
            p.update(t);
        }
    }
    void render(const GLuint program) {
        glUseProgram(program);
        for (const auto& x : particles) {
            rain.set_scale(x.life == -1.f ? 10.f : 2.5f);
            rain.set_location(x.centre);
            rain.render(program);
        }
    }

private:
    std::array<particle_t, NUM_PARTICLES> particles;
};
