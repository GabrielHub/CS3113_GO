#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "SheetSprite.h"

class Object {
public:
	Object();
	Object(glm::vec3 position, glm::vec3 direction, SheetSprite sprite, float width, float height, float velocity, float acceleration, bool isStatic);

	bool EntityCollision(Object &object);

	//template entity variables
	glm::vec3 position; // position[1] = x, position[2] = y, position[3] = z *keep zero
	glm::vec3 direction; // direction[1] = x direction...
	SheetSprite sprite;
	float width;
	float height;
	float velocity;
	float acceleration;
	bool isStatic;

	//game specific variables
};