//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"

//Pour avoir le monde
#include "world.h"

//Pour avoir l'avatr
#include "avatar.h"

#include "my_physics.h"

//Variable globale
NYWorld * g_world;

NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;

bool g_lockCursor = false;

//Time of day
float g_dayDuration = 2.0f;
NYVert3Df g_sunPosition = NYVert3Df(10.0f, 0.f, 0.f);

//Soleil
NYVert3Df g_sun_dir;
NYColor g_sun_color;
float g_mn_lever = 6.0f * 60.0f;
float g_mn_coucher = 19.0f * 60.0f;
float g_tweak_time = 0.f;
bool g_fast_time = false;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;

//Avatar
NYAvatar * g_Avatar = NULL;

GLuint g_program;
GLuint g_programWaves;

//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);

	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if (g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;
	}

	//Rendu
	g_renderer->render(elapsed);

	//Update avatar
	g_Avatar->update(elapsed);
}


void render2d(void)
{
	g_screen_manager->render();
}

bool getSunDirection(NYVert3Df & sun, float mnLever, float mnCoucher)
{
	bool nuit = false;

	SYSTEMTIME t;
	GetLocalTime(&t);

	//On borne le tweak time à une journée (cyclique)
	while (g_tweak_time > 24 * 60)
		g_tweak_time -= 24 * 60;

	//Temps écoulé depuis le début de la journée
	float fTime = (float)(t.wHour * 60 + t.wMinute);
	fTime += g_tweak_time;
	while (fTime > 24 * 60)
		fTime -= 24 * 60;

	//Si c'est la nuit
	if (fTime < mnLever || fTime > mnCoucher)
	{
		nuit = true;
		if (fTime < mnLever)
			fTime += 24 * 60;
		fTime -= mnCoucher;
		fTime /= (mnLever + 24 * 60 - mnCoucher);
		fTime *= M_PI;
	}
	else
	{
		//c'est le jour
		nuit = false;
		fTime -= mnLever;
		fTime /= (mnCoucher - mnLever);
		fTime *= M_PI;
	}

	//Position en fonction de la progression dans la journée
	sun.X = cos(fTime);
	sun.Y = 0.2f;
	sun.Z = sin(fTime);
	sun.normalize();

	return nuit;
}

