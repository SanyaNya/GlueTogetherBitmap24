#include <stdio.h>
#include <string.h>
#include "GlueTogetherBitmap24.h"

#define FILE_PATH_MAX_LEN 512
#define FILE_PATHS_COUNT 3

char ScanOption_glueMode()
{
	printf("\nChoose glue together options:\n");
	printf("0 - Horizontal\n");
	printf("1 - Vertical\n");

	int option;

	while (1)
	{
		printf(">");
		scanf("%i", &option);

		if (option != 0 && option != 1)
			printf("Option %i does not exist\n", option);
		else break;
	}

	return option;
}

char ScanOption_colorMode()
{
	printf("\nChoose color options:\n");
	printf("0 - Normal\n");
	printf("1 - Inverted\n");

	int option;

	while (1)
	{
		printf(">");
		scanf("%i", &option);

		if (option != 0 && option != 1)
			printf("Option %i does not exist\n", option);
		else break;
	}

	return option;
}

char ScanOption_angle()
{
	printf("\nChoose color options:\n");
	printf("0 - 0 degrees\n");
	printf("1 - 90 degrees\n");
	printf("2 - 180 degrees\n");
	printf("3 - 270 degrees\n");

	int option;

	while (1)
	{
		printf(">");
		scanf("%i", &option);

		if (option < 0 || option > 3)
			printf("Option %i does not exist\n", option);
		else break;
	}

	return option;
}

int main()
{
	char glueMode = ScanOption_glueMode();

	char colorModes[3];
	char angles[3];

	const char* filePaths[FILE_PATHS_COUNT];
	char buffer[FILE_PATHS_COUNT * FILE_PATH_MAX_LEN];

	for (size_t x = 0, bufferOffset = 0; x < FILE_PATHS_COUNT; x++, bufferOffset += strlen(buffer + bufferOffset) + 1)
	{
		printf("\nPrint file %i path:\n", x + 1);

		filePaths[x] = buffer + bufferOffset;
		printf(">");
		scanf("%s", buffer + bufferOffset);

		colorModes[x] = ScanOption_colorMode();

		angles[x] = ScanOption_angle();
	}

	char outFilePath[FILE_PATH_MAX_LEN];

	printf("\nPrint out file path:\n");
	printf(">");
	scanf("%s", outFilePath);

	int err = GlueTogetherBitmap24(outFilePath, filePaths, glueMode, colorModes, angles);

	if (err != GLUE_TOGETHER_BITMAP24_NO_ERRORS)
	{
		printf(GlueTogetherBitmap24_GetErrorString(err));
		getchar(); getchar();
	}

	return 0;
}