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
	Object(glm::vec3 position, glm::vec3 direction, float width, float height, glm::vec2 velocity, glm::vec2 acceleration);

	bool EntityCollision(Object &object);

	bool GravityCheck(Object &object);

	//template entity variables
	glm::vec3 position; // position.x, position.y ... etc
	glm::vec3 direction; // direction[1] = x direction...
	float width;
	float height;

	//textures
	SheetSprite defaultSprite;
	SheetSprite walk1Sprite;
	SheetSprite walk2Sprite;
	SheetSprite jumpSprite;

	//physics
	glm::vec2 velocity;
	glm::vec2 acceleration;
	float friction;
};