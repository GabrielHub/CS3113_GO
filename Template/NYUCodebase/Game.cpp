#include "Game.h"

GameState::GameState() {
	accumulator = 0.0f;
	lastFrameTicks = 0.0f;
	elapsed = 0.0f;

	gravity = -0.4f;

	done = false;

	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);

	//Model Matrices
	modelMatrix = glm::mat4(1.0f);
}