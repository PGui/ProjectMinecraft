#include "types_3d.h"

//Intersection sphère et segment [p1,p2]
//Attention, part du principe que le segment coupe la sphère en un seul endroit !!!!
bool interDroiteSphere(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float pcx, float pcy, float pcz, float rayon, NYVert3Df & inter)
{
	float a = (p2x-p1x)*(p2x-p1x) + (p2y-p1y)*(p2y-p1y) + (p2z-p1z)*(p2z-p1z);
	float b = 2*((p2x-p1x)*(p1x-pcx) + (p2y-p1y)*(p1y-pcy) + (p2z-p1z)*(p1z-pcz));
	float c = pcx*pcx + pcy*pcy + pcz*pcz + p1x*p1x + p1y*p1y + p1z*p1z - 2*(pcx*p1x+pcy*p1y+pcz*p1z) - rayon * rayon;

	//if(isNaN(a) || isNaN(b) || isNaN(c))
		//_cprintf("erreur!");

	float delta = b*b - 4*a*c;
	if(delta > 0)
	{
		float racDelta = sqrt(delta);

		float u = (-b-racDelta)/(2*a);
		inter.X = p1x + u*(p2x-p1x);
		inter.Y = p1y + u*(p2y-p1y);
		inter.Z = p1z + u*(p2z-p1z);

/*		if((inter.X > p1.X && inter.X > p2.X) ||
		   (inter.X < p1.X && inter.X < p2.X) ||
		   (inter.Y > p1.Y && inter.Y > p2.Y) ||
		   (inter.Y < p1.Y && inter.Y < p2.Y) ||
		   (inter.Z > p1.Z && inter.Z > p2.Z) ||
		   (inter.Z < p1.Z && inter.Z < p2.Z))
		{
			u = (-b+racDelta)/(2*a);
			inter.X = p1.X + u*(p2.X-p1.X);
			inter.Y = p1.Y + u*(p2.Y-p1.Y);
			inter.Z = p1.Z + u*(p2.Z-p1.Z);

			if((inter.X > p1.X && inter.X > p2.X) ||
			   (inter.X < p1.X && inter.X < p2.X) ||
			   (inter.Y > p1.Y && inter.Y > p2.Y) ||
			   (inter.Y < p1.Y && inter.Y < p2.Y) ||
			   (inter.Z > p1.Z && inter.Z > p2.Z) ||
			   (inter.Z < p1.Z && inter.Z < p2.Z))
			   return false;
			else
				return true;
		}*/

		return true;
	}

	if(delta == 0)
	{
		float u = (-b)/(2*a);
		inter.X = p1x + u*(p2x-p1x);
		inter.Y = p1y + u*(p2y-p1y);
		inter.Z = p1z + u*(p2z-p1z);

		/*if((inter.X > p1.X && inter.X > p2.X) ||
		   (inter.X < p1.X && inter.X < p2.X) ||
		   (inter.Y > p1.Y && inter.Y > p2.Y) ||
		   (inter.Y < p1.Y && inter.Y < p2.Y) ||
		   (inter.Z > p1.Z && inter.Z > p2.Z) ||
		   (inter.Z < p1.Z && inter.Z < p2.Z))
		   return false;*/
	}


	return false;
}

