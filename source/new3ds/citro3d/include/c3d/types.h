#pragma once
#if defined(__3DS__) || defined(_3DS)
#include <3ds.h>
#else
#include <stdbool.h>
#include <stdint.h>
typedef uint8_t u8;
typedef uint32_t u32;
#endif

#ifndef CITRO3D_NO_DEPRECATION
#define C3D_DEPRECATED __attribute__ ((deprecated))
#else
#define C3D_DEPRECATED
#endif

typedef u32 C3D_IVec;

static inline C3D_IVec IVec_Pack(u8 x, u8 y, u8 z, u8 w)
{
	return (u32)x | ((u32)y << 8) | ((u32)z << 16) | ((u32)w << 24);
}


typedef union
{
	struct
	{
		float w;
		float z;
		float y;
		float x;
	};

	struct
	{
		float r;
		float k;
		float j;
		float i;
	};

	float c[4];
} C3D_FVec;

typedef C3D_FVec C3D_FQuat;

typedef union
{
	C3D_FVec r[4];
	float m[4*4];
} C3D_Mtx;
