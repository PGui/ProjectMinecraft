#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h" 
#include "engine/render/text_engine.h" 
#include "engine/log/log.h"

//BACKGROUND COLOR FOND
#define ROUGE_FOND 176.0f/255.0f
#define VERT_FOND 192.0f/255.0f
#define BLEU_FOND 198.0f/255.0f

#define DEFAULT_SCREEN_WIDTH 1200
#define DEFAULT_SCREEN_HEIGHT 800

class NYRenderer 
{
	public :
		NYCamera * _Camera; ///< Gestion du point de vue
		int _ScreenWidth; ///< Largeur ecran en pixels
		int _ScreenHeight; ///< Hauteur ecran en pixels
		
		void (*_RenderObjectsFun)(void); ///< Fonction de rendu des objets uniquement (parfois fait n fois dans certains rendus)
		void (*_RenderObjectsDepthOnlyFun)(void); ///< Fonction de rendu d'objets uniquement dans le z buffer (pour certains rendus uniquement)
		void (*_Render2DFun)(void); ///< Rendu en 2d (en passe en mode camera ortho, etc...)
		void (*_SetLights)(void); ///< Choisit la position des lumieres (besoin de le faire dans le bon referentiel, après matrice view défine)
		
		//Post Process
		bool _DoPostProcess;
		GLuint _ColorTexPP; ///< Rendu ecran pour le post process
		GLuint _ColorWasteTex; ///< Buffer couleur a binder mais inutilisé par la suite (poubelle) 
		GLuint _DepthTexPP; ///< Rendu du buffer de profondeur, pour post process
		GLuint _FBO; ///< Front buffer Object : pour effectuer un render to texture
		GLuint _ProgramPP; ///< Le programme de shader pour le post process

		NYColor _BackGroundColor; ///< Couleur de fond. La modifier avec setBackgroundColor()

		NYTextEngine * _TextEngine; ///< Rendu de texte

		HWND _WHnd; ///< Handle de la fenetre principale

		static float _DeltaTime; ///< Temps écoulé depuis la dernière frame (passe a la fonction render)
		static float _DeltaTimeCumul; ///< Temps écoulé depuis le lancement de l'appli

	private : 
		static NYRenderer * _Me; ///< Singleton

		NYRenderer()
		{
			_Camera = new NYCamera();
			_Camera->setPosition(NYVert3Df(0,-190,0));
			_Camera->setLookAt(NYVert3Df(0,0,0));
			_ScreenWidth = DEFAULT_SCREEN_WIDTH;
			_ScreenHeight = DEFAULT_SCREEN_HEIGHT;
			_RenderObjectsFun = NULL;
			_RenderObjectsDepthOnlyFun = NULL;
			_Render2DFun = NULL;
			_SetLights = NULL;
			_WHnd = WindowFromDC(wglGetCurrentDC());
			_TextEngine = new NYTextEngine(wglGetCurrentDC());
			_TextEngine->buildFont(0,12);
			_TextEngine->buildFont(1,14);
			_TextEngine->SelectFont(0);
			_BackGroundColor.R = ROUGE_FOND;
			_BackGroundColor.V = VERT_FOND;
			_BackGroundColor.B = BLEU_FOND;
			_DoPostProcess = false;
		}

	public :
		
		static NYRenderer * getInstance()
		{
			if(_Me == NULL)
				_Me = new NYRenderer();
			return _Me;
		}

		/**
		  * Changement de camera (pour une sous classe par exemple)
		  */
		void setCam(NYCamera * cam)
		{
			_Camera = cam;
		}
		
		/**
		  * Initialisation du moteur
		  */
		void initialise(bool postProcess = false)
		{
			_DoPostProcess = postProcess;

			if(postProcess)
			{
				this->initFBO();
				this->initShadersPostProcess();
			}
			
			glClearColor(_BackGroundColor.R,_BackGroundColor.V,_BackGroundColor.B,_BackGroundColor.A);

			glDepthFunc(GL_LEQUAL);	
			glEnable(GL_DEPTH_TEST);

			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
			glEnable(GL_BLEND);
			
			glEnable(GL_LIGHTING);

			glDisable(GL_COLOR_MATERIAL);
			GLfloat gris[] = { 0.7F,0.7F,0.7F,1.0F };
			glMaterialfv(GL_FRONT,GL_DIFFUSE,gris);
		}

		void resize(int screen_width, int screen_height)
		{
			_ScreenWidth = screen_width;
			_ScreenHeight = screen_height;

			if(_DoPostProcess)
			{
				this->killFBO();
				this->initFBO();
			}
		}

		void setBackgroundColor(NYColor color)
		{
			_BackGroundColor = color;
		}

		void setRenderObjectFun(void (*fun)(void))
		{
			_RenderObjectsFun = fun;
		}

