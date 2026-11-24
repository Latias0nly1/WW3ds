#include <float.h>
#include <c3d/maths.h>

C3D_FQuat Quat_Pow(C3D_FQuat q, float p)
{
	if (p > -FLT_EPSILON && p < FLT_EPSILON)
		return Quat_Identity();

	float mag = FVec4_Magnitude(q);

	if (fabsf(q.r / mag) > 1.0f - FLT_EPSILON && fabsf(q.r / mag) < 1.0f + FLT_EPSILON)
		return Quat_New(0.0f, 0.0f, 0.0f, powf(q.r, p));

	float angle    = acosf(q.r / mag);
	float newAngle = angle * p;
	float div      = sinf(newAngle) / sinf(angle);
	float Mag      = powf(mag, p - 1.0f);

	return Quat_New(q.i*div*Mag, q.j*div*Mag, q.k*div*Mag, cosf(newAngle)*mag*Mag);
}
