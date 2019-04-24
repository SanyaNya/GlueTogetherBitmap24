#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "GlueTogetherBitmap24.h"

#define _FWRITE_VAL(buf, val, size, stream) buf = val; fwrite(&buf, size, 1, stream)
#define _ZERO_COUNT(width) (3 * (width)) % 4 != 0 ? 4 - ((3 * (width)) % 4) : 0

#define WRITE_BUFFER_SIZE 4096

static const char* GlueTogetherBitmap24_ErrorString[] =
{
	"No errors",

	"File 1 open error",
	"File 2 open error",
	"File 3 open error",

	"File 1 invalid BitmapInfo size",
	"File 2 invalid BitmapInfo size",
	"File 3 invalid BitmapInfo size",

	"File 1 width <= 0",
	"File 2 width <= 0",
	"File 3 width <= 0",

	"File 1 BitCount must be 24",
	"File 2 BitCount must be 24",
	"File 3 BitCount must be 24",

	"Incorrect sizes of images for gluing",

	"Out of memory",

	"Out file open error"
};

static struct BitmapSize
{
	int32_t Width;
	int32_t Height;
};

static char writeBuffer[WRITE_BUFFER_SIZE];

//Инициализирует data для каждого файла и ставит указатель в положение пиксельных данных
static int _BitmapGetSizesAndSeekToOffBits(FILE* files[3], struct BitmapSize sizes[3])
{
	uint32_t OffBits, InfoSize;
	uint16_t BitCount;

	for (char x = 0; x < 3; x++)
	{
		fseek(files[x], 10, SEEK_SET);

		fread(&OffBits, 4, 1, files[x]);
		fread(&InfoSize, 4, 1, files[x]);

		if (InfoSize == 12)
		{
			sizes[x].Width = 0;
			sizes[x].Height = 0;

			fread(&sizes[x].Width, 2, 1, files[x]);
			fread(&sizes[x].Height, 2, 1, files[x]);
		}
		else if (InfoSize == 40 || InfoSize == 108 || InfoSize == 124)
		{
			fread(&sizes[x].Width, 4, 1, files[x]);
			fread(&sizes[x].Height, 4, 1, files[x]);
		}
		else return (enum GlueTogetherBitmap24_Error)(GLUE_TOGETHER_BITMAP24_FILE1_INVALID_BITMAPINFO_SIZE + x);

		if (sizes[x].Width < 1) return (enum GlueTogetherBitmap24_Error)(GLUE_TOGETHER_BITMAP24_FILE1_WIDTH_LESS_THAN_OR_EQUAL_TO_ZERO + x);

		fseek(files[x], 2, SEEK_CUR);
		fread(&BitCount, 2, 1, files[x]);

		if (BitCount != 24) return (enum GlueTogetherBitmap24_Error)(GLUE_TOGETHER_BITMAP24_FILE1_BITCOUNT_MUST_BE_24 + x);

		setvbuf(files[x], 0, _IONBF, 0);

		fseek(files[x], OffBits, SEEK_SET);
	}

	return GLUE_TOGETHER_BITMAP24_NO_ERRORS;
}

//Создает bmp файл с указанной шириной и высотой, указатель становится в положение пиксельных данных
static FILE* _CreateBitmap24InfoV3(const char* outFilePath, struct BitmapSize size, int lineEndZeroCount)
{
	FILE* outFile = fopen(outFilePath, "wb");

	if (outFile == NULL) return outFile;

	setvbuf(outFile, writeBuffer, _IOFBF, WRITE_BUFFER_SIZE);

	uint32_t x;

	//BITMAP_FILE_HEADER
	_FWRITE_VAL(x, 0x4d42, 2, outFile); //Type
	_FWRITE_VAL(x, 0x36 + 3 * (size.Width + lineEndZeroCount) * size.Height, 4, outFile); //Size
	_FWRITE_VAL(x, 0, 2, outFile); //Reserved1
	_FWRITE_VAL(x, 0, 2, outFile); //Reserved2
	_FWRITE_VAL(x, 0x36, 4, outFile); //OffBits

	//BITMAP_INFO_V3
	_FWRITE_VAL(x, 40, 4, outFile); //Size
	fwrite(&size.Width, 4, 1, outFile); //Width
	fwrite(&size.Height, 4, 1, outFile); //Height
	_FWRITE_VAL(x, 1, 2, outFile); //Planes
	_FWRITE_VAL(x, 24, 2, outFile); //BitCount
	x = 0;
	fwrite(&x, 4, 1, outFile); //Compression
	fwrite(&x, 4, 1, outFile); //SizeImage
	fwrite(&x, 4, 1, outFile); //XPelsPerMeter
	fwrite(&x, 4, 1, outFile); //YPelsPerMeter
	fwrite(&x, 4, 1, outFile); //ClrUsed
	fwrite(&x, 4, 1, outFile); //ClrImportant

	return outFile;
}

