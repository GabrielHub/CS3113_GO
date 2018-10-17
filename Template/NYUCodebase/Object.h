#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

class Object {
public:
	Object();
	Object(float x, float y, float rotation, int textureID, float width, float height, float velocity, float dirX, float dirY);

	void const Draw(ShaderProgram &p);

	//member var
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float velocity;
	float dirX;
	float dirY;
	//Added member var
};