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
	HW4: Platformer game! A and D movement, Space to jump. everytime you hit kanye you get bigger. Let's get this bread
	Notes: Tried to do some animation but I couldn't get it to work. couldn't fully do floor collision without a lot of bugs, so made a shortcut
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
	SheetSprite ye_texture = SheetSprite(spritesheet_id, 0.0f, 0.0f, 255.0 / 512.0f, 363.0 / 512.0f, 0.25f); //init enemy texture
	//Sprites for player animation
	SheetSprite playerTexture1 = SheetSprite(spritesheet_id, 271.0f / 512.0f, 438.0f / 512.0f, 60.0f / 512.0f, 71.0f / 512.0f, 0.05f); //right
	game.player.spriteArray.push_back(playerTexture1);
	SheetSprite playerTexture2 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 71.0 / 512.0f, 63.0 / 512.0f, 72.0 / 512.0f, 0.05f);
	game.player.spriteArray.push_back(playerTexture2);
	SheetSprite playerTexture3 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 217.0 / 512.0f, 61.0 / 512.0f, 73.0 / 512.0f, 0.05f);
	game.player.spriteArray.push_back(playerTexture3);
	SheetSprite playerTexture4 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 145.0 / 512.0f, 62.0 / 512.0f, 70.0 / 512.0f, 0.05f);
	game.player.spriteArray.push_back(playerTexture4);
	SheetSprite playerTexture5 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 0.0 / 512.0f, 63.0 / 512.0f, 69.0 / 512.0f, 0.05f);
	game.player.spriteArray.push_back(playerTexture5);
	SheetSprite playerTexture6 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 292.0 / 512.0f, 61.0 / 512.0f, 70.0 / 512.0f, 0.05f);
	game.player.spriteArray.push_back(playerTexture6);
	SheetSprite playerTexture7 = SheetSprite(spritesheet_id, 257.0 / 512.0f, 0.0 / 512.0f, 63.0 / 512.0f, 69.0 / 512.0f, 0.05f); //left
	game.player.spriteArray.push_back(playerTexture7);
	//Ground and wall sprites
	SheetSprite floor = SheetSprite(spritesheet_id, 0.0f, 437.0f / 512.0f, 70.0f / 512.0f, 70.0f / 512.0f, 0.5f);
	SheetSprite walls = SheetSprite(spritesheet_id, 0.0f, 365.0f / 512.0f, 70.0f / 512.0f, 70.0f / 512.0f, 0.5f);
	
	/* Create Objects, example:
		Object example(xposition, yposition, rotation (angle), textureID, width, height, velocity, direction x, direction y);
		Object(glm::vec3 position, SheetSprite sprite, float width, float height, bool isStatic);
		^ remember to intialize width and height by the sprite values * size
	*/
	glm::vec3 playerPosition(-0.5f, 0.0f, 0.0f);
	glm::vec3 enemyPosition(0.0f, 5.0f, 0.0f);
	game.player = Object(playerPosition, playerTexture2, (60.0f / 512.0f) * 0.05, (71.0f / 512.0f) * 0.05, false);
	game.enemy = Object(enemyPosition, ye_texture, (255.0f / 512.0f) * 0.25f, (353.0f / 512.0f) * 0.25f, false);
	//Static Objects
	game.floor.push_back(Object(glm::vec3(-0.25f, -0.8f, 0.0f), floor, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true)); // Floor Left Start of floor
	game.floorMatrix.push_back(glm::mat4(1.0f));
	game.floor.push_back(Object(glm::vec3(0.25f, -0.8f, 0.0f), floor, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true)); // Floor Right
	game.floorMatrix.push_back(glm::mat4(1.0f));
	game.floor.push_back(Object(glm::vec3(-0.75f, -0.8f, 0.0f), floor, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true)); // Floor Far Left
	game.floorMatrix.push_back(glm::mat4(1.0f));
	game.floor.push_back(Object(glm::vec3(0.75f, -0.8f, 0.0f), floor, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true)); // Floor Far Right
	game.floorMatrix.push_back(glm::mat4(1.0f));

	game.lWall = Object(glm::vec3(-1.25f, -0.4f, 0.0f), walls, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true); //Left Wall
	game.rWall = Object(glm::vec3(1.25f, -0.4f, 0.0f), walls, (70.0f / 512.0f) * 0.5f, (70.0f / 512.0f) * 0.5f, true); //Right Wall

	//make enemy defaults
	game.enemy.onFloor = false;
	game.enemy.acceleration = 0.5f;
	game.enemy.direction.x = 1.0f;
	game.trumpScale = 1.0f;
}

