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

class Enemy {
	int dx;
	ObjFile *obj;
	Texture tex;
public:
	Vector3 pos;
	float speed;
	float W, H;
	Enemy(ObjFile*, Vector3);
	void Move(int);
	void Draw(Texture*);
};


Enemy::Enemy(ObjFile *obj_in, Vector3 obj_pos) {
	W = 2;
	H = 2;
	pos = obj_pos;
	dx = 0;
	speed = 0.02;
	obj = obj_in;
}
void Enemy::Draw(Texture *tex) {
	glPushMatrix();
	pos.setCoords(pos.X(), pos.Y() + dx * speed, pos.Z());
	glTranslatef(pos.X(), pos.Y(), pos.Z());
	glScalef(0.02, 0.02, 0.02);
	glRotatef(90, 1, 0, 0);
	glRotatef(-90, 0, 1, 0);
	tex->bindTexture();
	obj->DrawObj();
	glPopMatrix();
}

void Enemy::Move(int d) {
	dx = d;
}