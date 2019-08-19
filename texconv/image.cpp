#include "image.h"
#include <iostream>

// https://github.com/aap/librwgta/blob/master/tools/storiesconv/rslconv.cpp


void Unswizzlers::PSP(unsigned int* destination, unsigned int* source, image_data img)
{
	unsigned int stride = img.width * img.bits / 8;
	unsigned int nbx = stride / 16;
	unsigned int nby = (img.height + 7) / 8;
	stride >>= 2;
	unsigned int *row, *b;

	while (nby--) {
		row = destination;
		for (unsigned int x = 0; x < nbx; x++) {
			b = row;
			for (unsigned int n = 0; n < 8; n++) {
				memcpy(b, source, 16);
				b += stride;
				source += 4;
			}
			row += 4;
		}
		destination += stride * 8;
	}
}

void Unswizzlers::PS2(unsigned char * dest, unsigned char * src, image_data img)
{
	unsigned int logw = 0;
	for (unsigned int i = 1; i < img.width; i *= 2) logw++;
	for (unsigned int y = 0; y < img.height; y++)
		for (unsigned int x = 0; x < img.width; x++)
			dest[(y << logw) + x] = src[swizzlePS2(x, y, logw)];
}

unsigned int Unswizzlers::swizzlePS2(unsigned int x, unsigned int y, unsigned int logw)
{
#define X(n) ((x>>(n))&1)
#define Y(n) ((y>>(n))&1)

	unsigned nx, ny, n;
	x ^= (Y(1) ^ Y(2)) << 2;
	nx = x & 7 | (x >> 1)&~7;
	ny = y & 1 | (y >> 1)&~1;
	n = Y(1) | X(3) << 1;
	return n | nx << 2 | ny << logw - 1 + 2;
}

