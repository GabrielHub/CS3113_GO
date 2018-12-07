#include "Object.h"

Object::Object() {}

//Object(glm::vec3 position, glm::vec3 direction, float rotation, SheetSprite sprite, float width, float height, float velocity);
Object::Object(glm::vec3 position, glm::vec3 direction, float width, float height, glm::vec2 velocity, glm::vec2 acceleration) : position(position), direction(direction), width(width), height(height), velocity(velocity), acceleration(acceleration) {}

//Collision for dynamic objects
bool Object::EntityCollision(Object &object) {
	if (position.x <= object.position.x + 0.1f && position.x >= object.position.x - 0.1f) {
		if (position.y <= (object.position.y + 0.15f) && position.y >= (object.position.y - 0.15f)) {
			return true;
		}
	}
	return false;
}

//Check if on floor (incomplete)
bool Object::GravityCheck(Object& object) {
	if ((position.y - height / 2) <= (object.position.y + 0.27f)) {
		return true;
	}
	else {
		return false;
	}
}