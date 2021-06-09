#include <iostream>
#include <fstream>
#include <GL/glut.h>
using namespace std;

#define GL_CLAMP_TO_EDGE 0x812F 
#define TO_RAD (3.14159265/180.0); 

const float GREY[4] = { 0.2, 0.2, 0.2, 1.0 },
WHITE[4] = { 1.0, 1.0, 1.0, 1.0 },
BLACK[4] = { 0.0, 0.0, 0.0, 1.0 },
GOLD[4] = { 0.8, 0.7, 0.3, 1 },
BROWN[4] = { 0.5, 0.3, 0.1, 1 },
YELLOW[4] = { 0.675, 0.566, 0.324, 1 };

GLuint texId[9];
enum Texture { SKYBOX_LEFT, SKYBOX_FRONT, SKYBOX_RIGHT, SKYBOX_BACK, SKYBOX_TOP, SKYBOX_BOTTOM, SAND, SANDSTONE_BRICK, CHECKER };

struct Vertex {
	float x;
	float y;
	float z;
};

struct Door {
	bool open = false;
	bool opening = false;
	bool closing = false;
	float angle = 0;
} door;

struct MovingObject {
	float x;
	float y;
	float z;
	float angle;
} cam, beetle,camel,mummy,bowl;

struct Leg {
	bool increasing = false;
	float angle = 0;
} clFrontLeft, clFrontRight, clBackLeft, clBackRight,
blFrontLeft, blFrontRight, blMiddleLeft, blMiddleRight, blBackLeft, blBackRight;

bool charCamEnabled = false;
bool light1Enabled = true;
bool mummyPaused = false;
void loadTGA(const char* filename)
{
	char id, cmap, imgtype, bpp, c_garb;
	char* imageData, temp;
	short int s_garb, wid, hgt;
	int nbytes, size, indx;
	ifstream file(filename, ios::in | ios::binary);
	if (!file)
	{
		cout << "*** Error opening image file: " << filename << endl;
		exit(1);
	}
	file.read(&id, 1);
	file.read(&cmap, 1);
	file.read(&imgtype, 1);
	if (imgtype != 2 && imgtype != 3)   //2= colour (uncompressed),  3 = greyscale (uncompressed)
	{
		cout << "*** Incompatible image type: " << (int)imgtype << endl;
		exit(1);
	}
	//Color map specification
	file.read((char*)&s_garb, 2);
	file.read((char*)&s_garb, 2);
	file.read(&c_garb, 1);
	//Image specification
	file.read((char*)&s_garb, 2);  //x origin
	file.read((char*)&s_garb, 2);  //y origin
	file.read((char*)&wid, 2);     //image width								    
	file.read((char*)&hgt, 2);     //image height
	file.read(&bpp, 1);     //bits per pixel
	file.read(&c_garb, 1);  //img descriptor
	nbytes = bpp / 8;           //No. of bytes per pixels
	size = wid * hgt * nbytes;  //Total number of bytes to be read
	imageData = new char[size];
	file.read(imageData, size);
	if (nbytes > 2)   //swap R and B
	{
		for (int i = 0; i < wid * hgt; i++)
		{
			indx = i * nbytes;
			temp = imageData[indx];
			imageData[indx] = imageData[indx + 2];
			imageData[indx + 2] = temp;
		}
	}

	switch (nbytes)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, 1, wid, hgt, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, imageData);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, 3, wid, hgt, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, 4, wid, hgt, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		break;
	}
	delete imageData;
}

float atan_degrees_360(float opp, float adj) {
	if (adj == 0) {
		if (opp > 0) {
			return 90;
		}
		else {
			return -90;
		}
	}
	else {
		if (adj >= 0) {
			return atan(opp / adj) / TO_RAD;
		}
		else {
			return 180 + atan(opp / adj) / TO_RAD;
		}
	}
}

void vertex(Vertex* v) {
	glVertex3f(v->x, v->y, v->z);
}

void normal(Vertex* v1, Vertex* v2, Vertex* v3, bool reversed) {
	float x1 = v1->x, y1 = v1->y, z1 = v1->z;
	float x2 = v2->x, y2 = v2->y, z2 = v2->z;
	float x3 = v3->x, y3 = v3->y, z3 = v3->z;

	float nx, ny, nz;
	nx = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
	ny = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
	nz = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);

	if (reversed) {
		glNormal3f(nx, ny, nz);
	}
	else {
		glNormal3f(-nx, -ny, -nz);
	}
}

