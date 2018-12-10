#include "Game.h"

GameState::GameState() {
	accumulator = 0.0f;
	lastFrameTicks = 0.0f;
	elapsed = 0.0f;

	gravity = -0.8f;

	done = false;

	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);

	//Model Matrices
	glm::mat4 player1Matrix = glm::mat4(1.0f);
	glm::mat4 player2Matrix = glm::mat4(1.0f);
	glm::mat4 bullet1Matrix = glm::mat4(1.0f);
	glm::mat4 bullet2Matrix = glm::mat4(1.0f);
	glm::mat4 textMatrix = glm::mat4(1.0f);

	map = 1;
}