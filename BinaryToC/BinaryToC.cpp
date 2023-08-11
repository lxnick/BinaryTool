// BinaryToC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <algorithm>
#include <string>
#include <io.h>


void usage(void)
{
	printf("\nBinaryToC\n");
	printf("\t<binary filename> <c style filename> <data array name>\n");
	printf("example:\n");
	printf("\tBinaryToC hello_world_float.tflite hello_world_float_model hello_world_float_data\n");
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

bool PrintHexData(unsigned char * bin, unsigned long bin_length, unsigned char * hex, unsigned long hex_length)
{
	int left = bin_length;
	memset(hex, 0, hex_length);

	unsigned char * tail = hex + hex_length - 1;

	while (left > 0)
	{
		if (left >= 16)
		{
			int printout
				= sprintf_s((char*)hex, tail - hex, "\t0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,\n",
					bin[0], bin[1], bin[2], bin[3], bin[4], bin[5], bin[6], bin[7], bin[8], bin[9], bin[10], bin[11], bin[12], bin[13], bin[14], bin[15]);
			left -= 16;
			bin += 16;
			hex += printout;
			continue;
		}

		hex += sprintf_s((char*)hex, tail - hex, "\t");
		for (int i = 0; i < left; i++)
		{
			hex += sprintf_s((char*)hex, tail - hex, "0x%02x,", bin[i]);
		}
		left -= left;
	}

	return true;
}

bool MakeHeaderFile(char* dataname, char* pszfile, unsigned char * buffer, unsigned long length)
{
	FILE* pOut = NULL;
	errno_t err;

	std::string filename = pszfile;
	filename += ".h";

	if ((err = fopen_s(&pOut, filename.c_str(), "w+t")) != 0)
	{
		printf("Write Header File %s failed\r\n", filename.c_str());
		return false;
	}

	if ((dataname == NULL) || strlen(dataname) == 0)
	{
		printf("No data name %s\r\n", dataname);
		return false;
	}

	std::string data_define = dataname;
	std::string data_upper = dataname;
	std::transform(data_upper.begin(), data_upper.end(), data_upper.begin(), ::toupper);

	fprintf(pOut, "#ifndef %s\n", data_upper.c_str());
	fprintf(pOut, "#define %s\n", data_upper.c_str());
	fprintf(pOut, "\n");

	fprintf(pOut, "#ifdef __cplusplus \nextern \"C\" {\n#endif\n");
	fprintf(pOut, "\n");

	fprintf(pOut, "extern const unsigned char %s_data[];\n", data_define.c_str());
	fprintf(pOut, "extern const unsigned long %s_length;\n", data_define.c_str());
	fprintf(pOut, "\n");

	fprintf(pOut, "#ifdef __cplusplus \n}\n#endif\n");
	fprintf(pOut, "\n");

	fprintf(pOut, "#endif\n");

	fclose(pOut);
	return true;
}

bool MakeSourceFile(char* dataname, char* pszfile, unsigned char * buffer, unsigned long length)
{
	FILE* pOut = NULL;
	errno_t err;

	std::string header_filename = pszfile;
	header_filename += ".h";
	std::string c_filename = pszfile;
	c_filename += ".c";

	if ((err = fopen_s(&pOut, c_filename.c_str(), "w+t")) != 0)
	{
		printf("Write C File %s failed\r\n", c_filename.c_str());
		return false;
	}

	std::string data_define = dataname;

	fprintf(pOut, "#include \"%s\"\n", header_filename.c_str());
	fprintf(pOut, "\n");

	fprintf(pOut, "const unsigned long %s_length =%d;\n", data_define.c_str(), length);
	fprintf(pOut, "\n");

	fprintf(pOut, "const unsigned char %s_data[]= \n", data_define.c_str());
	fprintf(pOut, "{\n");

	unsigned long	hex_size = length * 8;
	unsigned char * hex_buffer = new unsigned char[hex_size];

	PrintHexData(buffer, length, hex_buffer, hex_size);
	fprintf(pOut, "%s\n", hex_buffer);


	fprintf(pOut, "};\n");
	fprintf(pOut, "\n");

	delete[] hex_buffer;
	fclose(pOut);
	return true;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		usage();
		return 0;
	}

	printf("BinaryToC.exe file<%s> destination<%s> data<%s>\r\n", argv[1], argv[2], argv[3]);

	unsigned char * in_buffer = NULL;
	unsigned long in_length = 0;

	if (!LoadFile(argv[1], &in_buffer, &in_length))
		return 0;

	if (!MakeHeaderFile(argv[3], argv[2], in_buffer, in_length))
		return 0;

	if (!MakeSourceFile(argv[3], argv[2], in_buffer, in_length))
		return 0;

	printf("\r\n Make done\r\n");

	return 0;
}
