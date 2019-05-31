#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GL/freeglut.h>


typedef struct {
	float pos[3];						//点的位置
	float nor[3];
}VertexData;

typedef struct {
	int index[4];						//面的顶点信息
}FaceData;

typedef struct
{
	GLint plane[2];
	GLfloat ball[3];
}Injection;



GLint		DISPLAY_MODE = 0;
GLint		ModelInfo[3] = { 0,0,0 };
VertexData* vData;									//顶点数据
FaceData* fData;									//面数据
GLint		win_width = 0, win_height = 0;			//窗口大小
Injection	startInj, currentInj;					//平面到球面的映射
GLdouble	rotateAngle;							//转角
GLfloat		rotateNormal[3] = { 0,0,0 };			//转轴
GLdouble	modelBC[3] = { 0,0,0 };					//重心位置
GLboolean	isClicked = false, isRClicked = false;	//判断是否点击
GLboolean	isSetLookAt = true;						//控制gluLookAt只设置一遍
GLboolean	isDragging = false;						//判断鼠标是否正在拖拽



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

	lightDisplay();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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
					vData[fData[i].index[j]].pos[1] / 1000,
					vData[fData[i].index[j]].pos[2] / 1000);
			}
		}
		glEnd();
	}
}

void getBarycenter(VertexData * vData, GLint * modelInfo, GLdouble * barycenter) {
	for (int i = 0; i < modelInfo[0]; i++) {
		barycenter[0] += vData[i].pos[0];
		barycenter[1] += vData[i].pos[1];
		barycenter[2] += vData[i].pos[2];
	}
	barycenter[0] /= modelInfo[0];
	barycenter[1] /= modelInfo[0];
	barycenter[2] /= modelInfo[0];
}

void displayFunc() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3d(1.0, 1.0, 1.0);

	if (isSetLookAt) {
		gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		isSetLookAt = false;
	}
	glPushMatrix();
	drawModel();
	glTranslated(0, 0, modelBC[2] / 1000);
	glutWireSphere(1.4, 20, 20);
	glPopMatrix();

	glutPostRedisplay();
	glutSwapBuffers();
}

void idleFunc() {
	if (isDragging) {
		glTranslated(modelBC[0] / 1000, modelBC[1] / 1000, modelBC[2] / 1000);
		glRotatef(rotateAngle, rotateNormal[0], rotateNormal[1], rotateNormal[2]);
		glTranslated(-modelBC[0] / 1000, -modelBC[1] / 1000, -modelBC[2] / 1000); drawModel();
	}
}

void reshapeFunc(int w, int h) {
	win_width = w;
	win_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0 * w / h, 1.0, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void getRotateAngle(Injection sInj, Injection cInj, GLdouble * angle, GLfloat * normal) {
	GLfloat x0 = sInj.ball[0];
	GLfloat y0 = sInj.ball[1];
	GLfloat z0 = sInj.ball[2];
	GLfloat x = cInj.ball[0];
	GLfloat y = cInj.ball[1];
	GLfloat z = cInj.ball[2];
	normal[0] = y0 * z - y * z0;
	normal[1] = x * z0 - x0 * z;
	normal[2] = x0 * y - y0 * x;
	GLfloat r1 = sqrt(1.0 * x0 * x0 + 1.0 * y0 * y0 + 1.0 * z0 * z0);
	GLfloat r2 = sqrt(1.0 * x * x + 1.0 * y * y + 1.0 * z * z);
	*angle = acos((1.0 * x0 * x + 1.0 * y0 * y + 1.0 * z0 * z) / r1 / r2) * 57.3 * 1.5;	//弧度化角度
}

void ballInjection(Injection * inj) {
	GLint	x = inj->plane[0];
	GLint	y = win_height - inj->plane[1];
	inj->ball[0] = 2.0 * (1.0 * x / win_width - 0.5);
	inj->ball[1] = 2.0 * (1.0 * y / win_height - 0.5);
	GLfloat X = inj->ball[0];
	GLfloat Y = inj->ball[1];
	GLfloat zz = 1.0 - 1.0 * X * X - 1.0 * Y * Y;
	if (zz >= 0.25) inj->ball[2] = sqrt(zz);
	else inj->ball[2] = 0.5 * 1 / (1 + 1.0 * (0.5 - 1.0 * X * X - 1.0 * Y * Y) 
		* (0.5 - 1.0 * X * X - 1.0 * Y * Y));			//超出球的正投影的部分映射到一个平滑曲面上
}

void motionFunc(int x, int y) {
	currentInj.plane[0] = x;
	currentInj.plane[1] = y;
	ballInjection(&currentInj);
	getRotateAngle(startInj, currentInj, &rotateAngle, rotateNormal);
	startInj = currentInj;
}

void mouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (!isClicked) {
			startInj.plane[0] = x;
			startInj.plane[1] = y;
			ballInjection(&startInj);
			isClicked = true;
			isDragging = true;
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isClicked = false;
		isDragging = false;
	}
	/*else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		isRClicked = true;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		isRClicked = false;*/
}

void keyboardFunc(unsigned char key, int x, int y) {
	switch (key) {
	case 27:		//ESC
		exit(0);
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(1080, 720);
	glutInitWindowPosition(200, 50);
	glutCreateWindow(argv[0]);

	if (init() != 0) {
		char pause = getchar();
		return -1;
	}
	else {
		printf("Model data:\nvertices = %d\nfaces = %d\n"
			"Read successfully.\n", ModelInfo[0], ModelInfo[1]);
		getBarycenter(vData, ModelInfo, modelBC);
		printf("\nBarycenter:\nx : %g\ny : %g\nz : %g\n", modelBC[0], modelBC[1], modelBC[2]);
	}

	glutDisplayFunc(displayFunc);
	glutIdleFunc(idleFunc);
	glutReshapeFunc(reshapeFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);
	glutKeyboardFunc(keyboardFunc);

	glutFullScreen();

	glutMainLoop();
	return 0;
}