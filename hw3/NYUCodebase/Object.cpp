#include "Object.h"

Object::Object() {}

Object::Object(float x, float y, float rotation, SheetSprite sprite, float width, float height, float velocity, float dirX, float dirY) : x(x), y(y), rotation(rotation), sprite(sprite), width(width), height(height), velocity(velocity), dirX(dirX), dirY(dirY), alive(true) {}

void const Object::Draw(ShaderProgram &p) {
	/*float vertices[] = {
			x + (-0.5f * width), y + (0.5f * height),
			x + (-0.5f * width), y + (-0.5f * height),
			x + (0.5f * width), y + (0.5f * height),
			x + (0.5f * width), y + (-0.5f * height),
			x + (0.5f * width), y + (0.5f * height),
			x + (-0.5f * width), y + (-0.5f * height)
			// orig coord: -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5
	};*/
	//glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	//glEnableVertexAttribArray(p.positionAttribute);

	//float uv[] = { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	/*glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, uv);
	glEnableVertexAttribArray(p.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, sprite);*/

	/*glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(p.positionAttribute);
	glDisableVertexAttribArray(p.texCoordAttribute);*/
	sprite.Draw(p);
}