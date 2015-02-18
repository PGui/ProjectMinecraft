#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"

class NYAvatar
{
public:
	float Mass;
	NYVert3Df Position;
	NYVert3Df Speed;
	float MaxVelocity = 90.0f;
	float Acceleration = 5.0f;
	float Desacceleration = 0.9f;

	NYVert3Df MoveDir;
	NYVert3Df RightDir;//To strafe
	bool Move;
	bool Jump;
	float Height;
	float Width;

	bool avance;
	bool recule;
	bool gauche;
	bool droite;

	bool Standing;

	bool picking = false;
	NYVert3Df pickA;
	NYVert3Df pickB;
	bool drawSphere = false;
	NYVert3Df posDrawSphere;

	NYCamera * Cam;
	NYWorld * World;

	NYAvatar(NYCamera * cam, NYWorld * world)
	{
		Position = NYVert3Df(30, 30, 50);
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
		Mass = 5.0f;
	}


	void render(void)
	{
		/*glPushMatrix();
		glTranslated(Position.X, Position.Y, Position.Z);
		glutSolidCube(Width / 2);
		glPopMatrix();*/
		if (picking)
		{
			
			glBegin(GL_LINES);
			glColor3d(1, 0, 0);
			glVertex3d(pickA.X,pickA.Y, pickA.Z);
			glVertex3d(pickB.X, pickB.Y, pickB.Z);
			glEnd();

			if (drawSphere){
				glPushMatrix();
				glTranslated(posDrawSphere.X, posDrawSphere.Y, posDrawSphere.Z);
				glutSolidSphere(1.0f, 10, 10);
				glPopMatrix();
			}
				
		}
	}

	int getCurrentHeight()
	{
		return World->_MatriceHeights[(int)Position.X][(int)Position.Y];
	}

	void ApplyGravity()
	{

		if (Position.Z < getCurrentHeight())
		{
			Position.Z = getCurrentHeight();
			Speed.Z = 0.0f;
		}
		else
		{
			Speed.Z -= 9.8f;
		}
	}

	void pick()
	{
		int radiusTest = 2;
		for (int x = (int)Position.X - radiusTest; x < (int)Position.X + radiusTest; ++x)
		{
			for (int y = (int)Position.Y - radiusTest; y < (int)Position.Y + radiusTest; ++y)
			{
				for (int z = (int)Position.Z - radiusTest; z < (int)Position.Z + radiusTest; ++z)
				{

					if (intersectionLinePlan(pickA, pickB, NYVert3Df(0, 0, 1), NYVert3Df(x, y, z + 0.5f), posDrawSphere))
					{
						drawSphere = true;
						cout << "Found something in " << posDrawSphere.X << " ," << posDrawSphere.Y << " ," << posDrawSphere.Z << endl;
					}
				}
			}
		}
	}


	void update(float elapsed)
	{
		/*Cam->_Position = Position;
		Cam->_Position.Z += 1;
		
		Move = false;
		if (avance)
		{

			MoveDir = Cam->_Direction;
			MoveDir.Z = 0;
			Speed += MoveDir * Acceleration;

			Move = true;
		}

		if (recule)
		{

			MoveDir = Cam->_Direction;
			MoveDir.Z = 0;
			Speed += MoveDir * -Acceleration;


			Move = true;
		}

		if (gauche)
		{

			MoveDir = Cam->_NormVec;
			MoveDir.Z = 0;
			Speed += MoveDir * -Acceleration;


			Move = true;
		}

		if (droite)
		{

			MoveDir = Cam->_NormVec;
			MoveDir.Z = 0;
			Speed += MoveDir * Acceleration;


			Move = true;
		}


		if (!Move)
		{
			Speed *= Desacceleration;
		}

		Position += Speed *elapsed;
		Position.Z += Speed.Z * elapsed;
		ApplyGravity();

*/


		if (picking)
		{
			
			pickA = NYVert3Df(Position.X, Position.Y, Position.Z - 0.2f);
			pickB = Position + Cam->_Direction*15.0f;
			pick();
		}
		else
		{
			drawSphere = false;
		}

		Cam->moveTo(Position);
		//Par defaut, on applique la gravité (-100 sur Z)
		NYVert3Df force = NYVert3Df(0, 0, -1) * 100.0f;

		//Si l'avatar n'est pas au sol, alors il ne peut pas sauter
		if (!Standing)
			Jump = false;


		//Si il est au sol, on applique les controles "ground"
		if (Standing)
		{
			if (avance)
				force += Cam->_Direction * 400;
			if (recule)
				force += Cam->_Direction * -400;
			if (gauche)
				force += Cam->_NormVec * -400;
			if (droite)
				force += Cam->_NormVec * 400;
		}
		else //Si il est en l'air, c'est du air control
		{
			if (avance)
				force += Cam->_Direction * 50;
			if (recule)
				force += Cam->_Direction * -50;
			if (gauche)
				force += Cam->_NormVec * -50;
			if (droite)
				force += Cam->_NormVec * 50;
		}

		//On applique le jump
		if (Jump)
		{
			force += NYVert3Df(0, 0, 1) * 50.0f / elapsed; //(impulsion, pas fonction du temps)
			Jump = false;
		}

		//On applique les forces en fonction du temps écoulé
		Speed += force * elapsed;

		//On met une limite a sa vitesse horizontale
		NYVert3Df horSpeed = Speed;
		horSpeed.Z = 0;
		if (horSpeed.getSize() > 70.0f)
		{
			horSpeed.normalize();
			horSpeed *= 70.0f;
			Speed.X = horSpeed.X;
			Speed.Y = horSpeed.Y;
		}

		//On le déplace, en sauvegardant son ancienne position
		NYVert3Df oldPosition = Position;
		Position += (Speed * elapsed);

		//On recup la collision a la nouvelle position
		NYCollision collidePrinc = 0x00;
		NYCollision collide = World->collide_with_world(Position, Width, Height, collidePrinc);
		if (collide & NY_COLLIDE_BOTTOM && Speed.Z < 0)
		{
			Position.Z = oldPosition.Z;
			Speed *= pow(0.01f, elapsed);
			Speed.Z = 0;
			Standing = true;
		}
		else
			Standing = false;

		if (collide & NY_COLLIDE_UP && !Standing && Speed.Z > 0)
		{
			Position.Z = oldPosition.Z;
			Speed.Z = 0;
		}

		//On a regle le probleme du bottom et up, on gère les collision sur le plan (x,y)
		collide = World->collide_with_world(Position, Width, Height, collidePrinc);

		//En fonction des cotés, on annule une partie des déplacements
		if (collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT)
		{
			Position.Y = oldPosition.Y;
			Speed.Y = 0;
		}

		if (collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT)
		{
			Position.Y = oldPosition.Y;
			Speed.Y = 0;
		}

		if (collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK)
		{
			Position.X = oldPosition.X;
			Speed.X = 0;
		}

		if (collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK)
		{
			Position.X = oldPosition.X;
			Speed.X = 0;
		}

		//Si je collide sur un angle
		if (!(collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_FRONT) && !(collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_RIGHT))
			if (collide & (NY_COLLIDE_BACK | NY_COLLIDE_FRONT | NY_COLLIDE_RIGHT | NY_COLLIDE_LEFT))
			{
				Position.Y = oldPosition.Y;
				Position.X = oldPosition.X;
			}
	}
};

#endif