void setLightsBasedOnDayTime(void)
{
	//On active la light 0
	glEnable(GL_LIGHT0);

	//On recup la direciton du soleil
	bool nuit = getSunDirection(g_sun_dir, g_mn_lever, g_mn_coucher);

	//On définit une lumière directionelle (un soleil)
	float position[4] = { g_sun_dir.X, g_sun_dir.Y, g_sun_dir.Z, 0 }; ///w = 0 donc c'est une position a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	//Pendant la journée
	if (!nuit)
	{
		//On definit la couleur
		NYColor sunColor(1, 1, 0.8, 1);
		NYColor skyColor(0, 181.f / 255.f, 221.f / 255.f, 1);
		NYColor downColor(0.9, 0.5, 0.1, 1);
		sunColor = sunColor.interpolate(downColor, (abs(g_sun_dir.X)));
		skyColor = skyColor.interpolate(downColor, (abs(g_sun_dir.X)));

		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R, sunColor.V, sunColor.B, 1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R, sunColor.V, sunColor.B, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
	else
	{
		//La nuit : lune blanche et ciel noir
		NYColor sunColor(1, 1, 1, 1);
		NYColor skyColor(0, 0, 0, 1);
		g_renderer->setBackgroundColor(skyColor);

		float color[4] = { sunColor.R / 3.f, sunColor.V / 3.f, sunColor.B / 3.f, 1 };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
		float color2[4] = { sunColor.R / 7.f, sunColor.V / 7.f, sunColor.B / 7.f, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
		g_sun_color = sunColor;
	}
}

void renderObjects(void)
{

	//Speculaire pour toutes les faces
	GLfloat whiteSpecularMaterial[] = { 0.3, 0.3, 0.3, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterial);
	GLfloat mShininess = 100;
	glMaterialf(GL_FRONT, GL_SHININESS, mShininess);

	//RENDER THE SUN
	////Time of the day
	//NYColor colorDay (0, 181.f / 255.f, 221.f / 255.f, 1);
	//NYColor colorNight(0.2f, 0.1f, 0.1f, 1);
	//NYColor currentColor = colorNight.interpolate(colorDay, cos(NYRenderer::_DeltaTimeCumul / g_dayDuration) *0.5f + 0.5f);
	//g_renderer->setBackgroundColor(currentColor);
	//
	//
	////draw cube
	//glPushMatrix();
	////Diffuse
	//GLfloat materialDiffuseSun[] = { currentColor.R, currentColor.V, currentColor.B, 1.0 };
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseSun);
	////Ambient
	//GLfloat materialAmbientSun[] = { currentColor.R, currentColor.V, currentColor.B, 1.0 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientSun);
	////Emissive
	//GLfloat emissive[] = { 1.0, 0.0, 0.0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
	//glColor4d(currentColor.R, currentColor.B, currentColor.V, currentColor.A);
	//glRotated((NYRenderer::_DeltaTimeCumul / g_dayDuration / (2.f * M_PI))*360.0f - 90.0f, 0.0, 1.0, 0.0f);
	//glTranslated(g_sunPosition.X, g_sunPosition.Y, g_sunPosition.Z);
	//glutSolidSphere(1, 20, 20);
	//GLfloat emissiveNeutral[] = { 0.0, 0.0, 0.0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_EMISSION, emissiveNeutral);
	//glPopMatrix();


	//Rendu des axes
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	glColor3d(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(10000, 0, 0);
	glColor3d(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 10000, 0);
	glColor3d(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 10000);
	glEnd();

	//glEnable(GL_LIGHTING);
	//glShadeModel(GL_SMOOTH);
	//glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
	//glTranslatef(sqrt(8.0f), 0.0f, 0.0f);
	//glPushMatrix();

	//Active la lumière
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	//Rendu du soleil

	//On sauve la matrice
	glPushMatrix();

	//Position du soleil
	glTranslatef(g_renderer->_Camera->_Position.X, g_renderer->_Camera->_Position.Y, g_renderer->_Camera->_Position.Z);
	glTranslatef(g_sun_dir.X * 1000, g_sun_dir.Y * 1000, g_sun_dir.Z * 1000);

	//Material du soleil : de l'emissive
	GLfloat sunEmissionMaterial[] = { 0.0, 0.0, 0.0, 1.0 };
	sunEmissionMaterial[0] = g_sun_color.R;
	sunEmissionMaterial[1] = g_sun_color.V;
	sunEmissionMaterial[2] = g_sun_color.B;
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);

	//On dessine un cube pour le soleil
	glutSolidCube(50.0f);

	//On reset le material emissive pour la suite
	sunEmissionMaterial[0] = 0.0f;
	sunEmissionMaterial[1] = 0.0f;
	sunEmissionMaterial[2] = 0.0f;
	glMaterialfv(GL_FRONT, GL_EMISSION, sunEmissionMaterial);

	//Reset de la matrice
	glPopMatrix();



	//RENDER THE WORLD
	//Au lieu de rendre notre cube dans sa sphère (mais on laisse le soleil)

	glUseProgram(g_program);

	GLuint elap = glGetUniformLocation(g_program, "elapsed");
	glUniform1f(elap, NYRenderer::_DeltaTimeCumul);

	GLuint amb = glGetUniformLocation(g_program, "ambientLevel");
	glUniform1f(amb, 0.4);

	GLuint invView = glGetUniformLocation(g_program, "invertView");
	glUniformMatrix4fv(invView, 1, true, g_renderer->_Camera->_InvertViewMatrix.Mat.t);

	glPushMatrix();
	//g_world->render_world_old_school();
	g_world->render_world_vbo();
	glPopMatrix();

	glUseProgram(0);

	//RENDER THE AVATR
	g_Avatar->render();

	////Rotation du cube
	//glRotatef(NYRenderer::_DeltaTimeCumul * 100,
	//	g_slider->Value*10.0f, 1, cos(NYRenderer::_DeltaTimeCumul));
	////Rendu du cube
	//glBegin(GL_TRIANGLES);
	////TOP DOWN
	////Diffuse
	//GLfloat materialDiffuseTopDown[] = { 0.0, 0.7, 0.0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseTopDown);
	////Ambient
	//GLfloat materialAmbientTopDown[] = { 0, 0.2, 0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientTopDown);
	////Bottom
	//glColor4d(0, 0, 0.8, 1);
	//glNormal3f(0.0, 0.0, -1.0);//Normal
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(-0.5, 0.5, -0.5);
	//glVertex3d(0.5, 0.5, -0.5);
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(-0.5, -0.5, -0.5);
	//glVertex3d(-0.5, 0.5, -0.5);
	////Top
	//glNormal3f(0.0, 0.0, 1.0);//Normal
	//glVertex3d(0.5, 0.5, 0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	//glVertex3d(0.5, -0.5, 0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	//glVertex3d(-0.5, -0.5, 0.5);
	//glVertex3d(0.5, -0.5, 0.5);
	////FRONT BACK
	////Diffuse
	//GLfloat materialDiffuseFrontBack[] = { 0.7, 0.0, 0.0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseFrontBack);
	////Ambient
	//GLfloat materialAmbientFrontBack[] = { 0.2, 0.0, 0, 1.0 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientFrontBack);
	//glColor4d(0.8, 0, 0, 1);
	////Back
	//glNormal3f(-1.0, 0.0, 0.0);//Normal
	//glVertex3d(-0.5, -0.5, -0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	//glVertex3d(-0.5, 0.5, -0.5);
	//glVertex3d(-0.5, -0.5, -0.5);
	//glVertex3d(-0.5, -0.5, 0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	////Front
	//glNormal3f(1.0, 0.0, 0.0);//Normal
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(0.5, 0.5, -0.5);
	//glVertex3d(0.5, 0.5, 0.5);
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(0.5, 0.5, 0.5);
	//glVertex3d(0.5, -0.5, 0.5);
	////LEFT RIGHT
	////Diffuse
	//GLfloat materialDiffuseLeftRight[] = { 0.0, 0.0, 0.7, 1.0 };
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseLeftRight);
	////Ambient
	//GLfloat materialAmbientLeftRight[] = { 0, 0.0, 0.2, 1.0 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientLeftRight);
	//glColor4d(0, 0.8, 0, 1);
	////Left
	//glNormal3f(0.0, -1.0, 0.0);//Normal
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(-0.5, -0.5, 0.5);
	//glVertex3d(-0.5, -0.5, -0.5);
	//glVertex3d(0.5, -0.5, -0.5);
	//glVertex3d(0.5, -0.5, 0.5);
	//glVertex3d(-0.5, -0.5, 0.5);
	////Right
	//glNormal3f(0.0, 1.0, 0.0);//Normal
	//glVertex3d(0.5, 0.5, -0.5);
	//glVertex3d(-0.5, 0.5, -0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	//glVertex3d(0.5, 0.5, -0.5);
	//glVertex3d(-0.5, 0.5, 0.5);
	//glVertex3d(0.5, 0.5, 0.5);
	//glEnd();
	//glPopMatrix();
	////Sphère blanche transparente pour bien voir le shading et le reflet du soleil
	//GLfloat whiteSpecularMaterialSphere[] = { 0.3, 0.3, 0.3, 0.8 };
	//glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecularMaterialSphere);
	//mShininess = 100;
	//glMaterialf(GL_FRONT, GL_SHININESS, mShininess);
	//GLfloat materialDiffuseSphere[] = { 0.7, 0.7, 0.7, 0.8 };
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuseSphere);
	//GLfloat materialAmbientSphere[] = { 0.2, 0.2, 0.2, 0.8 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbientSphere);
	//glutSolidSphere(2, 30, 30);

}



