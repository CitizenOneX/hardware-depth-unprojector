/*
 * HDU - Hardware Depth Unprojector C library implementation
 *
 * Copyright 2020 (C) Bartosz Meglicki <meglickib@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "hdu.h"

#include <stdio.h> //fprintf
#include <stdlib.h> //malloc

 // YUV -> RGB conversion macros
#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )
#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

//in binary 10 ones followed by 6 zeroes
static const uint16_t P010LE_MAX = 0xFFC0;

struct hdu
{
	float ppx;
	float ppy;
	float fx;
	float fy;
	float depth_unit;
	float min_depth;
	float max_depth;
};

struct hdu *hdu_init(const struct hdu_config *c)
{
	struct hdu *h;

	if( ( h = (struct hdu*)malloc(sizeof(struct hdu))) == NULL )
	{
		fprintf(stderr, "hdu: not enough memory for hdu\n");
		//errno = ENOMEM;
		return NULL;
	}

	h->ppx = c->ppx;
	h->ppy = c->ppy;
	h->fx = c->fx;
	h->fy = c->fy;
	h->depth_unit = c->depth_unit;
	h->min_depth = c->min_margin;
	h->max_depth = P010LE_MAX * c->depth_unit - c->max_margin;

	return h;
}

void hdu_close(struct hdu *h)
{
	if(h == NULL)
		return;

	free(h);
}

void hdu_unproject(const struct hdu *h, const struct hdu_depth *depth, struct hdu_point_cloud *pc)
{
	const int pc_size = pc->size;
	const color32 default_color = 0xFFFFFFFF; // RGBA(255, 255, 255, 255), opaque white
	int points=0;
	float d;
	// Y-plane length depth->color_stride * depth->height
	const uint8_t* colorY = depth->colors;
	// UV interleaved plane, length depth->color_stride * (depth->height / 2) in bytes (half of that in short ints)
	const uint16_t* colorUV = ((uint8_t*)depth->colors) + depth->color_stride * depth->height;
	uint8_t Y, R, G, B = 0;
	uint16_t UV = 0;

	for(int r=0;r<depth->height;++r)
		for(int c=0;c<depth->width && points < pc_size;++c)
		{
			d = depth->data[r * depth->depth_stride / 2 + c] * h->depth_unit;

			pc->data[points][0] = d * (c - h->ppx) / h->fx;
			pc->data[points][1] = -d * (r - h->ppy) / h->fy;
			pc->data[points][2] = d;

			if (depth->colors)
			{
				// combine Y and UV values from NV12 here to RGBA color32 struct
				Y = colorY[r * depth->color_stride + c];
				UV = colorUV[(r/2) * (depth->color_stride/2) + (c/2)];
				
				// combine for RGB
				R = YUV2R(Y, UV >> 8, UV & 0xFF);
				G = YUV2G(Y, UV >> 8, UV & 0xFF);
				B = YUV2B(Y, UV >> 8, UV & 0xFF);

				//pc->colors[points] = (R << 24) | (G << 16) | (B << 8) | 0xFF; // big-endian
				pc->colors[points] = (0xFF << 24) | (B << 16) | (G << 8) | R & 0xFF; // little-endian
			}
			else
			{
				pc->colors[points] = default_color;
			}
			++points;
		}

	pc->used = points;
	return;
}
