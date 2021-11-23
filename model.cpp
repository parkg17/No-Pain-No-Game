#include "cgmath.h"			// slee's simple math library
#include "cgut2.h"			// slee's OpenGL utility		
#include "goal.h"
#include "enemy.h"
#include "obstacle.h"
#include "background.h"
#include "model.h"	



void render_player(float t, std::vector<background_t>& backgrounds, std::vector<enemy_t>& enemies,
	std::vector<obstacle_t>& obstacles, std::vector<goal_t>& goal, std::vector<model_t>& models, GLuint program, mesh2* pMesh_player)
{
	//auto backgrounds = std::move(background);
	for (auto& m : models)
	{
		if (!m.is_stop) {
			m.move(m.get_move(), t);			
			m.check_collision(obstacles, enemies, goal, backgrounds, 0);
			m.update(t);
		}
		GLint uloc;
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, m.color);	// pointer version
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, m.model_matrix);
		glUniform1i(glGetUniformLocation(program, "MODE"), 0);
		glBindVertexArray(pMesh_player->vertex_array);
		for (size_t k = 0, kn = pMesh_player->geometry_list.size(); k < kn; k++) {
			geometry& g = pMesh_player->geometry_list[k];

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
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh_player->index_buffer);
			glDrawElements(GL_TRIANGLES, g.index_count, GL_UNSIGNED_INT, (GLvoid*)(g.index_start * sizeof(GLuint)));
		}
	}

}