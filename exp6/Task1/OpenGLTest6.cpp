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



GLint		DISPLAY_MODE = 0;
GLint		ModelInfo[3] = { 0,0,0 };
VertexData* vData;									//顶点数据
FaceData* fData;									//面数据
GLint		win_width = 0, win_height = 0;			//窗口大小
GLdouble	modelBC[3] = { 0,0,0 };					//重心位置
GLboolean	isSetLookAt = true;						//控制gluLookAt只设置一遍
GLint		min_index = -1;							//拾取点的下标



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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	return 0;
}

void highlightPoint() {
	glColor3d(1.0, 0.0, 0.0);
	glPushMatrix();
	glTranslated(vData[min_index].pos[0], vData[min_index].pos[1], vData[min_index].pos[2]);
	glutSolidSphere(5.0, 20, 20);
	glPopMatrix();
	glColor3d(1.0, 1.0, 1.0);
}

void drawModelPointwise() {
	if (sizeof(*vData) > 1) {
		glBegin(GL_POINTS);
		for (int i = 0; i < ModelInfo[0]; i++) {
			glNormal3d(vData[i].nor[0], vData[i].nor[1], vData[i].nor[2]);
			glVertex3d(vData[i].pos[0], vData[i].pos[1], vData[i].pos[2]);
		}
		glEnd();
	}
}

void getBarycenter(VertexData* vData, GLint* modelInfo, GLdouble* barycenter) {
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

	glLoadIdentity();

	if(min_index==-1)
		gluLookAt(0.0, 0.0, 800.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	else {
		gluLookAt(vData[min_index].pos[0] - modelBC[0], vData[min_index].pos[1] - modelBC[1], 100,
			vData[min_index].pos[0] - modelBC[0], vData[min_index].pos[1] - modelBC[1], 0.0, 0.0, 1.0, 0.0);
		highlightPoint();
	}
	
	drawModelPointwise();

	glTranslated(modelBC[0], modelBC[1], modelBC[2]);

	glutPostRedisplay();
	glutSwapBuffers();
}

void reshapeFunc(int w, int h) {
	win_width = w;
	win_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0*w / h, 1.0, 5000);
	glMatrixMode(GL_MODELVIEW);

	glutPostRedisplay();
	glutSwapBuffers();
}

void mouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		GLdouble modelview_matrix[16];
		GLdouble projection_matrix[16];
		GLint viewport[4];

		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);

		GLdouble min_distance = 20.0;
		GLint last_min_index = min_index;
		GLdouble win_x = 0, win_y = 0, win_z = 0;
		GLdouble new_y = viewport[3] - 1.0 * y;
		
		for (int i = 0; i < ModelInfo[0]; i++) {
			gluProject(vData[i].pos[0] - modelBC[0], vData[i].pos[1] - modelBC[1], vData[i].pos[2] - modelBC[2],
				modelview_matrix, projection_matrix, viewport, &win_x, &win_y, &win_z);
			GLdouble distance = (win_x - x) * (win_x - x) + (win_y - new_y) * (win_y - new_y);
			if (min_distance > distance) {
				min_distance = distance;
				min_index = i;
			}
		}

		if (last_min_index == min_index) min_index = -1;

		printf("\nClick = (%d, %d)\n", x, y);
		printf("Chosen = %d\n", min_index);
	}
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

	glutInitWindowSize(720, 1000);
	glutInitWindowPosition(500, 10);
	glutCreateWindow(argv[0]);

	if (init() != 0) {
		char pause = getchar();
		return -1;
	}
	else {
		printf("Model data:\nvertices = %d\nfaces = %d\n"
			"\nRead successfully.\n", ModelInfo[0], ModelInfo[1]);
		getBarycenter(vData, ModelInfo, modelBC);
		printf("\nBarycenter:\nx : %g\ny : %g\nz : %g\n\n", modelBC[0], modelBC[1], modelBC[2]);
	}

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutMouseFunc(mouseFunc);
	glutKeyboardFunc(keyboardFunc);

	glutMainLoop();
	return 0;
}