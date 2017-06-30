#ifndef BREAKDOWN_HEAD
#define BREAKDOWN_HEAD

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

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

#endif
