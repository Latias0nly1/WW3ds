#include <c3d/maths.h>

void Mtx_PerspStereoTilt(C3D_Mtx* mtx, float fovx, float invaspect, float near, float far, float iod, float screen, bool isLeftHanded)
{

	float fovx_tan = tanf(fovx/2.0f);
	float fovx_tan_invaspect = fovx_tan*invaspect;
	float shift = iod / (2.0f*screen);

	Mtx_Zeros(mtx);

	mtx->r[0].y = 1.0f / fovx_tan;
	mtx->r[1].x = -1.0f / fovx_tan_invaspect;
	mtx->r[1].w = iod / 2.0f;
	mtx->r[2].w = near * far / (near - far);
	mtx->r[3].z = isLeftHanded ? 1.0f : -1.0f;
	mtx->r[1].z = -mtx->r[3].z * shift / fovx_tan_invaspect;
	mtx->r[2].z = -mtx->r[3].z * near / (near - far);
}
