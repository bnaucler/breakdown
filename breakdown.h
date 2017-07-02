#ifndef BREAKDOWN_HEAD
#define BREAKDOWN_HEAD

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

typedef struct Movement {
	int move;
	int tl;
	int tr;
	int tu;
	int td;
} Movement;

typedef struct Object {
	SDL_Rect *rect;
	Movement *mvmt;
} Object;

typedef struct Block {
	SDL_Rect *rect;
	int opacity;
	int active;
} Block;

typedef struct Scorecard {
	int score;
	TTF_Font *font;
	SDL_Surface *text;
	SDL_Texture *texture;
	SDL_Rect *clip, *rndspace;
	SDL_Color col;
} Scorecard;

#endif