void loadGLTextures() {			// Load bitmaps And Convert To Textures
	glGenTextures(9, texId); 		// Create texture ids
	// left
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_LEFT]);
	loadTGA("textures/skybox/left.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//front
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_FRONT]);
	loadTGA("textures/skybox/front.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//right
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_RIGHT]);
	loadTGA("textures/skybox/right.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//back
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_BACK]);
	loadTGA("textures/skybox/back.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// top
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_TOP]);
	loadTGA("textures/skybox/top.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// down 
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_BOTTOM]);
	loadTGA("textures/skybox/down.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, texId[SAND]);
	loadTGA("textures/sand.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texId[SANDSTONE_BRICK]);
	loadTGA("textures/sandstone_brick.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texId[CHECKER]);
	loadTGA("textures/checker.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void drawSkybox() {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	////////////////////// LEFT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_LEFT]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1000, 0, 1000);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1000, 0, -1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1000, 1000, -1000);
	glTexCoord2f(1.0, 1.0); glVertex3f(-1000, 1000, 1000);
	glEnd();

	////////////////////// FRONT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_FRONT]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1000, 0, -1000);
	glTexCoord2f(0.0, 0.0); glVertex3f(1000, 0, -1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(1000, 1000, -1000);
	glTexCoord2f(1.0, 1.0); glVertex3f(-1000, 1000, -1000);
	glEnd();

	////////////////////// RIGHT WALL ///////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_RIGHT]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(1000, 0, -1000);
	glTexCoord2f(0.0, 0.0); glVertex3f(1000, 0, 1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(1000, 1000, 1000);
	glTexCoord2f(1.0, 1.0); glVertex3f(1000, 1000, -1000);
	glEnd();


	////////////////////// REAR WALL ////////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_BACK]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(1000, 0, 1000);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1000, 0, 1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1000, 1000, 1000);
	glTexCoord2f(1.0, 1.0); glVertex3f(1000, 1000, 1000);
	glEnd();

	/////////////////////// TOP //////////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_TOP]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 1.0); glVertex3f(-1000, 1000, -1000);
	glTexCoord2f(1.0, 0.0); glVertex3f(1000, 1000, -1000);
	glTexCoord2f(0.0, 0.0); glVertex3f(1000, 1000, 1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1000, 1000, 1000);
	glEnd();

	/////////////////////// FLOOR //////////////////////////
	glBindTexture(GL_TEXTURE_2D, texId[SKYBOX_BOTTOM]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1000, 0, 1000);
	glTexCoord2f(1.0, 0.0); glVertex3f(1000, 0, 1000);
	glTexCoord2f(1.0, 1.0); glVertex3f(1000, 0, -1000);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1000, 0, -1000);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawGround() {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId[SAND]);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	for (int x = -400; x <= 400; x += 20) {
		for (int z = -400; z <= 400; z += 20) {
			glTexCoord2f(0.0, 0.0); glVertex3f(x, 0, z);
			glTexCoord2f(0.0, 1.0); glVertex3f(x, 0, z + 20);
			glTexCoord2f(1.0, 1.0); glVertex3f(x + 20, 0, z + 20);
			glTexCoord2f(1.0, 0.0); glVertex3f(x + 20, 0, z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawFloor() {
	const int MIN = -75, MAX = 75, INC = 15;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId[CHECKER]);

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	for (int x = MIN; x < MAX; x += INC) {
		for (int z = MIN; z < MAX; z += INC) {
			glTexCoord2f(0.0, 0.0); glVertex3f(x, 0, z);
			glTexCoord2f(0.0, 1.0); glVertex3f(x, 0, z + INC);
			glTexCoord2f(1.0, 1.0); glVertex3f(x + INC, 0, z + INC);
			glTexCoord2f(1.0, 0.0); glVertex3f(x + INC, 0, z);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawDoor() {
	glTranslatef(0.15, 0, 0);
	glRotatef(door.angle, 0, 1, 0);
	glTranslatef(-0.15, 0, 0);

	glPushMatrix();
	glColor4fv(GOLD);
	glScalef(0.15, 0.5, 0.04);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_SPECULAR, BLACK);
	glColor4f(0.15, 0.15, 0.15, 1);
	glScalef(0.3, 1, 0.02);
	glutSolidCube(1);
	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glPopMatrix();

	glColor4f(1, 1, 1, 1);
}
//画影子
void drawShadowPyramid() {
	Vertex v[5];
	v[0] = { 0, 3, 0 };
	v[1] = { -1.5, 0, -1.5 };
	v[2] = { -1.5, 0, 1.5 };
	v[3] = { 1.5, 0, 1.5 };
	v[4] = { 1.5, 0, -1.5 };

	glTranslatef(100, 0, -100);
	glScalef(50, 50, 50);
	glRotatef(90, 0, 1, 0);

	glBegin(GL_TRIANGLES);

	// left face
	normal(&v[0], &v[2], &v[1], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[2].x, v[2].y, v[2].z);
	glVertex3f(v[1].x, v[1].y, v[1].z);

	// right face
	normal(&v[0], &v[3], &v[2], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[3].x, v[3].y, v[3].z);
	glVertex3f(v[2].x, v[2].y, v[2].z);

	// back face
	normal(&v[0], &v[4], &v[3], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[4].x, v[4].y, v[4].z);
	glVertex3f(v[3].x, v[3].y, v[3].z);

	// front face
	normal(&v[0], &v[1], &v[4], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[1].x, v[1].y, v[1].z);
	glVertex3f(v[4].x, v[4].y, v[4].z);

	glEnd();
}

//画整个金字塔
void drawPyramid(bool reversed) {
	Vertex v[20];

	// Bottom Corners
	v[0] = { 0, 0, 0 };
	v[1] = { 0, 0, 3 };
	v[2] = { 3, 0, 3 };
	v[3] = { 3, 0, 0 };

	// Roof Corners
	v[4] = { 1, 2, 1 };
	v[5] = { 1, 2, 2 };
	v[6] = { 2, 2, 2 };
	v[7] = { 2, 2, 1 };

	// Front Face Bottom
	v[8] = { 1.2, 0, 0 };
	v[9] = { 1.8, 0, 0 };

	// Front Face Mid
	v[10] = { 1.2, 1, 0.5 };
	v[11] = { 1.8, 1, 0.5 };
	v[12] = { 0.5, 1, 0.5 };
	v[13] = { 2.5, 1, 0.5 };

	// Front Face Top
	v[14] = { 1.2, 2, 1 };
	v[15] = { 1.8, 2, 1 };

	// Entrance Bottom
	v[16] = { 1.2, 0, -0.25 };
	v[17] = { 1.8, 0, -0.25 };

	// Entrance Roof
	v[18] = { 1.2, 1, -0.25 };
	v[19] = { 1.8, 1, -0.25 };

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId[SANDSTONE_BRICK]);

	glBegin(GL_QUADS);

	// left face
	normal(&v[0], &v[4], &v[5], reversed);
	glTexCoord2f(3, 0); glVertex3f(v[0].x, v[0].y, v[0].z);
	glTexCoord2f(2, 2); glVertex3f(v[4].x, v[4].y, v[4].z);
	glTexCoord2f(1, 2); glVertex3f(v[5].x, v[5].y, v[5].z);
	glTexCoord2f(0, 0); glVertex3f(v[1].x, v[1].y, v[1].z);

	// back face
	normal(&v[1], &v[5], &v[6], reversed);
	glTexCoord2f(3, 0); glVertex3f(v[1].x, v[1].y, v[1].z);
	glTexCoord2f(2, 2); glVertex3f(v[5].x, v[5].y, v[5].z);
	glTexCoord2f(1, 2); glVertex3f(v[6].x, v[6].y, v[6].z);
	glTexCoord2f(0, 0); glVertex3f(v[2].x, v[2].y, v[2].z);

	// right face
	normal(&v[2], &v[6], &v[7], reversed);
	glTexCoord2f(3, 0); glVertex3f(v[2].x, v[2].y, v[2].z);
	glTexCoord2f(2, 2); glVertex3f(v[6].x, v[6].y, v[6].z);
	glTexCoord2f(1, 2); glVertex3f(v[7].x, v[7].y, v[7].z);
	glTexCoord2f(0, 0); glVertex3f(v[3].x, v[3].y, v[3].z);

	// front face A //
	normal(&v[0], &v[8], &v[10], reversed);
	glTexCoord2f(v[0].x, v[0].y); glVertex3f(v[0].x, v[0].y, v[0].z);
	glTexCoord2f(v[8].x, v[8].y); glVertex3f(v[8].x, v[8].y, v[8].z);
	glTexCoord2f(v[10].x, v[10].y); glVertex3f(v[10].x, v[10].y, v[10].z);
	glTexCoord2f(v[12].x, v[12].y); glVertex3f(v[12].x, v[12].y, v[12].z);

	// front face E //
	normal(&v[9], &v[3], &v[13], reversed);
	glTexCoord2f(v[9].x, v[9].y); glVertex3f(v[9].x, v[9].y, v[9].z);
	glTexCoord2f(v[3].x, v[3].y); glVertex3f(v[3].x, v[3].y, v[3].z);
	glTexCoord2f(v[13].x, v[13].y); glVertex3f(v[13].x, v[13].y, v[13].z);
	glTexCoord2f(v[11].x, v[11].y); glVertex3f(v[11].x, v[11].y, v[11].z);

	// front face B //
	normal(&v[12], &v[10], &v[14], reversed);
	glTexCoord2f(v[12].x, v[12].y); glVertex3f(v[12].x, v[12].y, v[12].z);
	glTexCoord2f(v[10].x, v[10].y); glVertex3f(v[10].x, v[10].y, v[10].z);
	glTexCoord2f(v[14].x, v[14].y); glVertex3f(v[14].x, v[14].y, v[14].z);
	glTexCoord2f(v[4].x, v[4].y); glVertex3f(v[4].x, v[4].y, v[4].z);

	// front face D //
	normal(&v[11], &v[13], &v[7], reversed);
	glTexCoord2f(v[11].x, v[11].y); glVertex3f(v[11].x, v[11].y, v[11].z);
	glTexCoord2f(v[13].x, v[13].y); glVertex3f(v[13].x, v[13].y, v[13].z);
	glTexCoord2f(v[7].x, v[7].y); glVertex3f(v[7].x, v[7].y, v[7].z);
	glTexCoord2f(v[15].x, v[15].y); glVertex3f(v[15].x, v[15].y, v[15].z);

	// front face C //
	normal(&v[10], &v[11], &v[15], reversed);
	glTexCoord2f(v[10].x, v[10].y); glVertex3f(v[10].x, v[10].y, v[10].z);
	glTexCoord2f(v[11].x, v[11].y); glVertex3f(v[11].x, v[11].y, v[11].z);
	glTexCoord2f(v[15].x, v[15].y); glVertex3f(v[15].x, v[15].y, v[15].z);
	glTexCoord2f(v[14].x, v[14].y); glVertex3f(v[14].x, v[14].y, v[14].z);

	// entrance left wall //
	normal(&v[8], &v[16], &v[18], reversed);
	glTexCoord2f(v[8].z, v[8].y); glVertex3f(v[8].x, v[8].y, v[8].z);
	glTexCoord2f(v[16].z, v[16].y); glVertex3f(v[16].x, v[16].y, v[16].z);
	glTexCoord2f(v[18].z, v[18].y); glVertex3f(v[18].x, v[18].y, v[18].z);
	glTexCoord2f(v[10].z, v[10].y); glVertex3f(v[10].x, v[10].y, v[10].z);

	// entrance right wall //
	normal(&v[17], &v[9], &v[11], reversed);
	glTexCoord2f(v[17].z, v[17].y); glVertex3f(v[17].x, v[17].y, v[17].z);
	glTexCoord2f(v[9].z, v[9].y); glVertex3f(v[9].x, v[9].y, v[9].z);
	glTexCoord2f(v[11].z, v[11].y); glVertex3f(v[11].x, v[11].y, v[11].z);
	glTexCoord2f(v[19].z, v[19].y); glVertex3f(v[19].x, v[19].y, v[19].z);

	// entrance roof //
	normal(&v[18], &v[19], &v[11], reversed);
	glTexCoord2f(v[18].x, v[18].z); glVertex3f(v[18].x, v[18].y, v[18].z);
	glTexCoord2f(v[19].x, v[19].z); glVertex3f(v[19].x, v[19].y, v[19].z);
	glTexCoord2f(v[11].x, v[11].z); glVertex3f(v[11].x, v[11].y, v[11].z);
	glTexCoord2f(v[10].x, v[10].z); glVertex3f(v[10].x, v[10].y, v[10].z);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}
void drawPyramidion() {
	Vertex v[5];

	v[0] = { 1.5, 3, 1.5 };
	v[1] = { 1, 2, 1 };
	v[2] = { 1, 2, 2 };
	v[3] = { 2, 2, 2 };
	v[4] = { 2, 2, 1 };

	glColor4fv(GOLD);
	glBegin(GL_TRIANGLES);

	// left face
	normal(&v[0], &v[2], &v[1], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[2].x, v[2].y, v[2].z);
	glVertex3f(v[1].x, v[1].y, v[1].z);

	// right face
	normal(&v[0], &v[3], &v[2], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[3].x, v[3].y, v[3].z);
	glVertex3f(v[2].x, v[2].y, v[2].z);

	// back face
	normal(&v[0], &v[4], &v[3], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[4].x, v[4].y, v[4].z);
	glVertex3f(v[3].x, v[3].y, v[3].z);

	// front face
	normal(&v[0], &v[1], &v[4], false);
	glVertex3f(v[0].x, v[0].y, v[0].z);
	glVertex3f(v[1].x, v[1].y, v[1].z);
	glVertex3f(v[4].x, v[4].y, v[4].z);

	glEnd();
	glColor4fv(WHITE);
}
void drawCompletePyramid() {
	glTranslatef(100, 0, -100);
	glScalef(50, 50, 50);
	glRotatef(90, 0, 1, 0);
	glTranslatef(-1.5, 0, -1.5); // to centre the pyramid

	// Left Door
	glPushMatrix();
	glTranslatef(1.35 + 0.001, 0.5 - 0.001, -0.235);
	glScalef(-1, 1, 1);
	drawDoor();
	glPopMatrix();

	// Right Door
	glPushMatrix();
	glTranslatef(1.65 - 0.001, 0.5 - 0.001, -0.235);
	drawDoor();
	glPopMatrix();

	glDisable(GL_LIGHT0);
	if (light1Enabled) {
		glEnable(GL_LIGHT1);
	}

	glPushMatrix();
	glTranslatef(0.03, 0, 0.01);
	glScalef(0.98, 0.98, 0.98);
	drawPyramid(true);
	glPopMatrix();

	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);

	drawPyramidion();
	drawPyramid(false);
}

//画骆驼
void drawEye() {
	glPushMatrix();
	glColor4fv(WHITE);
	glScalef(1, 1, 0.3);
	glutSolidTorus(0.5, 1, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glColor4fv(BLACK);
	glScalef(0.5, 0.5, 0.15);
	glutSolidTorus(1, 1, 16, 16);
	glPopMatrix();
}
void drawCamelLeg(Leg* camelLeg) {
	glScalef(1.5, 1, 1.5);

	glTranslatef(0, 5, 0);
	glRotatef(camelLeg->angle, 1, 0, 0);
	glTranslatef(0, -5, 0);

	// upper
	glPushMatrix();
	glColor4f(0.8, 0.7, 0.5, 1);
	glScalef(1, 10, 1);
	glutSolidCube(1);
	glPopMatrix();

	// top of hoof
	glPushMatrix();
	glColor4f(0.9, 0.85, 0.7, 1);
	glTranslatef(0, -5, 0);
	glutSolidCube(1);
	glPopMatrix();

	// bottom of hoof
	glPushMatrix();
	glColor4f(0.4, 0.3, 0.1, 1);
	glTranslatef(0, -6, 0);
	glutSolidCube(1);
	glPopMatrix();

	glColor4f(0.8, 0.7, 0.5, 1);
}
void drawCamel() {
	glColor4f(0.8, 0.7, 0.5, 1);

	glPushMatrix();
	glScalef(6, 8, 10);
	glutSolidCube(1);
	glPopMatrix();

	// hump
	glPushMatrix();
	glTranslatef(0, 3.5, -2);
	glScalef(2.5, 5, 3);
	glutSolidSphere(1, 16, 16);
	glPopMatrix();

	// NECK //
	glPushMatrix();
	glTranslatef(0, 0, 6.5);
	glScalef(3, 4, 3);
	glutSolidCube(1);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(0, 5, 7.25);
	glScalef(2, 6, 1.5);
	glutSolidCube(1);
	glPopMatrix();

	// HEAD //
	// back of head
	glPushMatrix();
	glTranslatef(0, 10, 8);
	glScalef(3, 4.5, 3);
	glutSolidCube(1);
	glPopMatrix();

	// front of head
	glPushMatrix();
	glTranslatef(0, 9.5, 10.75);
	glScalef(2, 3.5, 2.5);
	glutSolidCube(1);
	glPopMatrix();

	// left eye
	glPushMatrix();
	glTranslatef(1.5, 10.5, 8.5);
	glScalef(0.4, 0.4, 0.4);
	glRotatef(90, 0, 1, 0);
	drawEye();
	glPopMatrix();

	// right eye
	glPushMatrix();
	glTranslatef(-1.5, 10.5, 8.5);
	glScalef(0.4, 0.4, 0.4);
	glRotatef(-90, 0, 1, 0);
	drawEye();
	glPopMatrix();

	glColor4f(0.5, 0.4, 0.25, 1);

	// left ear
	glPushMatrix();
	glTranslatef(1.2, 12.5, 6.9);
	glScalef(0.4, 1.5, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	// right ear
	glPushMatrix();
	glTranslatef(-1.2, 12.5, 6.9);
	glScalef(0.4, 1.5, 0.6);
	glutSolidCube(1);
	glPopMatrix();

	// LEGS //
	// front left leg
	glPushMatrix();
	glTranslatef(2.25, -8, 4.25);
	drawCamelLeg(&clFrontLeft);
	glPopMatrix();

	// front right leg
	glPushMatrix();
	glTranslatef(-2.25, -8, 4.25);
	drawCamelLeg(&clFrontRight);
	glPopMatrix();

	// back left leg
	glPushMatrix();
	glTranslatef(2.25, -8, -4.25);
	drawCamelLeg(&clBackLeft);
	glPopMatrix();

	// back right leg
	glPushMatrix();
	glTranslatef(-2.25, -8, -4.25);
	drawCamelLeg(&clBackRight);
	glPopMatrix();

	glColor4fv(WHITE);
}

void drawBeetleLeg(Leg* beetleLeg) {
	glColor4f(0.715, 0.105, 0, 1);

	//glTranslatef(0.75, 0, 0);
	glRotatef(beetleLeg->angle, 0, 1, 0);
	//glTranslatef(-0.75, 0, 0);

	// upper leg
	glPushMatrix();
	glScalef(1.5, 0.3, 0.3);
	glutSolidCube(1);
	glPopMatrix();

	// lower leg
	glPushMatrix();
	glTranslatef(0.6, 0, 0.6);
	glScalef(0.3, 0.3, 1);
	glutSolidCube(1);
	glPopMatrix();
}
void drawBeetle() {
	glColor4f(0.05, 0.05, 0.05, 1);

	// abdomen
	glPushMatrix();
	glScalef(1.55, 1, 2.5);
	glutSolidCube(1);
	glPopMatrix();

	// head
	glPushMatrix();
	glTranslatef(0, 0, 1.5);
	glScalef(1, 0.75, 0.5);
	glRotatef(-90, 1, 0, 0);
	//gluCylinder(1, 1, 16, 16);
	glPopMatrix();

	// LEGS //
	// front left
	glPushMatrix();
	glTranslatef(0.7, -0.3, 0.85);
	drawBeetleLeg(&blFrontLeft);
	glPopMatrix();

	// front right
	glPushMatrix();
	glTranslatef(-0.7, -0.3, 0.85);
	glScalef(-1, 1, 1);
	drawBeetleLeg(&blFrontRight);
	glPopMatrix();

	// middle left
	glPushMatrix();
	glTranslatef(1.2, -0.3, 0);
	glScalef(1, 1, -1);
	drawBeetleLeg(&blMiddleLeft);
	glPopMatrix();

	// middle right
	glPushMatrix();
	glTranslatef(-1.2, -0.3, 0);
	glScalef(-1, 1, -1);
	drawBeetleLeg(&blMiddleRight);
	glPopMatrix();

	// back left
	glPushMatrix();
	glTranslatef(0.7, -0.3, -0.85);
	glScalef(1, 1, -1);
	drawBeetleLeg(&blBackLeft);
	glPopMatrix();

	// back right
	glPushMatrix();
	glTranslatef(-0.7, -0.3, -0.85);
	glScalef(-1, 1, -1);
	drawBeetleLeg(&blBackRight);
	glPopMatrix();

	glColor4fv(WHITE);
}

void drawMummy() {
	const float spotPos[] = { 0, 3, 0, 1 },
		spotDir[] = { 0, 1, 0 };

	glPushMatrix();
	glColor4fv(YELLOW);
	glTranslatef(0, -3, 0);
	glutSolidCube(7);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 15);

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
	glLightfv(GL_LIGHT1, GL_POSITION, spotPos);

	// head objectF
	glPushMatrix();
	glColor4fv(BROWN);
	glScalef(1, 1.5, 1);
	glutSolidSphere(3,32,32);
	glPopMatrix();
	glPopMatrix();
	glColor4fv(WHITE);
}
void drawCoffin() {
	const float HEIGHT = 10;

	Vertex v[6] = { 0 }, w[6] = { 0 };
	v[0] = { 7.5, 0, -25 };
	v[1] = { 10, 0, 0 };
	v[2] = { 8, 0, 30 };
	v[3] = { -8, 0, 30 };
	v[4] = { -10, 0, 0 };
	v[5] = { -7.5, 0, -25 };

	for (int i = 0; i < 6; i++) {
		w[i].x = v[i].x;
		w[i].y = HEIGHT;
		w[i].z = v[i].z;
	}

	glColor4fv(GOLD);
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < 6; i++) {
		if (i > 0) {
			normal(&w[i - 1], &v[i - 1], &v[i], false);
		}
		vertex(&v[i]);
		if (i > 0) {
			normal(&w[i - 1], &v[i], &w[i], false);
		}
		vertex(&w[i]);
	}
	vertex(&v[0]);
	vertex(&w[0]);
	glEnd();

	glBegin(GL_QUADS);
	normal(&v[0], &v[1], &v[4], false);
	vertex(&v[0]);
	vertex(&v[1]);
	vertex(&v[4]);
	vertex(&v[5]);

	normal(&v[1], &v[2], &v[3], false);
	vertex(&v[1]);
	vertex(&v[2]);
	vertex(&v[3]);
	vertex(&v[4]);
	glEnd();
	glColor4fv(WHITE);
}

void drawBowl() {
	const float ROTATIONS = 144, COLOUR_INC = 0.005;
	const float ROT_ANGLE = (360 / ROTATIONS) * TO_RAD;
	const int N = 50;
	Vertex v[N];
	Vertex w[N];
	float colourAdd;

	for (int i = 0; i < N; i++) {
		v[i].y = i * 0.25;
		v[i].x = sqrt(v[i].y);
		v[i].z = 0;
	}

	for (int j = 0; j < ROTATIONS; j++) {
		if (j >= ROTATIONS / 2) {
			colourAdd = (ROTATIONS - j) * COLOUR_INC;
		}
		else {
			colourAdd = j * COLOUR_INC;
		}
		glColor4f(0.25, 0.1875 + colourAdd, 0.6 - colourAdd, 1);
		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < N; i++) {
			w[i].x = v[i].x * cos(ROT_ANGLE) + v[i].z * sin(ROT_ANGLE);
			w[i].y = v[i].y;
			w[i].z = -v[i].x * sin(ROT_ANGLE) + v[i].z * cos(ROT_ANGLE);

			if (i > 0) {
				normal(&w[i], &v[i - 1], &v[i], false);
			}
			vertex(&v[i]);
			if (i > 0) {
				normal(&w[i - 1], &v[i], &w[i], false);
			}
			vertex(&w[i]);

			v[i].x = w[i].x;
			v[i].y = w[i].y;
			v[i].z = w[i].z;
		}
		glEnd();
	}

	glColor4fv(WHITE);
}
void drawPedestal() {
	const float COL_HEIGHT = 17;

	glPushMatrix();
	glColor4fv(YELLOW);
	glTranslatef(0, 2 - COL_HEIGHT, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(6, 2, 32, 32);
	glPopMatrix();

	// rotating pieces
	glPushMatrix();
	glRotatef(bowl.angle, 0, 1, 0);
	
	// column
	glPushMatrix();
	glColor4fv(BROWN);
	glTranslatef(0, 2 - COL_HEIGHT,0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(4, COL_HEIGHT, 32, 32);
	glPopMatrix();


	// bowl
	glPushMatrix();
	drawBowl();
	glPopMatrix();

	glPopMatrix();
}

void display() {
	float lpos[4] = { -1000.0, 1000.0, 1000.0, 1.0 };  //light's position
	float shadowMat[16] = {
			lpos[1], 0, 0, 0,	-lpos[0], 0, -lpos[2], -1,
			0, 0, lpos[1], 0,	0, 0, 0, lpos[1]
	};

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    //GL_LINE = Wireframe;   GL_FILL = Solid
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	if (charCamEnabled) {
		float angleInRads = camel.angle * TO_RAD;
		gluLookAt(camel.x, camel.y + 15, camel.z, camel.x + 10000 * sin(angleInRads), cam.y, cam.z + 10000 * cos(angleInRads), 0, 1, 0);
	}
	else {
		gluLookAt(cam.x, cam.y, cam.z, cam.x + cos(cam.angle), cam.y, cam.z + sin(cam.angle), 0, 1, 0);
	}
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);   //set light position

	glPushMatrix();
	glTranslatef(0, -2000, 0);
	glScalef(5, 4, 5);
	drawSkybox();
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glColor4f(0.15, 0.15, 0.15, 1.0);
	glTranslatef(0, 0.01, 0);
	glMultMatrixf(shadowMat);
	drawShadowPyramid();
	glColor4f(1, 1, 1, 0);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	// NORMAL LIGHTING //
	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);

	glPushMatrix();
	drawGround();
	glPopMatrix();

	glPushMatrix();
	drawCompletePyramid();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(camel.x, camel.y, camel.z);
	glRotatef(camel.angle, 0, 1, 0);
	drawCamel();
	glPopMatrix();

	glDisable(GL_LIGHT0);
	if (light1Enabled) {
		glEnable(GL_LIGHT1);
	}

	glPushMatrix();
	glTranslatef(100, 0.02, -100);
	drawFloor();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(beetle.x, beetle.y, beetle.z);
	glRotatef(beetle.angle, 0, 1, 0);
	drawBeetle();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(85, 15, -120);
	drawPedestal();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(130, 4, -100);
	glRotatef(90, 0, 1, 0);
	drawMummy();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(130, 0.05, -100);
	glRotatef(90, 0, 1, 0);
	drawCoffin();
	glPopMatrix();
	// NORMAL LIGHTING //
	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);

	glFlush();
}
void initialize() {
	loadGLTextures();

	glEnable(GL_LIGHTING);					//Enable OpenGL states
	glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);

	glLightfv(GL_LIGHT0, GL_AMBIENT, GREY);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, WHITE);
	glLightfv(GL_LIGHT0, GL_SPECULAR, WHITE);

	glLightfv(GL_LIGHT1, GL_AMBIENT, GREY);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, WHITE);
	glLightfv(GL_LIGHT1, GL_SPECULAR, WHITE);

	glClearColor(0, 0, 0, 0);	//Background colour

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 1, 10000);  //The camera view volume

	// Set camera's initial position & angle //
	cam.x = -50;
	cam.y = 30;
	cam.z = -250;
	cam.angle = 45.0 * TO_RAD

		// Set camel's initial position
		camel.x = 0;
	camel.y = 15;
	camel.z = -220;

	// Setup camel legs to swing alternately
	clFrontLeft.increasing = true;
	clBackRight.increasing = true;

	beetle.x = 70;
	beetle.y = 1;
	beetle.z = -100;
	beetle.angle = 0;

	// Setup beetle legs to swing alternately
	blFrontLeft.increasing = true;
	blMiddleLeft.increasing = true;
	blFrontRight.increasing = true;
	blMiddleRight.increasing = true;

	bowl.angle = 90;
}
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		if (!charCamEnabled) {
			cam.y++;
		}
		break;
	case 's':
		if (!charCamEnabled) {
			cam.y--;
		}
		break;
	case 'a':
		charCamEnabled = !charCamEnabled;
		break;
	}
	glutPostRedisplay();
}

