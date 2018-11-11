#include "Game.h"

GameState::GameState() {
	accumulator = 0.0f;
	lastFrameTicks = 0.0f;
	elapsed = 0.0f;

	done = false;

	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);

	//Model Matrices
	playerMatrix = glm::mat4(1.0f);
	staticMatrix = glm::mat4(1.0f);
	floorMatrix1 = glm::mat4(1.0f);
	floorMatrix2 = glm::mat4(1.0f);

	//movement
	velocity = 0.0f;
	acceleration = 0.0f;
	friction = 2.0f;
}