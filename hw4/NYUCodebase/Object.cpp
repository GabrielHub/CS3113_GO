#include "Object.h"

Object::Object() {}

//Object(glm::vec3 position, glm::vec3 direction, float rotation, SheetSprite sprite, float width, float height, float velocity);
Object::Object(glm::vec3 position, SheetSprite sprite, float width, float height, bool isStatic) : position(position), sprite(sprite), width(width), height(height), isStatic(isStatic) {
	velocity.x = 0.0f;
	velocity.y = 0.0f;
	acceleration = 0.0f;
	friction = 2.0f;
	onFloor = false;
}

//Collision
bool Object::EntityCollision(Object &object) {
	if (position.x <= object.position.x + 0.1f && position.x >= object.position.x - 0.1f) {
		if (position.y <= (object.position.y + 0.15f) && position.y >= (object.position.y - 0.15f)) {
			return true;
		}
	}
	return false;
}

bool Object::GravityCheck(Object& object) {
	/*if ((position.x) <= (object.position.x + 0.25f) && position.x >= (object.position.x - 0.25f)) {
		if ((position.y - height / 2) <= (object.position.y + 0.27f)) {
			return true;
		}
	}*/
	if ((position.y - height / 2) <= (object.position.y + 0.27f)) {
		return true;
	}
	else {
		return false;
	}
}