bool camWithinDoorway() {
	return (cam.x > 10 && cam.x < 50 && cam.z > -115 && cam.z < -85);
}
bool camWithinWalls() {
	return (cam.x > 25 && cam.x < 175 && cam.z > -175 && cam.z < -25) &&
		!(cam.x > 50 && cam.x < 150 && cam.z > -150 && cam.z < -50);
}
bool camWithinDoorwayWalls() {
	return cam.x > 10 && cam.x < 50 &&
		((cam.z > -120 && cam.z < -110 || cam.z > -90 && cam.z < -80) ||
			(cam.z > -110 && cam.z < -90) && (!door.open || door.closing));
}

void changeCamX(float amount) {
	cam.x += amount;
	if ((camWithinWalls() && !(camWithinDoorway())) || camWithinDoorwayWalls()) {
		cam.x -= amount;
	}
}
void changeCamZ(float amount) {
	cam.z += amount;
	if ((camWithinWalls() && !(camWithinDoorway())) || camWithinDoorwayWalls()) {
		cam.z -= amount;
	}
}

void special(int key, int x, int y) {
	const float CHANGE_VIEW_ANGLE = 2.0;
	const float MOVE_DISTANCE = 2.0;

	switch (key) {
	case GLUT_KEY_LEFT:
		if (!charCamEnabled) {
			cam.angle -= CHANGE_VIEW_ANGLE * TO_RAD;
		}
		break;
	case GLUT_KEY_RIGHT:
		if (!charCamEnabled) {
			cam.angle += CHANGE_VIEW_ANGLE * TO_RAD;
		}
		break;
	case GLUT_KEY_UP:
		if (!charCamEnabled) {
			changeCamX(MOVE_DISTANCE * cos(cam.angle));
			changeCamZ(MOVE_DISTANCE * sin(cam.angle));
		}
		break;
	case GLUT_KEY_DOWN:
		if (!charCamEnabled) {
			changeCamX(-MOVE_DISTANCE * cos(cam.angle));
			changeCamZ(-MOVE_DISTANCE * sin(cam.angle));
		}
		break;
	
	}

	glutPostRedisplay();
}

