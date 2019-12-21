#include "Render.h"

#include <iostream>
#include <cstdlib>
#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"
#include "Player.h"

#include "Render.h"
#include <sstream>
#include "MyOGL.h"
#include "Player.h"
#include "Enemy.h"
#include "bullet.h"
#include "npc.h"

bool play_mod = false;
char map[50][50]{
	"            ",
	"  11111111  ",
	"  22222222  ",
	"  33333333  ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"            ",
	"      0     "};
int cell_size = 3;

Player *Pl;
list<Enemy*> Enemies;
list<Bullet*> Bullets;
list<NPC*> NPCs;
ObjFile UFO, ply, enem1, enem2, enem3, bull;

Texture space, tex_ply, tex_enem, tex_bull, board, lose_tex, win_tex;

GuiTextRectangle rec;

int lose = -1;

bool textureMode = true;
bool lightMode = true;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;
Texture SkyBoxSides;
Texture score_tex;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;


//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);

		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom = 1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON) && play_mod == false)
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON) && play_mod == false)
	{
		offsetX -= 1.0*dx / ogl->getWidth() / zoom;
		offsetY += 1.0*dy / ogl->getHeight() / zoom;
	}
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta * 0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL *ogl, int key)
{
	if (OpenGL::isKeyPressed('L') && play_mod == false)
	{
		lightMode = !lightMode;
	}

	if (OpenGL::isKeyPressed('T') && play_mod == false)
	{
		textureMode = !textureMode;
	}

	if (key == 'R'  && play_mod == false)
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F'  && play_mod == false)
	{
		light.pos = camera.pos;
	}

	if (key == 'S'  && play_mod == false)
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (OpenGL::isKeyPressed('W'))
	{
		Bullets.push_back(new Bullet(Pl->pos));
	}

	if (OpenGL::isKeyPressed('S'))
	{
		play_mod = !play_mod;
		if (play_mod == true)
		{
			camera.fi1 = 0;
			camera.fi2 = 0.9;
			camera.camDist = 70;
			light.pos.setCoords(0, 0, 10);
			for (list<Enemy*>::iterator it = Enemies.begin(); it != Enemies.end(); it++)
			{
				(*it)->Move(1);
			}
		}
		else
		{
			Enemies.clear();
			NPCs.clear();
			Bullets.clear();
			for (int i = 0; i < 50; i++)
				for (int j = 0; j < 50; j++)
				{
					if (map[i][j] == '0')
						Pl = new Player(Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1));
					if (map[i][j] == '1')
						Enemies.push_back(new Enemy(&enem1, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
					if (map[i][j] == '2')
						Enemies.push_back(new Enemy(&enem2, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
					if (map[i][j] == '3')
						Enemies.push_back(new Enemy(&enem3, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));

				}
		}
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{

}

void SkyBox() {
	float W = 100;
	glColor3b(1, 1, 1);
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex3f(W, -W, -5);
	glTexCoord2d(0, 1);
	glVertex3f(W, W, -5);
	glTexCoord2d(1, 1);
	glVertex3f(-W, W, -5);
	glTexCoord2d(1, 0);
	glVertex3f(-W, -W, -5);
	glEnd();
}

void SBoard() {
	float W = 20;
	glColor3b(1, 1, 1);
	glBegin(GL_POLYGON);
	glTexCoord2d(1, 0);
	glVertex3f(W, -W, -5);
	glTexCoord2d(1, 1);
	glVertex3f(W, W, -5);
	glTexCoord2d(0, 1);
	glVertex3f(-W, W, -5);
	glTexCoord2d(0, 0);
	glVertex3f(-W, -W, -5);
	glEnd();
}

void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 2,0 };
	double C[] = { 2,2 };
	double D[] = { 0,2 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 1);
	glVertex2dv(A);
	glTexCoord2d(0, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 0);
	glVertex2dv(C);
	glTexCoord2d(1, 1);
	glVertex2dv(D);
	glEnd();
}
//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	camera.fi1 = 0;
	camera.fi2 = 0.9;
	camera.camDist = 70;
	light.pos.setCoords(0, 0, 40);
	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);




	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем


	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем



	glActiveTexture(GL_TEXTURE0);
	space.loadTextureFromFile("textures\\space.bmp");
	
	loadModel("models\\ship.obj_m", &ply);
	loadModel("models\\E1.obj_m", &enem1);
	loadModel("models\\E2.obj_m", &enem2);
	loadModel("models\\E3.obj_m", &enem3);
	tex_enem.loadTextureFromFile("textures\\E.bmp");
	tex_ply.loadTextureFromFile("textures\\ship.bmp");
	board.loadTextureFromFile("textures\\board.bmp");
	lose_tex.loadTextureFromFile("textures\\lose.bmp");
	win_tex.loadTextureFromFile("textures\\win.bmp");
	for(int i = 0; i < 50; i++)
		for (int j = 0; j < 50; j++)
		{
			if (map[i][j] == '0')
				Pl = new Player(Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1));
			if (map[i][j] == '1')
				Enemies.push_back(new Enemy(&enem1, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
			if (map[i][j] == '2')
				Enemies.push_back(new Enemy(&enem2, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
			if (map[i][j] == '3')
				Enemies.push_back(new Enemy(&enem3, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
				
		}
	loadModel("models\\UFO.obj_m", &UFO);
	tick_n = GetTickCount();
	tick_o = tick_n;
}

void Render(OpenGL *ogl)
{
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	if (play_mod == true)
	if (rand()%200 == 1)
	{
		NPCs.push_back(new NPC("textures\\ship.bmp", Vector3(-9 * cell_size, 15 * cell_size - 6 * cell_size, 1)));
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	if(play_mod == true)
		Pl->Keys();
	Pl->Draw(&ply, &tex_ply);

	for (list<Enemy*>::iterator it = Enemies.begin(); it != Enemies.end(); it++)
	{
		if ((*it)->pos.Y() >= 12 * cell_size - 6 * cell_size)
		{
			(*it)->Move(-1);
			(*it)->pos.setCoords((*it)->pos.X() + 1, (*it)->pos.Y(), (*it)->pos.Z());
		}
		if ((*it)->pos.Y() <= -6 * cell_size)
		{
			(*it)->Move(1);
			(*it)->pos.setCoords((*it)->pos.X() + 1, (*it)->pos.Y(), (*it)->pos.Z());
		}
		(*it)->Draw(&tex_enem);
		if ((*it)->pos.X() <= -9 * cell_size)
		{
			lose = 1;
			play_mod = false;
		}
	}
	for (list<NPC*>::iterator it = NPCs.begin(); it != NPCs.end(); it++)
	{
		(*it)->Draw(&UFO);
	}
	for (list<Bullet*>::iterator jt = Bullets.begin(); jt != Bullets.end(); jt++)
	{
		bool deleted = false;
		for (list<Enemy*>::iterator it = Enemies.begin(); it != Enemies.end(); it++)
		{
			if ((*jt)->Collide((*it)->pos, (*it)->W, (*it)->H)) {
				for (list<Enemy*>::iterator kt = Enemies.begin(); kt != Enemies.end(); kt++) {
					(*kt)->speed += 0.1;
				}
				delete *it;
				Enemies.remove(*it);
				deleted = true;
				break;
			}
		}
		for (list<NPC*>::iterator it = NPCs.begin(); it != NPCs.end(); it++)
		{
			if ((*jt)->Collide((*it)->pos, (*it)->W, (*it)->H)) {
				delete *it;
				NPCs.remove(*it);
				deleted = true;
				lose = 1;
				play_mod = false;
				break;
			}
		}
		if (deleted == true)
		{
			delete *jt;
			Bullets.remove(*jt);
			break;
		}
	}
	if (Enemies.empty())
	{
		lose = 2;
		play_mod = false;
	}
	for (list<Bullet*>::iterator it = Bullets.begin(); it != Bullets.end(); it++)
	{
		if ((*it)->pos.X() > 50)
		{
			Bullets.remove(*it);
			delete *it;
		}
		(*it)->Draw();
	}
	space.bindTexture();
	SkyBox();

	if (play_mod == false)
	{
		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glTranslatef(0, 0, 2);
		board.bindTexture();
		SBoard();
		glPopMatrix();
	}
	if (lose == 1 && play_mod == false)
	{
		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glTranslatef(-30, 0, 2);
		lose_tex.bindTexture();
		SBoard();
		glPopMatrix();
	}
	else if (lose == 2 && play_mod == false)
	{
		glPushMatrix();
		glRotatef(90, 0, 0, 1);
		glTranslatef(-50, 0, 2);
		win_tex.bindTexture();
		SBoard();
		glPopMatrix();
	}
	if (play_mod == false)
	{
		Enemies.clear();
		NPCs.clear();
		Bullets.clear();
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 50; j++)
			{
				if (map[i][j] == '0')
					Pl = new Player(Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1));
				if (map[i][j] == '1')
					Enemies.push_back(new Enemy(&enem1, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
				if (map[i][j] == '2')
					Enemies.push_back(new Enemy(&enem2, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));
				if (map[i][j] == '3')
					Enemies.push_back(new Enemy(&enem3, Vector3(i * cell_size - 9 * cell_size, j * cell_size - 6 * cell_size, 1)));

			}
	}
}



//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{

	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	glActiveTexture(GL_TEXTURE0);
	if (play_mod == false)
	{
		rec.Draw();
	}

	Shader::DontUseShaders();
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