static void InitTurnedSizes(struct BitmapSize sizes[3], char angles[3], struct BitmapSize* turnedSizes)
{
	if (angles[0] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_0 || angles[0] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_180)
		turnedSizes[0] = sizes[0];
	else
	{
		turnedSizes[0].Width = sizes[0].Height;
		turnedSizes[0].Height = sizes[0].Width;
	}

	if (angles[1] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_0 || angles[1] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_180)
		turnedSizes[1] = sizes[1];
	else
	{
		turnedSizes[1].Width = sizes[1].Height;
		turnedSizes[1].Height = sizes[1].Width;
	}

	if (angles[2] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_0 || angles[2] == GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_180)
		turnedSizes[2] = sizes[2];
	else
	{
		turnedSizes[2].Width = sizes[2].Height;
		turnedSizes[2].Height = sizes[2].Width;
	}
}

static int CheckSizesForGlueTogether(struct BitmapSize turnedSizes[3], char glueMode)
{
	return (glueMode == 0 && (turnedSizes[0].Height != turnedSizes[1].Height || turnedSizes[0].Height != turnedSizes[2].Height)) ||
		   (glueMode == 1 && (turnedSizes[0].Width != turnedSizes[1].Width || turnedSizes[0].Width != turnedSizes[2].Width));
}

static struct BitmapSize GetBitmapSize(struct BitmapSize turnedSizes[3], char glueMode)
{
	struct BitmapSize res;

	if (glueMode == 0)
	{
		res.Width = turnedSizes[0].Width + turnedSizes[1].Width + turnedSizes[2].Width;
		res.Height = turnedSizes[0].Height;
	}
	else
	{
		res.Width = turnedSizes[0].Width;
		res.Height = turnedSizes[0].Height + turnedSizes[1].Height + turnedSizes[2].Height;
	}

	return res;
}

static char* GetPixel_0(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	return pixels + (3 * x + y * (3 * turnedSize.Width + lineEndZeroCount));
}
static char* GetPixel_90(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	return pixels + (3 * (turnedSize.Height - y - 1) + x * (3 * turnedSize.Height + lineEndZeroCount));
}
static char* GetPixel_180(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	return pixels + (3 * x + (turnedSize.Height - y - 1) * (3 * turnedSize.Width + lineEndZeroCount));
}
static char* GetPixel_270(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	return pixels + (3 * y + (turnedSize.Width - x - 1) * (3 * turnedSize.Height + lineEndZeroCount));
}

static char* GetInvPixel_0(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	pixels += (3 * x + y * (3 * turnedSize.Width + lineEndZeroCount));

	pixels[0] = 255 - pixels[0];
	pixels[1] = 255 - pixels[1];
	pixels[2] = 255 - pixels[2];

	return pixels;
}
static char* GetInvPixel_90(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	pixels += (3 * (turnedSize.Height - y - 1) + x * (3 * turnedSize.Height + lineEndZeroCount));

	pixels[0] = 255 - pixels[0];
	pixels[1] = 255 - pixels[1];
	pixels[2] = 255 - pixels[2];

	return pixels;
}
static char* GetInvPixel_180(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	pixels += (3 * x + (turnedSize.Height - y - 1) * (3 * turnedSize.Width + lineEndZeroCount));

	pixels[0] = 255 - pixels[0];
	pixels[1] = 255 - pixels[1];
	pixels[2] = 255 - pixels[2];

	return pixels;
}
static char* GetInvPixel_270(char* pixels, struct BitmapSize turnedSize, int lineEndZeroCount, int x, int y)
{
	pixels += (3 * y + (turnedSize.Width - x - 1) * (3 * turnedSize.Height + lineEndZeroCount));

	pixels[0] = 255 - pixels[0];
	pixels[1] = 255 - pixels[1];
	pixels[2] = 255 - pixels[2];

	return pixels;
}

static char*(*GetPixel[8])(char*, struct BitmapSize, int, int, int) = 
{
	GetPixel_0,
	GetPixel_90,
	GetPixel_180,
	GetPixel_270,

	GetInvPixel_0,
	GetInvPixel_90,
	GetInvPixel_180,
	GetInvPixel_270
};

