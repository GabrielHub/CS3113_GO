#include "Object.h"

Object::Object() {}

//Object(glm::vec3 position, glm::vec3 direction, float rotation, SheetSprite sprite, float width, float height, float velocity);
Object::Object(glm::vec3 position, glm::vec3 direction, SheetSprite sprite, float width, float height, float velocity, float acceleration, bool isStatic) : position(position), direction(direction), sprite(sprite), width(width), height(height), velocity(velocity), acceleration(acceleration), isStatic(isStatic) {}

//Collision
bool Object::EntityCollision(Object &object) {
	return true;
}