#include "glSetup.h"

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <iostream>
#include <fstream>
using namespace std;

void init();
void quit();
void render(GLFWwindow* window);
void keyboard(GLFWwindow* window, int key, int code, int action, int mods);

bool readMesh(const char* filename);
void deleteMesh();

//Camera

vec3 eyeTopView(0, 10, 0); //Top View
vec3 upTopView(0, 0, -1);

vec3 eyeFrontView(0, 0, 10); //Front View
vec3 upFrontView(0, 1, 0);

vec3 eyeBirdView(0, -10, 4); //Bird eye View
vec3 upBirdView(0, 1, 0);
vec3 center(0, 0, 0);

//Light configuration
vec4 lightInitialP(0.0, 0.0, 0.8, 1); // Initial light position

//Global coordinate frame
bool axes = true;

float AXIS_LENGTH = 1.25;
float AXIS_LINE_WIDTH = 2;

//Colors

GLfloat bgColor[4] = { 1,1,1,1, };

// control variable

int view = 2;
int selection = 1;

bool rotationObject = false;
bool rotationLight = false;

float thetaModel = 0;
float thetaLight[3];

bool lightOn[3];
bool attenuation = false;

bool exponent = false;
float exponentInitial = 0.0;	//[0,128]
float exponentValue = exponentInitial;
float exponentNorm = exponentValue / 128.0f; //[0,1]

bool cutoff = false;
float cutoffMax = 60;
float cutoffInitial = 30.0;
float cutoffValue = cutoffInitial;
float cutoffNorm = cutoffValue / cutoffMax;

bool shininess = false;
float shininessInitial = 10.0;
float shininessValue = shininessInitial;
float shininessNorm = shininessValue / 128.0f;

int material = 0;
// paly configuration
bool pause = true;

float timeStep = 1.0f / 120; //120fps
float period = 4.0f;
float period2 = 10.0f;
// Current frame
int frame = 0;

void
reinitialize()
{
	frame = 0;

	lightOn[0] = true; // Turn on only the point light
	lightOn[1] = false; // 
	lightOn[2] = false; // 

	thetaModel = 0;
	for (int i = 0; i < 3; i++)
		thetaLight[i] = 0;

	exponentValue = exponentInitial;
	exponentNorm = exponentValue / 128.0f;

	cutoffValue = cutoffInitial;
	cutoffNorm = cutoffValue / cutoffMax;

	shininessValue = shininessInitial;
	shininessNorm = shininessValue / 128.0f;

}

void animate()
{
	frame += 1;
	//Rotation angle of the light
	if (rotationLight)
	{
		for (int i = 0; i < 3; i++)
			if (lightOn[i]) thetaLight[i] += 4 / period; //degree
	}
	// Rotate angle the models
	if (rotationObject) thetaModel += 4 / period; //degree

	if (lightOn[2] && exponent)
	{
		exponentNorm += float(radians(4.0 / period) / M_PI);
		exponentValue = float(128.0f * (acos(cos(exponentNorm * M_PI)) / M_PI));

	}
	if (lightOn[2] && cutoff)
	{
		cutoffNorm += float(radians(4.0 / period) / M_PI);
		cutoffValue = float(cutoffMax * (acos(cos(cutoffNorm * M_PI)) / M_PI));
	}

	if (lightOn[2] && shininess)
	{
		
	}

}

int main(int argc, char* argv[])
{
	vsync = 0;

	fovy = 16.1f;

	GLFWwindow* window = initializeOpenGL(argc, argv, bgColor);
	if (window == NULL) return -1;

	// callbacks
	glfwSetKeyCallback(window, keyboard);

	//Depth test
	glEnable(GL_DEPTH_TEST);

	//normal vec after trasnformation
	glEnable(GL_NORMALIZE);

	//Viewport and perspective setting
	reshape(window, windowW, windowH);

	//Initialization - main loop - finalization

	init();

	//Main loop
	float previous = (float)glfwGetTime();
	float elapsed = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float now = (float)glfwGetTime();
		float delta = now - previous;
		previous = now;

		elapsed += delta;

		if (elapsed > timeStep)
		{
			if (!pause) animate();

			elapsed = 0;
		}
		render(window);
		glfwSwapBuffers(window);

	}
	quit();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// Sphere, cylinder

