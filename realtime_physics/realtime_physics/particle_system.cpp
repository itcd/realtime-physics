#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GL/freeglut.h>

// OpenGL Mathematics (GLM) http://www.g-truc.net/project-0016.html
#include "../glm/glm/ext.hpp"
//// OpenGL Image (GLI) http://www.g-truc.net/project-0024.html
//#include "../gli/gli/gtx/gl_texture2d.hpp"

// NVIDIA Widgets
#include "../nvWidgets/nvWidgets_include.h"

#include "tga.h"
bool button_bounding_sphere = true;
bool button_AABB = false;
bool button_gravity = false;
#include "physics.h"

vector<Particle> particles;
vector<Plane> planes;
vector<RigidBody> rigidBodies;

nv::GlutExamine manipulator;
nv::GlutUIContext ui;
bool button_rotate = false;
bool button_particle_texture = false;
bool button_particle = false;
bool button_rigid_bodies = true;
bool button_add_rigid_body = false;

int width = 700, height = 700;
GLuint texture1 = 1, texture2 = 2;
int frame = 0, timebase = 0, fps = 0;

/// update button states for ui widgets
inline void updateButtonState( const nv::ButtonState &bs, nv::GlutManipulator &manip, int button) {
	int modMask = 0;

	if (bs.state & nv::ButtonFlags_Alt) modMask |= GLUT_ACTIVE_ALT;
	if (bs.state & nv::ButtonFlags_Shift) modMask |= GLUT_ACTIVE_SHIFT;
	if (bs.state & nv::ButtonFlags_Ctrl) modMask |= GLUT_ACTIVE_CTRL;
	if (bs.state & nv::ButtonFlags_End)
		manip.mouse( button, GLUT_UP, modMask, bs.cursor.x, height - bs.cursor.y);
	if (bs.state & nv::ButtonFlags_Begin)
		manip.mouse( button, GLUT_DOWN, modMask, bs.cursor.x, height - bs.cursor.y);
}

// Do UI.
void doUI() {
	nv::Rect null;
	ui.begin();

	ui.beginGroup();

	ui.beginGroup( nv::GroupFlags_GrowRightFromTop);
	ui.doCheckButton(null, "rotate", &button_rotate);
	ui.doCheckButton(null, "particles", &button_particle);
	ui.doCheckButton(null, "particle texture", &button_particle_texture);
	ui.doCheckButton(null, "show RB", &button_rigid_bodies);
	ui.doButton(null, "add RB", &button_add_rigid_body);
	ui.doCheckButton(null, "sphere", &button_bounding_sphere);
	ui.doCheckButton(null, "AABB", &button_AABB);
	ui.doCheckButton(null, "gravity", &button_gravity);
	ui.endGroup();

	ui.beginGroup();
	char s[256];
	sprintf(s, "%d particles    %d frames per second", particles.size(), fps);
	ui.doLabel(null, s);
	ui.endGroup();

	ui.endGroup();

	// Pass non-ui mouse events to the manipulator
	if (!ui.isOnFocus()) {
		const nv::ButtonState &lbState = ui.getMouseState( 0);
		const nv::ButtonState &mbState = ui.getMouseState( 1);
		const nv::ButtonState &rbState =  ui.getMouseState( 2);
		manipulator.motion( ui.getCursorX(), height - ui.getCursorY());
		updateButtonState( lbState, manipulator, GLUT_LEFT_BUTTON);
		updateButtonState( mbState, manipulator, GLUT_MIDDLE_BUTTON);
		updateButtonState( rbState, manipulator, GLUT_RIGHT_BUTTON);
	}

	ui.end();
}

inline void draw_cube()
{
	glBegin(GL_QUADS);

	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad

	// Back Face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad

	// Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad

	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad

	// Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad

	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad

	glEnd();
}

inline void draw_partial_cube()
{
	glBegin(GL_QUADS);

	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad

	//// Back Face
	//glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	//glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	//glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	//glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad

	// Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad

	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad

	// Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad

	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad

	glEnd();
}

