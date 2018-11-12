#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#include "Object.h"


//GameState
class GameState {
public:
	GameState();

	// GLOBAL VARIABLES for the game world
	SDL_Window* displayWindow;
	//loading shaders
	ShaderProgram program;
	//Time
	float elapsed;
	float accumulator;
	float lastFrameTicks;
	//Event var
	SDL_Event event;
	//Loop var
	bool done;
	//Matrices
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 playerMatrix; //Model Matrices
	glm::mat4 enemyMatrix;
	glm::mat4 rWallMatrix;
	glm::mat4 lWallMatrix;
	//map object matrices
	std::vector<glm::mat4> floorMatrix;

	//fontsheet
	GLuint font;

	//Objects
	Object player;
	Object enemy;
	std::vector<Object> floor;
	Object lWall;
	Object rWall;
	//movement
	float gravity;
	float trumpScale;
};