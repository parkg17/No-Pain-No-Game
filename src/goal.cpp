#include "cgmath.h"			// slee's simple math library
#include "cgut2.h"			// slee's OpenGL utility
#include "goal.h"		// Obstacle


void render_goal(float t, std::vector<goal_t>& goal, GLuint program, mesh2* pMesh_goal)
{
	
	for (auto& g : goal)
	{
		g.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, g.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glBindVertexArray(pMesh_goal->vertex_array);
		for (size_t k = 0, kn = pMesh_goal->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_goal->geometry_list[k];

			if (g.mat->textures.diffuse) {
				glBindTexture(GL_TEXTURE_2D, g.mat->textures.diffuse->id);
				glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0
				glUniform1i(glGetUniformLocation(program, "use_texture"), true);
			}
			else {
				glUniform4fv(glGetUniformLocation(program, "diffuse"), 1, (const float*)(&g.mat->diffuse));
				glUniform1i(glGetUniformLocation(program, "use_texture"), false);
			}

			// render vertices: trigger shader programs to process vertex data
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_goal->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

}