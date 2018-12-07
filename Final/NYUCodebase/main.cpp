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
	Template: Needs update to objects
*/

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
//Gamestate also holds global variables
GameState game;
GameMode mode = STATE_MAIN_MENU;
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

	/*
	Sound Code: (int) Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
	^ Initializes SDL_mixer with frequency, format, channel and buffer size.

	Load Sound: Mix_Chunk *someSound;
	someSound = Mix_LoadWAV(“some_sound.wav");

	Playing Sound: (int) Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
	Setting Volume: Mix_VolumeMusic(30); // set music volume (from 0 to 128), Mix_VolumeChunk(shootSound, 10); // set sound volume (from 0 to 128)
	*/

	/*
	Music Code:
	Mix_Music *music;
	music = Mix_LoadMUS( "music.mp3" );
	Playing: (int) Mix_PlayMusic(Mix_Music *music, int loops);
	*/

	/*Load Textures
	Ex: example = LoadTexture(RESOURCE_FOLDER "example.example");
	SheetSprite exampleTexture = SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
	^ remember to divide u, v, width and height by pixels
	*/
	GLuint character_texture = LoadTexture(RESOURCE_FOLDER "aliens.png"); //Load character spritesheet
	GLuint tiles_texture = LoadTexture(RESOURCE_FOLDER "tiles_spritesheet.png"); //Load other spritesheet for map building
	GLuint font_texture = LoadTexture(RESOURCE_FOLDER "font.png"); //Load font ssheet
	game.font = font_texture;

	//Map sprites
	SheetSprite floorSprite = SheetSprite(tiles_texture, 144.0f / 1024.0f, 576.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.2f); //sprite for every floor
	
	//sprites for animation
	SheetSprite p1_standing = SheetSprite(character_texture, 676.0f / 1024.0f, 368.0f / 512.0f, 66.0f / 1024.0f, 92.0f / 512.0f, 0.5f); //default sprite for player 1

	/* Create Objects, example:
		Object(glm::vec3 position, glm::vec3 direction, float width, float height, glm::vec2 velocity, glm::vec2 acceleration);
		^ remember to intialize width and height by the sprite values * size, and to set sprites after
	*/
	game.map1.push_back(Object(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f), 70.0f / 1024.0f, 70.0f / 1024.0f, glm::vec2(0.0f), glm::vec2(0.0f)));
	game.map1[0].defaultSprite = floorSprite;
	game.map1Matrix.push_back(glm::mat4(1.0f));
	game.map1.push_back(Object(glm::vec3(0.0f), glm::vec3(0.0f), 70.0f / 1024.0f, 70.0f / 1024.0f, glm::vec2(0.0f), glm::vec2(0.0f)));
	game.map1[1].defaultSprite = floorSprite;
	game.map1Matrix.push_back(glm::mat4(1.0f));

	game.player = Object(glm::vec3(0.0f), glm::vec3(0.0f), 70.0f / 1024.0f, 70.0f / 1024.0f, glm::vec2(0.0f), glm::vec2(0.0f));
	game.player.defaultSprite = p1_standing;
}

//Process inputs
void Event() {
	while (SDL_PollEvent(&game.event)) {
		if (game.event.type == SDL_QUIT || game.event.type == SDL_WINDOWEVENT_CLOSE) {
			game.done = true;
		}
		else if (game.event.type == SDL_KEYDOWN) {
			if (mode == STATE_MAIN_MENU) {
				mode = STATE_GAME_LEVEL;
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				
			}
		}
	}

	//For polling input, ex. if (keys[SDL_SCANCODE_A]) {}
	
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	switch (mode) {
	case STATE_GAME_LEVEL:
		//Changes
		/*
			Matrix exmaple: modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f (x), 1.0f (y), keep 0);
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
		for (int i = 0; i < game.map1Matrix.size(); i++) {
			game.map1Matrix[i] = glm::mat4(1.0f);
			game.map1Matrix[i] = glm::translate(game.map1Matrix[i], glm::vec3(game.map1[i].position.x, game.map1[i].position.y, 0.0f));
		}

		break;
	}
}

//Render all objects in the game, render UI elements
void Render() {
	switch (mode) {
	case STATE_MAIN_MENU:
		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);
		game.program.SetModelMatrix(game.player1Matrix);

		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		DrawText(game.program, game.font, "Odin ... You have one shot", 0.05f, 0.0f);

		break;
	case STATE_GAME_LEVEL:
		//Screen Color
		glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);
		game.program.SetModelMatrix(game.player1Matrix);

		//Draw Objects, using spritesheet object example.draw(program)
		for (int i = 0; i < game.map1.size(); i++) {
			game.program.SetModelMatrix(game.map1Matrix[i]);
			game.map1[i].defaultSprite.Draw(game.program);
		} // draw map objects
		

		//Player
		break;
	}
}

//Clean up memory
void Clean() {
	//remove sounds, Mix_FreeChunk(game.jumpSound);
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