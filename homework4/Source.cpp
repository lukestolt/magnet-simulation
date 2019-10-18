#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

#define PI 3.141592654

struct Magnet {
	// -1 Neg, 0 Neutral, 1 Positive
	int polarity;
	float x;
	float y;
	float z;
	GLfloat* color;
	float vx = 0;
	float vy = 0;
	float vz = 0;
};

//colors
static GLfloat mediumred[] = { 0.6, 0.0, 0.0, 1.0 };
static GLfloat gray_translucent[] = { 0.7, 0.7, 0.7, 0.1 };
static GLfloat gray[] = { 0.5, 0.5, 0.5,  1.0};
static GLfloat blue[] = { 0.0, 0.0, 0.8, 1 };
static GLfloat white[] = { 1.0 , 1.0, 1.0, 1.0 };
static GLfloat white_translucent[] = { 1.0 , 1.0, 1.0, 7.0 };
static GLfloat black[] = { 0.0 , 0.0, 0.0, 0.0 };
static GLfloat red[] = { 1.0, 0.0, 0.0, 1.0 };

//light
static GLfloat lpos[] = { 0.0, 1.0, 0.0, 0.0 };

//camera
static GLdouble cpos[3];
static float alpha = PI / 2.0;
static float beta = PI / 6.0;

//color = colors[polarity+1];
GLfloat colors[3][4] = { {0,0,1,1} ,{0.5, 0.5, 0.5, 1},{1,0,0,1} };

//stat magnets
int numMagnets = 4;
Magnet magnets[8];
GLfloat mRadius = .1;
int initPol[4] = {-1, 1, -1, 0};
GLfloat initPos [4][3] = { {-1,mRadius,1}, {-1,mRadius,-1}, {1,mRadius,-1}, {1,mRadius,1} };

//flying magnets
int numFMagnets = 1;
Magnet fMagnets[10];

//window and screen variables
static GLint ww, hh;

//dragging
int isCtrlSelected = 0;
int dragging = 0;
Magnet* curDragMagnet;

//physics
float fMass = 1;

float gravity = 2;
float pAttraction = .7;
//mass of flying magnets
float fMagRadius = 0.08;

//util functions
void initStatMagnents() {
	for (int x = 0; x < numMagnets; x++) {
		magnets[x].polarity = (rand() % 3) - 1;
		magnets[x].x = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - mRadius);
		magnets[x].y = mRadius;
		magnets[x].z = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - mRadius);
		magnets[x].color = colors[magnets[x].polarity + 1];
	}
}
void initDefaultStatMagnets() {
	for (int x = 0; x < 4; x++) {
		magnets[x].polarity = initPol[x];
		magnets[x].x = initPos[x][0];
		magnets[x].y = initPos[x][1];
		magnets[x].z = initPos[x][2];
		magnets[x].color = colors[magnets[x].polarity + 1];
	}
}
void randomizeStatPositions() {
	for (int x = 0; x < numMagnets; x++) {
		magnets[x].x = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - mRadius);
		magnets[x].y = mRadius;
		magnets[x].z = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - mRadius);
	}
}
void initFlyMagnents() {
	for (int x = 0; x < numFMagnets; x++) {
		fMagnets[x].polarity = (rand() % 3) - 1;
		fMagnets[x].x = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - fMagRadius);
		fMagnets[x].y = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3))) + (0.5 + fMagRadius);
		fMagnets[x].z = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - fMagRadius);
		fMagnets[x].color = colors[fMagnets[x].polarity + 1];
	}
}
void printMagnets(Magnet* magArray,int magCount) {
	for (int x = 0; x < magCount; x++) {
		printf("\nMagnet %i", x);
		printf("\nPolarity %i", magArray[x].polarity);
		printf("\nX Point %f", magArray[x].x);
		printf("\nY Point %f", magArray[x].y);
		printf("\nZ Point %f\n", magArray[x].z);
	}
}
void init() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}
void writeMessage() {
	printf("Homework 4 created by Luke Stoltenberg.");
	printf("\n\nThe stationary magnets are the larger cubes and the flying magents are the smaller ones.");
	printf("\nThe blue color represents negative polarity, red means positive, and gray means neutral.");
	printf("\n\n\tCONTROLS\n\t-----------------------\n");
	
	printf("\n\t hold LMB and drag mouse --------------------- moves stationary magents.");
	printf("\n\t ctrl + click -------------------------------- change polarity of stationary magnet cyclically.");
	printf("\n\t right click --------------------------------- context menu");
	printf("\n\t arrow keys ---------------------------------- move camera");
	printf("\n\t k ------------------------------------------- increase number of flying magnets");
	printf("\n\t j ------------------------------------------- decrease number of flying magnets");
	printf("\n\t > ------------------------------------------- increase polarity force between magnets");
	printf("\n\t < ------------------------------------------- decrease polarity force between magnets");
	printf("\n\t . ------------------------------------------- increase graivty");
	printf("\n\t , ------------------------------------------- decrease graivty");
	printf("\n\t m ------------------------------------------- increase mass of flying magnet");
	printf("\n\t n ------------------------------------------- decrease mass of flying magnet");
}
void reshape(int w, int h) {
	ww = w;
	hh = h;
	glViewport(0, 0, (GLsizei)ww, (GLsizei)hh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, (GLfloat)ww / (GLfloat)hh, 1, 20.0);

}
// drawing
void drawMagnets(GLenum mode) {
	for (int x = 0; x < numMagnets; x++) {
		if (mode == GL_SELECT) glLoadName(x);
		//blue -> gray -> red
		if (isCtrlSelected) {
			if (magnets[x].polarity == 0)
				magnets[x].color = red;
			else if (magnets[x].polarity == 1)
				magnets[x].color = blue;
			else
				magnets[x].color = gray;
		}
		else {
			if (magnets[x].polarity == -1)
				magnets[x].color = blue;
			else if (magnets[x].polarity == 0)
				magnets[x].color = gray;
			else
				magnets[x].color = red;
		}

		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, magnets[x].color);
		glPushMatrix();
		glTranslatef(magnets[x].x, magnets[x].y, magnets[x].z);	
		glutSolidCube(mRadius*2);
		glPopMatrix();
	}
}

