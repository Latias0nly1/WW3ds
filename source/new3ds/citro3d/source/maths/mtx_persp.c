#include <c3d/maths.h>

void Mtx_Persp(C3D_Mtx* mtx, float fovy, float aspect, float near, float far, bool isLeftHanded)
{
	float fovy_tan = tanf(fovy/2.0f);

	Mtx_Zeros(mtx);


	mtx->r[0].x = 1.0f / (aspect * fovy_tan);
	mtx->r[1].y = 1.0f / fovy_tan;
	mtx->r[2].w = far*near / (near - far);
	mtx->r[3].z = isLeftHanded ? 1.0f : -1.0f;
	mtx->r[2].z = -mtx->r[3].z * near / (near - far);
}
