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

class NPC {
	int dx;
	ObjFile obj;
	Texture tex;
	char* tex_str;
	char* obj_str;
public:
	Vector3 pos;
	float speed;
	float W, H;
	NPC(char*, Vector3);
	void Move(int);
	void Draw(ObjFile*);
};


NPC::NPC(char* obj_tex, Vector3 obj_pos) {
	W = 3;
	H = 3;
	pos = obj_pos;
	dx = -1;
	speed = 0.1;
	tex_str = obj_tex;
}
void NPC::Draw(ObjFile *obj) {
	glPushMatrix();
	pos.setCoords(cos(pos.Y()), pos.Y() + dx * speed, pos.Z());
	glTranslatef(pos.X(), pos.Y(), pos.Z());
	glScalef(0.02, 0.02, 0.02);
	glRotatef(-80, 0, 0, 1);
	obj->DrawObj();
	glPopMatrix();
}

void NPC::Move(int d) {
	dx = d;
}