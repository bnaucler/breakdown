/*
 *
 *		breakdown.c
 *
 */

#include "breakdown.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <SDL2/SDL.h>

// Windoow
#define WINW 640
#define WINH 480

// Paddle
#define PSTPOSX 320
#define PSTPOSY 430
#define PHEIGHT 20
#define PWIDTH 150
#define PSPEED 10

// Ball
#define BSTPOSX 100
#define BSTPOSY 100
#define BSPEED 3
#define BSZ 5

// Block
#define BLINES 3
#define BLWIDTH 40
#define BLHEIGHT 20
#define BLMXOPA 230

static int die(const char *err, int ret) {

	if(errno) fprintf(stderr, "Error: %s\n", strerror(errno));
	if(err[0]) fprintf(stderr, "Error: %s\n", err);

	SDL_Quit();
	exit(ret);
}

// Set window color to RGB
static void setbkg(SDL_Renderer *rend, int r, int g, int b) {

    SDL_SetRenderDrawColor(rend, r, g, b, 255);
    SDL_RenderFillRect(rend, NULL);
}

// Process incoming events
static int readevent(SDL_Event *event, Movement *mvmt) {

	while (SDL_PollEvent(event)) {
		switch (event->type) {

		case SDL_QUIT:
			return 1;

		case SDL_KEYDOWN:
			switch (event->key.keysym.scancode) {
				case SDL_SCANCODE_Q:
					return 1;

				case SDL_SCANCODE_A:
				case SDL_SCANCODE_LEFT:
					mvmt->tl = 1;
					break;

				case SDL_SCANCODE_D:
				case SDL_SCANCODE_RIGHT:
					mvmt->tr = 1;
					break;

				default:
					break;
			}
			break;

		case SDL_KEYUP:
			switch (event->key.keysym.scancode) {

				case SDL_SCANCODE_A:
				case SDL_SCANCODE_LEFT:
					mvmt->tl = 0;
					break;

				case SDL_SCANCODE_D:
				case SDL_SCANCODE_RIGHT:
					mvmt->tr = 0;
					break;

				default:
					break;
			}
			break;
		}
	}

	return 0;
}

static void mvpaddle(Object *paddle) {

	if(paddle->mvmt->tl) paddle->rect->x -= PSPEED;
	if(paddle->mvmt->tr) paddle->rect->x += PSPEED;

	if(paddle->rect->x <= 0) paddle->rect->x = 0;
	if(paddle->rect->x + PWIDTH >= WINW) paddle->rect->x = WINW - PWIDTH;
}

static void swap(int *a, int *b) {

	int tmp = *a;
	*a = *b;
	*b = tmp;
}

static int chlinecollision(Object *ball, Block **bline, int num) {

	unsigned int a = 0;

	if(ball->rect->y <= (bline[0]->rect->y + bline[0]->rect->h)) {
		for(a = 0; a < num; a++) {
			if(ball->rect->x >= bline[a]->rect->x && ball->rect->x <= bline[a]->rect->x + BLWIDTH && bline[a]->active) {
				swap(&ball->mvmt->tu, &ball->mvmt->td);
				bline[a]->active = 0;
				return 1;
			}
		}
	}

	return 0;
}

static int chstackcollision(Object *ball, Block ***bstack, int lines, int num) {

	unsigned int a = 0;

	for(a = 0; a < lines; a++) {
		if(chlinecollision(ball, bstack[a], num)) return 1;
	}

	return 0;
}

static int mvball(Object *paddle, Object *ball, Block ***bstack, int lines, int num) {

	int ret = 0;

	if(ball->rect->x <= 0) swap(&ball->mvmt->tr, &ball->mvmt->tl);
	if(ball->rect->y <= 0) swap(&ball->mvmt->tu, &ball->mvmt->td);
	if(ball->rect->x + BSZ >= WINW) swap(&ball->mvmt->tr, &ball->mvmt->tl);

	chstackcollision(ball, bstack, lines, num);

	if(ball->rect->y + BSZ >= paddle->rect->y) {
		if(ball->rect->x >= paddle->rect->x && ball->rect->x <= paddle->rect->x + PWIDTH)
			swap(&ball->mvmt->tu, &ball->mvmt->td);
		else ret = 2;
	}

	if(ball->mvmt->tl) ball->rect->x -= BSPEED;
	if(ball->mvmt->tr) ball->rect->x += BSPEED;
	if(ball->mvmt->tu) ball->rect->y -= BSPEED;
	if(ball->mvmt->td) ball->rect->y += BSPEED;

	return ret;
}