void setLights(void)
{
	glEnable(GL_LIGHTING);

	//On active la light 0
	glEnable(GL_LIGHT0);

	//glRotated((NYRenderer::_DeltaTimeCumul / g_dayDuration / (2 * M_PI))*360.0f - 90.0f, 0.0, 1.0, 0.0f);
	//glTranslated(g_sunPosition.X, g_sunPosition.Y, g_sunPosition.Z);

	//On définit une lumière directionelle (un soleil)
	//float direction[4] = {0,0,7,0}; ///w = 0 donc elle est a l'infini
	float direction[4] = { cos(NYRenderer::_DeltaTimeCumul / g_dayDuration), 0, -sin(NYRenderer::_DeltaTimeCumul / g_dayDuration), 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, direction);
	float color[4] = { 0.5f, 0.5f, 0.5f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	float color2[4] = { 0.3f, 0.3f, 0.3f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, color2);
	float color3[4] = { 0.3f, 0.3f, 0.3f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, color3);


}

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width, height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2)
{


	//On change de mode de camera
	if (key == GLUT_KEY_LEFT)
	{
		std::cout << "LEFT" << std::endl;
		//Rotate 
		//g_renderer->_Camera->rotate(g_slider->Value);

		//Rotate Around
		g_renderer->_Camera->rotateAround(g_slider->Value);


	}

	if (key == GLUT_KEY_RIGHT)
	{

		//Rotate
		//g_renderer->_Camera->rotate(-g_slider->Value);

		//Rotate Around
		g_renderer->_Camera->rotateAround(-g_slider->Value);
	}

	if (key == GLUT_KEY_UP)
	{
		//Rotate
		//g_renderer->_Camera->rotateUp(g_slider->Value);

		//Rotate Around
		g_renderer->_Camera->rotateUpAround(g_slider->Value);
	}

	if (key == GLUT_KEY_DOWN)
	{
		//Rotate
		//g_renderer->_Camera->rotateUp(-g_slider->Value);

		//Rotate Around
		g_renderer->_Camera->rotateUpAround(-g_slider->Value);
	}


}