GLUquadricObj* sphere = NULL;
GLUquadricObj* cylinder = NULL;
GLUquadricObj* cone = NULL;

void init()
{
	//Anitmation system
	reinitialize();

	//Prepare quadric shapes
	sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	gluQuadricOrientation(sphere, GLU_OUTSIDE);
	gluQuadricTexture(sphere, GL_FALSE);

	cylinder = gluNewQuadric();
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluQuadricOrientation(cylinder, GLU_OUTSIDE);
	gluQuadricTexture(cylinder, GL_FALSE);

	cone = gluNewQuadric();
	gluQuadricDrawStyle(cone, GLU_FILL);
	gluQuadricNormals(cone, GLU_SMOOTH);
	gluQuadricOrientation(cone, GLU_OUTSIDE);
	gluQuadricTexture(cone, GL_FALSE);

	//Prepare mesh
	readMesh("m01_bunny.off");

	//Keyboard
	cout << endl;
	cout << "Keyboard input: space for play/pause" << endl;
	cout << "Keyboard input: i for reinitialization" << endl;
	cout << "Keyboard input: up for increasing period" << endl;
	cout << "Keyboard input: down for decreasing period" << endl;
	cout << "Keyboard input: v for the top, front, bird-eye view" << endl;
	cout << endl;
	cout << "Keyboard input: p for a point light" << endl;
	cout << "Keyboard input: d for a distant light" << endl;
	cout << "Keyboard input: s for a spot light" << endl;
	cout << "Keyboard input: a for light attenuation" << endl;
	cout << "Keyboard input: e for time dependent exponent of a spot light" << endl;
	cout << "Keyboard input: c for time dependent cutoff of a spot light" << endl;
	cout << "Keyboard input: m for predefined material parameters" << endl;
	cout << "Keyboard input: l for rotation of lights" << endl;
	cout << "Keyboard input: o for rotation of objects" << endl;
	cout << "Keyboard input: 1 for 3x3 spheres" << endl;
	cout << "Keyboard input: 2 for 3x3 flat bunnies" << endl;
	cout << "Keyboard input: 3 for 3x3 smooth bunnies" << endl;
	cout << "Keyboard input: 4 for a plane with many small squres" << endl;

}

void quit()
{
	gluDeleteQuadric(sphere);
	gluDeleteQuadric(cylinder);
	gluDeleteQuadric(cone);

	deleteMesh();
}

void setupColoredMaterial(const vec3& color)
{
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_diffuse[4] = { color[0],color[1],color[2],1.0f };
	GLfloat mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess = 100;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}
void setupWhiteShinyMaterial()
{
	GLfloat mat_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat mat_diffuse[4] = { 1.0f, 1.0f, 1.0f,1.0f };
	GLfloat mat_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_shininess = shininessValue;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}
void setupBrassMaterial()
{
	GLfloat mat_ambient[4] = { 0.33f, 0.22f, 0.33f, 1.0f };
	GLfloat mat_diffuse[4] = { 0.78f, 0.57f, 0.11f, 1.0f };
	GLfloat mat_specular[4] = { 0.99f, 0.91f, 0.81f, 1.0f };
	GLfloat mat_shininess = 27.8f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}
void setupRedPlasticMaterial()
{
	GLfloat mat_ambient[4] = { 0.3f, 0.0f, 0.0f, 1.0f };
	GLfloat mat_diffuse[4] = { 0.6f, 0.0f, 0.0f, 1.0f };
	GLfloat mat_specular[4] = { 0.8f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_shininess = 32.0f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
}

void drawSquare()
{
	glBegin(GL_QUADS);

	glNormal3f(0, 0, 1);
	glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 1);
	glVertex3f(1, 1, 1);
	glVertex3f(0, 1, 1);

	glEnd();

}

void drawSphere(float radius, int slices, int stacks)
{
	gluSphere(sphere, radius, slices, stacks);
}

void drawCylinder(float radius, float height, int slices, int stacks)
{
	gluCylinder(cylinder, radius, radius, height, slices, stacks);
}

