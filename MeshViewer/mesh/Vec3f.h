#pragma once

#include "REAL.h"

struct Vec3f
{
	REAL x, y, z;

	Vec3f(REAL x = 0, REAL y = 0, REAL z = 0)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};