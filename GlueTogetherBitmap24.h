#pragma once

#define GLUE_TOGETHER_BITMAP24_OPTION_HORIZONTAL 0
#define GLUE_TOGETHER_BITMAP24_OPTION_VERTICAL   1

#define GLUE_TOGETHER_BITMAP24_OPTION_NORMAL_COLOR   0
#define GLUE_TOGETHER_BITMAP24_OPTION_INVERTED_COLOR 1

#define GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_0   0
#define GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_90  1
#define GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_180 2
#define GLUE_TOGETHER_BITMAP24_OPTION_ANGLE_270 3

enum GlueTogetherBitmap24_Error
{
	GLUE_TOGETHER_BITMAP24_NO_ERRORS,

	GLUE_TOGETHER_BITMAP24_FILE1_OPEN_ERROR,
	GLUE_TOGETHER_BITMAP24_FILE2_OPEN_ERROR,
	GLUE_TOGETHER_BITMAP24_FILE3_OPEN_ERROR,

	GLUE_TOGETHER_BITMAP24_FILE1_INVALID_BITMAPINFO_SIZE,
	GLUE_TOGETHER_BITMAP24_FILE2_INVALID_BITMAPINFO_SIZE,
	GLUE_TOGETHER_BITMAP24_FILE3_INVALID_BITMAPINFO_SIZE,

	GLUE_TOGETHER_BITMAP24_FILE1_WIDTH_LESS_THAN_OR_EQUAL_TO_ZERO,
	GLUE_TOGETHER_BITMAP24_FILE2_WIDTH_LESS_THAN_OR_EQUAL_TO_ZERO,
	GLUE_TOGETHER_BITMAP24_FILE3_WIDTH_LESS_THAN_OR_EQUAL_TO_ZERO,

	GLUE_TOGETHER_BITMAP24_FILE1_BITCOUNT_MUST_BE_24,
	GLUE_TOGETHER_BITMAP24_FILE2_BITCOUNT_MUST_BE_24,
	GLUE_TOGETHER_BITMAP24_FILE3_BITCOUNT_MUST_BE_24,

	GLUE_TOGETHER_BITMAP24_INCORRECT_SIZES_OF_IMAGES_FOR_GLUING,

	GLUE_TOGETHER_BITMAP24_OUT_OF_MEMORY,

	GLUE_TOGETHER_BITMAP24_OUT_FILE_OPEN_ERROR
};

int GlueTogetherBitmap24(const char* outFilePath, const char* sourceFilePaths[3], char glueMode, char colorModes[3], char angles[3]);

const char* GlueTogetherBitmap24_GetErrorString(int err);