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
#include <iostream>
#include <random>
#include <SDL_mixer.h>

//60FPS (1.0/60.0)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

/*
	Final Project by Gabriel Ong:
	A, D for player 1 movement. W to jump, SPACE to fire bullet
	LEFT, RIGHT for player 2 movement. UP to jump, M to fire bullet

	Explanation:
	Both players have only one bullet. New bullets spawn only if both players have fired. Whoever wins 2 out of 3 maps wins.

	TODO:
	Generate a box to get a new bullet if both players have fired

	If player falls off map, kill and go to next map

	Build map 2 and 3 and change code for it
	
	Add game sounds and background music
*/

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_BETWEEN_MENU, STATE_GAME_OVER};
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

	Music Code:
	Mix_Music *music;
	music = Mix_LoadMUS( "music.mp3" );
	Playing: (int) Mix_PlayMusic(Mix_Music *music, int loops);
	*/
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	game.jumpSound = Mix_LoadWAV("jump_sound.wav");
	Mix_VolumeChunk(game.jumpSound, 15);

	/*Load Textures
	Ex: example = LoadTexture(RESOURCE_FOLDER "example.example");
	SheetSprite exampleTexture = SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size);
	^ remember to divide u, v, width and height by pixels
	*/
	GLuint character_texture = LoadTexture(RESOURCE_FOLDER "aliens.png"); //Load character spritesheet
	GLuint tiles_texture = LoadTexture(RESOURCE_FOLDER "tiles_spritesheet.png"); //Load other spritesheet for map building
	GLuint font_texture = LoadTexture(RESOURCE_FOLDER "font.png"); //Load font ssheet
	GLuint bullet_texture = LoadTexture(RESOURCE_FOLDER "bullets.png");
	game.font = font_texture;

	//Map sprites
	SheetSprite floorSprite = SheetSprite(tiles_texture, 144.0f / 1024.0f, 576.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.3f); //sprite for every floor
	SheetSprite ammoSprite = SheetSprite(tiles_texture, 0.0f / 1024.0f, 720.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.1f); //sprite for ammo

	//bullet sprites
	SheetSprite bullet1Sprite = SheetSprite(bullet_texture, 0.0f / 128.0f, 93.0f / 256.0f, 91.0f / 128.0f, 91.0f / 256.0f, 0.02f);
	SheetSprite bullet2Sprite = SheetSprite(bullet_texture, 0.0f / 128.0f, 0.0f / 256.0f, 91.0f / 128.0f, 91.0f / 256.0f, 0.02f);
	
	//sprites for animation for player1
	SheetSprite p1_standing = SheetSprite(character_texture, 676.0f / 1024.0f, 368.0f / 512.0f, 66.0f / 1024.0f, 92.0f / 512.0f, 0.15f); //default sprite for player 1
	SheetSprite p1_jumping = SheetSprite(character_texture, 478.0f / 1024.0f, 270.0f / 512.0f, 66.0f / 1024.0f, 93.0f / 512.0f, 0.15f); //jump sprite for player 1
	SheetSprite p1_walk1 = SheetSprite(character_texture, 544.0f / 1024.0f, 267.0f / 512.0f, 66.0f / 1024.0f, 93.0f / 512.0f, 0.15f); //walk1 sprite for player 1
	SheetSprite p1_walk2 = SheetSprite(character_texture, 345.0f / 1024.0f, 175.0f / 512.0f, 67.0f / 1024.0f, 96.0f / 512.0f, 0.15f); //walk1 sprite for player 1

	//sprites for animation for player2
	SheetSprite p2_standing = SheetSprite(character_texture, 676.0f / 1024.0f, 184.0f / 512.0f, 66.0f / 1024.0f, 92.0f / 512.0f, 0.15f); //default sprite for player 1
	SheetSprite p2_jumping = SheetSprite(character_texture, 345.0f / 1024.0f, 343.0f / 512.0f, 67.0f / 1024.0f, 93.0f / 512.0f, 0.15f); //jump sprite for player 1
	SheetSprite p2_walk1 = SheetSprite(character_texture, 277.0f / 1024.0f, 276.0f / 512.0f, 68.0f / 1024.0f, 93.0f / 512.0f, 0.15f); //walk1 sprite for player 1
	SheetSprite p2_walk2 = SheetSprite(character_texture, 0.0f / 1024.0f, 290.0f / 512.0f, 70.0f / 1024.0f, 96.0f / 512.0f, 0.15f); //walk1 sprite for player 1

	/* Create Objects, example:
		Object(glm::vec3 position, float width, float height, SheetSprite sprite);
		^ width is size * width / height, height is size

		Player(glm::vec3 position, glm::vec3 direction, float width, float height, glm::vec2 velocity, glm::vec2 acceleration, float friction);

		Bullet(glm::vec3 pos, float w, float h, glm::vec2 v, glm::vec2 a, SheetSprite s)
	*/

	//create map1
	for (int i = 0; i < 9; i++) {
		//Map 1 is a straight floor
		game.map1.push_back(Object(glm::vec3(-1.2f + i*(0.30f), -0.5f, 0.0f), floorSprite.size * (floorSprite.width / floorSprite.height), floorSprite.size, floorSprite));
		game.map1Matrix.push_back(glm::mat4(1.0f));
	}
	game.map1.push_back(Object(glm::vec3(0.0f, 0.0f, 0.0f), ammoSprite.size * (ammoSprite.width / ammoSprite.height), ammoSprite.size, ammoSprite)); //add Ammobox
	game.map1Matrix.push_back(glm::mat4(1.0f));

	//create map2

	//create map3

	//create player1
	game.player1 = Player(glm::vec3(0.0f), glm::vec3(1.0f), p1_standing.size * (p1_standing.width / p1_standing.height), p1_standing.size, glm::vec2(0.0f), glm::vec2(0.0f), 0.8f);
	game.player1.standingSprite = p1_standing;
	game.player1.jumpSprite = p1_jumping;
	game.player1.walk1Sprite = p1_walk1;
	game.player1.walk2Sprite = p1_walk2;
	game.player1.currentSprite = &game.player1.standingSprite;
	game.player1.bullet = new Bullet(glm::vec3(0.0f), bullet1Sprite.size * bullet1Sprite.width / bullet1Sprite.height, bullet1Sprite.size, glm::vec2(0.0f), glm::vec2(0.0f), bullet1Sprite); //create initial bullet

	//create player2
	game.player2 = Player(glm::vec3(0.0f), glm::vec3(1.0f), p2_standing.size * (p2_standing.width / p2_standing.height), p2_standing.size, glm::vec2(0.0f), glm::vec2(0.0f), 0.8f);
	game.player2.standingSprite = p2_standing;
	game.player2.jumpSprite = p2_jumping;
	game.player2.walk1Sprite = p2_walk1;
	game.player2.walk2Sprite = p2_walk2;
	game.player2.currentSprite = &game.player2.standingSprite;
	game.player2.bullet = new Bullet(glm::vec3(0.0f), bullet2Sprite.size * bullet2Sprite.width / bullet2Sprite.height, bullet2Sprite.size, glm::vec2(0.0f), glm::vec2(0.0f), bullet2Sprite); //create initial bullet

	//random number gen
	srand(static_cast <unsigned> (time(0)));
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
			else if (mode == STATE_BETWEEN_MENU) {
				mode = STATE_GAME_LEVEL;
				game.map++;
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_W) {
				if (game.player1.state != Player::STATE_JUMPING) {
					game.player1.position.y += 0.02f;
					game.player1.velocity.y = 1.0f;
					game.player1.onFloor = false;

					//JumpSound
					Mix_PlayChannel(-1, game.jumpSound, 0);
				}
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_P) {
				game.done = true;
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				game.player1.bullet->state = Bullet::STATE_FIRED;
				if (game.player1.direction.x > 0) {
					game.player1.bullet->acceleration.x = 1.5f;
				}
				else {
					game.player1.bullet->acceleration.x = -1.5f;
				}
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_M) {
				game.player2.bullet->state = Bullet::STATE_FIRED;
				if (game.player2.direction.x > 0) {
					game.player2.bullet->acceleration.x = 1.5f;
				}
				else {
					game.player2.bullet->acceleration.x = -1.5f;
				}
			}
			else if (game.event.key.keysym.scancode == SDL_SCANCODE_UP) {
				if (game.player2.state != Player::STATE_JUMPING) {
					game.player2.position.y += 0.02f;
					game.player2.velocity.y = 1.0f;
					game.player2.onFloor = false;

					//JumpSound
					Mix_PlayChannel(-1, game.jumpSound, 0);
				}
			}
		}
	}

	//For polling input, ex. if (keys[SDL_SCANCODE_A]) {}
	if (keys[SDL_SCANCODE_A]) {
		game.player1.acceleration.x = -0.4f;
		if (game.player1.state != Player::STATE_JUMPING) {
			game.player1.state = Player::STATE_WALKING;
			game.player1.direction.x = -1.0f;
		}
	}
	else if (keys[SDL_SCANCODE_D]) {
		game.player1.acceleration.x = 0.4f;
		if (game.player1.state != Player::STATE_JUMPING) {
			game.player1.state = Player::STATE_WALKING;
			game.player1.direction.x = 1.0f;
		}
	}
	else {
		game.player1.acceleration.x = 0.0f;
		if (game.player1.state != Player::STATE_JUMPING) {
			game.player1.state = Player::STATE_STANDING;
		}
	}

	if (keys[SDL_SCANCODE_LEFT]) {
		game.player2.acceleration.x = -0.4f;
		if (game.player2.state != Player::STATE_JUMPING) {
			game.player2.state = Player::STATE_WALKING;
			game.player2.direction.x = -1.0f;
		}
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		game.player2.acceleration.x = 0.4f;
		if (game.player2.state != Player::STATE_JUMPING) {
			game.player2.state = Player::STATE_WALKING;
			game.player2.direction.x = 1.0f;
		}
	}
	else {
		game.player2.acceleration.x = 0.0f;
		if (game.player2.state != Player::STATE_JUMPING) {
			game.player2.state = Player::STATE_STANDING;
		}
	}
}