void drawCone(float radius, float height, int slices, int stacks)
{
	gluCylinder(cone, 0, radius, height, slices, stacks);
}
//Read a mesh form a given OFF file
int nVertices = 0, nFaces = 0, nEdges = 0;
vec3* vertex = NULL;
vec3* vnormal = NULL; // Vertex normal
vec3* fnormal = NULL; // face normal
vec3* mPoints = NULL; // middle vertex
int* face[3] = { NULL, NULL, NULL };

bool
readMesh(const char* filename) {
	ifstream is(filename);
	if (is.fail()) return false;
	char magicNumber[256];
	is >> magicNumber;
	// # vertices, # faces, # edges
	is >> nVertices >> nFaces >> nEdges;
	cout << "# vertices = " << nVertices << endl;
	cout << "# faces = " << nFaces << endl;

	// Vertices
	vertex = new vec3[nVertices];
	for (int i = 0; i < nVertices; i++)
		is >> vertex[i].x >> vertex[i].y >> vertex[i].z;

	// Vertex normals
	vnormal = new glm::vec3[nVertices];
	for (int i = 0; i < nVertices; i++)
		vnormal[i] = glm::vec3(0, 0, 0);
	// Face normals
	fnormal = new glm::vec3[nFaces];

	// Faces
	face[0] = new	int[nFaces];	// Only support triangles
	face[1] = new	int[nFaces];
	face[2] = new	int[nFaces];




	int n;
	for (int i = 0; i < nFaces; i++) {
		is >> n >> face[0][i] >> face[1][i] >> face[2][i];
		if (n != 3) cout << "# vertices of the " << i << "-th faces = " << n << endl;
		//normal vector of the face
		glm::vec3 v1 = vertex[face[1][i]] - vertex[face[0][i]];
		glm::vec3 v2 = vertex[face[2][i]] - vertex[face[0][i]];
		glm::vec3 v = normalize(cross(v1, v2));

		fnormal[i] = v;

		vnormal[face[0][i]] += v;
		vnormal[face[1][i]] += v;
		vnormal[face[2][i]] += v;

	}
	for (int i = 0; i < nVertices; i++)
		vnormal[i] = normalize(vnormal[i]);


	return true;
}
void
deleteMesh()
{
	if (vertex) { delete[]	vertex; vertex = NULL; }
	if (vnormal) { delete[] vnormal; vnormal = NULL; }
	if (fnormal) { delete[] fnormal; fnormal = NULL; }
	if (face[0]) { delete[]	face[0];	face[0] = NULL; }
	if (face[1]) { delete[]	face[1];	face[1] = NULL; }
	if (face[2]) { delete[]	face[2];	face[2] = NULL; }
}

void
drawFlatMesh()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nFaces; i++)
	{
		glNormal3fv(value_ptr(fnormal[i]));
		for (int j = 0; j < 3; j++)
			glVertex3fv(value_ptr(vertex[face[j][i]]));
	}
	glEnd();

}

void
drawSmoothMesh()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nFaces; i++)
		for (int j = 0; j < 3; j++)
		{
			glNormal3fv(value_ptr(vnormal[face[j][i]]));
			glVertex3fv(value_ptr(vertex[face[j][i]]));
		}

	glEnd();
}

// Compute the rotation axis and angle from a to b

//axis is not normalized
//theta is represented in degree
//
void computeRotation(const vec3& a, const vec3& b, float& theta, vec3& axis)
{
	axis = cross(a, b);
	float sinTheta = length(axis);
	float cosTheta = dot(a, b);
	theta = float(atan2(sinTheta, cosTheta) * 180.0 / M_PI);
}

void setupLight(const vec4& p, int i) {

	GLfloat ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

	glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0 + i, GL_POSITION, value_ptr(p));

	//Attenuation for the point and spot light
	if ((i == 0 || i == 2) && attenuation)
	{
		glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.1f);
		glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.05f);
	}
	else
	{
		glLightf(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, 0.0f);
		glLightf(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, 0.0f);
	}

	if (i == 2)
	{
		vec3 spotDirection = -vec3(p);
		glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, value_ptr(spotDirection));
		glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, cutoffValue);
		glLightf(GL_LIGHT0 + i, GL_SPOT_DIRECTION, exponentValue);
		glLightf(GL_LIGHT0 + i, GL_SHININESS, shininessValue);

	}
	else
	{
		glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 180);
	}
}

