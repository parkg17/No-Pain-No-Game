#pragma once

#include "cgmath.h"
#include "cgut.h"

#include "tiny_obj_loader.h"

#include <iostream>

class object_t {
public:
    void init(const char* obj_path, const char* tex_path) {
        if (tex_path != nullptr) {
            m.texture = cg_create_texture(tex_path);
            std::cout << "load " << tex_path << '\n';
        }

        tinyobj::ObjReaderConfig reader_config;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(std::string(obj_path), reader_config)) {
            if (!reader.Error().empty()) {
                std::cout << "TinyObjReader: " << reader.Error();
            }
            return;
        }
        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        const auto& attrib = reader.GetAttrib();
        const auto& shapes = reader.GetShapes();

        vertex vtx;
        for (const auto& shape : shapes) {
            size_t index_offset = 0;
            for (const auto& face : shape.mesh.num_face_vertices) {
                const auto fv = static_cast<size_t>(face);
                for (size_t i = 0; i != fv; ++i) {
                    const tinyobj::index_t index = shape.mesh.indices[index_offset + i];
                    vtx.pos = vec3(
                        attrib.vertices[3 * size_t(index.vertex_index) + 0],
                        attrib.vertices[3 * size_t(index.vertex_index) + 1],
                        attrib.vertices[3 * size_t(index.vertex_index) + 2]);
                    vtx.norm = vec3(
                        attrib.normals[3 * size_t(index.normal_index) + 0],
                        attrib.normals[3 * size_t(index.normal_index) + 1],
                        attrib.normals[3 * size_t(index.normal_index) + 2]);
                    vtx.tex = vec2(
                        attrib.texcoords[2 * size_t(index.texcoord_index) + 0],
                        attrib.texcoords[2 * size_t(index.texcoord_index) + 1]);
                    m.vertex_list.push_back(vtx);
                    m.index_list.push_back(index_offset + i);
                }
                index_offset += fv;
            }
        }

        glGenBuffers(1, &m.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * m.vertex_list.size(), m.vertex_list.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &m.index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m.index_list.size(), m.index_list.data(), GL_STATIC_DRAW);

        std::cout << "done " << m.vertex_list.size() << ' ' << m.index_list.size() << std::endl;

        m.vertex_array = cg_create_vertex_array(m.vertex_buffer, m.index_buffer);
    }
    void update() {}
    void render(const GLuint program) {
        glUseProgram(program);

        const auto model_mat = mat4::translate(location) * r_mat * mat4::scale(scale);
        glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_mat);

        glBindVertexArray(m.vertex_array);

        if (m.texture) {
            glUniform1i(glGetUniformLocation(program, "MODE"), 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m.texture);
            glUniform1i(glGetUniformLocation(program, "TEX"), 0);
        } else {
            // TODO: color
            glUniform1i(glGetUniformLocation(program, "MODE"), 0);
        }
        glDrawElements(GL_TRIANGLES, m.index_list.size(), GL_UNSIGNED_INT, nullptr);
    }

    void set_scale(const GLfloat scale) { this->scale = scale; }
    void set_rotate(const vec3 axis, const GLfloat angle) { r_mat *= mat4::rotate(axis, angle); }
    void set_location(const vec3 location) { this->location = location; }

    GLfloat get_x_loc() const { return location.x; }

protected:
    mesh m {};
    GLfloat scale = 1.f;
    mat4 r_mat = mat4();
    vec3 location {};
};
