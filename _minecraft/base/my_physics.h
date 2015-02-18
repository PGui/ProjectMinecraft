#pragma once

#include "external/gl/glew.h"
#include "external/gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "engine/utils/ny_utils.h"

inline bool intersectionLinePlan(NYVert3Df A, NYVert3Df B, NYVert3Df v1Plan, NYVert3Df v2Plan, NYVert3Df pointPlan, NYVert3Df & collisionPoint)
{
	NYVert3Df result = v1Plan.vecProd(v2Plan);
	float d = -(result.X * pointPlan.X + result.Y * pointPlan.Y + result.Z * pointPlan.Z);

	float num = -(result.X*A.X + result.Y*A.Y + result.Z*A.Z + d);
	if (num == 0)
	{
		return false;
	}

	float t = (num / (result.X*(B.X - A.X) + result.Y*(B.Y - A.Y) + result.Z*(B.Z - A.Z)));

	collisionPoint.X = A.X + (B.X - A.X)*t;
	collisionPoint.Y = A.Y + (B.Y - A.Y)*t;
	collisionPoint.Z = A.Z + (B.Z - A.Z)*t;

	return true;
}