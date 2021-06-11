#include <stdlib.h>
#include <cbm.h>
#include <stdio.h>
#include <peekpoke.h>
#include "structs.h"

// functions //
void main();
void setup();
void loadGraphics();
void draw();
void keyPressed();
void initMap(struct Map *m);
void initMapData(struct Map *m, char data[]);
void renderMap(struct Map *m);
void clearScreen();
void loadPalette();

// global variables //
char testMap[] = {5,5,0,0,2,255,255,
	3,3,3,3,3,
	2,4,4,5,2,
	2,5,5,6,2,
	2,5,6,6,2,
	3,3,3,3,3,
};
extern unsigned char redgraphics[];
extern unsigned char greengraphics[];
extern unsigned char bluegraphics[];
extern unsigned char yellowgraphics[];
extern unsigned char tilegraphics[];
extern unsigned char customPalette[];
unsigned char keyCode;
struct Map *m;

void main() {
	setup();

	while (1) {
		__asm__("jsr $FFE4");
		__asm__("cmp #0");
		__asm__("beq %g", afterKeyPressed);
		//__asm__("jsr $FFD2");
		__asm__("sta %v", keyCode);
		keyPressed();
		afterKeyPressed:
		draw();
		waitvsync();
	}
}

void setup() {
	unsigned short i;

	POKE(0x9F29,0x71 & 0xBF);
	POKE(0x9F25,0);
	POKE(0x9F2A,0x40);
	POKE(0x9F2B,0x40);
	POKE(0x9F30,0); // reset scroll
	POKE(0x9F31,0);
	POKE(0x9F32,0);
	POKE(0x9F33,0);
	POKE(0x9F37,0);
	POKE(0x9F38,0);
	POKE(0x9F39,0);
	POKE(0x9F3A,0);

	POKE(0x9F2D,0x62); // default map height & width, 4 bpp tiles
	POKE(0x9F2E,0x00); // map based at vram addr 0
	POKE(0x9F2F,0x43); // tile base addr 0x8000

	POKE(0x9F34,0x62); // default map height & width, 4 bpp tiles
	POKE(0x9F35,0x20); // map based at vram addr 0x8000 (0x40 * 512)
	POKE(0x9F36,0x43); // tile base addr 0x8000

	POKE(0x9F20,0x00);
	POKE(0x9F21,0x80);
	POKE(0x9F22,0x10);
	for (i = 0; i < 4096; i++) {
		POKE(0x9F23,redgraphics[i]);
	}
	POKE(0x9F20,0x00);
	POKE(0x9F21,0x90); // 0x80 + 0x20/2
	for (i = 0; i < 4096; i++) {
		POKE(0x9F23,greengraphics[i]);
	}
	POKE(0x9F20,0x00);
	POKE(0x9F21,0xA0);
	for (i = 0; i < 4096; i++) {
		POKE(0x9F23,bluegraphics[i]);
	}
	POKE(0x9F20,0x00);
	POKE(0x9F21,0xB0);
	for (i = 0; i < 4096; i++) {
		POKE(0x9F23,yellowgraphics[i]);
	}
	POKE(0x9F20,0x00);
	POKE(0x9F21,0xC0);
	for (i = 0; i < 4096; i++) {
		POKE(0x9F23,tilegraphics[i]);
	}
	loadPalette();

	clearScreen();
	initMapData(m,testMap);
}

void draw() {
	renderMap(m);
}

void keyPressed() {
	if (keyCode == 'W') {
		if (m->c->y != 0) {m->c->y--;}
	} else if (keyCode == 'A') {
		if (m->c->x != 0) {m->c->x--;}
	} else if (keyCode == 'S') {
		if (m->c->y < m->boardHeight) {m->c->y++;}
	} else if (keyCode == 'D') {
		if (m->c->x < m->boardWidth) {m->c->x++;}
	}
}

void loadPalette() {
	unsigned char i;

	POKE(0x9F20,00);
	POKE(0x9F21,0xFA);
	POKE(0x9F22,0x11);
	i = 0;
	do {
		POKE(0x9F23,customPalette[i]);
		i++;
	} while (i != 0);
	do {
		POKE(0x9F23,customPalette[256+i]);
		i++;
	} while (i != 0);
	return;
}

void clearScreen() {
	unsigned char i = 0;

	POKE(0x9F20,0);
	POKE(0x9F21,0);
	POKE(0x9F22,0x10);
	while (PEEK(0x9F21) < 0x20) {
		POKE(0x9F23,28);
		POKE(0x9F23,0);
	}
	POKE(0x9F20,0);
	POKE(0x9F21,0x40);
	while (PEEK(0x9F21) < 0x60) {
		POKE(0x9F23,28);
		POKE(0x9F23,0);
	}

	return;
}