void drawArrow(const vec3& p, bool tailOnly)
{
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// Common material
	GLfloat mat_specular[4] = { 1, 1, 1 ,1 };
	GLfloat mat_shininess = 25;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	glPushMatrix();

	glTranslatef(p.x, p.y, p.z);
	if (!tailOnly)
	{
		float theta;
		vec3 axis;
		computeRotation(vec3(0, 0, 1), vec3(0, 0, 0) - vec3(p), theta, axis);
		glRotatef(theta, axis.x, axis.y, axis.z);
	}

	// Tail sphere
	float arrowTailRadius = 0.05f;
	glColor3f(1, 0, 0); // ambient and diffuse
	drawSphere(arrowTailRadius, 16, 16);

	if (!tailOnly)
	{
		// Shaft cylinder
		float arrowShaftRadius = 0.02f;
		float arrowShaftLength = 0.2f;
		glColor3f(0, 1, 0);
		drawCylinder(arrowShaftRadius, arrowShaftLength, 16, 5);
		// Head	cone
		float	arrowheadHeight = 0.09f;
		float	arrowheadRadius = 0.06f;
		glTranslatef(0, 0, arrowShaftLength + arrowheadHeight);
		glRotatef(180, 1, 0, 0);
		glColor3f(0, 0, 1); // ambient and diffuse
		drawCone(arrowheadRadius, arrowheadHeight, 16, 5);

	}

	glPopMatrix();

	// For convential material setting
	glDisable(GL_COLOR_MATERIAL);
}
void
drawSpotLight(const vec3& p, float cutoff)
{
	glPushMatrix();

	glTranslatef(p.x, p.y, p.z);

	float	theta;
	vec3	axis;
	computeRotation(vec3(0, 0, 1), vec3(0, 0, 0) - vec3(p), theta, axis);
	glRotatef(theta, axis.x, axis.y, axis.z);

	// Color
	setupColoredMaterial(vec3(0, 0, 1));

	// tan(cutoff) = r/h
	float	h = 0.15f;
	float	r = h * tan(radians(cutoff));
	drawCone(r, h, 16, 5);
	// Color
	setupColoredMaterial(vec3(1, 0, 0));

	// Apex
	float apexRadius = 0.06f * (0.5f + exponentValue / 128.0f);
	drawSphere(apexRadius, 16, 16);

	glPopMatrix();

}

