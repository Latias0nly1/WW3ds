#include <c3d/maths.h>

void Mtx_OrthoTilt(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded)
{
	Mtx_Zeros(mtx);


	mtx->r[0].y = 2.0f / (top - bottom);
	mtx->r[0].w = (bottom + top) / (bottom - top);
	mtx->r[1].x = 2.0f / (left - right);
	mtx->r[1].w = (left + right) / (right - left);
	if (isLeftHanded)
		mtx->r[2].z = 1.0f / (far - near);
	else
		mtx->r[2].z = 1.0f / (near - far);
	mtx->r[2].w = 0.5f*(near + far) / (near - far) - 0.5f;
	mtx->r[3].w = 1.0f;
}
