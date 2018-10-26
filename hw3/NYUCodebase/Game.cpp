#include "Game.h"

GameState::GameState() {
	accumulator = 0.0f;
	lastFrameTicks = 0.0f;
	elapsed = 0.0f;
	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	done = false;
	fire = false;

	textMatrix = glm::mat4(1.0f); //Model Matrices
	playerMatrix = glm::mat4(1.0f);
	bulletMatrix = glm::mat4(1.0f);
	enemyMatrix = glm::mat4(1.0f);
}