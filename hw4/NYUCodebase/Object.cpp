#include "Object.h"

Object::Object() {}

//Object(glm::vec3 position, glm::vec3 direction, float rotation, SheetSprite sprite, float width, float height, float velocity);
Object::Object(glm::vec3 position, glm::vec3 direction, SheetSprite sprite, float width, float height, bool isStatic) : position(position), direction(direction), sprite(sprite), width(width), height(height), isStatic(isStatic) {}

//Collision
bool Object::EntityCollision(Object &object) {
	//Check top
	if ((position.x + sprite.size / 2) <= (object.position.y + object.sprite.size / 2)) {
		/*position.y += 0.5f;
		object.position.y -= 0.5f;*/
		return true;
	}
	else {
		return false;
	}

	//check bottom
	if ((position.y + sprite.size / 2) >= (object.position.y - object.sprite.size / 2)) {
		/*
		position.y -= 0.5f;
		object.position.y += 0.5f;
		*/
		return true;
	}
	else {
		return false;
	}

	//check right
	if ((position.x + sprite.size / 2) <= (object.position.x - object.sprite.size / 2)) {
		/*
		position.x -= 0.5f;
		object.position.x += 0.5f;
		*/
		return true;
	}
	else {
		return false;
	}

	//check left
	if ((position.x - sprite.size / 2) >= (object.position.x + object.sprite.size / 2)) {
		/*
		position.x += 0.5f;
		object.position.x -= 0.5f;
		*/
		return true;
	}
	else {
		return false;
	}

	return true;
}