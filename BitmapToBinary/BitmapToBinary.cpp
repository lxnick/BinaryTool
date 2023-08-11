// BitmapToBinary.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <io.h>

#define BMP_LINE_ALIGNMENT(bits) ((((bits) + 31) & ~31) >> 3)


void usage(void)
{
	printf("\nBitmapToBinary\n");
	printf("\t<bitmap filename> <binary filename> \n");
	printf("example:\n");
	printf("\tBitmapToBinary input.bmp output.bin\n");
}

void PrintBitmapInfo(BITMAPFILEHEADER bmf, BITMAPINFOHEADER bmi)
{

}

unsigned char ConvertQuadToGray(RGBQUAD quad)
{
	float r = quad.rgbRed;
	float g = quad.rgbGreen;
	float b = quad.rgbBlue;

	float y = (float) (r*0.299 + g*0.587 + b*0.114);
	int n = int(y + 0.5);

	return n > 255 ? 255 : n;
}

bool LoadFile(char* pszfile, unsigned char ** buffer, unsigned long* length)
{
	FILE* pIn = NULL;
	errno_t err;

	if ((err = fopen_s(&pIn, pszfile, "r+b")) != 0)
	{
		printf("Load File %s failed\r\n", pszfile);
		return 0;
	}
	unsigned long nFileLength = _filelength(_fileno(pIn));
	if (nFileLength == 0)
	{
		printf("Load File %s failed, no data\r\n", pszfile);
		return 0;
	}

	unsigned char * pBuffer = new unsigned char[nFileLength];
	fread(pBuffer, 1, nFileLength, pIn);
	fclose(pIn);

	*buffer = pBuffer;
	*length = nFileLength;
	return true;
}

bool WriteBinaryFile(char* pszfile, unsigned char * buffer, unsigned long length)
{
	FILE* pOut = NULL;
	errno_t err;

	if ((err = fopen_s(&pOut, pszfile, "w+b")) != 0)
	{
		printf("Write File %s failed\r\n", pszfile);
		return 0;
	}

	fwrite(buffer, 1, length, pOut);

	fclose(pOut);
	return true;
}

bool GetBitmapPixel(unsigned char * bmp_ptr, unsigned long bmp_len, unsigned char ** bin_ptr, unsigned long * bin_len)
{
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;
	RGBQUAD palette[256];

	if (bmp_len < (sizeof(bmf) + sizeof(bmi) + sizeof(palette)))
		return false;

	bmf = *(LPBITMAPFILEHEADER)bmp_ptr;
	bmi = *(LPBITMAPINFOHEADER) (bmp_ptr + sizeof(bmf));
	memcpy(&palette[0], bmp_ptr + sizeof(bmf) + sizeof(bmi), sizeof(palette));

	if (bmf.bfType != 0x4d42)
		goto DUMP_BITMAP_INMO;

	if (bmi.biSize != sizeof(BITMAPINFOHEADER))
		goto DUMP_BITMAP_INMO;

	if (bmi.biPlanes != 1)
		goto DUMP_BITMAP_INMO;

	if (bmi.biBitCount != 8)
		goto DUMP_BITMAP_INMO;

	unsigned long length = bmi.biWidth * bmi.biHeight;
	if (length <= 0)
		goto DUMP_BITMAP_INMO;

	unsigned char * buffer = new unsigned char[length];
	memset(buffer, 0, sizeof(unsigned char) * length);

	int bmp_line_width = BMP_LINE_ALIGNMENT(bmi.biWidth * bmi.biBitCount);
	unsigned char * pixel = bmp_ptr + sizeof(bmf) + sizeof(bmi) + sizeof(palette);
	unsigned char * ptr = buffer;
	for (int i = 0; i < bmi.biHeight; i++)
	{
		unsigned char * line = pixel + (bmi.biHeight - i - 1)* bmp_line_width;
		for (int j = 0; j < bmi.biWidth; j++)
		{
			RGBQUAD color = palette[line[j]];
			*ptr++ = ConvertQuadToGray(color);
		}
	}

	*bin_ptr = buffer;
	*bin_len = length;
	return true;

DUMP_BITMAP_INMO:
	PrintBitmapInfo(bmf, bmi);

	return false;
}


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		usage();
		return 0;
	}

	printf("BitmapToBinary.exe bmp<%s> bin<%s> \r\n", argv[1], argv[2]);

	unsigned char * in_buffer = NULL;
	unsigned long in_length = 0;

	if (!LoadFile(argv[1], &in_buffer, &in_length))
		return 0;

	unsigned char * pixel_buffer = NULL;
	unsigned long pixel_length = 0;

	if (!GetBitmapPixel(in_buffer, in_length, &pixel_buffer,&pixel_length))
		return 0;

	if (!WriteBinaryFile(argv[2], pixel_buffer, pixel_length))
		return 0;

	printf("\r\nWrite file %s done\r\n", argv[2]);

	return 0;
}

