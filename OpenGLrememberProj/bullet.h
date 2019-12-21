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

using namespace std;

class Bullet {
	float dx;
	float angle;
	Vector3 os_rotate;
	Vector3 scale;
	float W, H;
public:
	float speed;
	Vector3 pos;
	Bullet(Vector3);
	void Draw();
	void Rotate(float, float, float, float);
	void Edit_pos(float, float, float);
	bool Collide(Vector3, float, float);
	void View_model();
};

Bullet::Bullet(Vector3 ps)
{
	dx = -1;
	W = 2;
	H = 2;
	pos = ps;
	speed = 1;
	angle = 0;
	os_rotate = *new Vector3(0, 0, 1);
	scale.setCoords(0.2, 0.2, 0.2);
}

void Bullet::Draw()
{
	glPushMatrix();
	angle += 5;
	pos.setCoords(pos.X() + dx * speed, pos.Y(), pos.Z());
	glTranslatef(pos.X(), pos.Y(), pos.Z());
	glRotatef(angle, os_rotate.X(), os_rotate.Y(), os_rotate.Z());
	glScalef(scale.X(), scale.Y(), scale.Z());
	View_model();
	glColor3d(1, 0, 0);
	glRotatef(45, 1, 0, 0);
	glRotatef(45, 0, 0, 1);
	View_model();
	glPopMatrix();

}

void Bullet::Rotate(float ang, float x, float y, float z)
{
	angle = ang;
	os_rotate.setCoords(x, y, z);
}

void Bullet::Edit_pos(float x, float y, float z)
{
	pos.setCoords(x, y, z);
}

bool Bullet::Collide(Vector3 o_pos, float o_W, float o_H) {
	if ((o_pos.X() > pos.X() && o_pos.X() < pos.X() + W) && (o_pos.Y() > pos.Y() && o_pos.Y() < pos.Y() + H) ||
		(o_pos.X() + o_W > pos.X() && o_pos.X() + o_W < pos.X() + W) && (o_pos.Y() + o_H > pos.Y() && o_pos.Y() + o_H < pos.Y() + H))
	{
		return true;
	}
	return false;
}

void Bullet::View_model() {
	glBegin(GL_POLYGON);
	glVertex3f(W, -W, W);
	glVertex3f(W, W, W);
	glVertex3f(-W, W, W);
	glVertex3f(-W, -W, W);
	glEnd();

	// Ôèîëåòîâàÿ ñòîðîíà — ÏÐÀÂÀß
	glBegin(GL_POLYGON);
	glVertex3f(W, -W, -W);
	glVertex3f(W, W, -W);
	glVertex3f(W, W, W);
	glVertex3f(W, -W, W);
	glEnd();

	// Çåëåíàÿ ñòîðîíà — ËÅÂÀß
	glBegin(GL_POLYGON);
	glVertex3f(-W, -W, W);
	glVertex3f(-W, W, W);
	glVertex3f(-W, W, -W);
	glVertex3f(-W, -W, -W);
	glEnd();

	// Ñèíÿÿ ñòîðîíà — ÂÅÐÕÍßß
	glBegin(GL_POLYGON);
	glVertex3f(W, W, W);
	glVertex3f(W, W, -W);
	glVertex3f(-W, W, -W);
	glVertex3f(-W, W, W);
	glEnd();

	// Êðàñíàÿ ñòîðîíà — ÍÈÆÍßß
	glBegin(GL_POLYGON);
	glVertex3f(W, -W, -W);
	glVertex3f(W, -W, W);
	glVertex3f(-W, -W, W);
	glVertex3f(-W, -W, -W);
	glEnd();
}