void add_planes()
{
	// normals are pointing inside (the legal side of the barrier)
	// front
	planes.push_back(Plane(glm::vec3(0, 0, -1), glm::vec3(-1, -1, 1)));
	// top
	planes.push_back(Plane(glm::vec3(0, -1, 0), glm::vec3(-1, 1, -1)));
	// bottom
	planes.push_back(Plane(glm::vec3(0, 1, 0), glm::vec3(-1, -1, -1)));
	// left
	planes.push_back(Plane(glm::vec3(-1, 0, 0), glm::vec3(1, -1, -1)));
	// right
	planes.push_back(Plane(glm::vec3(1, 0, 0), glm::vec3(-1, -1, -1)));
}

Particle create_particle(float theta = 0)
{
	float mass = 1;
	glm::vec3 pos(0, 0, 0);
	glm::vec3 v(4 * cos(theta), 4 * sin(theta), cos(theta) + sin(theta));
	return Particle(mass, pos, v);
}

void add_particles()
{
	for (int i = 0; i < 10; i++)
	{
		particles.push_back(create_particle(glutGet(GLUT_ELAPSED_TIME) / 1000.f));
	}
}

void update_particles()
{
	if (button_particle)
	{
		add_particles();
	}
	for (int i=0; i<(int)particles.size(); i++)
	{
		particles[i].update();
		if (particles[i].is_alive())
		{
			for (auto j=planes.begin(); j!=planes.end(); j++)
			{
				(*j).collide(particles[i]);
			}
		}
		else
		{
			int last = particles.size() - 1;
			if (i < last)
			{
				particles[i] = particles[last];
			}
			particles.pop_back();
			i--;
		}
	}
}

void add_rigid_body()
{
	glm::quat orientation;
	std::shared_ptr<Geometry> cube(new Cube());
	std::shared_ptr<Geometry> octahedron(new Octahedron());

	auto p = 0.5f * random_vec3();
	auto v = 2.f * random_vec3();
	p.z = v.z = 0;
	rigidBodies.push_back(RigidBody(p, v, orientation, 2.f * random_vec3(), octahedron));
	p = 0.5f * random_vec3();
	v = 2.f * random_vec3();
	p.z = v.z = 0;
	rigidBodies.push_back(RigidBody(p, v, orientation, 2.f * random_vec3(), cube));
}

void update_rigid_bodies()
{
	for (auto i=rigidBodies.begin(); i!=rigidBodies.end(); i++)
	{
		(*i).update();
	}
	for (int i=0; i<(int)rigidBodies.size(); i++)
	{
		if (!rigidBodies[i].is_alive())
		{
			int last = rigidBodies.size() - 1;
			if (i < last)
			{
				rigidBodies[i] = rigidBodies[last];
			}
			rigidBodies.pop_back();
			i--;
		}
	}
	for (int i=0; i<(int)rigidBodies.size(); i++)
	{
		for (int j=i+1; j<(int)rigidBodies.size(); j++)
		{
			//rigidBodies[i].collide(rigidBodies[j]);
			GJK_collide(rigidBodies[i], rigidBodies[j]);
		}
	}
	for (auto i=rigidBodies.begin(); i!=rigidBodies.end(); i++)
	{
		for (auto j=planes.begin(); j!=planes.end(); j++)
		{
			//(*j).collide(*i);
			rb_plane_response(*i, *j);
		}
	}
}

void draw_rigid_bodies()
{
	for (auto i=rigidBodies.begin(); i!=rigidBodies.end(); i++)
	{
		(*i).draw();
	}
}

void draw_particles()
{
	for (auto i = particles.begin(); i != particles.end(); i++)
	{
		glPushMatrix();
		auto p = (*i).position;
		glTranslated(p.x, p.y, p.z);
		float c = (*i).age + 0.25f;
		c = c > 1 ? 1 : c;
		glColor3d((*i).age, c, 1);
		glutSolidSphere((*i).radius, 8, 8);
		glPopMatrix();
	}
}

