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

	return true;
}