void specialUpFunction(int key, int p1, int p2)
{

}

void keyboardDownFunction(unsigned char key, int p1, int p2)
{
	//Control Avatar
	//Avancer
	if (key == 'z')
	{
		g_Avatar->avance = true;
	}

	//Reculer
	if (key == 's')
	{
		g_Avatar->recule = true;
	}

	//gauche
	if (key == 'q')
	{
		g_Avatar->gauche = true;
	}

	//droite
	if (key == 'd')
	{
		g_Avatar->droite = true;
	}

	//stand
	

	if (key == VK_SPACE)
	{
		g_Avatar->Jump = true;
	}

	if (key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);
		exit(0);
	}

	if (key == 'f')
	{
		if (!g_fullscreen){
			glutFullScreen();
			g_fullscreen = true;
		}
		else if (g_fullscreen){
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0, 0);
			g_fullscreen = false;
		}
	}

	if (key == 'c')
	{
		g_lockCursor = !g_lockCursor;

	}
}

void keyboardUpFunction(unsigned char key, int p1, int p2)
{
	if (key == 'z')
	{
		g_Avatar->avance = false;
	}

	//Reculer
	if (key == 's')
	{
		g_Avatar->recule = false;
	}

	//gauche
	if (key == 'q')
	{
		g_Avatar->gauche = false;
	}

	//droite
	if (key == 'd')
	{
		g_Avatar->droite = false;
	}

	//stand
	if (key == 'x')
	{
		g_Avatar->Standing = false;
	}

	if (key == VK_SPACE)
	{
		g_Avatar->Jump = false;
	}
}

void mouseWheelFunction(int wheel, int dir, int x, int y)
{

	float speed = 20.0f;
	if (wheel == 3)//up
	{
		g_renderer->_Camera->move(NYVert3Df(0, 0, speed*NYRenderer::_DeltaTime));
	}

	if (wheel == 4)//Down
	{
		g_renderer->_Camera->move(NYVert3Df(0, 0, -speed*NYRenderer::_DeltaTime));
	}

}

void mouseFunction(int button, int state, int x, int y)
{
	//Gestion de la roulette de la souris
	if ((button & 0x07) == 3 && state)
		mouseWheelFunction(button, 1, x, y);
	if ((button & 0x07) == 4 && state)
		mouseWheelFunction(button, -1, x, y);

	//GUI
	g_mouse_btn_gui_state = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;

	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		g_Avatar->picking = true;
	}
	else
	{
		g_Avatar->picking = false;
	}
}

