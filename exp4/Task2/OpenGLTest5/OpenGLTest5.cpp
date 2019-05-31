#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

//网上扒下来的用C++写的轨迹球库
#include "ArcBall.h"


typedef struct {
	float pos[3];						//点的位置
	float nor[3];
}VertexData;

typedef struct {
	int index[4];						//面的顶点信息
}FaceData;


ArcBallT	arcBall(600.0f, 400.0f);
ArcBallT* ArcBall = &arcBall;

GLint		DISPLAY_MODE = 0;
GLint		ModelInfo[3] = { 0,0,0 };			//模型的顶点数，面片数，ply文件中end_header对应的行数
VertexData* vData;								//顶点数据
FaceData* fData;								//面数据
GLint		win_width = 0, win_height = 0;



int readInfo(int* info, const char* file_adress) {
	constexpr auto MAX_CHAR = 255;		//每行最多字符数
	char buf[MAX_CHAR];					//每行读取出的缓存
	FILE* fp;
	int lines = 0;						//到end_header一共几行

	if (fopen_s(&fp, file_adress, "r+"))
		return -1;						//打开失败

	if (fp != NULL) {
		while (fgets(buf, MAX_CHAR, fp) != NULL) {
			lines++;
			if (!strcmp(buf, "end_header\n")) {
				//获取头部结束位置
				info[2] = lines;
				break;
			}
			if (!strncmp(buf, "element vertex ", 15)) {
				//获取element vertex 后面的数字
				char* tmp = buf;
				while (strchr(tmp, ' ') != NULL)
				{
					tmp = strchr(tmp, ' ') + 1;
				}
				info[0] = atoi(tmp);
			}
			else if (!strncmp(buf, "element face ", 13)) {
				//获取element face 后面的数字
				char* tmp = buf;
				while (strchr(tmp, ' ') != NULL)
				{
					tmp = strchr(tmp, ' ') + 1;
				}
				info[1] = atoi(tmp);
			}
		}
		fclose(fp);
	}

	return 0;
}

int readData(VertexData* vd, FaceData* fd, int* info, const char* file_adress) {
	constexpr auto MAXLINE = 255;
	FILE* fp;
	char buf[MAXLINE];

	int line = 0;						//行数计数器
	int vertex = info[0];				//一共多少顶点
	int face = info[1];					//一共多少面
	int header = info[2];				//头部长度

	if (fopen_s(&fp, file_adress, "r+"))
		return -1;

	if (fp != NULL) {					//读取数据
		while (fgets(buf, MAXLINE, fp) != NULL) {
			line++;
			if (line > header && line <= header + vertex) {
				//读取顶点坐标和法向
				int t = 0;
				char* token = NULL;
				char* next_token = NULL;
				token = strtok_s(buf, " ", &next_token);
				while (t < 6 && token != NULL) {
					if (t < 3 && token != NULL) {
						vd[line - header - 1].pos[t] = (float)atof(token);
						token = strtok_s(NULL, " ", &next_token);
					}
					else if (t >= 3 && token != NULL) {
						vd[line - header - 1].nor[t - 3] = (float)atof(token);
						token = strtok_s(NULL, " ", &next_token);
					}
					t++;
				}
			}
			else if (line > header + vertex) {
				//读取面对应的点
				int t = 0;
				char* token = NULL;
				char* next_token = NULL;
				token = strtok_s(buf, " ", &next_token);
				while (t < 4 && token != NULL) {
					if (token != NULL) {
						fd[line - vertex - header - 1].index[t] = atoi(token);
						token = strtok_s(NULL, " ", &next_token);
					}
					t++;
				}
			}
		}
		fclose(fp);
	}
	return 0;
}

void lightDisplay(void) {

	//材质反光性设置
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };			//镜面反射参数
	GLfloat mat_shininess[] = { 120.0 };						//高光指数
	GLfloat light_position[] = { -1.0, 1.0, 1.0, 1.0 };			//最后1是开关
	GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat Light_Model_Ambient[] = { 1.0, 1.0, 1.0, 1.0 };		//环境光参数

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);

	//材质属性
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	//灯光设置
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);				//散射光属性
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);				//镜面反射光
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light_Model_Ambient);//环境光参数

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

int init(void) {
	const char* model_adress =
		"lizhenxiout-repaired.ply";

	if (readInfo(ModelInfo, model_adress) == -1) {
		perror("Cann't read Information");
		return -1;
	}

	vData = (VertexData*)malloc(6 * ModelInfo[0] * sizeof(float));
	fData = (FaceData*)malloc(4 * ModelInfo[1] * sizeof(int));

	if (readData(vData, fData, ModelInfo, model_adress) == -1) {
		perror("Cann't read data");
		return -2;
	}

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightDisplay();

	glEnable(GL_DEPTH_TEST);

	return 0;
}

void drawModel() {
	if (sizeof(*vData) > 1) {
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < ModelInfo[1]; i++) {
			for (int j = 1; j <= 3; j++) {
				glNormal3d(
					vData[fData[i].index[j]].nor[0],
					vData[fData[i].index[j]].nor[1],
					vData[fData[i].index[j]].nor[2]);
				glVertex3d(
					vData[fData[i].index[j]].pos[0] / 1000,
					vData[fData[i].index[j]].pos[1] / 1000 + 0.1,
					vData[fData[i].index[j]].pos[2] / 1000 + 0.7);
			}
		}
		glEnd();
	}
}

void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3d(1.0, 1.0, 1.0);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, 1.0);
	glScalef(1.0, 2.0, 1.0);
	glScalef(ArcBall->zoomRate, ArcBall->zoomRate, ArcBall->zoomRate);
	glMultMatrixf(ArcBall->Transform.M);
	drawModel();
	glutWireSphere(1.0, 24, 12);
	//glPopMatrix();

	glutPostRedisplay();
	glutSwapBuffers();
}

void reshapeFunc(int w, int h) {
	win_width = w;
	win_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1, 1, -1, 1, 1.0, 10);
	glMatrixMode(GL_MODELVIEW);
	ArcBall->setBounds((GLfloat)w, (GLfloat)h);
}

void motionFunc(int x, int y) {
	ArcBall->MousePt.s.X = x;
	ArcBall->MousePt.s.Y = y;
	ArcBall->upstate();
	glutPostRedisplay();
}

void mouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		ArcBall->isClicked = true;
		motionFunc(x, y);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		ArcBall->isClicked = false;
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		ArcBall->isRClicked = true;
		motionFunc(x, y);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		ArcBall->isRClicked = false;
	ArcBall->upstate();
	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 80);
	glutCreateWindow(argv[0]);

	if (init() != 0) {
		char pause = getchar();
		return -1;
	}
	else printf("Model data:\nvertices = %d\nfaces = %d\n"
		"Read successfully.\n", ModelInfo[0], ModelInfo[1]);

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);
	glutKeyboardFunc(keyboardFunc);

	glutFullScreen();

	glutMainLoop();
	return 0;
}