#include "cgmath.h"			// slee's simple math library
#include "cgut2.h"			// slee's OpenGL utility
#include "background.h"
	// Obstacle


void render_bg(float t,std::vector<background_t>& background,GLuint program,GLuint Background, GLuint bg_vertex_array)
{
	//auto backgrounds = std::move(background);
	for (auto& bg : background)
	{
		bg.update(t);
		GLint uloc;
		uloc = glGetUniformLocation(program, "model_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, bg.model_matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Background);
		glUniform1i(glGetUniformLocation(program, "MODE"), 1);
		glUniform1i(glGetUniformLocation(program, "TEX"), 0);
		glBindVertexArray(bg_vertex_array);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
}