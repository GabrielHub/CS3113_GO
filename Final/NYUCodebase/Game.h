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
	glm::mat4 bullet1Matrix;
	glm::mat4 bullet2Matrix;
	std::vector<glm::mat4> map1Matrix;
	std::vector<glm::mat4> map2Matrix;
	std::vector<glm::mat4> map3Matrix;

	//map number
	int map;

	//if ammo box is displayed or not
	bool ammoDisplay;

	//Sound and music
	Mix_Chunk *jumpSound;
	Mix_Chunk *fire;
	Mix_Chunk *reload;
	Mix_Chunk *die;
	Mix_Chunk *collision;
	Mix_Chunk *empty;
	Mix_Music *music;

	//Textures
	GLuint test;
	GLuint font;

	//Objects
	Player player1;
	Player player2;
	std::vector<Object> map1;
	std::vector<Object> map2;
	std::vector<Object> map3;

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