//Updating, Move all objects based on time and velocity
void Update(float elapsed) {
	switch (mode) {
	case STATE_MAIN_MENU:
		game.textMatrix = glm::mat4(1.0f);
		game.textMatrix = glm::translate(game.textMatrix, glm::vec3(-0.5f, 0.0f, 0.0f));
		break;
	case STATE_BETWEEN_MENU:
		game.textMatrix = glm::mat4(1.0f);
		game.textMatrix = glm::translate(game.textMatrix, glm::vec3(-0.5f, 0.0f, 0.0f));
		break;
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

		//animation
		game.animationElapsed += elapsed;

		//build map
		if (game.map == 1) {
			for (int i = 0; i < 9; i++) {
				game.map1Matrix[i] = glm::mat4(1.0f);
				game.map1Matrix[i] = glm::translate(game.map1Matrix[i], glm::vec3(game.map1[i].position.x, game.map1[i].position.y, 0.0f));
			}
		}
		else if (game.map == 2) {

		}
		else if (game.map == 3) {

		}
		

		//Player 1
			//Check if player is touching the ground and do gravity on player
		if (!game.player1.onFloor) {
			bool collision = false;
			for (int i = 0; i < 9; i++) {
				if (game.player1.EntityCollision(game.map1[i])) {
					collision = true;
					game.player1.velocity.y = 0.0f;
					game.player1.onFloor = true;
					game.player1.state = Player::STATE_STANDING;
				}
			}
			if (!collision) {
				game.player1.onFloor = false;
				game.player1.velocity.y += game.gravity * elapsed; // gravity
				game.player1.state = Player::STATE_JUMPING;
			}
		}
		else {
			//check if player has run off floor
			bool fallenOff = true;
			for (int i = 0; i < 9; i++) {
				if (game.player1.EntityCollision(game.map1[i])) {
					fallenOff = false;
					game.player1.velocity.y = 0.0f;
					game.player1.onFloor = true;
				}
			}
			if (fallenOff) {
				game.player1.onFloor = false;
				game.player1.state = Player::STATE_JUMPING;
			}
		}
			//player1 movement
		game.player1Matrix = glm::mat4(1.0f);
		game.player1.velocity.x = lerp(game.player1.velocity.x, 0.0f, elapsed * game.player1.friction);
		game.player1.velocity.y = lerp(game.player1.velocity.y, 0.0f, elapsed * -game.gravity);
		game.player1.velocity.x += game.player1.acceleration.x * elapsed;
		game.player1.position.x += game.player1.velocity.x * elapsed;
		game.player1.position.y += game.player1.velocity.y * elapsed;
		game.player1Matrix = glm::translate(game.player1Matrix, glm::vec3(game.player1.position.x, game.player1.position.y, 0.0f));
		
			//player 1 animation
		if (game.player1.state == Player::STATE_STANDING) {
			game.player1.currentSprite = &game.player1.standingSprite;
		}
		else if (game.player1.state == Player::STATE_JUMPING) {
			game.player1.currentSprite = &game.player1.jumpSprite;
		}
		else {
			//for walking
			if (game.player1.currentSprite == &game.player1.walk1Sprite && game.player1.currentIndex == 0) {
				game.player1.currentSprite = &game.player1.walk2Sprite;
			}
			else if (game.player1.currentSprite == &game.player1.walk2Sprite && game.player1.currentIndex == 1) {
				game.player1.currentSprite = &game.player1.walk1Sprite;
			}
			else {
				game.player1.currentSprite = &game.player1.walk1Sprite;
			}
		}
			//flip player depending on direction
		game.player1Matrix = glm::scale(game.player1Matrix, glm::vec3(game.player1.direction.x, 1.0f, 0.0f));

			//player1 bullet movement
		game.bullet1Matrix = glm::mat4(1.0f);
		game.player1.bullet->velocity.x += game.player1.bullet->acceleration.x * elapsed;
		game.player1.bullet->position.x += game.player1.bullet->velocity.x * elapsed;
		if (game.player1.bullet->state == Bullet::STATE_UNFIRED) {
			game.player1.bullet->position = game.player1.position;
		}
		game.bullet1Matrix = glm::translate(game.bullet1Matrix, glm::vec3(game.player1.bullet->position.x, game.player1.bullet->position.y, 0.0f));

			//bullet collision
		if (game.player1.bullet->state == Bullet::STATE_FIRED) {
			if (game.player1.bullet->position.x > 2.0f || game.player1.bullet->position.x < -2.0f) {
				game.player1.bullet->state = Bullet::STATE_DESTROYED;
			}
			else if (game.player1.bullet->EntityCollision(game.player2)) {
				game.player1.bullet->state = Bullet::STATE_DESTROYED;
				game.map1Winner = &game.player1;
				mode = STATE_BETWEEN_MENU;
			}
		}

		//Player 2
			//Check if player is touching the ground and do gravity on player
		if (!game.player2.onFloor) {
			bool collision = false;
			for (int i = 0; i < 9; i++) {
				if (game.player2.EntityCollision(game.map1[i])) {
					collision = true;
					game.player2.velocity.y = 0.0f;
					game.player2.onFloor = true;
					game.player2.state = Player::STATE_STANDING;
				}
			}
			if (!collision) {
				game.player2.onFloor = false;
				game.player2.velocity.y += game.gravity * elapsed; // gravity
				game.player2.state = Player::STATE_JUMPING;
			}
		}
		else {
			//check if player has run off floor
			bool fallenOff = true;
			for (int i = 0; i < 9; i++) {
				if (game.player2.EntityCollision(game.map1[i])) {
					fallenOff = false;
					game.player2.velocity.y = 0.0f;
					game.player2.onFloor = true;
				}
			}
			if (fallenOff) {
				game.player2.onFloor = false;
				game.player2.state = Player::STATE_JUMPING;
			}
		}
		//player1 movement
		game.player2Matrix = glm::mat4(1.0f);
		game.player2.velocity.x = lerp(game.player2.velocity.x, 0.0f, elapsed * game.player2.friction);
		game.player2.velocity.y = lerp(game.player2.velocity.y, 0.0f, elapsed * -game.gravity);
		game.player2.velocity.x += game.player2.acceleration.x * elapsed;
		game.player2.position.x += game.player2.velocity.x * elapsed;
		game.player2.position.y += game.player2.velocity.y * elapsed;
		game.player2Matrix = glm::translate(game.player2Matrix, glm::vec3(game.player2.position.x, game.player2.position.y, 0.0f));

		//player 1 animation
		if (game.player2.state == Player::STATE_STANDING) {
			game.player2.currentSprite = &game.player2.standingSprite;
		}
		else if (game.player2.state == Player::STATE_JUMPING) {
			game.player2.currentSprite = &game.player2.jumpSprite;
		}
		else {
			//for walking
			if (game.player2.currentSprite == &game.player2.walk1Sprite && game.player2.currentIndex == 0) {
				game.player2.currentSprite = &game.player2.walk2Sprite;
			}
			else if (game.player2.currentSprite == &game.player2.walk2Sprite && game.player2.currentIndex == 1) {
				game.player2.currentSprite = &game.player2.walk1Sprite;
			}
			else {
				game.player2.currentSprite = &game.player2.walk1Sprite;
			}
		}
		//flip player depending on direction
		game.player2Matrix = glm::scale(game.player2Matrix, glm::vec3(game.player2.direction.x, 1.0f, 0.0f));

		//player1 bullet movement
		game.bullet2Matrix = glm::mat4(1.0f);
		game.player2.bullet->velocity.x += game.player2.bullet->acceleration.x * elapsed;
		game.player2.bullet->position.x += game.player2.bullet->velocity.x * elapsed;
		if (game.player2.bullet->state == Bullet::STATE_UNFIRED) {
			game.player2.bullet->position = game.player2.position;
		}
		game.bullet2Matrix = glm::translate(game.bullet2Matrix, glm::vec3(game.player2.bullet->position.x, game.player2.bullet->position.y, 0.0f));

		//bullet collision
		if (game.player2.bullet->state == Bullet::STATE_FIRED) {
			if (game.player2.bullet->position.x > 2.0f || game.player2.bullet->position.x < -2.0f) {
				game.player2.bullet->state = Bullet::STATE_DESTROYED;
			}
			else if (game.player2.bullet->EntityCollision(game.player1)) {
				game.player2.bullet->state = Bullet::STATE_DESTROYED;
				game.map1Winner = &game.player2;
				mode = STATE_BETWEEN_MENU;
			}
		}

		//Ammobox
		if (game.player1.bullet->state == Bullet::STATE_DESTROYED && game.player2.bullet->state == Bullet::STATE_DESTROYED) {
			game.map1Matrix[9] = glm::mat4(1.0f);
			float rand = static_cast <float> (-0.5f) + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.5f - (-0.5f))));
			game.map1Matrix[9] = glm::translate(game.map1Matrix[9], glm::vec3(game.map1[9].position.x, game.map1[9].position.y, 0.0f));
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

		game.program.SetModelMatrix(game.textMatrix);

		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		DrawText(game.program, game.font, "Odin ... You have one shot", 0.08f, 0.0f);

		game.textMatrix = glm::translate(game.textMatrix, glm::vec3(-0.5f, -0.4f, 0.0f));
		game.program.SetModelMatrix(game.textMatrix);
		DrawText(game.program, game.font, "Press Any Key To Continue and P to Quit", 0.05f, 0.0f);

		break;
	case STATE_BETWEEN_MENU:
		//Set matrices
		game.program.SetProjectionMatrix(game.projectionMatrix);
		game.program.SetViewMatrix(game.viewMatrix);

		game.program.SetModelMatrix(game.textMatrix);

		//DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) to draw font
		if (game.map1Winner == &game.player1) {
			//Screen Color
			glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			DrawText(game.program, game.font, "Blue wins round 1", 0.08f, 0.0f);
		}
		else {
			//Screen Color
			glClearColor(0.2f, 0.8f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			DrawText(game.program, game.font, "Green wins round 1", 0.08f, 0.0f);
		}

		game.textMatrix = glm::translate(game.textMatrix, glm::vec3(-0.5f, -0.4f, 0.0f));
		game.program.SetModelMatrix(game.textMatrix);
		DrawText(game.program, game.font, "Press Any Key To Continue", 0.05f, 0.0f);
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
			game.map1[i].sprite.Draw(game.program);
		} // draw map objects
		

		//Player1
		game.program.SetModelMatrix(game.player1Matrix);
		game.player1.currentSprite->Draw(game.program);
		if (game.player1.bullet->state == Bullet::STATE_FIRED) {
			game.program.SetModelMatrix(game.bullet1Matrix);
			game.player1.bullet->sprite.Draw(game.program);
		}

		//Player2
		game.program.SetModelMatrix(game.player2Matrix);
		game.player2.currentSprite->Draw(game.program);
		if (game.player2.bullet->state == Bullet::STATE_FIRED) {
			game.program.SetModelMatrix(game.bullet2Matrix);
			game.player2.bullet->sprite.Draw(game.program);
		}

		//Animation
		if (game.animationElapsed > 1.0 / game.fps) {
			game.animationElapsed = 0.0f;
			game.player1.currentIndex++;
			game.player2.currentIndex++;

			if (game.player1.currentIndex > 0) {
				game.player1.currentIndex = 0;
			}
			if (game.player2.currentIndex > 0) {
				game.player2.currentIndex = 0;
			}
		}

		break;
	}
}

//Clean up memory
void Clean() {
	//remove sounds, Mix_FreeChunk(game.jumpSound);
	Mix_FreeChunk(game.jumpSound);

	delete game.player1.bullet;
	delete game.player2.bullet;
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