#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"

class NYAvatar
{
	public :
		float Mass;
		NYVert3Df Position;
		NYVert3Df Speed;

		NYVert3Df MoveDir;
		bool Move;
		bool Jump;
		float Height;
		float Width;
		bool avance;
		bool recule;
		bool gauche;
		bool droite;
		bool Standing;

		NYCamera * Cam;
		NYWorld * World;

		NYAvatar(NYCamera * cam,NYWorld * world)
		{
			Position = NYVert3Df(0,0,0);
			Height = 10;
			Width = 3;
			Cam = cam;
			avance = false;
			recule = false;
			gauche = false;
			droite = false;
			Standing = false;
			Jump = false;
			World = world;
		}


		void render(void)
		{
			glutSolidCube(Width/2);
		}



		void update(float elapsed)
		{
			//int force = 
		}
};

#endif