void drawFlyMagnets() {
	for (int x = 0; x < numFMagnets; x++) {
		// if the color is null then the magnet at that index should not be drawn
		if (fMagnets[x].color != NULL) {
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, fMagnets[x].color);
			glPushMatrix();
			glTranslatef(fMagnets[x].x, fMagnets[x].y, fMagnets[x].z);
			
			glutSolidCube(fMagRadius*2);
			glPopMatrix();
		}
	}
}
void drawSides() {

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gray_translucent);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, gray_translucent);
	
	//C - back face
	glNormal3f(-1, 0, 0);
	glBegin(GL_POLYGON);
		glVertex3f(-2, 0, -2);
		glVertex3f(-2, 0, 2);
		glVertex3f(-2, 4, 2);
		glVertex3f(-2, 4, -2);
	glEnd();

	// draw the magnets behind the front face
	drawMagnets(GL_RENDER);
	drawFlyMagnets();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gray_translucent);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, gray_translucent);
	//T top
	glNormal3f(0, 1, 0);
	glBegin(GL_POLYGON);
		glVertex3f(2, 4, -2);
		glVertex3f(-2, 4, -2);
		glVertex3f(-2, 4, 2);
		glVertex3f(2, 4, 2);
	glEnd();
	
	//B side
	glNormal3f(0, 0, -1);
	glBegin(GL_POLYGON);
		glVertex3f(2, 0, -2);
		glVertex3f(-2, 0, -2);
		glVertex3f(-2, 4, -2);
		glVertex3f(2, 4, -2);
	glEnd();
	//D side
	glNormal3f(0, 0, 1);
	glBegin(GL_POLYGON);
		glVertex3f(-2, 0, 2);
		glVertex3f(2, 0, 2);
		glVertex3f(2, 4, 2);
		glVertex3f(-2, 4, 2);
	glEnd();
	
	//A front face
	glNormal3f(1, 0, 0);
	glBegin(GL_POLYGON);
		glVertex3f(2, 0, 2);
		glVertex3f(2, 0, -2);
		glVertex3f(2, 4, -2);
		glVertex3f(2, 4, 2);
	 glEnd();	
	
}
void display(void) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	cpos[0] = 6.0 * cos(beta) * sin(alpha);
	cpos[1] = 6.0 * sin(beta);
	cpos[2] = 6.0 * cos(beta) * cos(alpha);
	
	gluLookAt(cpos[0], cpos[1], cpos[2], 0, 2, 0, 0, 1.0, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glPushMatrix();
	glTranslatef(lpos[0], lpos[1], lpos[2]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
	glMaterialfv(GL_FRONT, GL_EMISSION, white);
	glPopMatrix();
	glMaterialfv(GL_FRONT, GL_EMISSION, black);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_translucent);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	//drawing the base on xz-plane, center of base is 0,0,0
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gray);
	glNormal3f(0, 1, 0);
	glBegin(GL_POLYGON);
		glVertex3f(-2, 0, 2);
		glVertex3f(2, 0, 2);
		glVertex3f(2, 0, -2);
		glVertex3f(-2, 0, -2);
	glEnd();
	glPopMatrix();
	drawSides();
	glDisable(GL_BLEND);

	glFlush();
	glutSwapBuffers();
}
void specialkey(GLint key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_RIGHT:
		alpha = alpha + PI / 180;
		if (alpha > 2 * PI) alpha = alpha - 2 * PI;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		alpha = alpha - PI / 180;
		if (alpha < 0) alpha = alpha + 2 * PI;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		if (beta < 0.45*PI) beta = beta + PI / 180;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		if (beta > -0.45*PI) beta = beta - PI / 180;
		glutPostRedisplay();
		break;

	default:
		break;
	}
}
void menu(int value) {
	if (value >= 10) {
		switch (value) {
		case 10:
			numMagnets = 0;
			initStatMagnents();
			glutPostRedisplay();
			break;
		case 11:
			numMagnets = 1;
			initStatMagnents();
			glutPostRedisplay();
			break;
		case 12:
			numMagnets = 4;
			initStatMagnents();
			glutPostRedisplay();
			break;
		case 13:
			numMagnets = 8;
			initStatMagnents();
			glutPostRedisplay();
			break;
	// stationary magnets
		case 30:
			//negative
			for (int x = 0; x < numMagnets; x++) {
				magnets[x].polarity = -1;
				magnets[x].color = colors[0];
			}
			glutPostRedisplay();
			break;
		case 31:
			//neutral
			for (int x = 0; x < numMagnets; x++) {
				magnets[x].polarity = 0;
				magnets[x].color = colors[1];
			}
			glutPostRedisplay();
			break;
		case 32:
			//positive
			for (int x = 0; x < numMagnets; x++) {
				magnets[x].polarity = 1;
				magnets[x].color = colors[2];
			}
			glutPostRedisplay();
			break;
		case 33:
			//random
			for (int x = 0; x < numMagnets; x++) {
				magnets[x].polarity = (rand() %  3) - 1;
				magnets[x].color = colors[magnets[x].polarity + 1];
			}
			glutPostRedisplay();
			break;
	//flying magnets
		case 40:
			//negative
			for (int x = 0; x < numFMagnets; x++) {
				fMagnets[x].polarity = -1;
				fMagnets[x].color = colors[0];
			}
			glutPostRedisplay();
			break;
		case 41:
			//neutral
			for (int x = 0; x < numFMagnets; x++) {
				fMagnets[x].polarity = 0;
				fMagnets[x].color = colors[1];
			}
			glutPostRedisplay();
			break;
		case 42:
			//positive
			for (int x = 0; x < numFMagnets; x++) {
				fMagnets[x].polarity = 1;
				fMagnets[x].color = colors[2];
			}
			glutPostRedisplay();
			break;
		case 43:
			//random
			for (int x = 0; x < numFMagnets; x++) {
				fMagnets[x].polarity = (rand() % 3) - 1;
				fMagnets[x].color = colors[fMagnets[x].polarity + 1];
			}
			glutPostRedisplay();
			break;
		default: 
			return;

		}
	}
	// main menu options
	else {
		switch (value) {
		case 0:
			exit(0);
		case 2:
			randomizeStatPositions();
			printf("randomize positions");
			glutPostRedisplay();
		case 5:
			numMagnets = 4;
			numFMagnets = 1;
			initDefaultStatMagnets();
			initFlyMagnents();
			glutPostRedisplay();
			break;
		default:
			return;

		}
	}
	
}
void processHits(GLuint hits, GLuint buffer[])
{
	unsigned int k, m;
	GLuint names, *ptr;
	ptr = (GLuint *)buffer;
	for (k = 0; k < hits; k++) {
		names = *ptr;
		ptr += 3;
		for (m = 0; m < names; m++) {
			//setting the curDragMagnet
			curDragMagnet = &magnets[*ptr];
			if (isCtrlSelected && names == 1) {
				if (magnets[*ptr].polarity == 0)
					magnets[*ptr].polarity = 1;
				else if (magnets[*ptr].polarity == 1)
					magnets[*ptr].polarity = -1;
				else
					magnets[*ptr].polarity = 0;
			}
		}
	}
}

