// BinaryPrint.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <io.h>

void usage(void)
{
	printf("\nBinaryPrint\n");
	printf("\t<binary filename> <line size>\n");
	printf("example:\n");
	printf("\tBinaryPrint 01.bin 20\n");
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


bool PrintHex(unsigned char * buffer, unsigned long length, int line)
{
	int left = length;
	
	while (left > 0)
	{
		int count = left >= line ? line : left;

		printf("\n");
		for (int i = 0; i < count; i++)
			if (*buffer++ > 0x80)
				printf("FF ");
			else
				printf("00 ");

//			printf("%02x ", *buffer++);

		left -= count;
	}

	printf("\n");

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		usage();
		return 0;
	}

	int line = atoi(argv[2]);
	if ( line <= 0 )
		return 0;

	
	printf("Print Binary file %s at %d bytes per line\r\n", argv[1], line);

	unsigned char * buffer = NULL;
	unsigned long length = 0;

	if (!LoadFile(argv[1], &buffer, &length))
		return 0;

	PrintHex(buffer, length, line);

	return 0;
}

