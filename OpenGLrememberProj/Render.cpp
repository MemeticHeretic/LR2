#include "Render.h"

#include <sstream>
#include <iostream>
#include <corecrt_math.h>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

#define PI 3.14159265

bool textureMode = true;
bool lightMode = true;

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
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
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
		glDisable(GL_LIGHTING);

		
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

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


void figure(double**);
double* rotMatrix(double*, double*, double);


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	double V0[] = { 7, 5, 0 };
	double V1[] = { 6, 2, 0 };
	double V2[] = { 1, 0, 0 };
	double V3[] = { 9, -2, 0 };
	double V4[] = { -2, -4, 0 };
	double V5[] = { -5, -8, 0 };
	double V6[] = { -5, 2, 0 };
	double V7[] = { 0, 2, 0 };
	double V8[] = { -3, 5, 0 };
	double V9[] = { -1, -1, 0 };

	double inFan0[] = { 0, 0, 0 };
	double inFan1[] = { 0, 0, 0 };
	double inFan2[] = { 0, 0, 0 };
	double inFan3[] = { 0, 0, 0 };
	double inFan4[] = { 0, 0, 0 };
	double inFan5[] = { 0, 0, 0 };
	double inFan6[] = { 0, 0, 0 };
	double inFan7[] = { 0, 0, 0 };
	double inFan8[] = { 0, 0, 0 };
	double inFan9[] = { 0, 0, 0 };
	double inFan10[] = { 0, 0, 0 };

	double outFan0[] = { 0, 0, 0 };
	double outFan1[] = { 0, 0, 0 };
	double outFan2[] = { 0, 0, 0 };
	double outFan3[] = { 0, 0, 0 };
	double outFan4[] = { 0, 0, 0 };
	double outFan5[] = { 0, 0, 0 };
	double outFan6[] = { 0, 0, 0 };
	double outFan7[] = { 0, 0, 0 };
	double outFan8[] = { 0, 0, 0 };
	double outFan9[] = { 0, 0, 0 };
	double outFan10[] = { 0, 0, 0 };

	double* C0[] = { V0, V1, V2, V3, V4, V5, V6, V7, V8, V9 };

	double* fan1[] = { inFan0, inFan1, inFan2, inFan3, inFan4, inFan5, inFan6, inFan7, inFan8, inFan9, inFan10 };
	double* fan2[] = { outFan0, outFan1, outFan2, outFan3, outFan4, outFan5, outFan6, outFan7, outFan8, outFan9, inFan10 };

	double rotV0[] = { 2, 5, 0 };
	double rotV1[] = { -12.5, -3, 0 };
	double rot = 0;
	double angle = 67.3801350520 / 10;
	double* pointMod;
	int i = 0;
	double n[3];

	double offset = 7;

	glPushMatrix();
	glNormal3d(0, 0, -1);
	figure(C0);
	glPopMatrix();

	glPushMatrix();
	glNormal3d(0, 0, 1);
	glTranslated(0, 0, offset);
	figure(C0);
	glPopMatrix();

	glBegin(GL_QUAD_STRIP);

	glColor3d(0.2, 0.2, 0.4);

	for (int i = 0; i <= 5; i++)
	{
		C0[i+1][2] += offset;
		n[0] = C0[i][1] * C0[i+1][2] - C0[i][1] * C0[i+1][2];
		n[1] = -C0[i][0] * C0[i+1][2] + C0[i][0] * C0[i+1][2];
		n[2] = C0[i][0] * C0[i+1][1] - C0[i][0] * C0[i+1][1];
		C0[i+1][2] -= offset;
		glVertex3dv(C0[i]);
		C0[i][2] += offset;
		glVertex3dv(C0[i]);
		C0[i][2] -= offset;
	}

	glColor3d(0.4, 0.4, 0.4);
	
	for (rot = angle; rot < 67.3801350520; rot += angle)
	{
		fan2[i] = rotMatrix(C0[5], rotV1, rot);
		fan2[i + 1][2] += offset;
		n[0] = fan2[i][1] * fan2[i + 1][2] - fan2[i][1] * fan2[i + 1][2];
		n[1] = -fan2[i][0] * fan2[i + 1][2] + fan2[i][0] * fan2[i + 1][2];
		n[2] = fan2[i][0] * fan2[i + 1][1] - fan2[i][0] * fan2[i + 1][1];
		fan2[i + 1][2] -= offset;
		glVertex3dv(fan2[i]);
		fan2[i][2] += offset;
		glVertex3dv(fan2[i]);
		fan2[i][2] -= offset;
	}
	
	glColor3d(0.2, 0.2, 0.4);
	
	for (int i = 6; i <= 8; i++)
	{
		C0[i + 1][2] += offset;
		n[0] = C0[i][1] * C0[i + 1][2] - C0[i][1] * C0[i + 1][2];
		n[1] = -C0[i][0] * C0[i + 1][2] + C0[i][0] * C0[i + 1][2];
		n[2] = C0[i][0] * C0[i + 1][1] - C0[i][0] * C0[i + 1][1];
		C0[i + 1][2] -= offset;
		glVertex3dv(C0[i]);
		C0[i][2] += offset;
		glVertex3dv(C0[i]);
		C0[i][2] -= offset;
	}
	
	glColor3d(0.4, 0.4, 0.4);
	
	for (rot = 0; rot <= 180; rot += 18)
	{
		fan1[i] = rotMatrix(C0[0], rotV0, rot);
		fan1[i + 1][2] += offset;
		n[0] = fan1[i][1] * fan1[i + 1][2] - fan1[i][1] * fan1[i + 1][2];
		n[1] = -fan1[i][0] * fan1[i + 1][2] + fan1[i][0] * fan1[i + 1][2];
		n[2] = fan1[i][0] * fan1[i + 1][1] - fan1[i][0] * fan1[i + 1][1];
		fan1[i + 1][2] -= offset;
		glVertex3dv(fan1[i]);
		fan1[i][2] += offset;
		glVertex3dv(fan1[i]);
		fan1[i][2] -= offset;
	}

	glEnd();


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

