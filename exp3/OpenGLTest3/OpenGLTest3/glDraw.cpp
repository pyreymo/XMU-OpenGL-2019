#include "pch.h"
#include "glDraw.h"
#include <stdio.h>
#include <GL/freeglut.h>
#include <math.h>

const GLfloat pi = 3.14159265;


//-----------------------------------画细分四面体-------------------------------------------------------
GLfloat R0 = 1.0;													//细分四面体中四面体的标准外接球半径
GLfloat v[4][3] = { {0.0,0.0,1.0},{0.0,R0*0.942809,-R0 * 0.333333},
{-R0 * 0.816497,-R0 * 0.471405,-R0 * 0.333333},
{R0 * 0.816497,-R0 * 0.471405,-R0 * 0.333333} };

void normalize(GLfloat *p) {										//标准化细分后的图形大小
	double d = 0.0;
	int i;
	for (i = 0; i < 3; i++) d += p[i] * p[i];
	d = sqrt(d);
	if (d > 0.0) for (i = 0; i < 3; i++) p[i] = R0 * p[i] / d;
}

void drawTriangle(GLfloat *a, GLfloat *b, GLfloat *c) {				//画出每个三角面
	glBegin(GL_LINE_LOOP);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glEnd();
}

void divideTriangle(GLfloat *a, GLfloat *b, GLfloat *c, int n) {	//递归地细分每个面
	GLfloat v1[3], v2[3], v3[3];
	int j;
	if (n > 0) {
		for (j = 0; j < 3; j++) v1[j] = a[j] + b[j];
		normalize(v1);
		for (j = 0; j < 3; j++) v2[j] = a[j] + c[j];
		normalize(v2);
		for (j = 0; j < 3; j++) v3[j] = b[j] + c[j];
		normalize(v3);
		divideTriangle(a, v2, v1, n - 1);
		divideTriangle(c, v3, v2, n - 1);
		divideTriangle(b, v1, v3, n - 1);
		divideTriangle(v1, v2, v3, n - 1);
	}
	else drawTriangle(a, b, c);
}

void drawTetrahedron(int n) {										//调用细分函数并设定初值
	divideTriangle(v[0], v[1], v[2], n);
	divideTriangle(v[3], v[2], v[1], n);
	divideTriangle(v[0], v[3], v[1], n);
	divideTriangle(v[0], v[2], v[3], n);
}
//------------------------------------------------------------------------------------------------------


//画球
void drawBall(GLfloat R,GLint divide) {
	GLfloat dy = pi / divide;
	GLfloat dxz = pi / divide * 2;

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, R, 0.0);
	for (int i = 0; i <= divide; i++) {
		glVertex3f(R*sin(dy)*cos(dxz*i), R*cos(dy), R*sin(dy)*sin(dxz*i));
	}
	glEnd();


	for (int i = 1; i <= divide; i++) {
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= divide; j++) {
			glVertex3f(R*sin(dy*i)*cos(dxz*j), R*cos(dy*i), R*sin(dy*i)*sin(dxz*j));
			glVertex3f(R*sin(dy*(i + 1))*cos(dxz*j), R*cos(dy*(i + 1)), R*sin(dy*(i + 1))*sin(dxz*j));
		}
		glEnd();
	}


	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, -R, 0.0);
	for (int i = 0; i <= divide; i++) {
		glVertex3f(R*sin(dy)*cos(dxz*i), -R * cos(dy), R*sin(dy)*sin(dxz*i));
	}
	glEnd();
}

//画正四面体
void drawRegularTetrahedron() {
	GLfloat R = 3.0;					//正四面体外接球半径
	GLfloat a = R / 2;
	GLfloat b = sqrt(3)*a;

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, R, 0.0);
	glVertex3f(-b, -a, 0.0);
	glVertex3f(a, -a, b);
	glVertex3f(a, -a, -b);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-b, -a, 0.0);
	glVertex3f(a, -a, b);
	glVertex3f(a, -a, -b);
	glEnd();
}