void
render(GLFWwindow* window)
{
	// Background color
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vec3 eye(0), up(0);
	switch (view)
	{
	case	0:	eye = eyeTopView;	up = upTopView;	break;
	case	1:	eye = eyeFrontView;	up = upFrontView;	break;
	case	2:	eye = eyeBirdView;	up = upBirdView;	break;
	}

	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], up[0], up[1], up[2]);

	//	Axes
	if (axes)
	{
		glDisable(GL_LIGHTING);
		drawAxes(AXIS_LENGTH, AXIS_LINE_WIDTH * dpiScaling);

	}

	glShadeModel(GL_SMOOTH);

	// Rotation of the light or 3x3 models
	vec3	axis(0, 1, 0);


	//	Lighting
	//
	glEnable(GL_LIGHTING);

	//	Set up the lights
	vec4	lightP[3];
	for (int i = 0; i < 3; i++)
	{
		// Just turn off the i-th light, if not lit
		if (!lightOn[i]) { glDisable(GL_LIGHT0 + i); continue; }

		// Turn on the i-th light
		glEnable(GL_LIGHT0 + i);

		// Dealing with the distant light
		lightP[i] = lightInitialP;
		if (i == 1) lightP[i].w = 0;

		// Lights rotate around the center of the world coordinate system
		mat4 R = rotate(mat4(1.0), radians(thetaLight[i]), axis);;
		lightP[i] = R * lightP[i];

		// Set up the i-th light
		setupLight(lightP[i], i);
	}
	// Draw the geometries of the lights
	for (int i = 0; i < 3; i++)
	{
		if (!lightOn[i]) continue;
		if (i == 2) drawSpotLight(lightP[i], cutoffValue);
		else	drawArrow(lightP[i], i == 0);	// Tail
		// Draw objects 
		//
		
		
		cout << "value " << shininessValue << endl;
		
		GLfloat mat_ambient[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
		GLfloat mat_diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat mat_specular[4] = { 0.99f, 0.91f, 0.81f, 1.0f };
		GLfloat mat_shininess = shininessValue;

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
		
		if (selection == 4) // Plane with many squares
		{
			float width = 0.05f;
			float height = 0.05f;
			float depth = 0.1f;
			for (float i = -2.2f; i <= 2.2f; i += width)
				for (float j = -1.3f; j <= 1.3f; j += height)
				{

					glPushMatrix();
					glTranslatef(i, j, -1.0);
					glScalef(width, height, depth);
					glTranslatef(-0.5, -0.5, -0.5);
					drawSquare();
					glPopMatrix();
				}
		}
		else {
			glScalef(0.4f, 0.4f, 0.4f);
			vec3 u[3]; // y-axis
			vec3 v[3]; // x-axis
			u[0] = vec3(0, 1, 0) * 2.0f;
			u[1] = vec3(0, 0, 0);
			u[2] = -u[0];
			v[0] = -vec3(1, 0, 0) * 4.0f;
			v[1] = vec3(0, 0, 0);
			v[2] = -v[0];
			for (int i = 0; i < 3; i++)
			{
				glPushMatrix();
				glTranslatef(u[i].x, u[i].y, u[i].z);
				for (int j = 0; j < 3; j++)
				{
					glPushMatrix();
					glTranslatef(v[j].x, v[j].y, v[j].z);
					glRotatef(thetaModel, axis.x, axis.y, axis.z);
					switch (selection)
					{
					case 1:	drawSphere(0.7f, 64, 64);	break;
					case 2:	drawFlatMesh();	break;
					case 3:	drawSmoothMesh();	break;

					}
					glPopMatrix();
				}
				glPopMatrix();
			}
		}
	}
}
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key) { // Quit 
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			// Play on/off
		case GLFW_KEY_SPACE:	pause = !pause; break;
			//axes
		case GLFW_KEY_X: axes = !axes; break;
			// Initialization 
		case GLFW_KEY_I: reinitialize(); break;

			// Period control
		case GLFW_KEY_UP:	period += 0.1f; break;
		case GLFW_KEY_DOWN:	period = (float)std::max(period - 0.1, 0.1); break;
			// Top, front, bird-eye view
		case GLFW_KEY_V: view = (view + 1) % 3; break;
			// Turn on/off the point, distant, spot light
		case GLFW_KEY_P: lightOn[0] = !lightOn[0]; break;
		case GLFW_KEY_D: lightOn[1] = !lightOn[1]; break;
		case GLFW_KEY_S: lightOn[2] = !lightOn[2]; break;
			// Light attenuation
		case GLFW_KEY_A: attenuation = !attenuation; break;
			// Light: point, direction, spot
		case GLFW_KEY_L: rotationLight = !rotationLight; break;
			// Rotation: light position/direction^ models
		case GLFW_KEY_0: rotationObject = !rotationObject; break;
			// Time dependent exponent of a spot light
		case GLFW_KEY_E: exponent = !exponent; break;
			// Time dependent cutoff of a spot light
		case GLFW_KEY_C: cutoff = !cutoff; break;
			// Predefined material parameters
		case GLFW_KEY_M: material = (material + 1) % 3; break;

			//adjust shininess coefficient
		case GLFW_KEY_N: shininess = !shininess; break;
		case GLFW_KEY_RIGHT: if(shininessValue<120) shininessValue += period2; break;
		case GLFW_KEY_LEFT:	if(shininessValue>0)shininessValue -= period2; break;

			// Example selection
		case GLFW_KEY_1: selection = 1; break;
		case GLFW_KEY_2: selection = 2; break;
		case GLFW_KEY_3: selection = 3; break;
		case GLFW_KEY_4: selection = 4; break;



		}
	}
}