void Motion(int x, int y) {
	if (dragging == 1 && curDragMagnet) {
		GLint viewport[4];
		GLdouble mvmatrix[16], projmatrix[16];
		GLint realy;  //  OpenGL y coordinate position
		GLdouble wx, wy, wz;  //  returned world x, y, z coords
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
		realy = viewport[3] - (GLint)y - 1;
		gluUnProject((GLdouble)x, (GLdouble)realy, 0.0,
			mvmatrix, projmatrix, viewport, &wx, &wy, &wz);

		GLfloat a, b, c, t;
		a = wx;
		b = wy;
		c = wz;
		gluUnProject((GLdouble)x, (GLdouble)realy, 1.0,
			mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
		GLfloat d, e, f;
		d = wx;
		e = wy;
		f = wz;
		t = b / (b - e);

		// calculate the points
		GLfloat tmpx, tmpz;
		tmpx = a + (t * (d - a));
		tmpz = c + (t * (f - c));
		if (!(tmpx <= (-2 + mRadius) || tmpx >= (2 - mRadius))){
			curDragMagnet->x = tmpx;
		}		
		if (!(tmpz <= (-2 + mRadius) || tmpz >= (2 - mRadius))) {
			curDragMagnet->z = c + (t * (f - c));
		}
		// setting the y point to the radius so that it doesn't go through the bottom plane
		curDragMagnet->y = mRadius;
		
		glutPostRedisplay();
	}
}

#define BUFSIZE 512
void pickMagnets(int button, int state, int x, int y)
{
	GLuint selectBuf[BUFSIZE];
	GLint hits;
	GLint viewport[4];
	GLint ctrlMod = 2;

	//GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint realy;  //  OpenGL y coordinate position
	GLdouble wx, wy, wz;  //  returned world x, y, z coords
	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (glutGetModifiers() == ctrlMod) {
			isCtrlSelected = 1;
		}
		else
			isCtrlSelected = 0;

		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(BUFSIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 6.0, 6.0, viewport);
		gluPerspective(90.0, (GLfloat)ww / (GLfloat)hh, 1.0, 20.0);
		glMatrixMode(GL_MODELVIEW);
		drawMagnets(GL_SELECT);             // select mode doesn't update the frame buffer
		glMatrixMode(GL_PROJECTION);
		glFlush();
		glPopMatrix();

		hits = glRenderMode(GL_RENDER);
		processHits(hits, selectBuf);
		isCtrlSelected = 0;


		dragging = 1;
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
		realy = viewport[3] - (GLint)y - 1;
		gluUnProject((GLdouble)x, (GLdouble)realy, 0.0,
			mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
		gluUnProject((GLdouble)x, (GLdouble)realy, 1.0,
			mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
	}	
	else {
		dragging = 0;
		curDragMagnet = NULL;
		glutPostRedisplay();
	}
	glutPostRedisplay();
}
void physics(int) 
{
	float forces[3];	
	float ctime = .06;
	// for each magnet calculate the force of all the other magnets has on it
	for (int x = 0; x < numFMagnets; x++) {
		forces[0] = 0;
		forces[1] = -(gravity * fMass);
		forces[2] = 0;

		for (int d = 0; d < numFMagnets; d++) {
			// dont want to add a force of the same magnet
			if (x != d) {
			
				//x magnet is the currently selected one
				float distance = sqrt(pow(fMagnets[d].x - fMagnets[x].x, 2) + pow(fMagnets[d].y - fMagnets[x].y, 2) + pow(fMagnets[d].z - fMagnets[x].z, 2));
				if (isnan(distance))
					distance = .1;

				float uVector[3];
				uVector[0] = (fMagnets[d].x - fMagnets[x].x) / distance;
				uVector[1] = (fMagnets[d].y - fMagnets[x].y) / distance;
				uVector[2] = (fMagnets[d].z - fMagnets[x].z) / distance;

				int p;
				if (fMagnets[x].polarity == 0 || fMagnets[d].polarity == 0)
					p = 0;
				else if (fMagnets[x].polarity == fMagnets[d].polarity)
					p = 1;
				else
					p = -1;

				// adding the all the flying magnet forces together
				forces[0] += p * (pAttraction / pow(distance, 2) * uVector[0]);
				forces[1] += p * (pAttraction / pow(distance, 2) * uVector[1]);
				forces[2] += p * (pAttraction / pow(distance, 2) * uVector[2]);
			}
		}
		for (int s = 0; s < numMagnets; s++) {
			float distance = sqrt(pow(magnets[s].x - fMagnets[x].x, 2) + pow(magnets[s].y - fMagnets[x].y, 2) + pow(magnets[s].z - fMagnets[x].z, 2));
			float uVector[3];
			uVector[0] = (fMagnets[x].x - magnets[s].x) / distance;
			uVector[1] = (fMagnets[x].y - magnets[s].y) / distance;
			uVector[2] = (fMagnets[x].z - magnets[s].z) / distance;

			int p;
			if (fMagnets[x].polarity == 0 || magnets[s].polarity == 0)
				p = 0;
			else if (fMagnets[x].polarity == magnets[s].polarity)
				p = 1;
			else
				p = -1;

			// adding the all the stat magnet forces to the total forces
			forces[0] += p * (pAttraction / pow(distance, 2)) * uVector[0];
			forces[1] += p * (pAttraction / pow(distance, 2)) * uVector[1];
			forces[2] += p * (pAttraction / pow(distance, 2)) * uVector[2];
		}

		//velocity calculations
		float vxNext = fMagnets[x].vx + ((forces[0] / fMass) * ctime);
		float vyNext = fMagnets[x].vy + ((forces[1] / fMass) * ctime);
		float vzNext = fMagnets[x].vz + ((forces[2] / fMass) * ctime);

		if (isnan(vxNext))
			vxNext = .01;
		if (isnan(vyNext))
			vyNext = .01;
		if (isnan(vzNext))
			vzNext = .01;

		fMagnets[x].vx = vxNext;
		fMagnets[x].vy = vyNext;
		fMagnets[x].vz = vzNext;

		// tmp pos for checking velocity
		float tmpPos[3];
		tmpPos[0] = fMagnets[x].x + (fMagnets[x].vx * ctime);
		tmpPos[1] = fMagnets[x].y + (fMagnets[x].vy * ctime);
		tmpPos[2] = fMagnets[x].z + (fMagnets[x].vz * ctime);

		// change the velocity direction if hits the side of the box
		if (tmpPos[0] < -2 + fMagRadius || tmpPos[0] > 2 - fMagRadius) {
			float tmp = fMagnets[x].vx * .8;
			// needs to check if nan so doesn't crash when the floats get too small
			if(!isnan(tmp))
				fMagnets[x].vx *= -.8;
			else 
				fMagnets[x].vx = 0;
		}

		if (tmpPos[1] < fMagRadius || tmpPos[1] > 4 - fMagRadius) {
			float tmp = fMagnets[x].vy * .8;
			if (!isnan(tmp))
				fMagnets[x].vy *= -.8;
			else
				fMagnets[x].vy = 0;	
		}

		if (tmpPos[2] < -2 + fMagRadius || tmpPos[2] > 2 - fMagRadius) {
			float tmp = fMagnets[x].vz * .8;
			if (!isnan(tmp))
				fMagnets[x].vz *= -.8;
			else
				fMagnets[x].vz = 0;
		}

		// change the position if out of bounds
		if (tmpPos[0] < -2 + fMagRadius) 
			tmpPos[0] = -2 + fMagRadius;
		else if (tmpPos[0] > 2 - fMagRadius)
			tmpPos[0] = 2 - fMagRadius;

		if (tmpPos[1] < fMagRadius)
			tmpPos[1] = fMagRadius;
		else if (tmpPos[1] > 4 - fMagRadius)
			tmpPos[1] = 4 - fMagRadius;

		if (tmpPos[2] < -2 + fMagRadius)
			tmpPos[2] = -2 + fMagRadius;
		else if(tmpPos[2] > 2 - fMagRadius)
			tmpPos[2] = 2 - fMagRadius;

		//set the position
		fMagnets[x].x = tmpPos[0];
		fMagnets[x].y = tmpPos[1];
		fMagnets[x].z = tmpPos[2];
	}
	glutPostRedisplay();
	glutTimerFunc(10, physics, NULL);
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'k':
		if (numFMagnets < 10) {
			numFMagnets++;
			Magnet tmp;
			tmp.polarity = (rand() % 3) - 1;
		
			tmp.x = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - fMagRadius);
			tmp.y = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3))) + (0.5 + fMagRadius);
			tmp.z = (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.8))) - (2 - fMagRadius);
			tmp.color = colors[tmp.polarity +1];

			fMagnets[numFMagnets -1] = tmp;
			glutPostRedisplay();
		}
		break;
	case 'j':
		if (numFMagnets > 1) {
			Magnet tmp;
			tmp.polarity = NULL;
			tmp.x = NULL;
			tmp.y = NULL;
			tmp.z = NULL;
			tmp.color = NULL;

			fMagnets[numFMagnets - 1] = tmp;
			numFMagnets--;

			glutPostRedisplay();
		}
		break;
	case '>':
		if (pAttraction < 2) {
			pAttraction += .25;
			glutPostRedisplay();
		}
		break;
	case '<':
		if (pAttraction > 0) {
			pAttraction -= .25;
			glutPostRedisplay();
		}
		break;
	case '.':
		if (gravity < 8) {
			gravity += 1;
			glutPostRedisplay();
		}
		break;
	case ',':
		if (gravity >= 1) {
			gravity -= .5;
			glutPostRedisplay();
		}
		break;
	case 'm':
		if (fMass < 4) {
			fMass ++;
			glutPostRedisplay();
		}
		break;
	case 'n':
		if (fMass > 1) {
			fMass--;
			glutPostRedisplay();
		}
		break;

	default:
		return;
	}
	
}
int main(int argc, char** argv) {
	srand(time(NULL));
	int numMenu, statPolMenu, flyPolMenu, mainMenu;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(1366, 768);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	
	/*menu*/
	//number of magents sub menu
	numMenu = glutCreateMenu(menu);
	glutAddMenuEntry("0 Magnets", 10);
	glutAddMenuEntry("1 Magnet", 11);
	glutAddMenuEntry("4 Magnets", 12);
	glutAddMenuEntry("8 Magnets", 13);

	// station Magnets sub menu
	statPolMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Negative", 30);
	glutAddMenuEntry("Neutral", 31);
	glutAddMenuEntry("Positive", 32);
	glutAddMenuEntry("Random", 33);

	// flying magnet sub menu
	flyPolMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Negative", 40);
	glutAddMenuEntry("Neutral", 41);
	glutAddMenuEntry("Positive", 42);
	glutAddMenuEntry("Random", 43);

	mainMenu = glutCreateMenu(menu);
	// menu prefix 1
	glutAddSubMenu("Number of Magnets", numMenu);
	// menu prefix 2
	glutAddMenuEntry("Randomize Stationary Positions", 2);
	// menu prefix 3
	glutAddSubMenu("Stationary Polarity", statPolMenu);
	// menu prefix 4
	glutAddSubMenu("Flying Polarity", flyPolMenu);
	// menu prefix 5
	glutAddMenuEntry("Restart", 5);
	// menu prefix 0
	glutAddMenuEntry("Exit Program", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMouseFunc(pickMagnets);
	glutMotionFunc(Motion);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(10,physics,NULL);

	initDefaultStatMagnets();
	initFlyMagnents();

	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(specialkey);
	writeMessage();
	glutMainLoop();
	return 0;
}