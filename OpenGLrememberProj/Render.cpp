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

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
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
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
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

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

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


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  


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


   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
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