void draw_particles_texture()
{
	//
	// Set up for blending...
	//

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	//
	// Set up the OpenGL state machine for using point sprites...
	//

	// This is how will our point sprite's size will be modified by 
	// distance from the viewer
	float quadratic[] =  { 1.0f, 0.0f, 0.01f };
	glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );

	// Query for the max point size supported by the hardware
	float maxSize = 0.0f;
	glGetFloatv( GL_POINT_SIZE_MAX, &maxSize );

	// Clamp size to 100.0f or the sprites could get a little too big on some  
	// of the newer graphic cards. My ATI card at home supports a max point 
	// size of 1024.0f!
	if( maxSize > 10.0f )
		maxSize = 10.0f;

	glPointSize( maxSize );

	// The alpha of a point is calculated to allow the fading of points 
	// instead of shrinking them past a defined threshold size. The threshold 
	// is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
	// the minimum and maximum point sizes.
	glPointParameterf( GL_POINT_FADE_THRESHOLD_SIZE, 60.0f );

	glPointParameterf( GL_POINT_SIZE_MIN, 1.0f );
	glPointParameterf( GL_POINT_SIZE_MAX, maxSize );

	// Specify point sprite texture coordinate replacement mode for each 
	// texture unit
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE_ARB, GL_TRUE );

	//
	// Render point sprites...
	//

	glEnable( GL_POINT_SPRITE );
	glBegin(GL_POINTS);
	for (auto i = particles.begin(); i != particles.end(); i++)
	{
		auto p = (*i).position;
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();

	glDisable(GL_POINT_SPRITE);
	glDisable( GL_BLEND );
}

void frames_per_second()
{
	frame++;
	int time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		fps = (int)(frame * 1000.0 / (time - timebase));
		timebase = time;
		frame = 0;
	}
}

/* GLUT callback Handlers */
void display(void)
{
	frames_per_second();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	manipulator.applyTransform();

	glColor3d(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glPushMatrix();
	draw_partial_cube();
	glPopMatrix();

	glEnable(GL_LIGHTING);

	update_particles();
	glPushMatrix();
	if (button_particle_texture)
	{
		glColor3d(1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture2);
		draw_particles_texture();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		draw_particles();
	}
	glPopMatrix();

	if (button_add_rigid_body)
	{
		button_add_rigid_body = false;
		add_rigid_body();
	}

	if (button_rigid_bodies)
	{
		glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glColor3d(1, 1, 1);
		update_rigid_bodies();
		draw_rigid_bodies();
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	doUI();
	glutSwapBuffers();
}

void idle()
{
	if (button_rotate) {
		manipulator.idle();
	}
	glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	ui.keyboard(k, x, y);

	switch (k)
	{
	case 27:
	case 'q':
		exit(0);
		break;
	}
}

void special(int key, int x, int y)
{
	ui.specialKeyboard(key, x, y);
}

void resize(int w, int h)
{
	if (h == 0) h = 1;
	ui.reshape( w, h);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	manipulator.reshape(w, h);
	width = w;
	height = h;
}

void mouse(int button, int state, int x, int y)
{
	ui.mouse( button, state, glutGetModifiers(), x, y);
}

void motion(int x, int y)
{
	ui.mouseMotion( x, y);
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void init()                              // All Setup For OpenGL Goes Here
{
	srand((unsigned int)time(0));
	add_planes();
	//add_rigid_body();

	glewInit();

	if (!loadTGA ("ogl.tga", texture1))
		printf ("ogl.tga not found!\n");
	if (!loadTGA ("particle.tga", texture2))
		printf ("particle.tga not found!\n");

	glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);                        // Enable Smooth Shading
	glClearColor(0, 0, 0, 0);                   // Black Background
	glClearDepth(1.0f);                         // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);          // Really Nice Perspective Calculations

	// setup lighting
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}

/* Program entry point */
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(width,height);
	int x = glutGet(GLUT_SCREEN_WIDTH) - width;
	int y = glutGet(GLUT_SCREEN_HEIGHT) - height;
	if (x > 0 && y > 0)
	{
		glutInitWindowPosition(x/2, y/2);
	}
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("particle system");

	init();

	// configure a simple controller
	manipulator.setDollyActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_CTRL);
	manipulator.setPanActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_SHIFT);
	manipulator.setDollyPosition(-3);

	// setup event callback functions
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutIdleFunc(idle);
	glutKeyboardFunc(key);
	glutReshapeFunc(resize);

	printf("[q] Exit the application.\n\n");
	printf("Use the mouse to manipulate camera:\n");
	printf("Left mouse button to rotate around the object.\n");
	printf("Left mouse button + Ctrl to dolly.\n");
	printf("Left mouse button + Shift to pan.\n\n");

	glutMainLoop();

	return EXIT_SUCCESS;
}
