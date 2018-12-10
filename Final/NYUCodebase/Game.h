#pragma once
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#include <SDL_mixer.h>

#include "Object.h"
#include <vector>

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
	glm::mat4 player1Matrix; //Model Matrices
	glm::mat4 player2Matrix;
	glm::mat4 textMatrix;
	std::vector<glm::mat4> map1Matrix;
	glm::mat4 bullet1Matrix;
	glm::mat4 bullet2Matrix;

	int map;

	//Sound and music
	Mix_Chunk *jumpSound;

	//Textures
	GLuint test;
	GLuint font;

	//Objects
	Player player1;
	Player player2;
	std::vector<Object> map1;

	//win states
	Player* map1Winner;
	Player* map2Winner;
	Player* map3Winner;

	//global physics
	float gravity;

	//animation
	float animationElapsed;
	float fps = 30.0f;
};