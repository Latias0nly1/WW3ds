#include <c3d/maths.h>

void Mtx_PerspTilt(C3D_Mtx* mtx, float fovx, float invaspect, float near, float far, bool isLeftHanded)
{





	float fovx_tan = tanf(fovx/2.0f);

	Mtx_Zeros(mtx);

	mtx->r[0].y = 1.0f / fovx_tan;
	mtx->r[1].x = -1.0f / (fovx_tan*invaspect);
	mtx->r[2].w = far*near / (near - far);
	mtx->r[3].z = isLeftHanded ? 1.0f : -1.0f;
	mtx->r[2].z = -mtx->r[3].z * near / (near - far);
}
