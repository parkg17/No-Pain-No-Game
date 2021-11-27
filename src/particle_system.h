#pragma once

#include "cgmath.h"
#include "cgut.h"

#include "object.h"

#include <chrono>
#include <random>

class particle_system {
    static constexpr size_t NUM_PARTICLES = 256;

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
        void render(const GLuint program) {
            const auto scale = mat4::scale(life == -1.f ? 0.2f : 0.05f);
            const auto trans = mat4::translate(centre);
            const auto model = trans * scale;

            glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            if (life == -1.f) return;
            glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, mat4::translate(3.f * direction.x * -(0.5f - life), 0, 0) * model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, mat4::translate(3.f * direction.x * +(0.5f - life), 0, 0) * model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        void init(bool first_time = false) {
            static std::mt19937 rng;
            std::uniform_real_distribution<float> dx_range(-3.f, 3.f);     // x-velocity
            std::uniform_real_distribution<float> dy_range {-20.f, -30.f}; // y-velocity
            std::uniform_real_distribution<float> x_range {-5, 5};         // x-position

            direction = {dx_range(rng), dy_range(rng), 0};
            centre = {x_range(rng), first_time ? 10.f + x_range(rng) : 15.f, x_range(rng) / 5.f - 25.f};
            life = -1.f;
        }
    };

public:
    void init() {
        // generation of vertex buffer: use vertices as it is
        GLuint vertex_buffer = 0; // ID holder for vertex buffer
        std::array<vertex, 4> vertices;
        vertices[0].pos = vec3(-0.5f, -1.f, 0);
        vertices[1].pos = vec3(+0.5f, -1.f, 0);
        vertices[2].pos = vec3(+0.5f, +1.f, 0);
        vertices[3].pos = vec3(-0.5f, +1.f, 0);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        // geneation of index buffer
        GLuint index_buffer = 0; // ID holder for index buffer
        std::array<GLuint, 6> indices = {0, 1, 2, 2, 3, 0};

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), indices.data(), GL_STATIC_DRAW);

        vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);

        for (auto& p : particles) {
            p.init(true);
        }
    }
    void update(const float t) {
        for (auto& p : particles) {
            p.update(t);
        }
    }
    void render(const GLuint program) {
        glUseProgram(program);
        glBindVertexArray(vertex_array);

        glUniform4fv(glGetUniformLocation(program, "solid_color"), 1, vec4(0, 0, 1, 1)); // blue

        for (auto& p : particles) {
            p.render(program);
        }
    }

private:
    object_t rain;
    std::array<vec2, NUM_PARTICLES> locs;
};
