#include <c3d/maths.h>

C3D_FQuat Quat_FromMtx(const C3D_Mtx* m)
{

	C3D_FQuat q;
	C3D_FVec  diagonal = FVec4_New(m->r[0].x, m->r[1].y, m->r[2].z, 1.0f);

	if (diagonal.z <= 0.0f)
	{
		if (diagonal.x >= diagonal.y)
		{
			q.x = diagonal.w + diagonal.x - diagonal.y - diagonal.z;
			q.y = m->r[1].x + m->r[0].y;
			q.z = m->r[2].x + m->r[0].z;
			q.w = m->r[2].y - m->r[1].z;
		}
		else
		{
			q.x = m->r[1].x + m->r[0].y;
			q.y = diagonal.w - diagonal.x + diagonal.y - diagonal.z;
			q.z = m->r[2].y + m->r[1].z;
			q.w = m->r[0].z - m->r[2].x;
		}
	}
	else
	{
		if (-diagonal.x >= diagonal.y)
		{
			q.x = m->r[2].x + m->r[0].z;
			q.y = m->r[2].y + m->r[1].z;
			q.z = diagonal.w - diagonal.x - diagonal.y + diagonal.z;
			q.w = m->r[1].x - m->r[0].y;
		}
		else
		{
			q.x = m->r[2].y - m->r[1].z;
			q.y = m->r[0].z - m->r[2].x;
			q.z = m->r[1].x - m->r[0].y;
			q.w = diagonal.w + diagonal.x + diagonal.y + diagonal.z;
		}
	}

	return Quat_Normalize(q);
}
