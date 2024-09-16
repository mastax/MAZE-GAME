/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* args[] )
{
    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );

            //Update the surface
            SDL_UpdateWindowSurface( window );

            //Hack to get window to stay up
            SDL_Event e; bool quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }
        }
    }

    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}
static SDL_Renderer *renderer;
static color_t *colorBuffer;
static SDL_Texture *colorBufferTexture;
static SDL_Window *window;

/**
 * initializeWindow - Initialize window to display the maze
 * Return: true in case of success, false if it fails
*/
bool initializeWindow(void)
{
	SDL_DisplayMode display_mode;
	int fullScreenWidth, fullScreenHeight;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL.\n");
		return (false);
	}
	SDL_GetCurrentDisplayMode(0, &display_mode);
	fullScreenWidth = display_mode.w;
	fullScreenHeight = display_mode.h;
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		fullScreenWidth,
		fullScreenHeight,
		SDL_WINDOW_BORDERLESS
	);
	if (!window)
	{
		fprintf(stderr, "Error creating SDL window.\n");
		return (false);
	}
	renderer = SDL_CreateRenderer(window, -1, 1);
	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer.\n");
		return (false);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	/* allocate the total amount of bytes in memory to hold our colorbuffer */
	colorBuffer = malloc(sizeof(color_t) * SCREEN_WIDTH * SCREEN_HEIGHT);

	/* create an SDL_Texture to display the colorbuffer */
	colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	return (true);
}

/**
 * destroyWindow - destroy window when the game is over
 *
*/

