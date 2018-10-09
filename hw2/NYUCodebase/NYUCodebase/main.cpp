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

// GLOBAL VARIABLES :(
SDL_Window* displayWindow;
//loading shaders
ShaderProgram program;
//load Textures
GLuint paddleTex = LoadTexture(RESOURCE_FOLDER"paddle.png");
//Time Code
float lastFrameTicks = 0.0f;
float ticks;
float elapsed;
//Transformation var
float angle = 0.0f;
//Event var
SDL_Event event;
//Loop var
bool done = false;
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrixPaddle = glm::mat4(1.0f);
glm::mat4 modelMatrixBall = glm::mat4(1.0f);
//object var
float p1x = 0.0;
float p2x = 0.0;
float p1y = 0.0;
float p2y = 0.0;
float velocity = 3.0f;

//Setup Function
void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("GO", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	//setup
	glViewport(0, 0, 1280, 720);
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	//blending code
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program.programID);

	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
}

//Check for events
void Event() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_W) {
				p1y += velocity * elapsed;
				modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, velocity * elapsed, 0.0f));
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
				p1y -= velocity * elapsed;
				modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, -velocity * elapsed, 0.0f));
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			// event.x is the click x position
			// event.y is the click y position
			// event.button.button is the mouse button that was click
		}
	}
}

//Updating things in game 
void Update() {
	//Time code
	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	//Paddle Setup
	program.SetModelMatrix(modelMatrixPaddle);
	glBindTexture(GL_TEXTURE_2D, paddleTex);
	//creating first object
	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Screen Color
	glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//Color
	//program.SetColor(0.5f, 0.4f, 0.4f, 1.0f);
	//Set camera view
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	//Draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Clean up memory
void Clean() {
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
	Setup();

	while (!done) {
		Event();
		Update();
		Clean();
	}

	SDL_Quit();
	return 0;
}