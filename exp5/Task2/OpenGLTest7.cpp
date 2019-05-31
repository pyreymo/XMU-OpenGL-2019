#include <GL/freeglut.h>
#include <math.h>
#include <cstdio>

GLdouble	angle = 0.0;
GLfloat		day = 0.05f;

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
}

void glPrint(const char* str)
{
	int len, i;
	wchar_t* wstring;
	HDC hDC = wglGetCurrentDC();
	GLuint list = glGenLists(1);

	len = 0;
	for (i = 0; str[i] != '\0'; ++i)
	{
		if (IsDBCSLeadByte(str[i]))
			++i;
		++len;
	}

	wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	if (wstring != NULL) {
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
		wstring[len] = L'\0';

		for (i = 0; i < len; ++i)
		{
			wglUseFontBitmapsW(hDC, wstring[i], 1, list);
			glCallList(list);
		}

		free(wstring);
		glDeleteLists(list, 1);
	}
}

void glSetFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT);

	glSetFont(50, ANSI_CHARSET, "Microsoft YaHei");
	char str[32] = "天数 = ";
	char str2[16];
	_itoa_s((int)(angle / 360.0), str2, 16, 10);
	strcat_s(str, sizeof(str), str2);
	glRasterPos2d(-1.8, 4);
	glPrint(str);

	glPushMatrix();									//太阳
	glRotated(angle / 25.05, 0.0, 1.0, 0.0);		//自转 25.05天
	glColor3d(1.0, 69.0 / 255.0, 0.0);
	glutWireSphere(1.0, 20, 16);
	glPopMatrix();

	glPushMatrix();									//地球
	glColor3d(0.0, 191.0 / 255.0, 1.0);
	glRotated(angle / 365.24, 0.0, 1.0, 0.0);		//公转 365.24天
	glTranslated(0.0, 0.0, 7.0);
	glRotated(angle, 0.0, 1.0, 0.0);				//自转 1天
	glutWireSphere(0.2, 10, 8);

	//月球	自转=公转 27.32天
	glRotated(-angle - angle / 365.24 + angle / 27.32, 0.0, 1.0, 0.0);
	glTranslated(0.0, 0.0, 0.5);
	glColor3d(139.0 / 255.0, 139.0 / 255.0, 131.0 / 255.0);
	glutWireSphere(0.05, 10, 8);
	glPopMatrix();

	glutSwapBuffers();
}

void idleFunc() {
	angle += day;		//控制一天的长度
	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {
	switch (key) {
	case 27:		//ESC
		exit(0);
		break;
	}
}

void specialFunc(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		day += 0.1;
		break;
	case GLUT_KEY_DOWN:
		if (day > 0.1)day -= 0.1;
		break;
	default:
		break;
	}
}

void reshapeFunc(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0 * w / h, 1.0, 50.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 14.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(1080, 720);
	glutInitWindowPosition(200, 50);
	glutCreateWindow(argv[0]);

	glutReshapeFunc(reshapeFunc);
	glutDisplayFunc(displayFunc);
	glutKeyboardFunc(keyboardFunc);
	glutSpecialFunc(specialFunc);
	glutIdleFunc(idleFunc);

	glutMainLoop();
	return 0;
}