void destroyWindow(void)
{
	free(colorBuffer);
	SDL_DestroyTexture(colorBufferTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


/**
 * clearColorBuffer - clear buffer for every frame
 * @color: color buffer
*/

void clearColorBuffer(color_t color)
{
	int i;

	for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
		colorBuffer[i] = color;
}

/**
 * renderColorBuffer - render buffer for every frame
 *
*/

void renderColorBuffer(void)
{
	SDL_UpdateTexture(
		colorBufferTexture,
		NULL,
		colorBuffer,
		(int)(SCREEN_WIDTH * sizeof(color_t))
	);
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

/**
 * drawPixel - assign a color to each pixel
 * @x: x pixel coordinate
 * @y: y pixel coordinate
 * @color: pixel color
*/

void drawPixel(int x, int y, color_t color)
{
	colorBuffer[(SCREEN_WIDTH * y) + x] = color;
}

/**
 * changeColorIntensity - change color intensity
 * based on a factor value between 0 and 1
 * @factor: intensity factor
 * @color: color for intensity
*/

void changeColorIntensity(color_t *color, float factor)
{
	color_t a = (*color & 0xFF000000);
	color_t r = (*color & 0x00FF0000) * factor;
	color_t g = (*color & 0x0000FF00) * factor;
	color_t b = (*color & 0x000000FF) * factor;

	*color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}

/**
 * renderFloor - render floor projection
 *
 * @WallBottomPixel: wall bottom pixel
 * @texelColor: texture color for current pixel
 * @x: current element in the rays array
*/

void renderFloor(int wallBottomPixel, color_t *texelColor, int x)
{
	int y, texture_height, texture_width, textureOffsetY, textureOffsetX;
	float distance, ratio;

	texture_width = wallTextures[3].width;
	texture_height = wallTextures[3].height;

	for (y = wallBottomPixel - 1; y < SCREEN_HEIGHT; y++)
	{
		ratio = player.height / (y - SCREEN_HEIGHT / 2);
		distance = (ratio * PROJ_PLANE)
					/ cos(rays[x].rayAngle - player.rotationAngle);

		textureOffsetY = (int)fabs((distance * sin(rays[x].rayAngle)) + player.y);
		textureOffsetX = (int)fabs((distance * cos(rays[x].rayAngle)) + player.x);

		textureOffsetX = (int)(abs(textureOffsetX * texture_width / 30)
								% texture_width);
		textureOffsetY = (int)(abs(textureOffsetY * texture_height / 30)
								% texture_height);

		*texelColor = wallTextures[4].
					  texture_buffer[(texture_width * textureOffsetY) + textureOffsetX];
		drawPixel(x, y, *texelColor);
	}
}

/**
 * renderCeil - render Ceil projection
 * @WallTopPixel: wall top pixel
 * @texelColor: texture color for current pixel
 * @x: current element in the rays array
*/

void renderCeil(int wallTopPixel, color_t *texelColor, int x)
{
	int y, texture_width, texture_height, textureOffsetY, textureOffsetX;

	texture_width = wallTextures[3].width;
	texture_height = wallTextures[3].height;

	for (y = 0; y < wallTopPixel; y++)
	{
		float distance, ratio;

		ratio = player.height / (y - SCREEN_HEIGHT / 2);
		distance = (ratio * PROJ_PLANE)
					/ cos(rays[x].rayAngle - player.rotationAngle);

		textureOffsetY = (int)fabs((-distance * sin(rays[x].rayAngle)) + player.y);
		textureOffsetX = (int)fabs((-distance * cos(rays[x].rayAngle)) + player.x);

		textureOffsetX = (int)(abs(textureOffsetX * texture_width / 40)
								% texture_width);
		textureOffsetY = (int)(abs(textureOffsetY * texture_height / 40)
								% texture_height);

		*texelColor = wallTextures[6].
					  texture_buffer[(texture_width * textureOffsetY) + textureOffsetX];
		drawPixel(x, y, *texelColor);

	}
}

/**
 * renderWall - render wall projection
 *
*/
void renderWall(void)
{
	int x, y, texNum, texture_width, texture_height,
		textureOffsetX, wallBottomPixel, wallStripHeight,
		wallTopPixel, distanceFromTop, textureOffsetY;
	float perpDistance, projectedWallHeight;
	color_t texelColor;

	for (x = 0; x < NUM_RAYS; x++)
	{
		perpDistance = rays[x].distance * cos(rays[x].rayAngle
							- player.rotationAngle);
		projectedWallHeight = (TILE_SIZE / perpDistance) * PROJ_PLANE;
		wallStripHeight = (int)projectedWallHeight;
		wallTopPixel = (SCREEN_HEIGHT / 2) - (wallStripHeight / 2);
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;
		wallBottomPixel = (SCREEN_HEIGHT / 2) + (wallStripHeight / 2);
		wallBottomPixel = wallBottomPixel > SCREEN_HEIGHT
							? SCREEN_HEIGHT : wallBottomPixel;
		texNum = rays[x].wallHitContent - 1;
		texture_width = wallTextures[texNum].width;
		texture_height = wallTextures[texNum].height;
		renderFloor(wallBottomPixel, &texelColor, x);
		renderCeil(wallTopPixel, &texelColor, x);

		if (rays[x].wasHitVertical)
			textureOffsetX = (int)rays[x].wallHitY % TILE_SIZE;
		else
			textureOffsetX = (int)rays[x].wallHitX % TILE_SIZE;

		for (y = wallTopPixel; y < wallBottomPixel; y++)
		{
			distanceFromTop = y + (wallStripHeight / 2) - (SCREEN_HEIGHT / 2);
			textureOffsetY = distanceFromTop *
								((float)texture_height / wallStripHeight);
			texelColor = wallTextures[texNum].
						 texture_buffer[(texture_width * textureOffsetY) + textureOffsetX];
			if (rays[x].wasHitVertical)
				changeColorIntensity(&texelColor, 0.5);
			drawPixel(x, y, texelColor);
		}
	}
}
/**
* uPNG -- derived from LodePNG version 20100808
*
* Copyright (c) 2005-2010 Lode Vandevenne
* Copyright (c) 2010 Sean Middleditch
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
*		1. The origin of this software must not be misrepresented; you must not
*		claim that you wrote the original software. If you use this software
*		in a product, an acknowledgment in the product documentation would be
*		appreciated but is not required.
*
*		2. Altered source versions must be plainly marked as such, and must not be
*		misrepresented as being the original software.
*
*		3. This notice may not be removed or altered from any source
*		distribution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../headers/upng.h"

#define MAKE_BYTE(b) ((b) & 0xFF)
#define MAKE_DWORD(a,b,c,d) ((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I','H','D','R')
#define CHUNK_IDAT MAKE_DWORD('I','D','A','T')
#define CHUNK_IEND MAKE_DWORD('I','E','N','D')

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

#define NUM_DEFLATE_CODE_SYMBOLS 288
#define NUM_DISTANCE_SYMBOLS 32
#define NUM_CODE_LENGTH_CODES 19
#define MAX_SYMBOLS 288

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

#define SET_ERROR(upng,code) do { (upng)->error = (code); (upng)->error_line = __LINE__; } while (0)

#define upng_chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define upng_chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define upng_chunk_critical(chunk) (((chunk)[4] & 32) == 0)

typedef enum upng_state {
	UPNG_ERROR		= -1,
	UPNG_DECODED	= 0,
	UPNG_HEADER		= 1,
	UPNG_NEW		= 2
} upng_state;

typedef enum upng_color {
	UPNG_LUM		= 0,
	UPNG_RGB		= 2,
	UPNG_LUMA		= 4,
	UPNG_RGBA		= 6
} upng_color;

typedef struct upng_source {
	const unsigned char*	buffer;
	unsigned long			size;
	char					owning;
} upng_source;

struct upng_t {
	unsigned		width;
	unsigned		height;

	upng_color		color_type;
	unsigned		color_depth;
	upng_format		format;

	unsigned char*	buffer;
	unsigned long	size;

	upng_error		error;
	unsigned		error_line;

	upng_state		state;
	upng_source		source;
};

typedef struct huffman_tree {
	unsigned* tree2d;
	unsigned maxbitlen;
	unsigned numcodes;
} huffman_tree;

static const unsigned LENGTH_BASE[29] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
	67, 83, 99, 115, 131, 163, 195, 227, 258
};

static const unsigned LENGTH_EXTRA[29] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5,
	5, 5, 5, 0
};

static const unsigned DISTANCE_BASE[30] = {
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
	769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

static const unsigned DISTANCE_EXTRA[30] = {	/*the extra bits of backwards distances (added to base) */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
	11, 11, 12, 12, 13, 13
};

static const unsigned CLCL[NUM_CODE_LENGTH_CODES]	/*the order in which "code length alphabet code lengths" are stored, out of this the huffman tree of the dynamic huffman tree lengths is generated */
= { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

static const unsigned FIXED_DEFLATE_CODE_TREE[NUM_DEFLATE_CODE_SYMBOLS * 2] = {
	289, 370, 290, 307, 546, 291, 561, 292, 293, 300, 294, 297, 295, 296, 0, 1,
	2, 3, 298, 299, 4, 5, 6, 7, 301, 304, 302, 303, 8, 9, 10, 11, 305, 306, 12,
	13, 14, 15, 308, 339, 309, 324, 310, 317, 311, 314, 312, 313, 16, 17, 18,
	19, 315, 316, 20, 21, 22, 23, 318, 321, 319, 320, 24, 25, 26, 27, 322, 323,
	28, 29, 30, 31, 325, 332, 326, 329, 327, 328, 32, 33, 34, 35, 330, 331, 36,
	37, 38, 39, 333, 336, 334, 335, 40, 41, 42, 43, 337, 338, 44, 45, 46, 47,
	340, 355, 341, 348, 342, 345, 343, 344, 48, 49, 50, 51, 346, 347, 52, 53,
	54, 55, 349, 352, 350, 351, 56, 57, 58, 59, 353, 354, 60, 61, 62, 63, 356,
	363, 357, 360, 358, 359, 64, 65, 66, 67, 361, 362, 68, 69, 70, 71, 364,
	367, 365, 366, 72, 73, 74, 75, 368, 369, 76, 77, 78, 79, 371, 434, 372,
	403, 373, 388, 374, 381, 375, 378, 376, 377, 80, 81, 82, 83, 379, 380, 84,
	85, 86, 87, 382, 385, 383, 384, 88, 89, 90, 91, 386, 387, 92, 93, 94, 95,
	389, 396, 390, 393, 391, 392, 96, 97, 98, 99, 394, 395, 100, 101, 102, 103,
	397, 400, 398, 399, 104, 105, 106, 107, 401, 402, 108, 109, 110, 111, 404,
	419, 405, 412, 406, 409, 407, 408, 112, 113, 114, 115, 410, 411, 116, 117,
	118, 119, 413, 416, 414, 415, 120, 121, 122, 123, 417, 418, 124, 125, 126,
	127, 420, 427, 421, 424, 422, 423, 128, 129, 130, 131, 425, 426, 132, 133,
	134, 135, 428, 431, 429, 430, 136, 137, 138, 139, 432, 433, 140, 141, 142,
	143, 435, 483, 436, 452, 568, 437, 438, 445, 439, 442, 440, 441, 144, 145,
	146, 147, 443, 444, 148, 149, 150, 151, 446, 449, 447, 448, 152, 153, 154,
	155, 450, 451, 156, 157, 158, 159, 453, 468, 454, 461, 455, 458, 456, 457,
	160, 161, 162, 163, 459, 460, 164, 165, 166, 167, 462, 465, 463, 464, 168,
	169, 170, 171, 466, 467, 172, 173, 174, 175, 469, 476, 470, 473, 471, 472,
	176, 177, 178, 179, 474, 475, 180, 181, 182, 183, 477, 480, 478, 479, 184,
	185, 186, 187, 481, 482, 188, 189, 190, 191, 484, 515, 485, 500, 486, 493,
	487, 490, 488, 489, 192, 193, 194, 195, 491, 492, 196, 197, 198, 199, 494,
	497, 495, 496, 200, 201, 202, 203, 498, 499, 204, 205, 206, 207, 501, 508,
	502, 505, 503, 504, 208, 209, 210, 211, 506, 507, 212, 213, 214, 215, 509,
	512, 510, 511, 216, 217, 218, 219, 513, 514, 220, 221, 222, 223, 516, 531,
	517, 524, 518, 521, 519, 520, 224, 225, 226, 227, 522, 523, 228, 229, 230,
	231, 525, 528, 526, 527, 232, 233, 234, 235, 529, 530, 236, 237, 238, 239,
	532, 539, 533, 536, 534, 535, 240, 241, 242, 243, 537, 538, 244, 245, 246,
	247, 540, 543, 541, 542, 248, 249, 250, 251, 544, 545, 252, 253, 254, 255,
	547, 554, 548, 551, 549, 550, 256, 257, 258, 259, 552, 553, 260, 261, 262,
	263, 555, 558, 556, 557, 264, 265, 266, 267, 559, 560, 268, 269, 270, 271,
	562, 565, 563, 564, 272, 273, 274, 275, 566, 567, 276, 277, 278, 279, 569,
	572, 570, 571, 280, 281, 282, 283, 573, 574, 284, 285, 286, 287, 0, 0
};

static const unsigned FIXED_DISTANCE_TREE[NUM_DISTANCE_SYMBOLS * 2] = {
	33, 48, 34, 41, 35, 38, 36, 37, 0, 1, 2, 3, 39, 40, 4, 5, 6, 7, 42, 45, 43,
	44, 8, 9, 10, 11, 46, 47, 12, 13, 14, 15, 49, 56, 50, 53, 51, 52, 16, 17,
	18, 19, 54, 55, 20, 21, 22, 23, 57, 60, 58, 59, 24, 25, 26, 27, 61, 62, 28,
	29, 30, 31, 0, 0
};

static unsigned char read_bit(unsigned long *bitpointer, const unsigned char *bitstream)
{
	unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
	(*bitpointer)++;
	return result;
}

static unsigned read_bits(unsigned long *bitpointer, const unsigned char *bitstream, unsigned long nbits)
{
	unsigned result = 0, i;
	for (i = 0; i < nbits; i++)
		result |= ((unsigned)read_bit(bitpointer, bitstream)) << i;
	return result;
}

/* the buffer must be numcodes*2 in size! */
static void huffman_tree_init(huffman_tree* tree, unsigned* buffer, unsigned numcodes, unsigned maxbitlen)
{
	tree->tree2d = buffer;

	tree->numcodes = numcodes;
	tree->maxbitlen = maxbitlen;
}

/*given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate. maxbitlen is the maximum bits that a code in the tree can have. return value is error.*/
static void huffman_tree_create_lengths(upng_t* upng, huffman_tree* tree, const unsigned *bitlen)
{
	unsigned tree1d[MAX_SYMBOLS];
	unsigned blcount[MAX_BIT_LENGTH];
	unsigned nextcode[MAX_BIT_LENGTH+1];
	unsigned bits, n, i;
	unsigned nodefilled = 0;	/*up to which node it is filled */
	unsigned treepos = 0;	/*position in the tree (1 of the numcodes columns) */

	/* initialize local vectors */
	memset(blcount, 0, sizeof(blcount));
	memset(nextcode, 0, sizeof(nextcode));

	/*step 1: count number of instances of each code length */
	for (bits = 0; bits < tree->numcodes; bits++) {
		blcount[bitlen[bits]]++;
	}

	/*step 2: generate the nextcode values */
	for (bits = 1; bits <= tree->maxbitlen; bits++) {
		nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
	}

	/*step 3: generate all the codes */
	for (n = 0; n < tree->numcodes; n++) {
		if (bitlen[n] != 0) {
			tree1d[n] = nextcode[bitlen[n]]++;
		}
	}

	/*convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as many columns as codes - 1
	   a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what their 0 and 1 option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen */
	for (n = 0; n < tree->numcodes * 2; n++) {
		tree->tree2d[n] = 32767;	/*32767 here means the tree2d isn't filled there yet */
	}

	for (n = 0; n < tree->numcodes; n++) {	/*the codes */
		for (i = 0; i < bitlen[n]; i++) {	/*the bits for this code */
			unsigned char bit = (unsigned char)((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
			/* check if oversubscribed */
			if (treepos > tree->numcodes - 2) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}

			if (tree->tree2d[2 * treepos + bit] == 32767) {	/*not yet filled in */
				if (i + 1 == bitlen[n]) {	/*last bit */
					tree->tree2d[2 * treepos + bit] = n;	/*put the current code in it */
					treepos = 0;
				} else {	/*put address of the next step in here, first that address has to be found of course (it's just nodefilled + 1)... */
					nodefilled++;
					tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;	/*addresses encoded with numcodes added to it */
					treepos = nodefilled;
				}
			} else {
				treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
			}
		}
	}

	for (n = 0; n < tree->numcodes * 2; n++) {
		if (tree->tree2d[n] == 32767) {
			tree->tree2d[n] = 0;	/*remove possible remaining 32767's */
		}
	}
}

static unsigned huffman_decode_symbol(upng_t *upng, const unsigned char *in, unsigned long *bp, const huffman_tree* codetree, unsigned long inlength)
{
	unsigned treepos = 0, ct;
	unsigned char bit;
	for (;;) {
		/* error: end of input memory reached without endcode */
		if (((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return 0;
		}

		bit = read_bit(bp, in);

		ct = codetree->tree2d[(treepos << 1) | bit];
		if (ct < codetree->numcodes) {
			return ct;
		}

		treepos = ct - codetree->numcodes;
		if (treepos >= codetree->numcodes) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return 0;
		}
	}
}

/* get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
static void get_tree_inflate_dynamic(upng_t* upng, huffman_tree* codetree, huffman_tree* codetreeD, huffman_tree* codelengthcodetree, const unsigned char *in, unsigned long *bp, unsigned long inlength)
{
	unsigned codelengthcode[NUM_CODE_LENGTH_CODES];
	unsigned bitlen[NUM_DEFLATE_CODE_SYMBOLS];
	unsigned bitlenD[NUM_DISTANCE_SYMBOLS];
	unsigned n, hlit, hdist, hclen, i;

	/*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated */
	/*C-code note: use no "return" between ctor and dtor of an uivector! */
	if ((*bp) >> 3 >= inlength - 2) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	/* clear bitlen arrays */
	memset(bitlen, 0, sizeof(bitlen));
	memset(bitlenD, 0, sizeof(bitlenD));

	/*the bit pointer is or will go past the memory */
	hlit = read_bits(bp, in, 5) + 257;	/*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already */
	hdist = read_bits(bp, in, 5) + 1;	/*number of distance codes. Unlike the spec, the value 1 is added to it here already */
	hclen = read_bits(bp, in, 4) + 4;	/*number of code length codes. Unlike the spec, the value 4 is added to it here already */

	for (i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
		if (i < hclen) {
			codelengthcode[CLCL[i]] = read_bits(bp, in, 3);
		} else {
			codelengthcode[CLCL[i]] = 0;	/*if not, it must stay 0 */
		}
	}

	huffman_tree_create_lengths(upng, codelengthcodetree, codelengthcode);

	/* bail now if we encountered an error earlier */
	if (upng->error != UPNG_EOK) {
		return;
	}

	/*now we can use this tree to read the lengths for the tree that this function will return */
	i = 0;
	while (i < hlit + hdist) {	/*i is the current symbol we're reading in the part that contains the code lengths of lit/len codes and dist codes */
		unsigned code = huffman_decode_symbol(upng, in, bp, codelengthcodetree, inlength);
		if (upng->error != UPNG_EOK) {
			break;
		}

		if (code <= 15) {	/*a length code */
			if (i < hlit) {
				bitlen[i] = code;
			} else {
				bitlenD[i - hlit] = code;
			}
			i++;
		} else if (code == 16) {	/*repeat previous */
			unsigned replength = 3;	/*read in the 2 bits that indicate repeat length (3-6) */
			unsigned value;	/*set value to the previous code */

			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				break;
			}
			/*error, bit pointer jumps past memory */
			replength += read_bits(bp, in, 2);

			if ((i - 1) < hlit) {
				value = bitlen[i - 1];
			} else {
				value = bitlenD[i - hlit - 1];
			}

			/*repeat this value in the next lengths */
			for (n = 0; n < replength; n++) {
				/* i is larger than the amount of codes */
				if (i >= hlit + hdist) {
					SET_ERROR(upng, UPNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = value;
				} else {
					bitlenD[i - hlit] = value;
				}
				i++;
			}
		} else if (code == 17) {	/*repeat "0" 3-10 times */
			unsigned replength = 3;	/*read in the bits that indicate repeat length */
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				break;
			}

			/*error, bit pointer jumps past memory */
			replength += read_bits(bp, in, 3);

			/*repeat this value in the next lengths */
			for (n = 0; n < replength; n++) {
				/* error: i is larger than the amount of codes */
				if (i >= hlit + hdist) {
					SET_ERROR(upng, UPNG_EMALFORMED);
					break;
				}

				if (i < hlit) {
					bitlen[i] = 0;
				} else {
					bitlenD[i - hlit] = 0;
				}
				i++;
			}
		} else if (code == 18) {	/*repeat "0" 11-138 times */
			unsigned replength = 11;	/*read in the bits that indicate repeat length */
			/* error, bit pointer jumps past memory */
			if ((*bp) >> 3 >= inlength) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				break;
			}

			replength += read_bits(bp, in, 7);

			/*repeat this value in the next lengths */
			for (n = 0; n < replength; n++) {
				/* i is larger than the amount of codes */
				if (i >= hlit + hdist) {
					SET_ERROR(upng, UPNG_EMALFORMED);
					break;
				}
				if (i < hlit)
					bitlen[i] = 0;
				else
					bitlenD[i - hlit] = 0;
				i++;
			}
		} else {
			/* somehow an unexisting code appeared. This can never happen. */
			SET_ERROR(upng, UPNG_EMALFORMED);
			break;
		}
	}

	if (upng->error == UPNG_EOK && bitlen[256] == 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
	}

	/*the length of the end code 256 must be larger than 0 */
	/*now we've finally got hlit and hdist, so generate the code trees, and the function is done */
	if (upng->error == UPNG_EOK) {
		huffman_tree_create_lengths(upng, codetree, bitlen);
	}
	if (upng->error == UPNG_EOK) {
		huffman_tree_create_lengths(upng, codetreeD, bitlenD);
	}
}

/*inflate a block with dynamic of fixed Huffman tree*/
static void inflate_huffman(upng_t* upng, unsigned char* out, unsigned long outsize, const unsigned char *in, unsigned long *bp, unsigned long *pos, unsigned long inlength, unsigned btype)
{
	unsigned codetree_buffer[DEFLATE_CODE_BUFFER_SIZE];
	unsigned codetreeD_buffer[DISTANCE_BUFFER_SIZE];
	unsigned done = 0;

	huffman_tree codetree;
	huffman_tree codetreeD;

	if (btype == 1) {
		/* fixed trees */
		huffman_tree_init(&codetree, (unsigned*)FIXED_DEFLATE_CODE_TREE, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, (unsigned*)FIXED_DISTANCE_TREE, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
	} else if (btype == 2) {
		/* dynamic trees */
		unsigned codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
		huffman_tree codelengthcodetree;

		huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
		huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
		huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer, NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);
		get_tree_inflate_dynamic(upng, &codetree, &codetreeD, &codelengthcodetree, in, bp, inlength);
	}

	while (done == 0) {
		unsigned code = huffman_decode_symbol(upng, in, bp, &codetree, inlength);
		if (upng->error != UPNG_EOK) {
			return;
		}

		if (code == 256) {
			/* end code */
			done = 1;
		} else if (code <= 255) {
			/* literal symbol */
			if ((*pos) >= outsize) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}

			/* store output */
			out[(*pos)++] = (unsigned char)(code);
		} else if (code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX) {	/*length code */
			/* part 1: get length base */
			unsigned long length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];
			unsigned codeD, distance, numextrabitsD;
			unsigned long start, forward, backward, numextrabits;

			/* part 2: get extra bits and add the value of that to length */
			numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

			/* error, bit pointer will jump past memory */
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}
			length += read_bits(bp, in, numextrabits);

			/*part 3: get distance code */
			codeD = huffman_decode_symbol(upng, in, bp, &codetreeD, inlength);
			if (upng->error != UPNG_EOK) {
				return;
			}

			/* invalid distance code (30-31 are never used) */
			if (codeD > 29) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}

			distance = DISTANCE_BASE[codeD];

			/*part 4: get extra bits from distance */
			numextrabitsD = DISTANCE_EXTRA[codeD];

			/* error, bit pointer will jump past memory */
			if (((*bp) >> 3) >= inlength) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}

			distance += read_bits(bp, in, numextrabitsD);

			/*part 5: fill in all the out[n] values based on the length and dist */
			start = (*pos);
			backward = start - distance;

			if ((*pos) + length >= outsize) {
				SET_ERROR(upng, UPNG_EMALFORMED);
				return;
			}

			for (forward = 0; forward < length; forward++) {
				out[(*pos)++] = out[backward];
				backward++;

				if (backward >= start) {
					backward = start - distance;
				}
			}
		}
	}
}

