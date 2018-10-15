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

//60FPS (1.0/60.0)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

/*
	Template: Updated for homework 2
*/

// GLOBAL VARIABLES for the game world
SDL_Window* displayWindow;
//loading shaders
ShaderProgram program;
//Texture init, example: GLuint example;
//Time Code
/*Time Code
OLD CODE:
float lastFrameTicks = 0.0f;
float ticks;*/
float elapsed;
float accumulator = 0.0f;
//Event var
SDL_Event event;
//Loop var
bool done = false;
//Matrices
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f); //Model Matrices

class Object {
public:
	Object(float x, float y, float rotation, int textureID, float width, float height, float velocity, float dirX, float dirY) : x(x), y(y), rotation(rotation), textureID(textureID), width(width), height(height), velocity(velocity), dirX(dirX), dirY(dirY) {}

	void Draw(ShaderProgram &p) {
		float vertices[] = {
			x + (-0.5f * width), y + (0.5f * height),
			x + (-0.5f * width), y + (-0.5f * height),
			x + (0.5f * width), y + (0.5f * height),
			x + (0.5f * width), y + (-0.5f * height),
			x + (0.5f * width), y + (0.5f * height),
			x + (-0.5f * width), y + (-0.5f * height)
			// orig coord: -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5
		};

		glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(p.positionAttribute);

		float uv[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
		
		glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, uv);
		glEnableVertexAttribArray(p.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(p.positionAttribute);
		glDisableVertexAttribArray(p.texCoordAttribute);
	}

	//member var
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float velocity;
	float dirX;
	float dirY;

	//Added member var
};

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

//Setup Function
void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("GO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//Viewport and program setup
	glViewport(0, 0, 1280, 720);
	program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	glUseProgram(program.programID);
	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	//For Alpha Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Load Textures
	//Ex: example = LoadTexture(RESOURCE_FOLDER "example.example");

	/* Create Objects, example:
		Object example(xposition, yposition, rotation (angle), textureID, width, height, velocity, direction x, direction y);
	*/
}

//Check for events
void Event() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		//For single input
	}

	//For polling input
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
}

//Updating
void Update(float time) {
	/* Time code, old example:
	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;*/
	//Set matrices
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetViewMatrix(modelMatrix);

	//Changes
	/*
		Translation exmaple: modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f (x), 1.0f (y), keep 0); 
	*/
}

void Render() {
	//Screen Color
	glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Draw Objects, example.draw(program)
}

//Clean up memory
void Clean() {
	//glDisableVertexAttribArray(program.positionAttribute);
	//glDisableVertexAttribArray(program.texCoordAttribute);
}

int main(int argc, char *argv[])
{
	Setup();

	while (!done) {
		Event();

		//60 FPS updated time code
		elapsed += accumulator;
		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}
		while (elapsed >= FIXED_TIMESTEP) {
			Update(FIXED_TIMESTEP);
			elapsed -= FIXED_TIMESTEP;
		}
		accumulator = elapsed;

		Render();

		SDL_GL_SwapWindow(displayWindow);
	}

	Clean();

	SDL_Quit();
	return 0;
}