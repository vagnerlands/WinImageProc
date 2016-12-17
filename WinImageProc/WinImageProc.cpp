/*
* Based on the example of Guillaume Cottenceau
*  Algorithm to compare 2 similar PNG files and return how many 
* differences are between the 2 images
*       by Vagner Paludo Landskron (vagnerlands@gmail.com)
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <queue>

using namespace std;

struct SPngFile
{
	int width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *pPixels;
};

struct SNodeToVisit
{
	int x;
	int y;
};

char** pDiffBuffer;

void loadPngFile(SPngFile* inputPng, char *filename) {
	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	png_read_info(png, info);

	inputPng->width = png_get_image_width(png, info);
	inputPng->height = png_get_image_height(png, info);
	inputPng->color_type = png_get_color_type(png, info);
	inputPng->bit_depth = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if (inputPng->bit_depth == 16)
		png_set_strip_16(png);

	if (inputPng->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if (inputPng->color_type == PNG_COLOR_TYPE_GRAY && inputPng->bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (inputPng->color_type == PNG_COLOR_TYPE_RGB ||
		inputPng->color_type == PNG_COLOR_TYPE_GRAY ||
		inputPng->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if (inputPng->color_type == PNG_COLOR_TYPE_GRAY ||
		inputPng->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	inputPng->pPixels = (png_bytep*)malloc(sizeof(png_bytep) * inputPng->height);
	for (int y = 0; y < inputPng->height; y++) {
		inputPng->pPixels[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, inputPng->pPixels);

	fclose(fp);
}
/*
void write_png_file(char *filename) {
	int y;

	FILE *fp = fopen(filename, "wb");
	if (!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		width, height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	//png_set_filler(png, 0, PNG_FILLER_AFTER);

	png_write_image(png, pLeftFile);
	png_write_end(png, NULL);

	for (int y = 0; y < height; y++) {
		free(pLeftFile[y]);
	}
	free(pLeftFile);

	fclose(fp);
}
*/

/*

 // Recursive method - simply raised to many stack overflows...

void markClustersAsVisited(char** pDiffBuffer, int dx, int dy, int maxX, int maxY)
{
	// check all neighbors
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			if (((dy+y) >= 0) && ((dy+y) < maxY) && ((dx+x) >= 0) && ((dx + x) < maxX))
			{
				if (pDiffBuffer[(dy + y)][(dx + x)] == 1)
				{
					pDiffBuffer[(dy + y)][(dx + x)] = 2;
					markClustersAsVisited(pDiffBuffer, (dx + x), (dy + y), maxX, maxY);
				}
			}
		}
	}

}
*/

bool process_png_file(SPngFile* l, SPngFile* r) {
	bool retVal = true;
	if ((l->bit_depth != r->bit_depth) || (l->color_type != r->color_type) || (l->height != r->height) || (l->width != r->width))
	{
		retVal = false;
		printf(" [!] Files can't be compared with this algorithm...\n > Depth = %d\n > Color Type = %d\n > Width = %d\n > Height = %d\n",
			l->bit_depth, l->color_type, l->width, l->height);
	}

	if (retVal == true)
	{
		pDiffBuffer = new char*[l->height];
		for (int di = 0; di < l->height; di++)
		{
			pDiffBuffer[di] = new char[l->width];
		}

		for (int dy = 0; dy < l->height; dy++)
		{
			for (int dx = 0; dx < l->width; dx++)
			{
				pDiffBuffer[dy][dx] = 0;
			}
		}

		for (int y = 0; y < l->height; y++) {
			png_bytep rowLeft = l->pPixels[y];
			png_bytep rowRight = r->pPixels[y];
			for (int x = 0; x < l->width; x++) {
				png_bytep pxLeft = &(rowLeft[x * 4]);
				png_bytep pxRight = &(rowRight[x * 4]);
				if ((pxLeft[0] != pxRight[0]) || (pxLeft[1] != pxRight[1]) || (pxLeft[2] != pxRight[2]))
				{
					// mark it as different
					pDiffBuffer[y][x] = 1;
				}
				//px[0] = (px[0] * 2) % 256;
				//px[4] = 100;
				// Do something awesome for each pixel here...
				//printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
			}
		}

		int totalClusters = 0;

		// perform cluster count
		for (int dy = 0; dy < l->height; dy++)
		{
			for (int dx = 0; dx < l->width; dx++)
			{
				// diff found
				if (pDiffBuffer[dy][dx] == 1)
				{
					// increase clusters found
					totalClusters++;
					pDiffBuffer[dy][dx] = 2;
					queue<SNodeToVisit> localQ;
					SNodeToVisit newIt;
					newIt.x = dx;
					newIt.y = dy;
					localQ.push(newIt);
					bool pxDiff = true;
					while (pxDiff)
					{
						newIt = localQ.front();
						localQ.pop();
						if (localQ.size() == 0)
						{
							pxDiff = false;
						}
						// check all neighbors
						for (int y = -1; y <= 1; y++)
						{
							for (int x = -1; x <= 1; x++)
							{
								if (((newIt.y + y) >= 0) && ((newIt.y + y) < l->height) && ((newIt.x + x) >= 0) && ((newIt.x + x) < l->width))
								{
									if (pDiffBuffer[(newIt.y + y)][(newIt.x + x)] == 1)
									{
										pxDiff = true;
										pDiffBuffer[(newIt.y + y)][(newIt.x + x)] = 2;
										SNodeToVisit newIt2;
										newIt2.x = newIt.x + x;
										newIt2.y = newIt.y + y;
										localQ.push(newIt2);
										//markClustersAsVisited(pDiffBuffer, (dx + x), (dy + y), maxX, maxY);
									}
								}
							}
						}
					}
					//markClustersAsVisited(pDiffBuffer, dx, dy, l->width, l->height);
				} 
				else
				{
					// if no cluster found - mark the node as visited
					pDiffBuffer[dy][dx] = 2;
				}
			}
		}

		printf(" > Total differences: %d\n\n", totalClusters);

		printf(" [*] Freeing temporary buffers ...\n");
		//release diff buffer
		delete[] pDiffBuffer;
		pDiffBuffer = 0;
	}

	return retVal;

}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Compare two (2) .png images and declare how many differences are between them.\n");
		printf("    Dev. by Vagner Landskron (vagnerlands@gmail.com)\n\n");
		printf(" Syntax: WinImageProc.exe <image1.png> <image2.png>\n");
		exit(1);
	}

	SPngFile leftFile;
	SPngFile rightFile;

	printf(" [input 1] Loading %s\n", argv[1]);
	loadPngFile(&leftFile, argv[1]);
	printf(" [input 2] Loading %s\n", argv[2]);
	loadPngFile(&rightFile, argv[2]);

	printf(" > Start Process ...\n");
	process_png_file(&leftFile, &rightFile);

	printf(" [*] Freeing PNG files ...\n");
	for (int y = 0; y < leftFile.height; y++) {
		free(leftFile.pPixels[y]);
	}
	free(leftFile.pPixels);
	//write_png_file(argv[2]);

	return 0;
}