static void inflate_uncompressed(upng_t* upng, unsigned char* out, unsigned long outsize, const unsigned char *in, unsigned long *bp, unsigned long *pos, unsigned long inlength)
{
	unsigned long p;
	unsigned len, nlen, n;

	/* go to first boundary of byte */
	while (((*bp) & 0x7) != 0) {
		(*bp)++;
	}
	p = (*bp) / 8;		/*byte position */

	/* read len (2 bytes) and nlen (2 bytes) */
	if (p >= inlength - 4) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	len = in[p] + 256 * in[p + 1];
	p += 2;
	nlen = in[p] + 256 * in[p + 1];
	p += 2;

	/* check if 16-bit nlen is really the one's complement of len */
	if (len + nlen != 65535) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	if ((*pos) + len >= outsize) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	/* read the literal data: len bytes are now stored in the out buffer */
	if (p + len > inlength) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	for (n = 0; n < len; n++) {
		out[(*pos)++] = in[p++];
	}

	(*bp) = p * 8;
}

static upng_error uz_inflate_data(upng_t* upng, unsigned char* out, unsigned long outsize, const unsigned char *in, unsigned long insize, unsigned long inpos)
{
	unsigned long bp = 0;
	unsigned long pos = 0;

	unsigned done = 0;

	while (done == 0) {
		unsigned btype;

		if ((bp >> 3) >= insize) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return upng->error;
		}

		done = read_bit(&bp, &in[inpos]);
		btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

		if (btype == 3) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return upng->error;
		} else if (btype == 0) {
			inflate_uncompressed(upng, out, outsize, &in[inpos], &bp, &pos, insize);	/*no compression */
		} else {
			inflate_huffman(upng, out, outsize, &in[inpos], &bp, &pos, insize, btype);	/*compression, btype 01 or 10 */
		}

		/* stop if an error has occured */
		if (upng->error != UPNG_EOK) {
			return upng->error;
		}
	}

	return upng->error;
}

