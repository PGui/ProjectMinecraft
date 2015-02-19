#pragma once

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"

inline bool intersectionLinePlan(NYVert3Df A, NYVert3Df B, NYVert3Df normalPlan, NYVert3Df pointPlan, NYVert3Df & collisionPoint)
{
	//NYVert3Df result = v1Plan.vecProd(v2Plan);
	//float d = -(result.X * pointPlan.X + result.Y * pointPlan.Y + result.Z * pointPlan.Z);
	float d = -normalPlan.scalProd(pointPlan);

	float num = -(normalPlan.X*A.X + normalPlan.Y*A.Y + normalPlan.Z*A.Z + d);
	float denom = (normalPlan.X*(B.X - A.X) + normalPlan.Y*(B.Y - A.Y) + normalPlan.Z*(B.Z - A.Z));
	if (num == 0 || denom == 0)
	{
		return false;
	}

	float t = (num / denom);

	collisionPoint.X = A.X + (B.X - A.X)*t;
	collisionPoint.Y = A.Y + (B.Y - A.Y)*t;
	collisionPoint.Z = A.Z + (B.Z - A.Z)*t;

	if (t < 0 || t > 1)
		return false;

	return true;
}

inline bool intersecDroitePlan(NYVert3Df & debSegment, NYVert3Df & finSegment,
	NYVert3Df & p1Plan, NYVert3Df & p2Plan, NYVert3Df & p3Plan,
	NYVert3Df & inter)
{
	//Equation du plan :
	NYVert3Df nrmlAuPlan = (p1Plan - p2Plan).vecProd(p3Plan - p2Plan); //On a les a,b,c du ax+by+cz+d = 0
	float d = -(nrmlAuPlan.X * p2Plan.X + nrmlAuPlan.Y * p2Plan.Y + nrmlAuPlan.Z* p2Plan.Z); //On remarque que c'est un produit scalaire...

	//Equation de droite :
	NYVert3Df dirDroite = finSegment - debSegment;

	//On resout l'équation de plan
	float nominateur = -d - nrmlAuPlan.X * debSegment.X - nrmlAuPlan.Y * debSegment.Y - nrmlAuPlan.Z * debSegment.Z;
	float denominateur = nrmlAuPlan.X * dirDroite.X + nrmlAuPlan.Y * dirDroite.Y + nrmlAuPlan.Z * dirDroite.Z;

	if (denominateur == 0)
		return false;

	//Calcul de l'intersection
	float t = nominateur / denominateur;
	inter = debSegment + (dirDroite*t);

	//Si point avant le debut du segment
	if (t < 0 || t > 1)
		return false;

	return true;
}