//Process inputs
void Event() {

	while (SDL_PollEvent(&game.event)) {
		if (game.event.type == SDL_QUIT || game.event.type == SDL_WINDOWEVENT_CLOSE) {
			game.done = true;
		}
		/*For single input, Exmaple:
		else if (game.event.type == SDL_KEYDOWN), if (event.key.keysym.scancode == SDL_SCANCODE_SPACE
		*/
		else if (game.event.type == SDL_KEYDOWN && game.player.onFloor) {
			if (game.event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				game.player.position.y += 0.1f;
				game.player.velocity.y = 1.0f;
				game.player.onFloor = false;
			}
		}
	}

	//For polling input, ex. if (keys[SDL_SCANCODE_A]) {}
	if (keys[SDL_SCANCODE_A]) {
		game.player.acceleration = -1.0f;
	}
	else if (keys[SDL_SCANCODE_D]) {
		game.player.acceleration = 1.0f;
	}
	else {
		game.player.acceleration = 0.0f;
	}
	
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	switch (mode) {
	case STATE_GAME_LEVEL:
		//Changes
		/*
			Movement example: 
				game.playerMatrix = glm::mat4(1.0f);
				game.player.velocity.x = lerp(game.player.velocity.x, 0.0f, elapsed * game.player.friction);
				game.player.velocity.y = lerp(game.player.velocity.y, 0.0f, elapsed * -game.gravity);
				game.player.velocity.x += game.player.acceleration * elapsed;
				game.player.position.x += game.player.velocity.x * elapsed;
				game.player.position.y += game.player.velocity.y * elapsed;
				game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(game.player.position.x, game.player.position.y, 0.0f));
			REMEMBER: \
				to reset matrices at start of each frame
		*/

		//build map
		for (int i = 0; i < game.floorMatrix.size(); i++) {
			game.floorMatrix[i] = glm::mat4(1.0f);
			game.floorMatrix[i] = glm::translate(game.floorMatrix[i], glm::vec3(game.floor[i].position.x, game.floor[i].position.y, 0.0f));
		} //Floor build
			//Wall build
		game.lWallMatrix = glm::mat4(1.0f);
		game.rWallMatrix = glm::mat4(1.0f);
		game.lWallMatrix = glm::translate(game.lWallMatrix, glm::vec3(game.lWall.position.x, game.lWall.position.y, 0.0f));
		game.rWallMatrix = glm::translate(game.rWallMatrix, glm::vec3(game.rWall.position.x, game.rWall.position.y, 0.0f));

		//PLAYER STUFF
			//Check if player is touching the ground and do gravity on player
		if (!game.player.onFloor) {
			for (int i = 0; i < game.floor.size(); i++) {
				if (game.player.GravityCheck(game.floor[i])) {
					game.player.velocity.y = 0.0f;
					game.player.onFloor = true;
				}
				else {
					game.player.onFloor = false;
					game.player.velocity.y += game.gravity * elapsed; // gravity
				}
			}
		}
			//check if player hits the wall
		if (game.player.position.x <= game.lWall.position.x + 0.25f) {
			game.player.velocity.x = 1.0f;
		}
		else if (game.player.position.x >= game.rWall.position.x - 0.25f) {
			game.player.velocity.x = -1.0f;
		} //if player hits a wall, bounce em
		
			//player movement
		game.playerMatrix = glm::mat4(1.0f);
		game.player.velocity.x = lerp(game.player.velocity.x, 0.0f, elapsed * game.player.friction);
		game.player.velocity.y = lerp(game.player.velocity.y, 0.0f, elapsed * -game.gravity);
		game.player.velocity.x += game.player.acceleration * elapsed;
		game.player.position.x += game.player.velocity.x * elapsed;
		game.player.position.y += game.player.velocity.y * elapsed;
		game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(game.player.position.x, game.player.position.y, 0.0f));

		//ENEMY STUFF
		game.enemyMatrix = glm::mat4(1.0f);

		if (!game.enemy.onFloor) {
			for (int i = 0; i < game.floor.size(); i++) {
				if (game.enemy.GravityCheck(game.floor[i])) {
					game.enemy.velocity.y = 0.0f;
					game.enemy.onFloor = true;
				}
				else {
					game.enemy.onFloor = false;
					game.enemy.velocity.y += game.gravity * elapsed; // gravity
				}
			}
		}
		if (game.enemy.position.x <= game.lWall.position.x + 0.25f) {
			game.enemy.velocity.x = 0.5f;
			game.enemy.direction.x = 1.0f;
		}
		else if (game.enemy.position.x >= game.rWall.position.x - 0.25f) {
			game.enemy.velocity.x = -0.5f;
			game.enemy.direction.x = -1.0f;
		} //if enemy hits a wall, change direction
		game.enemy.velocity.x = lerp(game.enemy.velocity.x, 0.0f, elapsed * game.enemy.friction);
		game.enemy.velocity.y = lerp(game.enemy.velocity.y, 0.0f, elapsed * -game.gravity);
		if (game.enemy.direction.x == 1.0f) {
			game.enemy.velocity.x += game.enemy.acceleration * elapsed;
		}
		else if (game.enemy.direction.x == -1.0f) {
			game.enemy.velocity.x -= game.enemy.acceleration * elapsed;
		}
		game.enemy.position.x += game.enemy.velocity.x * elapsed;
		game.enemy.position.y += game.enemy.velocity.y * elapsed;
		game.enemyMatrix = glm::translate(game.enemyMatrix, glm::vec3(game.enemy.position.x, game.enemy.position.y, 0.0f));
		game.playerMatrix = glm::scale(game.playerMatrix, glm::vec3(game.trumpScale, game.trumpScale, 0.0f));

		//Player - Enemy Collision
		if (game.player.EntityCollision(game.enemy)) {
			game.trumpScale += 0.5f;
			game.enemy.position.y += 1.0f;
			game.enemy.onFloor = false;
		}

		//Create Scrolling Effect
		game.viewMatrix = glm::mat4(1.0f);
		game.viewMatrix = glm::scale(game.viewMatrix, glm::vec3(2.0f, 2.0f, 0.0f));
		game.viewMatrix = glm::translate(game.viewMatrix, glm::vec3(-game.player.position.x, -game.player.position.y - 0.15f, 0.0f)); // scrolling view

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

		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		DrawText(game.program, game.font, "main menu", 0.1f, 0.0f);
		break;

	case STATE_GAME_LEVEL:
		//Screen Color
		glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);
		

		//Draw Objects, using spritesheet object example.draw(program)
		for (int i = 0; i < game.floor.size(); i++) {
			game.program.SetModelMatrix(game.floorMatrix[i]);
			game.floor[i].sprite.Draw(game.program);
		} //Floors
		//Walls
		game.program.SetModelMatrix(game.lWallMatrix);
		game.lWall.sprite.Draw(game.program);
		game.program.SetModelMatrix(game.rWallMatrix);
		game.rWall.sprite.Draw(game.program);
		//Player
		game.program.SetModelMatrix(game.playerMatrix);
		game.player.sprite.Draw(game.program);
		//enemy
		game.program.SetModelMatrix(game.enemyMatrix);
		game.enemy.sprite.Draw(game.program);
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
		Event(); //Check for input

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