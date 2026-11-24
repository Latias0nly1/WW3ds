#include <c3d/maths.h>

void Mtx_Ortho(C3D_Mtx* mtx, float left, float right, float bottom, float top, float near, float far, bool isLeftHanded)
{
	Mtx_Zeros(mtx);


	mtx->r[0].x = 2.0f / (right - left);
	mtx->r[0].w = (left + right) / (left - right);
	mtx->r[1].y = 2.0f / (top - bottom);
	mtx->r[1].w = (bottom + top) / (bottom - top);
	if (isLeftHanded)
		mtx->r[2].z = 1.0f / (far - near);
	else
		mtx->r[2].z = 1.0f / (near - far);
	mtx->r[2].w = 0.5f*(near + far) / (near - far) - 0.5f;
	mtx->r[3].w = 1.0f;
}
