#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <ctime>

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

/*
	Pong game by Gabriel Ong, 10/9/2018
	WD for p1 and Arrows for P2, whoever last won will have a different paddle.
*/

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
//init textures
GLuint paddleTex;
GLuint winTex;
GLuint ballTex;
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
glm::mat4 modelMatrixPaddle2 = glm::mat4(1.0f);
//object var
float paddlePosition = 0.0f; //y Positions
float paddlePosition2 = 0.0f;
float p1Center = 0.0f; //position at the center for collision range check
float p2Center = 0.0f;
float velocity = 3.0f; //velocity of movement
float ballx = (float)(rand() % 5 + 1);
float bally = (float)(rand() % 8 - 1);
float ballposx = 0.0f;
float ballposy = 0.0f;
bool rightW = false;
bool leftW = false;

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
	program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	//blending code
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(program.programID);

	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

	//Load textures
	paddleTex = LoadTexture(RESOURCE_FOLDER "paddle.png");
	ballTex = LoadTexture(RESOURCE_FOLDER "ball.png");
	winTex = LoadTexture(RESOURCE_FOLDER "paddleWin.png");

	//initial states
	modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(-1.7f, 0.0f, 0.0f));
	modelMatrixPaddle2 = glm::translate(modelMatrixPaddle2, glm::vec3(1.7f, 0.0f, 0.0f));
	modelMatrixBall = glm::scale(modelMatrixBall, glm::vec3(0.2f, 0.2f, 1.0f));
	srand(time(NULL)); //rng for ball
}

//Check for events
void Event() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
		/*else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_W) {
				paddlePosition += velocity * elapsed;
				modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, velocity * elapsed, 0.0f));
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
				paddlePosition -= velocity * elapsed;
				modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, -velocity * elapsed, 0.0f));
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
				paddlePosition2 += velocity * elapsed;
				modelMatrixPaddle2 = glm::translate(modelMatrixPaddle2, glm::vec3(0.0f, velocity * elapsed, 0.0f));
			}
			else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
				paddlePosition2 -= velocity * elapsed;
				modelMatrixPaddle2 = glm::translate(modelMatrixPaddle2, glm::vec3(0.0f, -velocity * elapsed, 0.0f));
			}
		}*/
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_W]) {
		paddlePosition += velocity * elapsed;
		modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, velocity * elapsed, 0.0f));
		p1Center += 1;
	}
	else if (keys[SDL_SCANCODE_S]) {
		paddlePosition -= velocity * elapsed;
		modelMatrixPaddle = glm::translate(modelMatrixPaddle, glm::vec3(0.0f, -velocity * elapsed, 0.0f));
		p1Center -= 1;
	}
	if (keys[SDL_SCANCODE_UP]) {
		paddlePosition2 += velocity * elapsed;
		modelMatrixPaddle2 = glm::translate(modelMatrixPaddle2, glm::vec3(0.0f, velocity * elapsed, 0.0f));
		p2Center += 1;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		paddlePosition2 -= velocity * elapsed;
		modelMatrixPaddle2 = glm::translate(modelMatrixPaddle2, glm::vec3(0.0f, -velocity * elapsed, 0.0f));
		p2Center -= 1;
	}
}

//Updating and Rendering
void Update() {
	//Time code
	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	//Screen Color
	glClearColor(0.2f, 0.4f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Set camera view
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	//Paddle Setup
	program.SetModelMatrix(modelMatrixPaddle);
	if (leftW) {
		glBindTexture(GL_TEXTURE_2D, winTex);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, paddleTex);
	}
	//creating first paddle
	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 }; //vertex array counter clockwise
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//second paddle
	program.SetModelMatrix(modelMatrixPaddle2);
	if (rightW) {
		glBindTexture(GL_TEXTURE_2D, winTex);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, paddleTex);
	}
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Ball
	program.SetModelMatrix(modelMatrixBall);
	glBindTexture(GL_TEXTURE_2D, ballTex);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Ball movement and collision
	if (ballposy >= 4.45f || ballposy <= -4.45f) {
		bally *= -1;
	} // Top and Bottom collision
	//Win Conditions
	//contact with left or right
	if (ballposx <= -7.5f && p1Center - 5 < ballposy <= p1Center + 5) {
		ballx *= -1;
	}
	else if (ballposx >= 7.5f && p2Center - 5 < ballposy <= p2Center + 5) {
		ballx *= -1;
	}
	else if (ballposx == 8.0f) {
		ballx *= -1;
		leftW = true;
		rightW = false;
	}
	else if (ballposx == -8.0f) {
		ballx *= -1;
		leftW = false;
		rightW = true;
	}
	modelMatrixBall = glm::translate(modelMatrixBall, glm::vec3(ballx * elapsed, bally * elapsed, 0.0f));
	ballposx += ballx * elapsed;
	ballposy += bally * elapsed;
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