void moveDoors() {
	const float DETECT_X_LOWER = -20, DETECT_X_UPPER = 60;
	const float DETECT_Z_LOWER = -120, DETECT_Z_UPPER = -80;
	const float OPEN_ANGLE = 80, CLOSED_ANGLE = 0;

	if (door.opening) {
		door.angle += 1;
		if (door.angle >= OPEN_ANGLE) {
			door.opening = false;
			door.open = true;
		}
	}
	else if (door.closing) {
		door.angle -= 1;
		if (door.angle <= CLOSED_ANGLE) {
			door.closing = false;
			door.open = false;
		}
	}
	else if (cam.x > DETECT_X_LOWER && cam.x < DETECT_X_UPPER && cam.z > DETECT_Z_LOWER && cam.z < DETECT_Z_UPPER) {
		if (!door.open) {
			door.opening = true;
		}
	}
	else if (door.open) {
		door.closing = true;
	}
};
void moveCamelLegs() {
	const float LEG_ANGLE_LOWER = -14, LEG_ANGLE_UPPER = 14;
	const float LEG_ANGLE_CHANGE = 2;

	Leg* legs[4] = { &clFrontLeft, &clFrontRight, &clBackLeft, &clBackRight };
	Leg* leg;

	for (int i = 0; i < 4; i++) {
		leg = legs[i];
		if (leg->increasing) {
			if (leg->angle < LEG_ANGLE_UPPER) {
				leg->angle += LEG_ANGLE_CHANGE;
			}
			else {
				
				leg->increasing = false;
			}
		}
		else {
			if (leg->angle > LEG_ANGLE_LOWER) {
				leg->angle -= LEG_ANGLE_CHANGE;
			}
			else {
				leg->increasing = true;
			}
		}
	}
}
void moveCamel() {
	const float CAMEL_MOVE = 0.5;

	static float time;
	float x_change, z_change, angle;

	time += 0.01;

	x_change = sin(time);
	z_change = cos(2 * time);
	angle = atan_degrees_360(x_change, z_change);

	camel.x += CAMEL_MOVE * x_change;
	camel.z += CAMEL_MOVE * z_change;
	camel.angle = angle;
}
void moveBowl() {
	const float ANGLE_INC = 5;
	bowl.angle += ANGLE_INC;
}
void moveBeetleLegs() {
	const float LEG_ANGLE_LOWER = -14, LEG_ANGLE_UPPER = 14;
	const float LEG_ANGLE_CHANGE = 4;

	Leg* legs[6] = { &blFrontLeft, &blFrontRight, &blMiddleLeft, &blMiddleRight, &blBackLeft, &blBackRight };
	Leg* leg;

	for (int i = 0; i < 6; i++) {
		leg = legs[i];
		if (leg->increasing) {
			if (leg->angle < LEG_ANGLE_UPPER) {
				leg->angle += LEG_ANGLE_CHANGE;
			}
			else {
				leg->increasing = false;
			}
		}
		else {
			if (leg->angle > LEG_ANGLE_LOWER) {
				leg->angle -= LEG_ANGLE_CHANGE;
			}
			else {
				leg->increasing = true;
			}
		}
	}
}
void moveBeetle() {
	const float BEETLE_MOVE = 0.5;
	const float ANGLE_INC = 2;

	beetle.angle += ANGLE_INC;

	float angleInRads = beetle.angle * TO_RAD;
	beetle.x += BEETLE_MOVE * sin(angleInRads);
	beetle.z += BEETLE_MOVE * cos(angleInRads);
}


void timer(int value) {
	moveDoors();
	moveCamel();
	moveCamelLegs();
	moveBowl();
	moveBeetle();
	moveBeetleLegs();
	glutPostRedisplay();
	glutTimerFunc(25, timer, 0);
}
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1200, 720);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Egypt");
	initialize();

	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(25, timer, 0);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