void figure(double** C0)
{
	double rotV0[] = { 2, 5, 0 };
	double rotV1[] = { -12.5, -3, 0 };
	double rot = 0;
	double angle = 67.3801350520 / 10;
	double* pointMod;
	int i = 0;

	glBegin(GL_TRIANGLES);

	glPushMatrix();
	glColor3d(0.5, 0.5, 0.7);

	glVertex3dv(C0[0]);
	glVertex3dv(C0[1]);
	glVertex3dv(C0[2]);

	glVertex3dv(C0[2]);
	glVertex3dv(C0[3]);
	glVertex3dv(C0[4]);

	glVertex3dv(C0[0]);
	glVertex3dv(C0[2]);
	glVertex3dv(C0[7]);

	glVertex3dv(C0[7]);
	glVertex3dv(C0[2]);
	glVertex3dv(C0[4]);

	glVertex3dv(C0[7]);
	glVertex3dv(C0[9]);
	glVertex3dv(C0[6]);

	glVertex3dv(C0[9]);
	glVertex3dv(C0[4]);
	glVertex3dv(C0[5]);

	glVertex3dv(C0[0]);
	glVertex3dv(C0[7]);
	glVertex3dv(C0[8]);

	glEnd();

	glBegin(GL_TRIANGLE_FAN);

	glVertex3dv(rotV0);

	for (rot = 0; rot <= 180; rot += 18)
	{
		pointMod = rotMatrix(C0[0], rotV0, rot);
		glVertex3dv(pointMod);
	}

	glEnd();

	glBegin(GL_TRIANGLE_FAN);

	glVertex3dv(C0[9]);

	for (rot = 0; rot <= 67.3801350520; rot += angle)
	{
		pointMod = rotMatrix(C0[5], rotV1, rot);
		glVertex3dv(pointMod);
	}

	glPopMatrix();

	glEnd();
}

double* rotMatrix(double* point, double* pivot, double angle)
{
	double pointMod[3];
	pointMod[0] = (point[0] - pivot[0]) * cos(angle * PI / 180) - (point[1] - pivot[1]) * sin(angle * PI / 180);
	pointMod[1] = (point[0] - pivot[0]) * sin(angle * PI / 180) + (point[1] - pivot[1]) * cos(angle * PI / 180);
	pointMod[2] = point[2];
	pointMod[0] = pointMod[0] + pivot[0];
	pointMod[1] = pointMod[1] + pivot[1];
	return pointMod;
}