#include "gui.h"
#include "input.h"
#include "game.h"

bool GUI::RenderButton(float x, float y, float w, float h, Texture* texture, Vector4 tex_range, bool flipXY)
{
	Vector2 mouse = Input::mouse_position;
	float halfWidth = w * 0.5;
	float halfHeight = h * 0.5;
	float min_x = x - halfWidth;
	float max_x = x + halfWidth;
	float min_y = y - halfHeight;
	float max_y = y + halfHeight;

	bool hover = mouse.x >= min_x && mouse.x <= max_x && mouse.y >= min_y && mouse.y <= max_y;
	Vector4 buttonColor = hover ? Vector4(1, 1, 1, 1) : Vector4(1, 1, 1, 0.7f);

	RenderGui(x, y, w, h, texture, tex_range, buttonColor, flipXY);
	return Input::wasLeftMousePressed && hover;
}

void GUI::RenderGui(float x, float y, float w, float h, Texture* texture, Vector4 tex_range, Vector4 color, bool flipXY)
{
	int windowWidth = Game::instance->window_width; //quizas mejor pasarlo como parametro (más optimo?)
	int windowHeight = Game::instance->window_height;

	Mesh quad;
	quad.createQuad(x, y, w, h, false); //si es true, hay que cambiar la gui.fs (shader) y incluir clase gui 01:16:00

	Matrix44 quadModel;

	Camera cam2D; //no es la manera más optima, porque se crea para cada render, cambiar
	cam2D.setOrthographic(0, windowWidth, windowHeight, 0, -1, 1);

	Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/gui.fs");

	if (!shader) return;

	//enable shader
	shader->enable(); //si hay mas de una gui, hacer el enable solo una vez (fuera de la funcion)

	//upload uniforms
	shader->setUniform("u_color", color);
	shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);
	if (texture == NULL) { std::cout << "No texture" << std::endl; return; }
	shader->setUniform("u_texture", texture, 0);
	shader->setUniform("u_time", time);
	shader->setUniform("u_tex_range", tex_range);
	
	//quadModel.translate(sin(Game::instance->time) * 20, 0, 0);
	shader->setUniform("u_model", quadModel);

	//do the draw call
	quad.render(GL_TRIANGLES);

	//disable shader
	shader->disable();
}
