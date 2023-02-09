#ifndef GUI_H
#define GUI_H

#include "entity.h"
#include "world.h"

class GUI {
public:

	bool static RenderButton(float x, float y, float w, float h, Texture* texture, Vector4 tex_range, bool flipXY = false);
	void static RenderGui(float x, float y, float w, float h, Texture* texture, Vector4 tex_range, Vector4 color = Vector4(1, 1, 1, 1), bool flipXY = false);
};

#endif 