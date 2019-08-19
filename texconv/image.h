#pragma once


#define PSP_HEADER_SIZE 0xD0
#define PSP_PAL_SIZE   1024
#define PS2_HEADER_SIZE 0x250
#define PS2_PAL_SIZE 1024



struct bmp_info_header {
	int      biSize;
	int      biWidth;
	int      biHeight;
	short    biPlanes;
	short    biBitCount;
	int      biCompression;
	int      biSizeImage;
	int      biXPelsPerMeter;
	int      biYPelsPerMeter;
	int      biClrUsed;
	int      biClrImportant;
};

#pragma pack(push,1)
struct bmp_header {
	short  bfType;
	int    bfSize;
	short  bfReserved1;
	short  bfReserved2;
	int    bfOffBits;
};
#pragma pack(pop)



struct image_data {
	int  height;
	int  width;
	char bits;
};

// ps2 too
struct pal_psp_data {
	unsigned char r, g, b, a;
	//int rgba;
};

enum eTexture {
	PSP,
	PS2,
	GC
};

namespace Unswizzlers {
	void                PSP(unsigned int* destination, unsigned int* source, image_data img);
	void                PS2(unsigned char* dest, unsigned char* src, image_data img);
	static unsigned int swizzlePS2(unsigned int x, unsigned int y, unsigned int logw);
}


