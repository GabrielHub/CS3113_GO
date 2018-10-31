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
	Template: Updated for homework 3
*/

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
//Gamestate also holds global variables
GameState state;
GameMode mode = STATE_MAIN_MENU;
const Uint8 *keys = SDL_GetKeyboardState(NULL);

//Collision detection for 2 objects
bool detectCollision(Object one, Object two) {
	if (one.x >= two.x - two.width / 2 && one.x <= two.x + two.width / 2) {
		return true;
		if (one.y >= two.y - two.height / 2 && one.y <= two.y + two.height / 2) {
			return true;
		}
	}
	return false;
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

//Function to convert strings to text using spritesheets
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

//Setup Function
void Setup(GameState &state) {
	SDL_Init(SDL_INIT_VIDEO);
	state.displayWindow = SDL_CreateWindow("GO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(state.displayWindow);
	SDL_GL_MakeCurrent(state.displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//Viewport and program setup
	glViewport(0, 0, 1280, 720);
	state.program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	glUseProgram(state.program.programID);
	state.projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	//For Alpha Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Load Textures
	//Ex: example = LoadTexture(RESOURCE_FOLDER "example.example");
	state.test = LoadTexture(RESOURCE_FOLDER "test.jpg");

	/* Create Objects, example:
		Object example(xposition, yposition, rotation (angle), textureID, width, height, velocity, direction x, direction y);
	*/
}

//Process inputs
void Event() {
	while (SDL_PollEvent(&state.event)) {
		if (state.event.type == SDL_QUIT || state.event.type == SDL_WINDOWEVENT_CLOSE) {
			state.done = true;
		}
		/*For single input, Exmaple:
		else if (game.event.type == SDL_KEYDOWN)
		*/
	}

	//For polling input, ex. if (keys[SDL_SCANCODE_A])
	
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	/* Time code, old example:
	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;*/
	//Set matrices
	state.program.SetProjectionMatrix(state.projectionMatrix);
	state.program.SetViewMatrix(state.viewMatrix);
	state.program.SetModelMatrix(state.modelMatrix);
	//Changes
	/*
		Translation exmaple: modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f (x), 1.0f (y), keep 0); 
	*/
}

//Render all objects in the game, render UI elements
void Render() {
	//Screen Color
	glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Draw Objects, example.draw(program)
	//state.player.Draw(state.program);
}

//Clean up memory
void Clean() {
	
}

int main(int argc, char *argv[])
{
	Setup(state);

	while (!state.done) {
		Event(); //Check for input

		//Game code
		state.elapsed += state.accumulator;
		if (state.elapsed < FIXED_TIMESTEP) {
			state.accumulator = state.elapsed;
			continue;
		}
		//60 FPS updated time code
		while (state.elapsed >= FIXED_TIMESTEP) {
			Update(FIXED_TIMESTEP);
			
			state.elapsed -= FIXED_TIMESTEP;
		}
		state.accumulator = state.elapsed;

		//Graphics
		Render();

		SDL_GL_SwapWindow(state.displayWindow);
	}

	//Clear memory
	Clean();

	SDL_Quit();
	return 0;
}