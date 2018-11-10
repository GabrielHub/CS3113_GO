#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//Include classes
#include "Game.h"
#include "Object.h"
#include "SheetSprite.h"
#include <vector>
#include <cmath>

//60FPS (1.0/60.0)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

/*
	Template: Update for homework 4
	Currently on: Time code
*/

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
//Gamestate also holds global variables
GameState game;
GameMode mode = STATE_GAME_LEVEL;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

//Function that converts strings to drawn text from sprite sheet
void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
	float char_size = 1.0 / 16.0f;

	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];

		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + char_size,
			texture_x + char_size, texture_y,
			texture_x + char_size, texture_y + char_size,
			texture_x + char_size, texture_y,
			texture_x, texture_y + char_size,
			});
	}

	glBindTexture(GL_TEXTURE_2D, fontTexture);

	//glUseProgram(program.programID);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

//Load Texture Function
GLuint LoadTexture(const char* filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "unable to load image. Make sure the path is correct \n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

//Linear Interpolation
float lerp(float v0, float v1, float t) {
	return (1.0 - t) * v0 + t * v1;
}

//Setup Function
void Setup(GameState &game) {
	SDL_Init(SDL_INIT_VIDEO);
	game.displayWindow = SDL_CreateWindow("GO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(game.displayWindow);
	SDL_GL_MakeCurrent(game.displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//Viewport and program setup
	glViewport(0, 0, 1280, 720);
	game.program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	glUseProgram(game.program.programID);
	game.projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	//For Alpha Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Time Code
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	game.elapsed = ticks - game.lastFrameTicks;
	game.lastFrameTicks = game.elapsed;

	/*Load Textures
	Ex: example = LoadTexture(RESOURCE_FOLDER "example.example");
	SheetSprite exampleTexture = SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
	^ remember to divide u, v, width and height by pixels
	*/
	GLuint spritesheet_id = LoadTexture(RESOURCE_FOLDER "sprites.png"); //Load big spritesheet
	GLuint fontSheet = LoadTexture(RESOURCE_FOLDER "font.png"); //font spritesheet
	game.font = fontSheet;
	SheetSprite ye_texture = SheetSprite(spritesheet_id, 0.0f, 0.0f, 255.0 / 512.0f, 363.0 / 512.0f, 0.5f); //init enemy texture
	//Sprites for player animation
	SheetSprite playerTexture1 = SheetSprite(spritesheet_id, 271.0f / 512.0f, 438.0f / 512.0f, 60.0f / 512.0f, 71.0f / 512.0f, 0.05f); //right
	/*SheetSprite playerTexture2 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 71.0 / 512.0f, 63.0 / 512.0f, 72.0 / 512.0f, 0.8f);
	SheetSprite playerTexture3 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 217.0 / 512.0f, 61.0 / 512.0f, 73.0 / 512.0f, 0.8f);
	SheetSprite playerTexture4 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 145.0 / 512.0f, 62.0 / 512.0f, 70.0 / 512.0f, 0.8f);
	SheetSprite playerTexture5 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 0.0 / 512.0f, 63.0 / 512.0f, 69.0 / 512.0f, 0.8f);
	SheetSprite playerTexture6 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 292.0 / 512.0f, 61.0 / 512.0f, 70.0 / 512.0f, 0.8f);
	//SheetSprite playerTexture7 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 0.0 / 512.0f, 63.0 / 512.0f, 69.0 / 512.0f, 0.8f); //left*/
	
	/* Create Objects, example:
		Object example(xposition, yposition, rotation (angle), textureID, width, height, velocity, direction x, direction y);
		Object(glm::vec3 position, glm::vec3 direction, SheetSprite sprite, float width, float height, float velocity, float acceleration, bool isStatic);
		^ remember to intialize width and height by the sprite values * size
	*/
	glm::vec3 position(0.0f, 0.0f, 0.0f);
	glm::vec3 direction(1.0f, 1.0f, 1.0f);
	game.player = Object(position, direction, playerTexture1, 60.0 / 1280.0f, 71.0 / 720.0f, false);
}

//Process inputs
void Event(float elapsed) {

	while (SDL_PollEvent(&game.event)) {
		if (game.event.type == SDL_QUIT || game.event.type == SDL_WINDOWEVENT_CLOSE) {
			game.done = true;
		}
		/*For single input, Exmaple:
		else if (game.event.type == SDL_KEYDOWN)
		*/
	}

	//For polling input, ex. if (keys[SDL_SCANCODE_A]) {}
	if (keys[SDL_SCANCODE_A]) {
		game.acceleration = -0.1f;
	}
	else if (keys[SDL_SCANCODE_D]) {
		game.acceleration = 0.1f;
	}
	else {
		game.acceleration = 0.0f;
	}
	
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	switch (mode) {
	case STATE_GAME_LEVEL:
		//Changes
		/*
			Translation exmaple: modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f (x), 1.0f (y), keep 0);
			friction: velocity = lerp(velocity, 0.0f, elapsed * friction_x);
			acceleration: velocity_x += acceleration * elapsed;
			velocity: position_x += velocity_x * elpased;
		*/

		//player movement
		game.velocity = lerp(game.velocity, 0.0f, elapsed * game.friction);
		game.velocity += game.acceleration * elapsed;
		game.player.position.x += game.velocity * elapsed;

		game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(game.player.position.x, game.player.position.y, 0.0f));
		break;
	}
}

//Render all objects in the game, render UI elements
void Render() {
	switch (mode) {
	case STATE_MAIN_MENU:
		//Screen Color
		glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);
		//game.program.SetModelMatrix(game.playerMatrix);

		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		DrawText(game.program, game.font, "main menu", 0.1f, 0.0f);
		break;

	case STATE_GAME_LEVEL:
		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);
		game.program.SetModelMatrix(game.playerMatrix);

		//Draw Objects, using spritesheet object example.draw(program)
		//Player
		game.player.sprite.Draw(game.program);
		break;
	}
}

//Clean up memory
void Clean() {
	
}

int main(int argc, char *argv[])
{
	Setup(game);

	while (!game.done) {
		Event(FIXED_TIMESTEP); //Check for input

		//Game code
		game.elapsed += game.accumulator;
		if (game.elapsed < FIXED_TIMESTEP) {
			game.accumulator = game.elapsed;
			continue;
		}
		//60 FPS updated time code
		while (game.elapsed >= FIXED_TIMESTEP) {
			Update(FIXED_TIMESTEP);
			game.elapsed -= FIXED_TIMESTEP;
		}
		game.accumulator = game.elapsed;

		//Graphics
		Render();

		SDL_GL_SwapWindow(game.displayWindow);
	}

	//Clear memory
	Clean();

	SDL_Quit();
	return 0;
}