static void drawbline(SDL_Renderer *rend, Block **bline, int num) {

	unsigned int a = 0;

	for(a = 0; a < num; a++){
		if(bline[a]->active) {
			SDL_SetRenderDrawColor(rend, bline[a]->opacity, bline[a]->opacity, bline[a]->opacity, 0);
			SDL_RenderFillRect(rend, bline[a]->rect);
		}
	}
}

static void drawstack(SDL_Renderer *rend, Block ***bstack, int lines, int num) {

	unsigned int a = 0;

	for(a = 0; a < lines; a++) drawbline(rend, bstack[a], num);
}

static void draw(SDL_Renderer *rend, Object *paddle, Object *ball, Block ***bstack, int lines, int num) {

	// Background
	SDL_RenderClear(rend);
	setbkg(rend, 255, 255, 255);

	// Paddle
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
	SDL_RenderFillRect(rend, paddle->rect);

	// Ball
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
	SDL_RenderFillRect(rend, ball->rect);

	// Block stack
	drawstack(rend, bstack, lines, num);

	// Present
	SDL_RenderPresent(rend);
}

static Object *mkobj(int x, int y, int w, int h) {

	Object *obj = calloc(1, sizeof(Object));
	obj->rect = calloc(1, sizeof(SDL_Rect));
	obj->mvmt = calloc(1, sizeof(Movement));

	obj->rect->x = x;
	obj->rect->y = y;
	obj->rect->w = w;
	obj->rect->h = h;

	return obj;
}

static Block *mkblock(int x, int y, int w, int h, int o) {

	Block *block = calloc(1, sizeof(Block));
	block->rect = calloc(1, sizeof(SDL_Rect));

	block->rect->x = x;
	block->rect->y = y;
	block->rect->w = w;
	block->rect->h = h;

	block->active = 1;

	if(!o) block->opacity = random() % BLMXOPA;
	else block->opacity = o;

	return block;
}

static void freeblock(Block *block) {

	free(block->rect);
	free(block);
}

static void freeobj(Object *obj) {

	free(obj->rect);
	free(obj->mvmt);
	free(obj);
}

static Block **mkblockline(int ypos, int num, int spacing) {

	Block **barr = calloc(num, sizeof(Block*));
	unsigned int a = 0;

	int xpos = 0;

	for(a = 0; a < num; a++) {
		barr[a] = mkblock(xpos, ypos, BLWIDTH, BLHEIGHT, 0);
		xpos += (BLWIDTH + spacing);
	}

	return barr;
}

static Block ***mkblockstack(int ypos, int xpos, int num, int lines, int spacing) {

	Block ***bstack = calloc(lines, sizeof(Block**));
	unsigned int a = 0;

	for(a = 0; a < lines; a++) {
		bstack[a] = mkblockline(ypos, num, spacing);
		ypos += BLHEIGHT;
	}

	return bstack;
}

static void freeblockline(Block **bline, int num) {

	unsigned int a  = 0;

	for(a = 0; a < num; a++) freeblock(bline[a]);
	free(bline);
}

static void freeblockstack(Block ***bstack, int lines, int num) {

	unsigned int a = 0;

	for(a = 0; a < lines; a++) freeblockline(bstack[a], num);
	free(bstack);
}

int main(int argc, char **argv) {

	srandom(time(NULL));

	int blnum = WINW / BLWIDTH, rc = 0, score = 0;

	Object *paddle = mkobj(PSTPOSX, PSTPOSY, PWIDTH, PHEIGHT);
	Object *ball = mkobj(BSTPOSX, BSTPOSY, BSZ, BSZ);
	Block ***bstack = mkblockstack(0, 0, blnum, BLINES, 0);

	SDL_Event *event = calloc(1, sizeof(SDL_Event));

    SDL_Window *win = SDL_CreateWindow(argv[0],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINW, WINH, 0);

	SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_ShowCursor(SDL_DISABLE);

	ball->mvmt->td = 1;
	ball->mvmt->tl = 1;

	if(!win || !rend) die(SDL_GetError(), 1);
	errno = 0;

	while(!readevent(event, paddle->mvmt)) {
		mvpaddle(paddle);
		rc = mvball(paddle, ball, bstack, BLINES, blnum);

		if(rc == 1) score += 1;
		else if(rc == 2) goto cleanup;

		draw(rend, paddle, ball, bstack, BLINES, blnum);
		rc = 0;
	}

cleanup:
	freeblockstack(bstack, BLINES, blnum);
	freeobj(paddle);
	freeobj(ball);
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(rend);
	return die("", score);
}
