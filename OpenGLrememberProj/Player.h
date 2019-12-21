#pragma once
#include <windows.h>
#include <iostream>

#include "ObjLoader.h"
#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"
#include <string>
#include <math.h>
#include "MyVector3d.h"
#include "Texture.h"
#include "MyOGL.h"

using namespace std;

class Player {
	float speed, fair_speed;
	int dx;
public:
	Vector3 pos;
	Player(Vector3);
	void Keys();
	void Draw(ObjFile*, Texture*);
};

Player::Player(Vector3 obj_pos) {
	pos = obj_pos;
	dx = 0;
	speed = 0.5;
	fair_speed = 5;
}
void Player::Keys() {
	if (OpenGL::isKeyPressed('D'))
	{
		dx = 1;
	}
	if (OpenGL::isKeyPressed('A'))
	{
		dx = -1;
	}
}
void Player::Draw(ObjFile *obj, Texture *tex) {
	glPushMatrix();
	pos.setCoords(pos.X(), pos.Y() + dx * speed, pos.Z());
	glTranslatef(pos.X(), pos.Y(), pos.Z());
	glScalef(0.01, 0.01, 0.01);
	glRotatef(90, 1, 0, 0);
	glRotatef(-90, 0, 1, 0);
	tex->bindTexture();
	obj->DrawObj();
	dx = 0;
	glPopMatrix();
}