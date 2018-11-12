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
	enemyMatrix = glm::mat4(1.0f);
	rWallMatrix = glm::mat4(1.0f);
	lWallMatrix = glm::mat4(1.0f);

	//movement
	gravity = -0.4f;
}