static void GlueTogether(char* pixels[3], char glueMode, char colorModes[3], char angles[3], struct BitmapSize turnedSizes[3], int streamLineEndZeroCount, int lineEndZeroCounts[3], FILE* stream)
{
	char*(*GetImagePixel[3])(char*, struct BitmapSize, int, int, int) = 
	{
		GetPixel[angles[0] + colorModes[0] * 4],
		GetPixel[angles[1] + colorModes[1] * 4],
		GetPixel[angles[2] + colorModes[2] * 4]
	};

	if (glueMode == 0)
	{
		for (int y = 0; y < turnedSizes[0].Height; y++)
		{
			for (int i = 0; i < 3; i++)
				for (int x = 0; x < turnedSizes[i].Width; x++)
					fwrite(GetImagePixel[i](pixels[i], turnedSizes[i], lineEndZeroCounts[i], x, y), 3, 1, stream);

			for (int c = 0; c < streamLineEndZeroCount; c++)
				fputc(0, stream);
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int y = 0; y < turnedSizes[i].Height; y++)
			{
				for (int x = 0; x < turnedSizes[i].Width; x++)
					fwrite(GetImagePixel[i](pixels[i], turnedSizes[i], lineEndZeroCounts[i], x, y), 3, 1, stream);

				for (int c = 0; c < streamLineEndZeroCount; c++)
					fputc(0, stream);
			}
		}
	}
}

int GlueTogetherBitmap24(const char* outFilePath, const char* sourceFilePaths[3], char glueMode, char colorModes[3], char angles[3])
{
	FILE* inputFiles[3];

	for (char x = 0; x < 3; x++)
	{
		inputFiles[x] = fopen(sourceFilePaths[x], "rb");

		if (inputFiles[x] == NULL)
		{
			for (char y = x - 1; y >= 0; y--) fclose(inputFiles[y]);

			return (enum GlueTogetherBitmap24_Error)(GLUE_TOGETHER_BITMAP24_FILE1_OPEN_ERROR + x);
		}
	}

	struct BitmapSize sizes[3];

	int err = _BitmapGetSizesAndSeekToOffBits(inputFiles, sizes);

	if (err != GLUE_TOGETHER_BITMAP24_NO_ERRORS)
	{
		fclose(inputFiles[0]);
		fclose(inputFiles[1]);
		fclose(inputFiles[2]);

		return err;
	}

	struct BitmapSize turnedSizes[3];
	InitTurnedSizes(sizes, angles, turnedSizes);

	if (CheckSizesForGlueTogether(turnedSizes, glueMode))
	{
		fclose(inputFiles[0]);
		fclose(inputFiles[1]);
		fclose(inputFiles[2]);

		return GLUE_TOGETHER_BITMAP24_INCORRECT_SIZES_OF_IMAGES_FOR_GLUING;
	}

	struct BitmapSize size = GetBitmapSize(turnedSizes, glueMode);

	int lineEndZeroCount = _ZERO_COUNT(size.Width);
	int inputFileLineEndZeroCounts[3] = { _ZERO_COUNT(sizes[0].Width), _ZERO_COUNT(sizes[1].Width), _ZERO_COUNT(sizes[2].Width) };
	
	FILE* outFile = _CreateBitmap24InfoV3(outFilePath, size, lineEndZeroCount);

	if (outFile == NULL)
	{
		fclose(inputFiles[0]);
		fclose(inputFiles[1]);
		fclose(inputFiles[2]);

		return GLUE_TOGETHER_BITMAP24_OUT_FILE_OPEN_ERROR;
	}

	size_t bytesSize[3] =
	{
		(3 * sizes[0].Width + inputFileLineEndZeroCounts[0]) * sizes[0].Height,
		(3 * sizes[1].Width + inputFileLineEndZeroCounts[1]) * sizes[1].Height,
		(3 * sizes[2].Width + inputFileLineEndZeroCounts[2]) * sizes[2].Height
	};

	char* mem = (char*)malloc(bytesSize[0] + bytesSize[1] + bytesSize[2]);

	if (mem == NULL)
	{
		fclose(inputFiles[0]);
		fclose(inputFiles[1]);
		fclose(inputFiles[2]);

		fclose(outFile);

		return GLUE_TOGETHER_BITMAP24_OUT_OF_MEMORY;
	}

	char* pixels[3];

	pixels[0] = mem;
	pixels[1] = pixels[0] + bytesSize[0];
	pixels[2] = pixels[1] + bytesSize[1];

	fread(pixels[0], 1, bytesSize[0], inputFiles[0]);
	fread(pixels[1], 1, bytesSize[1], inputFiles[1]);
	fread(pixels[2], 1, bytesSize[2], inputFiles[2]);

	fclose(inputFiles[0]);
	fclose(inputFiles[1]);
	fclose(inputFiles[2]);

	GlueTogether(pixels, glueMode, colorModes, angles, turnedSizes, lineEndZeroCount, inputFileLineEndZeroCounts, outFile);

	free(mem);
	fclose(outFile);

	return GLUE_TOGETHER_BITMAP24_NO_ERRORS;
}

const char* GlueTogetherBitmap24_GetErrorString(int err)
{
	return GlueTogetherBitmap24_ErrorString[err];
}