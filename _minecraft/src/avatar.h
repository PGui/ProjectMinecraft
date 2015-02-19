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
	float pickingRadius = 5.0f;
	NYVert3Df pickA;
	NYVert3Df pickB;
	bool drawSphere = false;
	NYVert3Df posDrawSphere;

	NYCamera * Cam;
	NYWorld * World;

	NYAvatar(NYCamera * cam, NYWorld * world)
	{
		Position = NYVert3Df(30, 30, 30);
		Height = 0.5f;
		Width = 0.5f;
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
			
			/*glBegin(GL_LINES);
			glColor3d(1, 0, 0);
			glVertex3d(pickA.X,pickA.Y, pickA.Z);
			glVertex3d(pickB.X, pickB.Y, pickB.Z);
			glEnd();*/

			/*if (drawSphere){
				glPushMatrix();
				glTranslated(posDrawSphere.X, posDrawSphere.Y, posDrawSphere.Z);
				glutSolidSphere(1.0f, 10, 10);
				glPopMatrix();
			}*/
				
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
		for (int x = (int)Position.X - pickingRadius; x < (int)Position.X + pickingRadius; ++x)
		{
			for (int y = (int)Position.Y - pickingRadius; y < (int)Position.Y + pickingRadius; ++y)
			{
				for (int z = (int)Position.Z - pickingRadius; z < (int)Position.Z + pickingRadius; ++z)
				{
					//cout << "Found something in " << World->getCube(x, y, z)->isSolid() << endl;
					/*if ( intersectionLinePlan(pickA, pickB, NYVert3Df(0, 0, 1), NYVert3Df(x, y, z + 0.5f), posDrawSphere))
					{
						drawSphere = true;
							cout << "Found something in " << World->getCube(x, y, z)->_Type  << " " << posDrawSphere.X << " ," << posDrawSphere.Y << " ," << posDrawSphere.Z << endl;
					}*/
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
			
			pickA = NYVert3Df(Position.X, Position.Y, Position.Z/* - 0.2f*/);
			pickB = Position + Cam->_Direction*pickingRadius;
			//pick();
			NYVert3Df inter;
			int xCube = 0;
			int yCube = 0;
			int zCube = 0;
			if (World->getRayCollision(pickA, pickB, inter,
				xCube, yCube, zCube))
			{
				World->deleteCube(xCube, yCube, zCube);
				picking = false;
			}
		}
		else
		{
			drawSphere = false;
		}

		Cam->moveTo(Position);


		//Par defaut, on applique la gravité (-100 sur Z)
		NYVert3Df force = NYVert3Df(0, 0, -1) * 10.0f;

		//Si l'avatar n'est pas au sol, alors il ne peut pas sauter
		if (!Standing)
			Jump = false;


		//Si il est au sol, on applique les controles "ground"
		if (Standing)
		{
			if (avance)
				force += Cam->_Direction * 40;
			if (recule)
				force += Cam->_Direction * -40;
			if (gauche)
				force += Cam->_NormVec * -40;
			if (droite)
				force += Cam->_NormVec * 40;
		}
		else //Si il est en l'air, c'est du air control
		{
			if (avance)
				force += Cam->_Direction * 5;
			if (recule)
				force += Cam->_Direction * -5;
			if (gauche)
				force += Cam->_NormVec * -5;
			if (droite)
				force += Cam->_NormVec * 5;
		}

		//On applique le jump
		if (Jump)
		{
			force += NYVert3Df(0, 0, 1) * 7.0f / elapsed; //(impulsion, pas fonction du temps)
			Jump = false;
		}

		//On applique les forces en fonction du temps écoulé
		Speed += force * elapsed;

		//On met une limite a sa vitesse horizontale
		NYVert3Df horSpeed = Speed;
		horSpeed.Z = 0;
		if (horSpeed.getSize() > 7.0f)
		{
			horSpeed.normalize();
			horSpeed *= 7.0f;
			Speed.X = horSpeed.X;
			Speed.Y = horSpeed.Y;
		}

		//On le déplace, en sauvegardant son ancienne position
		NYVert3Df oldPosition = Position;
		Position += (Speed * elapsed);

		Standing = false;

		for (int i = 0; i < 3; i++)
		{
			float valueColMin = 0;
			NYAxis axis = World->getMinCol(Position, Width, Height, valueColMin, i);
			if (axis != 0)
			{
				valueColMin = max(abs(valueColMin), 0.0001f) * (valueColMin > 0 ? 1.0f : -1.0f);
				if (axis & NY_AXIS_X)
				{
					Position.X += valueColMin;
					Speed.X = 0;
				}
				if (axis & NY_AXIS_Y)
				{
					Position.Y += valueColMin;
					Speed.Y = 0;
				}
				if (axis & NY_AXIS_Z)
				{
					Speed.Z = 0;
					Position.Z += valueColMin;
					Speed *= pow(0.01f, elapsed);
					Standing = true;
				}
			}
		}


		//OLD CODE
		//Par defaut, on applique la gravité (-100 sur Z)
		//NYVert3Df force = NYVert3Df(0, 0, -1) * 10.0f;
		////Si l'avatar n'est pas au sol, alors il ne peut pas sauter
		//if (!Standing)
		//	Jump = false;
		////Si il est au sol, on applique les controles "ground"
		//if (Standing)
		//{
		//	if (avance)
		//		force += Cam->_Direction * 40;
		//	if (recule)
		//		force += Cam->_Direction * -40;
		//	if (gauche)
		//		force += Cam->_NormVec * -40;
		//	if (droite)
		//		force += Cam->_NormVec * 40;
		//}
		//else //Si il est en l'air, c'est du air control
		//{
		//	if (avance)
		//		force += Cam->_Direction * 5;
		//	if (recule)
		//		force += Cam->_Direction * -5;
		//	if (gauche)
		//		force += Cam->_NormVec * -5;
		//	if (droite)
		//		force += Cam->_NormVec * 5;
		//}
		////On applique le jump
		//if (Jump)
		//{
		//	force += NYVert3Df(0, 0, 1) * 5.0f / elapsed; //(impulsion, pas fonction du temps)
		//	Jump = false;
		//}
		////On applique les forces en fonction du temps écoulé
		//Speed += force * elapsed;
		////On met une limite a sa vitesse horizontale
		//NYVert3Df horSpeed = Speed;
		//horSpeed.Z = 0;
		//if (horSpeed.getSize() > 70.0f)
		//{
		//	horSpeed.normalize();
		//	horSpeed *= 70.0f;
		//	Speed.X = horSpeed.X;
		//	Speed.Y = horSpeed.Y;
		//}
		////On le déplace, en sauvegardant son ancienne position
		//NYVert3Df oldPosition = Position;
		//Position += (Speed * elapsed);
		////On recup la collision a la nouvelle position
		//NYCollision collidePrinc = 0x00;
		//NYCollision collide = World->collide_with_world(Position, Width, Height, collidePrinc);
		//if (collide & NY_COLLIDE_BOTTOM && Speed.Z < 0)
		//{
		//	Position.Z = oldPosition.Z;
		//	Speed *= pow(0.01f, elapsed);
		//	Speed.Z = 0;
		//	Standing = true;
		//}
		//else
		//	Standing = false;
		//if (collide & NY_COLLIDE_UP && !Standing && Speed.Z > 0)
		//{
		//	Position.Z = oldPosition.Z;
		//	Speed.Z = 0;
		//}
		////On a regle le probleme du bottom et up, on gère les collision sur le plan (x,y)
		//collide = World->collide_with_world(Position, Width, Height, collidePrinc);
		////En fonction des cotés, on annule une partie des déplacements
		//if (collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT)
		//{
		//	Position.Y = oldPosition.Y;
		//	Speed.Y = 0;
		//}
		//if (collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_LEFT)
		//{
		//	Position.Y = oldPosition.Y;
		//	Speed.Y = 0;
		//}
		//if (collide & NY_COLLIDE_RIGHT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK)
		//{
		//	Position.X = oldPosition.X;
		//	Speed.X = 0;
		//}
		//if (collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_FRONT && collide & NY_COLLIDE_BACK)
		//{
		//	Position.X = oldPosition.X;
		//	Speed.X = 0;
		//}
		////Si je collide sur un angle
		//if (!(collide & NY_COLLIDE_BACK && collide & NY_COLLIDE_FRONT) && !(collide & NY_COLLIDE_LEFT && collide & NY_COLLIDE_RIGHT))
		//	if (collide & (NY_COLLIDE_BACK | NY_COLLIDE_FRONT | NY_COLLIDE_RIGHT | NY_COLLIDE_LEFT))
		//	{
		//		Position.Y = oldPosition.Y;
		//		Position.X = oldPosition.X;
		//	}
	}
};

#endif