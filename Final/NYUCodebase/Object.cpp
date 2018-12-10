#include "Object.h"

Object::Object() {}

//Object(glm::vec3 position, glm::vec3 direction, float rotation, SheetSprite sprite, float width, float height, float velocity);
Object::Object(glm::vec3 position, float width, float height, SheetSprite sprite) : position(position), width(width), height(height), sprite(sprite) {}

Player::Player() {}

Player::Player(glm::vec3 pos, glm::vec3 direction, float w, float h, glm::vec2 velocity, glm::vec2 acceleration, float friction) : direction(direction), velocity(velocity), acceleration(acceleration), friction(friction) {
	position = pos;
	width = w;
	height = h;
	onFloor = false;
	state = STATE_STANDING;
	currentIndex = 0;
}

//Collision for dynamic objects
bool Player::EntityCollision(Object &object) {
	if ((position.x - (width / 2)) <= object.position.x + (object.width / 2) && (position.x + (width / 2)) >= object.position.x - (object.width / 2)) {
		if ((position.y - height / 2) <= (object.position.y + (object.height / 2)) && (position.y + height / 2) >= (object.position.y - (object.height / 2))) {
			return true;
		}
	}
	return false;
}

Bullet::Bullet() {}

Bullet::Bullet(glm::vec3 pos, float w, float h, glm::vec2 v, glm::vec2 a, SheetSprite s) {
	position = position;
	width = w;
	height = h;
	velocity = v;
	acceleration = a;
	sprite = s;
	state = STATE_UNFIRED;
}

bool Bullet::EntityCollision(Object &object) {
	if ((position.x - (width / 2)) <= object.position.x + (object.width / 2) && (position.x + (width / 2)) >= object.position.x - (object.width / 2)) {
		if ((position.y - height / 2) <= (object.position.y + (object.height / 2)) && (position.y + height / 2) >= (object.position.y - (object.height / 2))) {
			return true;
		}
	}
	return false;
}