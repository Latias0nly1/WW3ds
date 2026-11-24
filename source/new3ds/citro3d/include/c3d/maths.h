#pragma once
#include "types.h"
#include <math.h>
#include <string.h>


#define M_TAU (6.28318530717958647692528676655900576)

#ifndef M_PI
#define M_PI (M_TAU/2)
#endif

#define C3D_Angle(_angle) ((_angle)*M_TAU)

#define C3D_AngleFromDegrees(_angle) ((_angle)*M_TAU/360.0f)

#define C3D_AspectRatioTop (400.0f / 240.0f)
#define C3D_AspectRatioBot (320.0f / 240.0f)


static inline C3D_FVec FVec4_New(float x, float y, float z, float w)
{
	return (C3D_FVec){{ w, z, y, x }};
}

static inline C3D_FVec FVec4_Add(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec4_New(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w);
}

static inline C3D_FVec FVec4_Subtract(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec4_New(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z, lhs.w-rhs.w);
}

static inline C3D_FVec FVec4_Negate(C3D_FVec v)
{
	return FVec4_New(-v.x, -v.y, -v.z, -v.w);
}

static inline C3D_FVec FVec4_Scale(C3D_FVec v, float s)
{
	return FVec4_New(v.x*s, v.y*s, v.z*s, v.w*s);
}

static inline C3D_FVec FVec4_PerspDivide(C3D_FVec v)
{
	return FVec4_New(v.x/v.w, v.y/v.w, v.z/v.w, 1.0f);
}

static inline float FVec4_Dot(C3D_FVec lhs, C3D_FVec rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z + lhs.w*rhs.w;
}

static inline float FVec4_Magnitude(C3D_FVec v)
{
	return sqrtf(FVec4_Dot(v,v));
}

static inline C3D_FVec FVec4_Normalize(C3D_FVec v)
{
	float m = FVec4_Magnitude(v);

	return FVec4_New(v.x/m, v.y/m, v.z/m, v.w/m);
}

static inline C3D_FVec FVec3_New(float x, float y, float z)
{
	return FVec4_New(x, y, z, 0.0f);
}

