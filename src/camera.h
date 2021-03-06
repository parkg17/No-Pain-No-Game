#pragma once

struct camera {
    vec3 eye = vec3(0, 0, 3);
    vec3 at = vec3(0, 0, 0);
    vec3 up = vec3(0, 1, 0);
    mat4 view_matrix = mat4::look_at(eye, at, up);

    float fovy = PI / 4.0f; // must be in radian
    float aspect;
    float dnear = 1.0f;
    float dfar = 1000.0f;
    mat4 projection_matrix;

    void update(const GLfloat player_x) {
        eye.x = at.x = player_x;
        view_matrix = mat4::look_at(eye, at, up);
    }
};