static upng_error uz_inflate(upng_t* upng, unsigned char *out, unsigned long outsize, const unsigned char *in, unsigned long insize)
{
	/* we require two bytes for the zlib data header */
	if (insize < 2) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	if ((in[0] * 256 + in[1]) % 31 != 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	if (((in[1] >> 5) & 1) != 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}
	uz_inflate_data(upng, out, outsize, in, insize, 2);

	return upng->error;
}

static int paeth_predictor(int a, int b, int c)
{
	int p = a + b - c;
	int pa = p > a ? p - a : a - p;
	int pb = p > b ? p - b : b - p;
	int pc = p > c ? p - c : c - p;

	if (pa <= pb && pa <= pc)
		return a;
	else if (pb <= pc)
		return b;
	else
		return c;
}

static void unfilter_scanline(upng_t* upng, unsigned char *recon, const unsigned char *scanline, const unsigned char *precon, unsigned long bytewidth, unsigned char filterType, unsigned long length)
{
	unsigned long i;
	switch (filterType) {
	case 0:
		for (i = 0; i < length; i++)
			recon[i] = scanline[i];
		break;
	case 1:
		for (i = 0; i < bytewidth; i++)
			recon[i] = scanline[i];
		for (i = bytewidth; i < length; i++)
			recon[i] = scanline[i] + recon[i - bytewidth];
		break;
	case 2:
		if (precon)
			for (i = 0; i < length; i++)
				recon[i] = scanline[i] + precon[i];
		else
			for (i = 0; i < length; i++)
				recon[i] = scanline[i];
		break;
	case 3:
		if (precon) {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i] + precon[i] / 2;
			for (i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
		} else {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for (i = bytewidth; i < length; i++)
				recon[i] = scanline[i] + recon[i - bytewidth] / 2;
		}
		break;
	case 4:
		if (precon) {
			for (i = 0; i < bytewidth; i++)
				recon[i] = (unsigned char)(scanline[i] + paeth_predictor(0, precon[i], 0));
			for (i = bytewidth; i < length; i++)
				recon[i] = (unsigned char)(scanline[i] + paeth_predictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
		} else {
			for (i = 0; i < bytewidth; i++)
				recon[i] = scanline[i];
			for (i = bytewidth; i < length; i++)
				recon[i] = (unsigned char)(scanline[i] + paeth_predictor(recon[i - bytewidth], 0, 0));
		}
		break;
	default:
		SET_ERROR(upng, UPNG_EMALFORMED);
		break;
	}
}

static void unfilter(upng_t* upng, unsigned char *out, const unsigned char *in, unsigned w, unsigned h, unsigned bpp)
{
	unsigned y;
	unsigned char *prevline = 0;

	unsigned long bytewidth = (bpp + 7) / 8;
	unsigned long linebytes = (w * bpp + 7) / 8;

	for (y = 0; y < h; y++) {
		unsigned long outindex = linebytes * y;
		unsigned long inindex = (1 + linebytes) * y;
		unsigned char filterType = in[inindex];

		unfilter_scanline(upng, &out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
		if (upng->error != UPNG_EOK) {
			return;
		}

		prevline = &out[outindex];
	}
}

static void remove_padding_bits(unsigned char *out, const unsigned char *in, unsigned long olinebits, unsigned long ilinebits, unsigned h)
{
	unsigned y;
	unsigned long diff = ilinebits - olinebits;
	unsigned long obp = 0, ibp = 0;	/*bit pointers */
	for (y = 0; y < h; y++) {
		unsigned long x;
		for (x = 0; x < olinebits; x++) {
			unsigned char bit = (unsigned char)((in[(ibp) >> 3] >> (7 - ((ibp) & 0x7))) & 1);
			ibp++;

			if (bit == 0)
				out[(obp) >> 3] &= (unsigned char)(~(1 << (7 - ((obp) & 0x7))));
			else
				out[(obp) >> 3] |= (1 << (7 - ((obp) & 0x7)));
			++obp;
		}
		ibp += diff;
	}
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from the IDAT chunks*/
static void post_process_scanlines(upng_t* upng, unsigned char *out, unsigned char *in, const upng_t* info_png)
{
	unsigned bpp = upng_get_bpp(info_png);
	unsigned w = info_png->width;
	unsigned h = info_png->height;

	if (bpp == 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return;
	}

	if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
		unfilter(upng, in, in, w, h, bpp);
		if (upng->error != UPNG_EOK) {
			return;
		}
		remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
	} else {
		unfilter(upng, out, in, w, h, bpp);
	}
}

static upng_format determine_format(upng_t* upng) {
	switch (upng->color_type) {
	case UPNG_LUM:
		switch (upng->color_depth) {
		case 1:
			return UPNG_LUMINANCE1;
		case 2:
			return UPNG_LUMINANCE2;
		case 4:
			return UPNG_LUMINANCE4;
		case 8:
			return UPNG_LUMINANCE8;
		default:
			return UPNG_BADFORMAT;
		}
	case UPNG_RGB:
		switch (upng->color_depth) {
		case 8:
			return UPNG_RGB8;
		case 16:
			return UPNG_RGB16;
		default:
			return UPNG_BADFORMAT;
		}
	case UPNG_LUMA:
		switch (upng->color_depth) {
		case 1:
			return UPNG_LUMINANCE_ALPHA1;
		case 2:
			return UPNG_LUMINANCE_ALPHA2;
		case 4:
			return UPNG_LUMINANCE_ALPHA4;
		case 8:
			return UPNG_LUMINANCE_ALPHA8;
		default:
			return UPNG_BADFORMAT;
		}
	case UPNG_RGBA:
		switch (upng->color_depth) {
		case 8:
			return UPNG_RGBA8;
		case 16:
			return UPNG_RGBA16;
		default:
			return UPNG_BADFORMAT;
		}
	default:
		return UPNG_BADFORMAT;
	}
}

static void upng_free_source(upng_t* upng)
{
	if (upng->source.owning != 0) {
		free((void*)upng->source.buffer);
	}

	upng->source.buffer = NULL;
	upng->source.size = 0;
	upng->source.owning = 0;
}


upng_error upng_header(upng_t* upng)
{
	if (upng->error != UPNG_EOK) {
		return upng->error;
	}

	if (upng->state != UPNG_NEW) {
		return upng->error;
	}

	if (upng->source.size < 29) {
		SET_ERROR(upng, UPNG_ENOTPNG);
		return upng->error;
	}

	if (upng->source.buffer[0] != 137 || upng->source.buffer[1] != 80 || upng->source.buffer[2] != 78 || upng->source.buffer[3] != 71 || upng->source.buffer[4] != 13 || upng->source.buffer[5] != 10 || upng->source.buffer[6] != 26 || upng->source.buffer[7] != 10) {
		SET_ERROR(upng, UPNG_ENOTPNG);
		return upng->error;
	}

	/* check that the first chunk is the IHDR chunk */
	if (MAKE_DWORD_PTR(upng->source.buffer + 12) != CHUNK_IHDR) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	/* read the values given in the header */
	upng->width = MAKE_DWORD_PTR(upng->source.buffer + 16);
	upng->height = MAKE_DWORD_PTR(upng->source.buffer + 20);
	upng->color_depth = upng->source.buffer[24];
	upng->color_type = (upng_color)upng->source.buffer[25];

	/* determine our color format */
	upng->format = determine_format(upng);
	if (upng->format == UPNG_BADFORMAT) {
		SET_ERROR(upng, UPNG_EUNFORMAT);
		return upng->error;
	}

	if (upng->source.buffer[26] != 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	/* check that the compression method (byte 27) is 0 (only allowed value in spec) */
	if (upng->source.buffer[27] != 0) {
		SET_ERROR(upng, UPNG_EMALFORMED);
		return upng->error;
	}

	/* check that the compression method (byte 27) is 0 (spec allows 1, but uPNG does not support it) */
	if (upng->source.buffer[28] != 0) {
		SET_ERROR(upng, UPNG_EUNINTERLACED);
		return upng->error;
	}

	upng->state = UPNG_HEADER;
	return upng->error;
}

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
upng_error upng_decode(upng_t* upng)
{
	const unsigned char *chunk, *data;
	unsigned char* compressed;
	unsigned char* inflated;
	unsigned long compressed_size = 0, compressed_index = 0;
	unsigned long inflated_size;
	unsigned long length;
	upng_error error;

	(void)data;

	/* if we have an error state, bail now */
	if (upng->error != UPNG_EOK) {
		return upng->error;
	}

	/* parse the main header, if necessary */
	upng_header(upng);
	if (upng->error != UPNG_EOK) {
		return upng->error;
	}

	/* if the state is not HEADER (meaning we are ready to decode the image), stop now */
	if (upng->state != UPNG_HEADER) {
		return upng->error;
	}

	/* release old result, if any */
	if (upng->buffer != 0) {
		free(upng->buffer);
		upng->buffer = 0;
		upng->size = 0;
	}

	/* first byte of the first chunk after the header */
	chunk = upng->source.buffer + 33;

	/* scan through the chunks, finding the size of all IDAT chunks, and also
	 * verify general well-formed-ness */
	while (chunk < upng->source.buffer + upng->source.size) {

		/* make sure chunk header is not larger than the total compressed */
		if ((unsigned long)(chunk - upng->source.buffer + 12) > upng->source.size) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return upng->error;
		}

		/* get length; sanity check it */
		length = upng_chunk_length(chunk);
		if (length > INT_MAX) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return upng->error;
		}

		/* make sure chunk header+paylaod is not larger than the total compressed */
		if ((unsigned long)(chunk - upng->source.buffer + length + 12) > upng->source.size) {
			SET_ERROR(upng, UPNG_EMALFORMED);
			return upng->error;
		}

		/* get pointer to payload */
		data = chunk + 8;

		/* parse chunks */
		if (upng_chunk_type(chunk) == CHUNK_IDAT) {
			compressed_size += length;
		} else if (upng_chunk_type(chunk) == CHUNK_IEND) {
			break;
		} else if (upng_chunk_critical(chunk)) {
			SET_ERROR(upng, UPNG_EUNSUPPORTED);
			return upng->error;
		}

		chunk += upng_chunk_length(chunk) + 12;
	}

	/* allocate enough space for the (compressed and filtered) image data */
	compressed = (unsigned char*)malloc(compressed_size);
	if (compressed == NULL) {
		SET_ERROR(upng, UPNG_ENOMEM);
		return upng->error;
	}

	/* scan through the chunks again, this time copying the values into
	 * our compressed buffer.  there's no reason to validate anything a second time. */
	chunk = upng->source.buffer + 33;
	while (chunk < upng->source.buffer + upng->source.size) {
		unsigned long length;
		const unsigned char *data;	/*the data in the chunk */

		length = upng_chunk_length(chunk);
		data = chunk + 8;

		/* parse chunks */
		if (upng_chunk_type(chunk) == CHUNK_IDAT) {
			memcpy(compressed + compressed_index, data, length);
			compressed_index += length;
		} else if (upng_chunk_type(chunk) == CHUNK_IEND) {
			break;
		}

		chunk += upng_chunk_length(chunk) + 12;
	}
	inflated_size = ((upng->width * (upng->height * upng_get_bpp(upng) + 7)) / 8) + upng->height;
	inflated = (unsigned char*)malloc(inflated_size);
	if (inflated == NULL) {
		free(compressed);
		SET_ERROR(upng, UPNG_ENOMEM);
		return upng->error;
	}

	error = uz_inflate(upng, inflated, inflated_size, compressed, compressed_size);
	if (error != UPNG_EOK) {
		free(compressed);
		free(inflated);
		return upng->error;
	}
	free(compressed);
	upng->size = (upng->height * upng->width * upng_get_bpp(upng) + 7) / 8;
	upng->buffer = (unsigned char*)malloc(upng->size);
	if (upng->buffer == NULL) {
		free(inflated);
		upng->size = 0;
		SET_ERROR(upng, UPNG_ENOMEM);
		return upng->error;
	}
	post_process_scanlines(upng, upng->buffer, inflated, upng);
	free(inflated);

	if (upng->error != UPNG_EOK) {
		free(upng->buffer);
		upng->buffer = NULL;
		upng->size = 0;
	} else {
		upng->state = UPNG_DECODED;
	}

	upng_free_source(upng);

	return upng->error;
}

static upng_t* upng_new(void)
{
	upng_t* upng;

	upng = (upng_t*)malloc(sizeof(upng_t));
	if (upng == NULL) {
		return NULL;
	}

	upng->buffer = NULL;
	upng->size = 0;

	upng->width = upng->height = 0;

	upng->color_type = UPNG_RGBA;
	upng->color_depth = 8;
	upng->format = UPNG_RGBA8;

	upng->state = UPNG_NEW;

	upng->error = UPNG_EOK;
	upng->error_line = 0;

	upng->source.buffer = NULL;
	upng->source.size = 0;
	upng->source.owning = 0;

	return upng;
}

upng_t* upng_new_from_bytes(const unsigned char* buffer, unsigned long size)
{
	upng_t* upng = upng_new();
	if (upng == NULL) {
		return NULL;
	}

	upng->source.buffer = buffer;
	upng->source.size = size;
	upng->source.owning = 0;

	return upng;
}

upng_t* upng_new_from_file(const char *filename)
{
	upng_t* upng;
	unsigned char *buffer;
	FILE *file;
	long size;

	upng = upng_new();
	if (upng == NULL) {
		return NULL;
	}

	file = fopen(filename, "rb");
	if (file == NULL) {
		SET_ERROR(upng, UPNG_ENOTFOUND);
		return upng;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	buffer = (unsigned char *)malloc((unsigned long)size);
	if (buffer == NULL) {
		fclose(file);
		SET_ERROR(upng, UPNG_ENOMEM);
		return upng;
	}
	fread(buffer, 1, (unsigned long)size, file);
	fclose(file);
	upng->source.buffer = buffer;
	upng->source.size = size;
	upng->source.owning = 1;

	return upng;
}

void upng_free(upng_t* upng)
{
	if (upng->buffer != NULL) {
		free(upng->buffer);
	}
	upng_free_source(upng);

	free(upng);
}

upng_error upng_get_error(const upng_t* upng)
{
	return upng->error;
}

unsigned upng_get_error_line(const upng_t* upng)
{
	return upng->error_line;
}

unsigned upng_get_width(const upng_t* upng)
{
	return upng->width;
}

unsigned upng_get_height(const upng_t* upng)
{
	return upng->height;
}

unsigned upng_get_bpp(const upng_t* upng)
{
	return upng_get_bitdepth(upng) * upng_get_components(upng);
}

unsigned upng_get_components(const upng_t* upng)
{
	switch (upng->color_type) {
	case UPNG_LUM:
		return 1;
	case UPNG_RGB:
		return 3;
	case UPNG_LUMA:
		return 2;
	case UPNG_RGBA:
		return 4;
	default:
		return 0;
	}
}

unsigned upng_get_bitdepth(const upng_t* upng)
{
	return upng->color_depth;
}

unsigned upng_get_pixelsize(const upng_t* upng)
{
	unsigned bits = upng_get_bitdepth(upng) * upng_get_components(upng);
	bits += bits % 8;
	return bits;
}

upng_format upng_get_format(const upng_t* upng)
{
	return upng->format;
}

const unsigned char* upng_get_buffer(const upng_t* upng)
{
	return upng->buffer;
}

unsigned upng_get_size(const upng_t* upng)
{
	return upng->size;
}

static const char *textureFileNames[NUM_TEXTURES] = {
	"./images/redbrick.png",
	"./images/purplestone.png",
	"./images/mossystone.png",
	"./images/graystone.png",
	"./images/colorstone.png",
	"./images/bluestone.png",
	"./images/wood.png",
	"./images/eagle.png",
};

/**
 * WallTexturesready - load textures in the respective position
 *
*/
void WallTexturesready(void)
{
	int i;

	for (i = 0; i < NUM_TEXTURES; i++)
	{
		upng_t *upng;

		upng = upng_new_from_file(textureFileNames[i]);

		if (upng != NULL)
		{
			upng_decode(upng);
			if (upng_get_error(upng) == UPNG_EOK)
			{
				wallTextures[i].upngTexture = upng;
				wallTextures[i].width = upng_get_width(upng);
				wallTextures[i].height = upng_get_height(upng);
				wallTextures[i].texture_buffer = (color_t *)upng_get_buffer(upng);
			}
		}
	}

}

/**
 * freeWallTextures - free wall textures
 *
*/

void freeWallTextures(void)
{
	int i;

	for (i = 0; i < NUM_TEXTURES; i++)
		upng_free(wallTextures[i].upngTexture);
}

ray_t rays[NUM_RAYS];

static bool foundHorzWallHit, foundVertWallHit;
static float horzWallHitX, horzWallHitY, vertWallHitX, vertWallHitY;
static int horzWallContent, vertWallContent;


/**
 * horzIntersection - Finds horizontal intersection with the wall
 * @rayAngle: current ray angle
 *
 */

void horzIntersection(float rayAngle)
{
	float nextHorzTouchX, nextHorzTouchY, xintercept, yintercept, xstep, ystep;

	foundHorzWallHit = false;
	horzWallHitX = horzWallHitY = horzWallContent = 0;

	yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown(rayAngle) ? TILE_SIZE : 0;

	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

	ystep = TILE_SIZE;
	ystep *= isRayFacingUp(rayAngle) ? -1 : 1;
	xstep = TILE_SIZE / tan(rayAngle);
	xstep *= (isRayFacingLeft(rayAngle) && xstep > 0) ? -1 : 1;
	xstep *= (isRayFacingRight(rayAngle) && xstep < 0) ? -1 : 1;
	nextHorzTouchX = xintercept;
	nextHorzTouchY = yintercept;

	while (isInsideMap(nextHorzTouchX, nextHorzTouchY))
	{
		float xToCheck = nextHorzTouchX;
		float yToCheck = nextHorzTouchY + (isRayFacingUp(rayAngle) ? -1 : 0);

		if (DetectCollision(xToCheck, yToCheck))
		{
			horzWallHitX = nextHorzTouchX;
			horzWallHitY = nextHorzTouchY;
			horzWallContent = getMapValue((int)floor(yToCheck / TILE_SIZE),
									   (int)floor(xToCheck / TILE_SIZE));
			foundHorzWallHit = true;
			break;
		}
		nextHorzTouchX += xstep;
		nextHorzTouchY += ystep;
	}
}

/**
 * vertIntersection - Finds vertical intersection with the wall
 * @rayAngle: current ray angle
 *
 */

void vertIntersection(float rayAngle)
{
	float nextVertTouchX, nextVertTouchY;
	float xintercept, yintercept, xstep, ystep;

	foundVertWallHit = false;
	vertWallHitX = 0;
	vertWallHitY = 0;
	vertWallContent = 0;

	xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	xintercept += isRayFacingRight(rayAngle) ? TILE_SIZE : 0;
	yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

	xstep = TILE_SIZE;
	xstep *= isRayFacingLeft(rayAngle) ? -1 : 1;
	ystep = TILE_SIZE * tan(rayAngle);
	ystep *= (isRayFacingUp(rayAngle) && ystep > 0) ? -1 : 1;
	ystep *= (isRayFacingDown(rayAngle) && ystep < 0) ? -1 : 1;
	nextVertTouchX = xintercept;
	nextVertTouchY = yintercept;

	while (isInsideMap(nextVertTouchX, nextVertTouchY))
	{
		float xToCheck = nextVertTouchX + (isRayFacingLeft(rayAngle) ? -1 : 0);
		float yToCheck = nextVertTouchY;

		if (DetectCollision(xToCheck, yToCheck))
		{
			vertWallHitX = nextVertTouchX;
			vertWallHitY = nextVertTouchY;
			vertWallContent = getMapValue((int)floor(yToCheck / TILE_SIZE),
									   (int)floor(xToCheck / TILE_SIZE));
			foundVertWallHit = true;
			break;
		}
		nextVertTouchX += xstep;
		nextVertTouchY += ystep;
	}
}

/**
 * castRay - casting of each ray
 * @rayAngle: current ray angle
 * @stripId: ray strip identifier
 */

void castRay(float rayAngle, int stripId)
{
	float horzHitDistance, vertHitDistance;

	rayAngle = remainder(rayAngle, TWO_PI);
	if (rayAngle < 0)
		rayAngle = TWO_PI + rayAngle;

	horzIntersection(rayAngle);

	vertIntersection(rayAngle);

	horzHitDistance = foundHorzWallHit
		? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
		: FLT_MAX;
	vertHitDistance = foundVertWallHit
		? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
		: FLT_MAX;

	if (vertHitDistance < horzHitDistance)
	{
		rays[stripId].distance = vertHitDistance;
		rays[stripId].wallHitX = vertWallHitX;
		rays[stripId].wallHitY = vertWallHitY;
		rays[stripId].wallHitContent = vertWallContent;
		rays[stripId].wasHitVertical = true;
		rays[stripId].rayAngle = rayAngle;
	}
	else
	{
		rays[stripId].distance = horzHitDistance;
		rays[stripId].wallHitX = horzWallHitX;
		rays[stripId].wallHitY = horzWallHitY;
		rays[stripId].wallHitContent = horzWallContent;
		rays[stripId].wasHitVertical = false;
		rays[stripId].rayAngle = rayAngle;
	}

}

/**
 * castAllRays - cast of all rays
 *
 */

void castAllRays(void)
{
	int col;

	for (col = 0; col < NUM_RAYS; col++)
	{
		float rayAngle = player.rotationAngle +
							atan((col - NUM_RAYS / 2) / PROJ_PLANE);
		castRay(rayAngle, col);
	}
}

/**
 * renderRays - draw all the rays
 *
 */

void renderRays(void)
{
	int i;

	for (i = 0; i < NUM_RAYS; i += 50)
	{
		drawLine(
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
			rays[i].wallHitY * MINIMAP_SCALE_FACTOR,
			0xFF0000FF
		);
	}
}

/**
 * distanceBetweenPoints - Finds horizontal intersection with the wall
 * @x1: x coordinate of the starting point
 * @y1: y coordinate oh the starting point
 * @x2: x coordinate of the end point
 * @y2: y coordinate of the end point
 * Return: the distance between two points
 */

float distanceBetweenPoints(float x1, float y1, float x2, float y2)
{
	return (sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2)));
}

/**
 * isRayFacingDown - check if the ray is facing down
 * @angle: current ray angle
 * Return: true or false
 */

bool isRayFacingDown(float angle)
{
	return (angle > 0 && angle < PI);
}

/**
 * isRayFacingUp - check if the ray is facing up
 * @angle: current ray angle
 * Return: true or false
 */

bool isRayFacingUp(float angle)
{
	return (!isRayFacingDown(angle));
}

/**
 * isRayFacingRight - check if the ray is facing to the right
 * @angle: current ray angle
 * Return: true or false
 */

bool isRayFacingRight(float angle)
{
	return (angle < 0.5 * PI || angle > 1.5 * PI);
}

/**
 * isRayFacingLeft - check if the ray is facing to the right
 * @angle: current ray angle
 * Return: true or false
 */

bool isRayFacingLeft(float angle)
{
	return (!isRayFacingRight(angle));
}
/**
 * movePlayer - set the next position of the player
 * @DeltaTime: time elapsed since the last frame
*/

void movePlayer(float DeltaTime)
{
	float moveStep, newPlayerX, newPlayerY;

	player.rotationAngle += player.turnDirection * player.turnSpeed * DeltaTime;
	moveStep = player.walkDirection * player.walkSpeed * DeltaTime;

	newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	newPlayerY = player.y + sin(player.rotationAngle) * moveStep;

	if (!DetectCollision(newPlayerX, newPlayerY))
	{
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
}

/**
 * renderPlayer - render the player
 *
*/

void renderPlayer(void)
{
	drawRect(
		player.x * MINIMAP_SCALE_FACTOR,
		player.y * MINIMAP_SCALE_FACTOR,
		player.width * MINIMAP_SCALE_FACTOR,
		player.height * MINIMAP_SCALE_FACTOR,
		0xFFFFFFFF
	);
}
static const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
	{6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6},
	{6, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 6, 0, 0, 0, 6, 0, 0, 0, 6},
	{6, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 6, 0, 7, 7, 0, 0, 0, 0, 6},
	{6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 7, 0, 6},
	{6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 6},
	{6, 0, 0, 0, 0, 0, 7, 7, 7, 0, 0, 1, 0, 0, 0, 0, 7, 7, 0, 6},
	{6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
	{6, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 1, 0, 6},
	{6, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 1, 0, 6},
	{6, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
	{6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
	{6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
};

/**
 * DetectCollision - Checks if there could be a collision
 * with the wall in the next player advance
 * @x: next x coordinate
 * @y: next y coordinate
 * Return: true if collision is detected, false otherwise
*/

bool DetectCollision(float x, float y)
{
	int mapGridX, mapGridY;

	if (x < 0 || x >= MAP_NUM_COLS * TILE_SIZE ||
			y < 0 || y >= MAP_NUM_ROWS * TILE_SIZE)
		return (true);

	mapGridX = floor(x / TILE_SIZE);
	mapGridY = floor(y / TILE_SIZE);
	return (map[mapGridY][mapGridX] != 0);
}

/**
 * isInsideMap - check if we continue within the map
 * @x: next x coordinate
 * @y: next y coordinate
 * @Return: true if it is within the map, false otherwise
*/

bool isInsideMap(float x, float y)
{
	return (x >= 0 && x <= MAP_NUM_COLS * TILE_SIZE &&
				y >= 0 && y <= MAP_NUM_ROWS * TILE_SIZE);
}

/**
 * getMapValue - check if we continue within the map
 * @row: map row to check
 * @col: map column to check
 * @Return: The position value in the map
*/

int getMapValue(int row, int col)
{

	return (map[row][col]);

}

/**
 * renderMap - render the map
 *
*/

void renderMap(void)
{
	int i, j, tileX, tileY;
	color_t tileColor;

	for (i = 0; i < MAP_NUM_ROWS; i++)
	{
		for (j = 0; j < MAP_NUM_COLS; j++)
		{
			tileX = j * TILE_SIZE;
			tileY = i * TILE_SIZE;
			tileColor = map[i][j] != 0 ? 0xFFFFFFFF : 0x00000000;

			drawRect(
				tileX * MINIMAP_SCALE_FACTOR,
				tileY * MINIMAP_SCALE_FACTOR,
				TILE_SIZE * MINIMAP_SCALE_FACTOR,
				TILE_SIZE * MINIMAP_SCALE_FACTOR,
				tileColor
			);
		}
	}
}
bool GameRunning = false;
int TicksLastFrame;
player_t player;

/**
 * setup_game - initialize player variables and load wall textures
 *
*/

void setup_game(void)
{

	player.x = SCREEN_WIDTH / 2;
	player.y = SCREEN_HEIGHT / 2;
	player.width = 1;
	player.height = 30;
	player.walkDirection = 0;
	player.walkSpeed = 100;
	player.turnDirection = 0;
	player.turnSpeed = 45 * (PI / 180);
	player.rotationAngle = PI / 2;
	WallTexturesready();
}


/**
 * update_game - update_game delta time, the ticks last frame
 *          the player movement and the ray casting
 *
*/
void update_game(void)
{
	float DeltaTime;
	int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - TicksLastFrame);

	if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH)
	{
		SDL_Delay(timeToWait);
	}
	DeltaTime = (SDL_GetTicks() - TicksLastFrame) / 1000.0f;

	TicksLastFrame = SDL_GetTicks();

	movePlayer(DeltaTime);
	castAllRays();
}

/**
 * render - calls all functions needed for on-screen rendering
 *
*/

void render_game(void)
{
	clearColorBuffer(0xFF000000);

	renderWall();

	renderMap();
	renderRays();
	renderPlayer();

	renderColorBuffer();
}

/**
 * Destroy - free wall textures and destroy window
 *
*/
void destroy_game(void)
{
	freeWallTextures();
	destroyWindow();
}

/**
 * main - main function
 * Return: 0
*/

int main(void)
{
	GameRunning = initializeWindow();

	setup_game();

	while (GameRunning)
	{
		handleInput();
		update_game();
		render_game();
	}
	destroy_game();
	return (0);
}
/**
 * SDL_KEYDOWN_FUNC - process input when a key is down
 * @event: union that contains structures for the different event types
*/

void SDL_KEYDOWN_FUNC(SDL_Event event)
{
	if (event.key.keysym.sym == SDLK_ESCAPE)
		GameRunning = false;
	if (event.key.keysym.sym == SDLK_UP)
		player.walkDirection = +1;
	if (event.key.keysym.sym == SDLK_DOWN)
		player.walkDirection = -1;
	if (event.key.keysym.sym == SDLK_RIGHT)
		player.turnDirection = +1;
	if (event.key.keysym.sym == SDLK_LEFT)
		player.turnDirection = -1;
	if (event.key.keysym.sym == SDLK_w)
		player.walkDirection = +1;
	if (event.key.keysym.sym == SDLK_s)
		player.walkDirection = -1;
	if (event.key.keysym.sym == SDLK_a)
		player.turnDirection = -1;
	if (event.key.keysym.sym == SDLK_d)
		player.turnDirection = +1;
}

/**
 * SDL_KEYUP_FUNC - process input when a key is up
 * @event: union that contains structures for the different event types
*/

void SDL_KEYUP_FUNC(SDL_Event event)
{
	if (event.key.keysym.sym == SDLK_UP)
		player.walkDirection = 0;
	if (event.key.keysym.sym == SDLK_DOWN)
		player.walkDirection = 0;
	if (event.key.keysym.sym == SDLK_RIGHT)
		player.turnDirection = 0;
	if (event.key.keysym.sym == SDLK_LEFT)
		player.turnDirection = 0;
	if (event.key.keysym.sym == SDLK_w)
		player.walkDirection = 0;
	if (event.key.keysym.sym == SDLK_s)
		player.walkDirection = 0;
	if (event.key.keysym.sym == SDLK_a)
		player.turnDirection = 0;
	if (event.key.keysym.sym == SDLK_d)
		player.turnDirection = 0;
}

/**
 * handleInput - process input from the keyboard
 *
*/
void handleInput(void)
{
	SDL_Event event;

	SDL_PollEvent(&event);

	if (event.type == SDL_QUIT)
		GameRunning = false;
	else if (event.type == SDL_KEYDOWN)
		SDL_KEYDOWN_FUNC(event);
	else if (event.type == SDL_KEYUP)
		SDL_KEYUP_FUNC(event);
}
/**
 * drawRect - draw a rectangle
 * @x: x coordinate
 * @y: y coordinate
 * @width: rectangle width
 * @height: rectangle height
 * @color: pixel color
*/

void drawRect(int x, int y, int width, int height, color_t color)
{
	int i, j;

	for (i = x; i <= (x + width); i++)
		for (j = y; j <= (y + height); j++)
			drawPixel(i, j, color);
}

/**
 * drawLine - draw a line
 * @x0: x coordinate init
 * @y0: y coordinate init
 * @x1: x coordinate init
 * @y1: y coordinate end
 * @color: pixel color
*/

void drawLine(int x0, int y0, int x1, int y1, color_t color)
{
	float xIncrement, yIncrement, currentX, currentY;
	int i, longestSideLength, deltaX,  deltaY;

	deltaX = (x1 - x0);
	deltaY = (y1 - y0);

	longestSideLength = (abs(deltaX) >= abs(deltaY)) ? abs(deltaX) : abs(deltaY);

	xIncrement = deltaX / (float)longestSideLength;
	yIncrement = deltaY / (float)longestSideLength;

	currentX = x0;
	currentY = y0;

	for (i = 0; i < longestSideLength; i++)
	{
		drawPixel(round(currentX), round(currentY), color);
		currentX += xIncrement;
		currentY += yIncrement;
	}
}