static inline float FVec3_Dot(C3D_FVec lhs, C3D_FVec rhs)
{
	return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

static inline float FVec3_Magnitude(C3D_FVec v)
{
	return sqrtf(FVec3_Dot(v,v));
}

static inline C3D_FVec FVec3_Normalize(C3D_FVec v)
{
	float m = FVec3_Magnitude(v);

	return FVec3_New(v.x/m, v.y/m, v.z/m);
}

static inline C3D_FVec FVec3_Add(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec3_New(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}

static inline C3D_FVec FVec3_Subtract(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec3_New(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
}

static inline float FVec3_Distance(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec3_Magnitude(FVec3_Subtract(lhs, rhs));
}

static inline C3D_FVec FVec3_Scale(C3D_FVec v, float s)
{
	return FVec3_New(v.x*s, v.y*s, v.z*s);
}

static inline C3D_FVec FVec3_Negate(C3D_FVec v)
{
	return FVec3_New(-v.x, -v.y, -v.z);
}

static inline C3D_FVec FVec3_Cross(C3D_FVec lhs, C3D_FVec rhs)
{
	return FVec3_New(lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x);
}


static inline void Mtx_Zeros(C3D_Mtx* out)
{
	memset(out, 0, sizeof(*out));
}

static inline void Mtx_Copy(C3D_Mtx* out, const C3D_Mtx* in)
{
	*out = *in;
}

static inline void Mtx_Diagonal(C3D_Mtx* out, float x, float y, float z, float w)
{
	Mtx_Zeros(out);
	out->r[0].x = x;
	out->r[1].y = y;
	out->r[2].z = z;
	out->r[3].w = w;
}

static inline void Mtx_Identity(C3D_Mtx* out)
{
	Mtx_Diagonal(out, 1.0f, 1.0f, 1.0f, 1.0f);
}

void Mtx_Transpose(C3D_Mtx* out);

static inline void Mtx_Add(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs)
{
	for (int i = 0; i < 16; i++)
		out->m[i] = lhs->m[i] + rhs->m[i];
}

static inline void Mtx_Subtract(C3D_Mtx* out, const C3D_Mtx* lhs, const C3D_Mtx* rhs)
{
	for (int i = 0; i < 16; i++)
		out->m[i] = lhs->m[i] - rhs->m[i];
}

void Mtx_Multiply(C3D_Mtx* out, const C3D_Mtx* a, const C3D_Mtx* b);

float Mtx_Inverse(C3D_Mtx* out);

C3D_FVec Mtx_MultiplyFVec3(const C3D_Mtx* mtx, C3D_FVec v);

C3D_FVec Mtx_MultiplyFVec4(const C3D_Mtx* mtx, C3D_FVec v);

static inline C3D_FVec Mtx_MultiplyFVecH(const C3D_Mtx* mtx, C3D_FVec v)
{
	v.w = 1.0f;

	return Mtx_MultiplyFVec4(mtx, v);
}

void Mtx_FromQuat(C3D_Mtx* m, C3D_FQuat q);


void Mtx_Translate(C3D_Mtx* mtx, float x, float y, float z, bool bRightSide);

void Mtx_Scale(C3D_Mtx* mtx, float x, float y, float z);

void Mtx_Rotate(C3D_Mtx* mtx, C3D_FVec axis, float angle, bool bRightSide);

void Mtx_RotateX(C3D_Mtx* mtx, float angle, bool bRightSide);

void Mtx_RotateY(C3D_Mtx* mtx, float angle, bool bRightSide);

void Mtx_RotateZ(C3D_Mtx* mtx, float angle, bool bRightSide);


void Mtx_Ortho(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);

void Mtx_Persp(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);

void Mtx_PerspStereo(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);

void Mtx_OrthoTilt(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded);

void Mtx_PerspTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded);

void Mtx_PerspStereoTilt(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, float iod, float screen, bool isLeftHanded);

void Mtx_LookAt(C3D_Mtx* out, C3D_FVec cameraPosition, C3D_FVec cameraTarget, C3D_FVec cameraUpVector, bool isLeftHanded);


#define Quat_New(i,j,k,r) FVec4_New(i,j,k,r)

#define Quat_Negate(q) FVec4_Negate(q)

#define Quat_Add(lhs,rhs) FVec4_Add(lhs,rhs)

#define Quat_Subtract(lhs,rhs) FVec4_Subtract(lhs,rhs)

#define Quat_Scale(q,s) FVec4_Scale(q,s)

#define Quat_Normalize(q) FVec4_Normalize(q)

#define Quat_Dot(lhs,rhs) FVec4_Dot(lhs,rhs)

C3D_FQuat Quat_Multiply(C3D_FQuat lhs, C3D_FQuat rhs);

C3D_FQuat Quat_Pow(C3D_FQuat q, float p);

C3D_FVec Quat_CrossFVec3(C3D_FQuat q, C3D_FVec v);

C3D_FQuat Quat_Rotate(C3D_FQuat q, C3D_FVec axis, float r, bool bRightSide);

C3D_FQuat Quat_RotateX(C3D_FQuat q, float r, bool bRightSide);

C3D_FQuat Quat_RotateY(C3D_FQuat q, float r, bool bRightSide);

C3D_FQuat Quat_RotateZ(C3D_FQuat q, float r, bool bRightSide);

C3D_FQuat Quat_FromMtx(const C3D_Mtx* m);

static inline C3D_FQuat Quat_Identity(void)
{
	return Quat_New(0.0f, 0.0f, 0.0f, 1.0f);
}

static inline C3D_FQuat Quat_Conjugate(C3D_FQuat q)
{
	return Quat_New(-q.i, -q.j, -q.k, q.r);
}

static inline C3D_FQuat Quat_Inverse(C3D_FQuat q)
{
	C3D_FQuat c = Quat_Conjugate(q);
	float     d = Quat_Dot(q, q);
	return Quat_New(c.i/d, c.j/d, c.k/d, c.r/d);
}

static inline C3D_FVec FVec3_CrossQuat(C3D_FVec v, C3D_FQuat q)
{
	return Quat_CrossFVec3(Quat_Inverse(q), v);
}

C3D_FQuat Quat_FromPitchYawRoll(float pitch, float yaw, float roll, bool bRightSide);

C3D_FQuat Quat_LookAt(C3D_FVec source, C3D_FVec target, C3D_FVec forwardVector, C3D_FVec upVector);

C3D_FQuat Quat_FromAxisAngle(C3D_FVec axis, float angle);