		void setRenderObjectDepthOnlyFun(void (*fun)(void))
		{
			_RenderObjectsDepthOnlyFun = fun;
		}

		void setRender2DFun(void (*fun)(void))
		{
			_Render2DFun = fun;
		}

		void setLightsFun(void (*fun)(void))
		{
			this->_SetLights = fun;
		}

		void render(float elapsed)
		{
			//Le temps
			_DeltaTime = elapsed;
			_DeltaTimeCumul += elapsed;

			//On efface les buffers
			glClearColor(_BackGroundColor.R,_BackGroundColor.V,_BackGroundColor.B,_BackGroundColor.A);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Si on a active le post process
			if(_DoPostProcess)
			{
				//On passe en FBO pour pouvoir faire nos effets
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _FBO);
				checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");

				//Attach 2D texture to this FBO
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorTexPP, 0);
				checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_color_tex, 0);");

				//Attach depth texture to FBO
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _DepthTexPP, 0);
				checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depth_tex, 0);");

				//On active le FBO
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _FBO);
				checkGlError("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_FBO);");
			}
			else
			{
				//On passe en mode rendu normal
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			}
			
			//On efface
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//On efface les matrices
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			//On update et définit la caméra
			_Camera->update(elapsed);
			this->viewFromCam();	

			//On place les lumieres
			if(this->_SetLights)
				this->_SetLights();
			else
			{
				//On active la light 0
				glEnable(GL_LIGHT0);

				//On définit une lumière directionelle (un soleil)
				float direction[4] = {0.5,0.5,1,0}; ///w = 0 donc elle est a l'infini
				glLightfv(GL_LIGHT0, GL_POSITION, direction );
				float color[4] = {0.5f,0.5f,0.5f};
				glLightfv(GL_LIGHT0, GL_DIFFUSE, color );
				float color2[4] = {0.3f,0.3f,0.3f};
				glLightfv(GL_LIGHT0, GL_AMBIENT, color2 );
			}
			
			glColor3f(1.f,1.f,1.f);
			glEnable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);

			//Rendu de la scène
			if(_RenderObjectsFun != NULL)
				(*_RenderObjectsFun)();

			//Si on a active le post process
			if(_DoPostProcess)
			{
				//On detache le color buffer mais on laisse le depth
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorWasteTex, 0);
				checkGlError("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ColorWasteTex, 0);");

				//Rendu des objets de profondeur seulement (fantoooomes) QUE SI POST PROCESS !
				if(_RenderObjectsDepthOnlyFun != NULL)
					(*_RenderObjectsDepthOnlyFun)();

				//On passe en mode rendu normal
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

				glDisable(GL_CULL_FACE);

				//On met le bon programme de shader pour le post process
				glUseProgram(_ProgramPP);

				//On rend un quad de la taille de l'ecran
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();	
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//on set les variables du shader
				GLuint texLoc = glGetUniformLocation(_ProgramPP, "Texture0");
				glUniform1i(texLoc, 0);

				texLoc = glGetUniformLocation(_ProgramPP, "Texture1");
				glUniform1i(texLoc, 1);

				GLuint var = glGetUniformLocation(_ProgramPP, "screen_width");
				glUniform1f(var, (float)_ScreenWidth);

				var = glGetUniformLocation(_ProgramPP, "screen_height");
				glUniform1f(var, (float)_ScreenHeight);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, _ColorTexPP);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, _DepthTexPP);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

				//Restore
				glActiveTexture(GL_TEXTURE0);

				glBegin(GL_QUADS);
				glTexCoord2f( 0, 1); glVertex3f( -1.0f,  1.0f, 0);
				glTexCoord2f( 1, 1); glVertex3f(  1.0f,  1.0f, 0);
				glTexCoord2f( 1, 0); glVertex3f(  1.0f, -1.0f, 0);
				glTexCoord2f( 0, 0); glVertex3f( -1.0f, -1.0f, 0);
				glEnd();

				//Fin des shaders
				glUseProgram(0);
			}

			//Rendu 2D (UI et autres)

			//Mode 2D
			glColor3f ( 1.0f, 1.0f, 1.0f ) ;
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			
			glMatrixMode(GL_PROJECTION); // passe en mode matrice de projection
			glLoadIdentity(); // Réinitialisation
			glOrtho(0,_ScreenWidth,_ScreenHeight,0,-1.0f,2.0f);
								
			glMatrixMode(GL_MODELVIEW); // on repasse en mode matrice de repère
			glLoadIdentity(); // Réinitialisation
			
			//On fait le rendu 2D sur l'écran
			if(_Render2DFun != NULL)
				(*_Render2DFun)();

			//Fini
			glutSwapBuffers();
			glutPostRedisplay();
		}

		//GESTION DES SHADERS
		
		/**
		  * Permet de créer un programme de shaders, a activer quand on veut
		  */
		GLuint createProgram(char * filePixelShader, char * fileVertexShader)
		{
			GLuint fs = 0;
			if(filePixelShader != NULL)
				fs = loadShader(GL_FRAGMENT_SHADER,filePixelShader);

			GLuint vs = 0;
			if(fileVertexShader != NULL)
				vs = loadShader(GL_VERTEX_SHADER,fileVertexShader);

			if(fs > 0 || vs > 0)
			{
				GLuint prog = glCreateProgram();
				if(fs > 0)
				{
					glAttachShader(prog, fs);
					checkGlError("glAttachShader(prog, fs);");	
				}

				if(vs > 0)
				{
					glAttachShader(prog, vs);
					checkGlError("glAttachShader(prog, vs);");	
				}
				glLinkProgram(prog);
				checkGlError("glLinkProgram(prog);");

				return prog;
			}

			return 0;
		}

		static void checkGlError(char * call)
		{
			GLenum error = glGetError();

			if(error != 0)
			{
				switch(error)
				{	
				case GL_INVALID_ENUM: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_INVALID_ENUM) for call " + toString(call)).c_str()); break;
				case GL_INVALID_OPERATION: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_INVALID_OPERATION) for call " + toString(call)).c_str()); break;
				case GL_STACK_OVERFLOW: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_STACK_OVERFLOW) for call " + toString(call)).c_str()); break;
				case GL_STACK_UNDERFLOW: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_STACK_UNDERFLOW) for call " + toString(call)).c_str()); break;
				case GL_OUT_OF_MEMORY: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_OUT_OF_MEMORY) for call " + toString(call)).c_str()); break;
				case GL_TABLE_TOO_LARGE: Log::log(Log::ENGINE_ERROR,("Opengl error (GL_TABLE_TOO_LARGE) for call " + toString(call)).c_str()); break;
				default : Log::log(Log::ENGINE_ERROR,("Unknown Opengl error for call " + toString(call)).c_str()); break; 
				}
			}
		}

		void viewFromCam(void)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0,((float)_ScreenWidth) / (float)_ScreenHeight, 0.5, 10000.0);
			glMatrixMode(GL_MODELVIEW);	
			glLoadIdentity();
			_Camera->look();
		}

		//Donner z = 1 pour etre au fond du buffer de profondeur
		void unProjectMousePos(int mouseX, int mouseY, float mouseZ, double * posX, double * posY, double * posZ)
		{
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLfloat winX, winY, winZ;

			//Mode rendu du monde car sinon elle sont en mode rendu du quad de post process
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			this->viewFromCam();

			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );

			winX = (float)mouseX;
			winY = (float)viewport[3] - (float)mouseY;
			winZ = mouseZ;

			gluUnProject( winX, winY, winZ, modelview, projection, viewport, posX, posY, posZ);
		}

	private:

		void initShadersPostProcess(void)
		{
			_ProgramPP = glCreateProgram();
			GLuint shader = loadShader(GL_FRAGMENT_SHADER,"shaders/postprocess/pshader.glsl");
			glAttachShader(_ProgramPP, shader);
			checkGlError("glAttachShader(_ProgramPP, shader);");
			glLinkProgram(_ProgramPP);
			checkGlError("glLinkProgram(_ProgramPP);");
		}

		void killFBO(void)
		{
			glDeleteTextures(1,&_ColorTexPP);
			glDeleteTextures(1,&_ColorWasteTex);
			glDeleteTextures(1,&_DepthTexPP);
			glDeleteFramebuffers(1,&_FBO);
		}

		//Get Depth et Frame buffer to textures
		int initFBO(void)
		{
			//RGBA8 2D texture, 24 bit depth texture, 256x256
			glGenTextures(1, &_ColorTexPP);
			glBindTexture(GL_TEXTURE_2D, _ColorTexPP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//NULL means reserve texture memory, but texels are undefined
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _ScreenWidth, _ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glGenTextures(1, &_ColorWasteTex);
			glBindTexture(GL_TEXTURE_2D, _ColorWasteTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//NULL means reserve texture memory, but texels are undefined
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _ScreenWidth, _ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glGenTextures(1, &_DepthTexPP);
			glBindTexture(GL_TEXTURE_2D, _DepthTexPP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			//NULL means reserve texture memory, but texels are undefined
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _ScreenWidth, _ScreenHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			//-------------------------
			glGenFramebuffers(1, &_FBO);
			//On bind le FBO pour tester si tout est ok
			glBindFramebufferEXT(GL_FRAMEBUFFER, _FBO);
			//Attach 2D texture to this FBO
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _ColorTexPP, 0);
			//-------------------------
			//Attach depth texture to FBO
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, _DepthTexPP, 0);
			//-------------------------
			//Does the GPU support current FBO configuration?
			GLenum status;
			status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			switch(status)
			{
				case GL_FRAMEBUFFER_COMPLETE_EXT:
					//_cprintf("GPU ok for VBO with depth and color\n"); 
					break;
				default:
					Log::log(Log::ENGINE_ERROR,"GPU does not support VBO");
					Log::log(Log::USER_ERROR,"You graphic card is not able to run this software (no VBO support)");
					return -1;
			}

			//On debind
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

			return 0;
		}

		//X est en bleu
		//Y est en rouge  
		void drawRepere(float slotSize)
		{
			static int sizeRepere = 20; 
			glPushMatrix();
			glBegin(GL_LINES);
			for(int i = -sizeRepere; i<=sizeRepere;i++)
			{
				glColor3f(0.3f,0.3f,0.3f);

				if(i == 0)
					glColor3f(1.0f,1.0f,1.0f);

				glVertex3f(i*slotSize,-sizeRepere*slotSize,0);

				if(i == 0)
				{
					glColor3f(1.0f,1.0f,1.0f);
					glVertex3f(0.0f,0.0f,0.0f);
					glColor3f(1.0f,0.0f,0.0f);
					glVertex3f(0.0f,0.0f,0.0f);
				}

				glVertex3f(i*slotSize,sizeRepere*slotSize,0);

				if(i == 0)
					glColor3f(1.0,1.0,1.0);

				glVertex3f(-sizeRepere*slotSize,i*slotSize,0);

				if(i == 0)
				{
					glColor3f(1.0,1.0,1.0);
					glVertex3f(0,0,0);
					glColor3f(0.0,0.0,1.0);
					glVertex3f(0,0,0);
				}

				glVertex3f(sizeRepere*slotSize,i*slotSize,0);
			}
			glEnd();
			glPopMatrix();
		}

		char* loadSource(const char *filename)
		{
			char *src = NULL;   /* code source de notre shader */
			FILE *fp = NULL;    /* fichier */
			long size;          /* taille du fichier */
			long i;             /* compteur */


			/* on ouvre le fichier */
			fp = fopen(filename, "r");
			/* on verifie si l'ouverture a echoue */
			if(fp == NULL)
			{
				Log::log(Log::ENGINE_ERROR,(std::string("Unable to load shader file ")+ std::string(filename)).c_str());
				return NULL;
			}

			/* on recupere la longueur du fichier */
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);

			/* on se replace au debut du fichier */
			rewind(fp);

			/* on alloue de la memoire pour y placer notre code source */
			src = (char*)malloc(size+1); /* +1 pour le caractere de fin de chaine '\0' */
			if(src == NULL)
			{
				fclose(fp);
				Log::log(Log::ENGINE_ERROR,"Unable to allocate memory for shader file before compilation");
				return NULL;
			}

			/* lecture du fichier */
			for(i=0; i<size && !feof(fp); i++)
				src[i] = fgetc(fp);

			/* on place le dernier caractere a '\0' */
			src[i-1] = '\0';

			fclose(fp);

			return src;
		}

		GLuint loadShader(GLenum type, const char *filename)
		{
			GLuint shader = 0;
			GLsizei logsize = 0;
			GLint compile_status = GL_TRUE;
			char *log = NULL;
			char *src = NULL;

			// creation d'un shader de sommet 
			shader = glCreateShader(type);
			if(shader == 0)
			{
				Log::log(Log::ENGINE_ERROR,"Unable to create shader");
				return 0;
			}

			// chargement du code source 
			src = loadSource(filename);
			if(src == NULL)
			{
				// theoriquement, la fonction LoadSource a deja affiche un message
				// d'erreur, nous nous contenterons de supprimer notre shader
				// et de retourner 0 

				glDeleteShader(shader);
				return 0;
			}

			// assignation du code source 
			glShaderSource(shader, 1, (const GLchar**)&src, NULL);

			// compilation du shader 
			glCompileShader(shader);

			// liberation de la memoire du code source 
			free(src);
			src = NULL;

			//verification du succes de la compilation 
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
			if(compile_status != GL_TRUE)
			{
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

				log = (char*)malloc(logsize + 1);
				if(log == NULL)
				{
					Log::log(Log::ENGINE_ERROR,"Unable to allocate memory for shader compilation log");
					return 0;
				}
				memset(log, '\0', logsize + 1);

				glGetShaderInfoLog(shader, logsize, &logsize, log);
				Log::log(Log::ENGINE_ERROR,("Unable to compile shader " + toString(filename) + " : " + toString(log)).c_str());
				free(log);
				glDeleteShader(shader);

				return 0;
			}

			return shader;
		}

		


		

};

#endif 