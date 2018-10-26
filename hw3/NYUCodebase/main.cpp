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

//60FPS (1.0/60.0)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

/*
	Template: Updated for homework 3
*/

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
//Gamestate also holds global variables
GameState game;
GameMode mode = STATE_MAIN_MENU;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

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
	glDrawArrays(GL_TRIANGLES, 0, 6*text.size());

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
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

	//Load Textures
	//Ex: example = LoadTexture(RESOURCE_FOLDER "example.example"); SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
	game.sheet = LoadTexture(RESOURCE_FOLDER "sprites.png");
	game.font = LoadTexture(RESOURCE_FOLDER "pixel_font.png");
	SheetSprite playerTexture = SheetSprite(game.sheet, 0.0f, 46.0f / 128.0f, 59.0f / 128.0f, 62.0f / 128.0f, 0.2f);
	SheetSprite bulletTexture = SheetSprite(game.sheet, 0.0f, 110.0f / 128.0f, 28.0f / 128.0f, 14.0f / 128.0f, 0.02f);
	SheetSprite enemyTexture = SheetSprite(game.sheet, 0.0f, 0.0f, 83.0f / 128.0f, 44.0f / 128.0f, 0.2f);

	/* Create Objects, example:
		Object example(xposition, yposition, rotation (angle), spritesheet, width, height, velocity, direction x, direction y);
	*/
	//Create Player
	game.player = Object(0.0f, 0.0f, 0.0f, playerTexture, 25.0f, 25.0f, 0.2f, 1.0f, 1.0f);
		//set initial player position
	float angle = 90.0f * (3.1415926f / 180.0f);
	game.playerMatrix = glm::rotate(game.playerMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(-0.8f, 0.0f, 0.0f));

	//Create Bullet
	game.fire = false;
	game.bullet = Object(0.0f, 0.0f, 0.0f, bulletTexture, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	game.bulletMatrix = glm::rotate(game.bulletMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));

	//Create Enemies
	for (int i = 0; i < 4; i++) {
		game.enemies.push_back(Object(0.0f, 0.0f, 0.0f, enemyTexture, 0.0f, 0.0f, 1.0f, 1.0f, .0f));
	}

	//Time
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	game.elapsed = ticks - game.lastFrameTicks;
	game.lastFrameTicks = game.elapsed;
}

//Process inputs
void Event(float elapsed) {
	while (SDL_PollEvent(&game.event)) {
		if (game.event.type == SDL_QUIT || game.event.type == SDL_WINDOWEVENT_CLOSE) {
			game.done = true;
		}
		else if (game.event.type == SDL_KEYDOWN ) {
			if (mode == STATE_MAIN_MENU) {
				mode = STATE_GAME_LEVEL;
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				game.fire = true;
			}
		}
		//For single input
	}

	//For polling input
	if (keys[SDL_SCANCODE_A]) {
		game.player.x -= game.player.velocity * elapsed;
		game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(0.0f, game.player.velocity * elapsed, 0.0f));
		if (!game.fire) { //if bullet hasn't been fired, move it with the player
			game.bulletMatrix = glm::translate(game.bulletMatrix, glm::vec3(0.0f, game.bullet.velocity * elapsed, 0.0f));
		}
	}
	else if (keys[SDL_SCANCODE_D]) {
		game.player.x += game.player.velocity * elapsed;
		game.playerMatrix = glm::translate(game.playerMatrix, glm::vec3(0.0f, -game.player.velocity * elapsed, 0.0f));
		if (!game.fire) {
			game.bulletMatrix = glm::translate(game.bulletMatrix, glm::vec3(0.0f, game.bullet.velocity * elapsed, 0.0f));
		}
	}
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	/*
		Translation exmaple: modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f (x), 1.0f (y), keep 0); 
	*/
	switch (mode) {
	case STATE_GAME_LEVEL:
		//Bullet
		if (game.fire) {
			game.bulletMatrix = glm::translate(game.bulletMatrix, glm::vec3(game.bullet.velocity * elapsed, 0.0f, 0.0f));
		}
		/*if (!game.fire) {
			//when bullet expires, move to player
			game.bullet.x = game.player.x;
			game.bullet.y = game.player.y;
			game.bulletMatrix = glm::translate(game.bulletMatrix, glm::vec3(-1.0f, 0.0f, 0.0f));
		}*/
		
		break;
	}
}

//Render all objects in the game, render UI elements
void Render() {
	switch (mode) {
	case STATE_MAIN_MENU:
		//Set Matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);

		game.program.SetModelMatrix(game.textMatrix);
		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		DrawText(game.program, game.font, "Press Any Key To Continue", 0.05f, 0.0f);
	break;
	case STATE_GAME_LEVEL:
		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);

		//Screen Color
		glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Draw Objects, example.draw(program)
		game.program.SetModelMatrix(game.playerMatrix);
		game.player.Draw(game.program);
			//draw bullet only if it has been fired
		if (game.fire) {
			game.bullet.Draw(game.program);
		}
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
		Event(FIXED_TIMESTEP);

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

		Render();

		SDL_GL_SwapWindow(game.displayWindow);
	}

	Clean();

	SDL_Quit();
	return 0;
}