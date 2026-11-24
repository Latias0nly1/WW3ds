/*------------------------------------------------------------------------------
 * Copyright (c) 2017
 *     Michael Theall (mtheall)
 *
 * This file is part of citro3d.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *   2. Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *   3. This notice may not be removed or altered from any source distribution.
 *----------------------------------------------------------------------------*/
#pragma once
#ifdef CITRO3D_BUILD
#include "c3d/texture.h"
#else
#include <citro3d.h>
#endif

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Tex3DS_SubTexture
{
	u16   width;
	u16   height;
	float left;
	float top;
	float right;
	float bottom;
} Tex3DS_SubTexture;

typedef struct Tex3DS_Texture_s* Tex3DS_Texture;

Tex3DS_Texture Tex3DS_TextureImport(const void* input, size_t insize, C3D_Tex* tex, C3D_TexCube* texcube, bool vram);

Tex3DS_Texture Tex3DS_TextureImportCallback(C3D_Tex* tex, C3D_TexCube* texcube, bool vram, decompressCallback callback, void* userdata);

Tex3DS_Texture Tex3DS_TextureImportFD(int fd, C3D_Tex* tex, C3D_TexCube* texcube, bool vram);

Tex3DS_Texture Tex3DS_TextureImportStdio(FILE* fp, C3D_Tex* tex, C3D_TexCube* texcube, bool vram);

size_t Tex3DS_GetNumSubTextures(const Tex3DS_Texture texture);

const Tex3DS_SubTexture* Tex3DS_GetSubTexture(const Tex3DS_Texture texture, size_t index);

static inline bool
Tex3DS_SubTextureRotated(const Tex3DS_SubTexture* subtex)
{
	return subtex->top < subtex->bottom;
}

static inline void
Tex3DS_SubTextureBottomLeft(const Tex3DS_SubTexture* subtex, float* u, float* v)
{
	if (!Tex3DS_SubTextureRotated(subtex))
	{
		*u = subtex->left;
		*v = subtex->bottom;
	} else
	{
		*u = subtex->bottom;
		*v = subtex->left;
	}
}

static inline void
Tex3DS_SubTextureBottomRight(const Tex3DS_SubTexture* subtex, float* u, float* v)
{
	if (!Tex3DS_SubTextureRotated(subtex))
	{
		*u = subtex->right;
		*v = subtex->bottom;
	} else
	{
		*u = subtex->bottom;
		*v = subtex->right;
	}
}

static inline void
Tex3DS_SubTextureTopLeft(const Tex3DS_SubTexture* subtex, float* u, float* v)
{
	if (!Tex3DS_SubTextureRotated(subtex))
	{
		*u = subtex->left;
		*v = subtex->top;
	} else
	{
		*u = subtex->top;
		*v = subtex->left;
	}
}

static inline void
Tex3DS_SubTextureTopRight(const Tex3DS_SubTexture* subtex, float* u, float* v)
{
	if (!Tex3DS_SubTextureRotated(subtex))
	{
		*u = subtex->right;
		*v = subtex->top;
	} else
	{
		*u = subtex->top;
		*v = subtex->right;
	}
}

void Tex3DS_TextureFree(Tex3DS_Texture texture);

#ifdef __cplusplus
}
#endif
