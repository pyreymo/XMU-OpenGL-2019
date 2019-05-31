#pragma once
#ifndef GLDRAW
#include <GL/freeglut.h>

void normalize(GLfloat *p);
void drawTriangle(GLfloat *a, GLfloat *b, GLfloat *c);
void divideTriangle(GLfloat *a, GLfloat *b, GLfloat *c, int n);
void drawTetrahedron(int n);
void drawRegularTetrahedron();
void drawBall(GLfloat R,GLint divide);

#endif // !GLDRAW
