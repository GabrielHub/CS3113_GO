#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "SheetSprite.h"

//For static objects
class Object {
public:
	Object();
	Object(glm::vec3 position, float width, float height, SheetSprite sprite);

	//template entity variables
	glm::vec3 position; // position.x, position.y ... etc
	float width;
	float height;

	//textures
	SheetSprite sprite;

	
};

class Bullet : public Object {
public:
	Bullet();

	Bullet(glm::vec3 pos, float w, float h, glm::vec2 v, glm::vec2 a, SheetSprite s);

	bool EntityCollision(Object &object);

	//bulletstate
	enum bulletState { STATE_FIRED, STATE_UNFIRED };
	bulletState state;

	//physics
	glm::vec2 velocity;
	glm::vec2 acceleration;
};

class Player : public Object {
public:
	Player();

	Player(glm::vec3 position, glm::vec3 direction, float width, float height, glm::vec2 velocity, glm::vec2 acceleration, float friction);

	bool EntityCollision(Object &object);

	glm::vec3 direction;

	//playerstate
	enum playerState { STATE_STANDING, STATE_JUMPING, STATE_WALKING };
	playerState state;

	//textures
	SheetSprite standingSprite;
	SheetSprite walk1Sprite;
	SheetSprite walk2Sprite;
	SheetSprite jumpSprite;
	//animation
	SheetSprite* currentSprite;
	int currentIndex;

	//physics
	glm::vec2 velocity;
	glm::vec2 acceleration;
	float friction;
	bool onFloor;

	Bullet* bullet;
};