void mouseMoveFunction(int x, int y, bool pressed)
{

	


	bool mouseTraite = false;

	/*if (g_lockCursor)
		glutWarpPointer(g_renderer->_ScreenWidth / 2.0f, g_renderer->_ScreenHeight / 2.0f);*/

	static int PreviousX = 0;
	static int PreviousY = 0;

	static int lastx = -1;
	static int lasty = -1;

	if (!pressed)
	{
		lastx = x;
		lasty = y;

		float offsetX = x - PreviousX;
		float offsetY = y - PreviousY;


		bool ThirdPerson = false;
		float sensibility = 0.2f;
		if (ThirdPerson)
		{
			g_renderer->_Camera->rotateAround(offsetX*NYRenderer::_DeltaTime*sensibility);
			g_renderer->_Camera->rotateUpAround(offsetY*NYRenderer::_DeltaTime*sensibility);
		}
		else
		{
			g_renderer->_Camera->rotate(-offsetX*NYRenderer::_DeltaTime * sensibility);
			g_renderer->_Camera->rotateUp(-offsetY*NYRenderer::_DeltaTime*sensibility);
		}
	}
	/*else
	{

		if (lastx == -1 && lasty == -1)
		{
			lastx = x;
			lasty = y;
		}

		int dx = x - lastx;
		int dy = y - lasty;

		lastx = x;
		lasty = y;


		NYVert3Df strafe = g_renderer->_Camera->_NormVec;
		strafe.Z = 0;
		strafe.normalize();
		strafe *= (float)-dx / 50.0f;

		NYVert3Df avance = g_renderer->_Camera->_Direction;
		avance.Z = 0;
		avance.normalize();
		avance *= (float)dy / 50.0f;

		g_renderer->_Camera->move(avance + strafe);

	}*/


	PreviousX = x;
	PreviousY = y;


	mouseTraite = g_screen_manager->mouseCallback(x, y, g_mouse_btn_gui_state, 0, 0);
	if (pressed && mouseTraite)
	{
		//Mise a jour des variables liées aux sliders

	}

}

void mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x, y, true);
}
void mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x, y, false);
}


void clickBtnParams(GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam(GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{
	LogConsole::createInstance();

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv);
	glutInitContextVersion(3, 0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());
	bool gameMode = true;
	for (int i = 0; i < argc; i++)
	{
		if (argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO, "Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if (gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);

		char gameModeStr[200];
		sprintf(gameModeStr, "%dx%d:32@60", width, height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width, screen_height);
	}

	if (g_main_window_id < 1)
	{
		Log::log(Log::ENGINE_ERROR, "Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}

	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR, ("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s", glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacités du système
	Log::log(Log::ENGINE_INFO, ("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

	glutDisplayFunc(update);
	glutReshapeFunc(resizeFunction);
	glutKeyboardFunc(keyboardDownFunction);
	glutKeyboardUpFunc(keyboardUpFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(mouseMoveActiveFunction);
	glutPassiveMotionFunc(mouseMovePassiveFunction);
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRender2DFun(render2d);
	//g_renderer->setLightsFun(setLights);
	g_renderer->setLightsFun(setLightsBasedOnDayTime);
	NYColor skyColor(0.f, 181.f / 255.f, 221.f / 255.f, 0.f);
	//g_renderer->setBackgroundColor(skyColor);
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise(true);

	//Creation d'un programme de shader, avec vertex et fragment shaders
	g_program = g_renderer->createProgram("shaders/psbase.glsl", "shaders/vsbase.glsl");
	//g_programWaves = g_renderer->createProgram("shaders/psbase.glsl", "shaders/vsbase.glsl");

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);

	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen();

	g_screen_manager = new GUIScreenManager();

	//Bouton pour afficher les params
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick(clickBtnParams);
	g_screen_jeu->addElement(BtnParams);

	y += BtnParams->Height + 1;

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	//Ecran de parametrage
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y += 10;
	x += 10;

	GUILabel * label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider = new GUISlider();
	g_slider->setPos(x, y);
	g_slider->setMaxMin(1, 0);
	g_slider->Visible = true;
	g_slider->Value = 0.2;
	g_screen_params->addElement(g_slider);

	y += g_slider->Height + 1;
	y += 10;

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);

	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(50, 50, 50));
	g_renderer->_Camera->setLookAt(NYVert3Df(0, 0, 0));


	//Fin init moteur

	//Init application
	//A la fin du main, on genere un monde
	g_world = new NYWorld();
	g_world->_FacteurGeneration = 1;
	g_world->init_world();


	//Init Timer
	g_timer = new NYTimer();

	//Init Avatar
	g_Avatar = new NYAvatar(g_renderer->_Camera, g_world);

	//On start
	g_timer->start();

	

	glutMainLoop();

	return 0;
}

