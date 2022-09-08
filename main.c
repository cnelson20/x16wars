#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>
#include <string.h>

#include "structs.h"
#include "main.h"
#include "map.h"
#include "waitforjiffy.h"
#include "fastroutines.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* global variables */
/*
char testMapData[] = {19,12,// height and width
0x00, 0x04, 0x0D, 0x00, 0x05, 0x0E, 0x01, 0x06, 0x01, 0x00, 0x07, 0x0C, 0x01, 0x07, 0x13, 0x03, 0x07, 0x0B, 
0x01, 0x08, 0x12, 0x03, 0x08, 0x0B, 0x04, 0x08, 0x0A, 0x00, 0x09, 0x09, 0x02, 0x09, 0x10, 0x03, 0x09, 0x03, 
0x01, 0x0A, 0x11, 0x03, 0x0A, 0x03, 0x04, 0x0A, 0x02, 0x02, 0x0B, 0x10, 0x03, 0x0B, 0x02, 255, // red units
0x12, 0x04, 0x0C, 0x11, 0x05, 0x11, 0x11, 0x06, 0x01, 0x10, 0x07, 0x0B, 0x11, 0x07, 0x13, 0x0E, 0x08, 0x0A, 
0x10, 0x08, 0x0B, 0x11, 0x08, 0x13, 0x0E, 0x09, 0x02, 0x0F, 0x09, 0x03, 0x10, 0x09, 0x10, 0x12, 0x09, 0x09, 
0x0E, 0x0A, 0x03, 0x0F, 0x0A, 0x03, 0x11, 0x0A, 0x12, 0x0E, 0x0B, 0x02, 0x0F, 0x0B, 0x03, 255, // blue units
  
  0x07,0x07,0x01,0x01,0x01,0x01,0x06,0x05,0x06,0x01,0x06,0x05,0x06,0x01,0x01,0x01,0x01,0x07,0x07,
  0x07,0x05,0x06,0x01,0x01,0x01,0x05,0x44,0x05,0x01,0x05,0x45,0x05,0x01,0x01,0x01,0x06,0x05,0x07,
  0x07,0x05,0x06,0x01,0x01,0x01,0x06,0x05,0x07,0x01,0x07,0x05,0x06,0x01,0x01,0x01,0x06,0x05,0x07,
  0x43,0x05,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x05,0x05,0x43,
  0x05,0x43,0x07,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x07,0x43,0x05,
  0x05,0x05,0x06,0x05,0x05,0x05,0x05,0x06,0x05,0x05,0x05,0x06,0x05,0x05,0x05,0x05,0x06,0x05,0x05,
  0x03,0x03,0x03,0x03,0x03,0x03,0x06,0x43,0x06,0x03,0x06,0x43,0x06,0x03,0x03,0x03,0x03,0x03,0x03,
  0x05,0x02,0x05,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x05,0x05,0x02,0x05,
  0x07,0x02,0x06,0x05,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x05,0x05,0x06,0x02,0x07,
  0x05,0x02,0x05,0x05,0x05,0x06,0x07,0x05,0x06,0x01,0x06,0x05,0x07,0x06,0x05,0x05,0x05,0x02,0x05,
  0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
  0x05,0x06,0x05,0x05,0x06,0x05,0x05,0x43,0x05,0x01,0x05,0x43,0x05,0x05,0x05,0x06,0x05,0x05,0x06,
  // tiles
};
char *testMap = testMapData;
*/
extern unsigned char redgraphics[];
/* Using copies of red to save space 
extern unsigned char greengraphics[];
extern unsigned char bluegraphics[];
extern unsigned char yellowgraphics[];
*/
extern unsigned char customPalette[];
extern unsigned char player1team;
extern unsigned char player2team;
extern unsigned char returnToMenu;

unsigned char keyCode;
struct Map m;
struct Cursor c;
struct Cursor attackCursor;
unsigned char actionNo;
unsigned char selIndex;
struct possibleAttacks *pA = NULL;
struct Menu menuOptions;

unsigned char mouseEnabled;
unsigned short mouseX;
unsigned short mouseY;
unsigned char mouseButtons;

void main() {
  setup();
	while (1) {
		menu();
		game_start();
		returnToMenu = 0;
		while (!returnToMenu) {
			waitforjiffy();
			__asm__("jsr $FFE4");
			__asm__("sta %v", keyCode);
			if (keyCode != 0) {
				keyPressed();
			}
			draw();
		}
		/* Free memory */
		{
			unsigned short i;
			for (i = 0; i < m.totalTiles; ++i) {
					free(m.board[i].occupying->carrying);
					free(m.board[i].occupying);
					free(m.board[i].t);
					free(m.board[i].base);
			}
			free(m.board);
		}
	}
}

char **menu_files_array;
char menu_files_length;

char **map_files_array;
char map_files_length;

void load_dir_menu() {	
	unsigned short i;
	menu_files_array = NULL;
	menu_files_length = 0;
		
	cbm_k_setnam("$");
	cbm_k_setlfs(12, 8, 0);
	cbm_k_open();

	cbm_k_chkin(12);
	/* 4 prologue bytes */
	cbm_k_chrin();
	cbm_k_chrin();
	cbm_k_chrin();
	cbm_k_chrin();
	// Disk stuff done with help from irmen 's prog8 libraries 
	/* Read files from disk */
	
	while (1) {
		char *temp;
		if (cbm_k_readst()) {
			break;
		}
		++menu_files_length; 
		menu_files_array = realloc(menu_files_array, menu_files_length * 2); // sizeof(char *) 
		
		while (cbm_k_chrin() != '"') { // Discard up to first quote
			if (cbm_k_readst()) { break; }
		}
		if (cbm_k_readst()) { break; }
		
		temp = malloc(32);
		menu_files_array[menu_files_length - 1] = temp;
		while (1) {
			char c = cbm_k_chrin();
			if (c == 0) { break; }
			if (c == '"') { break; }
			*temp = c;
			++temp;
		}
		*temp = '\0';
		
		/* If not valid, remove from array */
		/* if (strstr(temp, ".map") != 0) {
			__asm__ ("brk");
			map_files_array = realloc(map_files_array, map_files_length << 1 + 2);
			map_files_array[map_files_length] = strdup(temp);
			++map_files_length;
		} */
		
		while (cbm_k_chrin() != 0) {} // Read rest of entry until the end 
		cbm_k_chrin();
		cbm_k_chrin();	
		
	}
	cbm_k_clrch();
	cbm_k_close(12);
	
	map_files_array = NULL;
	map_files_length = 0;
	for (i = 0; i < menu_files_length; ++i) {
		char *temp = menu_files_array[i];
		if (strstr(temp, ".map") != 0) {
			map_files_array = realloc(map_files_array, map_files_length * sizeof(char *) + 2);
			map_files_array[map_files_length] = temp;
			++map_files_length;
		} else {
			free(temp);
		}
	}
	free(menu_files_array);
	menu_files_array = map_files_array;
	menu_files_length = map_files_length;
}

void print_hex_number(unsigned char x) {
	unsigned char j = x >> 4;
	POKE(0x9F23, (j >= 0xA) ? j - 0xA + 0xA0 : j + 186);
	j = x & 0xF;
	POKE(0x9F23, (j >= 0xA) ? j - 0xA + 0xA0 : j + 186);
}

void print_ascii_str(char *string, unsigned char break_on_period) {
	while (*string) {		
		char c = *string;
		
		if (c >= 0x30 && c <= 0x39) {
			c = c + (186 - 0x30);
		} else if (c >= 'a' && c <= 'z') {
			c = c + (0xA0 - 'a');
		} else if (c == '.') {
			if (break_on_period) { return; }
			c = 28;
		} else if (c == ' ' || c == '_') {
			c = 28;
		}
		POKE(0x9F23, c);
		++string;
	}
}

void menu() {
	unsigned short i;
	char *map_space;
	setup_menu();
	change_directory("maps");
	load_dir_menu();
		
	POKE(0x9F20, 4 * 2);
	POKE(0x9F21, 0x41);
	POKE(0x9F22, 0x10);
	
	POKE(0x9F23, 3);
	POKE(0x9F23, 0x00);
	POKE(0x9F22, 0x20);
	print_ascii_str(" x16 wars ", 0);
	POKE(0x9F22, 0x10);
	POKE(0x9F23, 3);
	POKE(0x9F23, 0x24);
	
	POKE(0x9F20, 8);
	POKE(0x9F21, 0x40 + 4);
	POKE(0x9F22, 0x20);
	
	POKE(0x9F23, menu_files_length / 10 + 186);
	POKE(0x9F23, menu_files_length % 10 + 186);
	print_ascii_str(" maps found", 0);
	
	POKE(0x9F20, 2);
	POKE(0x9F21, 0x46);
	POKE(0x9F23, 196);
	for (i = 0; i < menu_files_length; ++i) {	
		POKE(0x9F20, 4);
		POKE(0x9F21, 0x46 + i);
		POKE(0x9F22, 0x20);	
		print_ascii_str(menu_files_array[i], 1);
	}
	
	player1team = 0;
  player2team = 2;
	
	while (1) {
		__asm__("jsr $FFE4");
		__asm__("sta %v", keyCode);
		
		if (keyCode != 0) {
			if (keyCode == 0x57 /* W */ && c.x > 0) {
				--c.x;
			} else if (keyCode == 0x53 /* S */ && c.x < menu_files_length - 1) {
				++c.x;
			} else if (keyCode == 0x0d /* Enter */) {
				break;
			} else if (keyCode == 0x51 /* Q */ || keyCode == 0x58 /* X */) {
				//__asm__ ("brk");
				__asm__ ("jmp ($FFFC)");
			} else if (keyCode >= 0x31 && keyCode <= 0x38 /* Between 1 & 8 */) {
				unsigned char tempteam = (keyCode - 0x31) % 4; 
				// change player 2 team 
				if (keyCode >= 0x35) {
					if (player1team != tempteam) { player2team = tempteam; }	
				// change player 1 team
				} else {
					if (player2team != tempteam) { player1team = tempteam; }
				}
			}
			/* redraw cursor showing selected map */
			POKE(0x9F20, 2);
			POKE(0x9F22, 0);
			for (i = 6; i < 15; ++i) {
				POKE(0x9F21, 0x40 + i);
				POKE(0x9F23, 0x88);
			}
			POKE(0x9F20, 2);
			POKE(0x9F21, c.x + 0x46);
			POKE(0x9F23, 196);
			
			POKE(0x9F20, 4 * 2 + 1);
			POKE(0x9F21, 0x41);
			//POKE(0x9F22, 0x10);
			POKE(0x9F23, player1team << 4);
			POKE(0x9F20, 15 * 2 + 1);
			POKE(0x9F23, (player2team << 4) | 0x4);
		}
	}
	cbm_k_setnam(menu_files_array[c.x]);
	cbm_k_setlfs(0xFF, 8, 0);
	map_space = malloc(768);
	cbm_k_load(0, (unsigned short)map_space);
	initMapData(map_space);
	free(map_space);
	change_directory("..");
	return;
}

char change_filename[32] = "cd:";
void change_directory(char *s) {
	strcpy(change_filename + 3, s);
	cbm_k_setnam(change_filename);
	cbm_k_setlfs(15, 8, 15);
	cbm_k_open();
	cbm_k_clrch();
	cbm_k_close(15);
}

void setup_menu() {
	initCursor();
	POKE(0x9F25, 0);
	POKE(0x9F29, 0x31);
	
	POKE(0x9F20, 0);
	POKE(0x9F21, 0);
	POKE(0x9F22, 0x10);
	while (PEEK(0x9F21) < 15) {
		POKE(0x9F23,0x88);
    POKE(0x9F23,0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20,0);
      __asm__ ("inc $9F21");
    }
	}
	POKE(0x9F20, 0);
	POKE(0x9F21, 0x40);
	while (PEEK(0x9F21) < 0x5F) {
		POKE(0x9F23,28);
    POKE(0x9F23,0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20,0);
      __asm__ ("inc $9F21");
    }
	}
}

extern unsigned short turncounter;
extern unsigned char unitsdeadthisturn;
extern unsigned char currentbases;
extern unsigned char oldbases;
extern unsigned char currentunitsprites;
extern unsigned char oldunitsprites;
extern unsigned char remove_old;

void game_start() {
	memset(&c, 0, sizeof(c));
	memset(&attackCursor, 0, sizeof(attackCursor));
	memset(&menuOptions, 0, sizeof(menuOptions));
	
	POKE(0x9F25, 0);
	POKE(0x9F29, 0x71);
  clearScreen();
  //initMapData(testMap);
  initCursor();
}

#define UNIT_CHR_FILELEN 16384
#define TILE_CHR_FILELEN 1154
#define SPRITE_CHR_FILELEN 1602
#define LETTER_CHR_FILELEN 4738

void setup() {
  unsigned char device_no = 8;

  //unsigned short i;
  unsigned char *load_address;

  POKE(0x9F29,0x71);
  __asm__ ("lda #$40");
  __asm__ ("sta $9F2A");
  __asm__ ("sta $9F2B");

  __asm__ ("lda #0");

  __asm__ ("sta $9F25");
  
  __asm__ ("sta $9F30");
  __asm__ ("sta $9F31");
  __asm__ ("sta $9F32");
  __asm__ ("sta $9F33");
  __asm__ ("sta $9F37");
  __asm__ ("sta $9F38");
  __asm__ ("sta $9F39");
  __asm__ ("sta $9F3A"); // reset scroll

  POKE(0x9F2D,0x62); // default map height & width, 4 bpp tiles
  POKE(0x9F2E,0x00); // map based at vram addr 0
  POKE(0x9F2F,0x43); // tile base addr 0x8000

  POKE(0x9F34,0x62); // default map height & width, 4 bpp tiles
  POKE(0x9F35,0x20); // map based at vram addr 0x8000 (0x40 * 512)
  POKE(0x9F36,0x43); // tile base addr 0x8000

  POKE(0x9F30,0); // Scroll
  POKE(0x9F37,0);
  POKE(0x9F31,0);
  POKE(0x9F38,0);
  POKE(0x9F33,0);
  POKE(0x9F3A,0);

  POKE(0x9F20,0x00);
  POKE(0x9F21,0x80);
  POKE(0x9F22,0x10);
  load_data_vera_from = redgraphics;
  load_data_vera(4096);
  POKE(0x9F20,0x00);
	load_data_vera(4096);
	POKE(0x9F20,0x00);
  load_data_vera(4096);
	POKE(0x9F20,0x00);
  load_data_vera(4096);
	
  load_address = malloc(5000); // 128 more than 4,736 (size of letter.c, biggest one)
  
  cbm_k_setnam("tile.chr");
  cbm_k_setlfs(0xFF, device_no, 0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00);
  POKE(0x9F21,0xC0);
  load_data_vera_from = load_address;
  load_data_vera(TILE_CHR_FILELEN);
  /*for (i = 0; i < TILE_CHR_FILELEN; ++i) {
    POKE(0x9F23,load_address[i]);
  }*/
  
  cbm_k_setnam("letter.chr");
  cbm_k_setlfs(0xFF, device_no, 0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00); 
  POKE(0x9F21,0xD0);
  load_data_vera(LETTER_CHR_FILELEN);
  /*for (i = 0; i < LETTER_CHR_FILELEN; ++i) {
    POKE(0x9F23,load_address[i]);
  }*/
  
  cbm_k_setnam("sprites.chr");
  cbm_k_setlfs(0xFF, device_no, 0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00);
  POKE(0x9F21,0x00);
  POKE(0x9F22,0x11);
  load_data_vera(SPRITE_CHR_FILELEN);
  /*for (i = 0; i < SPRITE_CHR_FILELEN; ++i) {
    POKE(0x9F23,load_address[i]);
  }*/
  
  free(load_address);
  loadPalette();
}  

void draw() {
  renderMap();
  drawUI();
}

unsigned char unitIndexes[] =
  {0,1,2,3,
   255,255,255,255,
   255,4,5,6,
   7,8,9,255,
   10,11,12,13,
   255,255,255,255,
   14,15,16,17};

unsigned char unitNames[][11] = {
  {0xa0, 0xaf, 0xa2}, // APC
  {0xb1, 0xa4, 0xa2, 0xae, 0xad}, // Recon
  {0xac, 0xa4, 0xa2, 0xa7}, // Mech
  {0xa8, 0xad, 0xa5, 0xa0, 0xad, 0xb3, 0xb1, 0xb8}, // Infantry
  {0xa0, 0xad, 0xb3, 0xa8, 0x1C, 0xa0, 0xa8, 0xb1}, // Anti Air
  {0xac, 0xa4, 0xa3, 0xa8, 0xb4, 0xac, 0x1C, 0xb3, 0xa0, 0xad, 0xaa}, // Medium Tank
  {0xb2, 0xac, 0xa0, 0xab, 0xab, 0x1c, 0xb3, 0xa0, 0xad, 0xaa}, // Small Tank
  {0xac, 0xa8, 0xb2, 0xb2, 0xa8, 0xab, 0xa4, 0xb2}, // Missiles
  {0xb1, 0xae, 0xa2, 0xaa, 0xa4, 0xb3, 0xb2}, // Rockets
  {0xa0, 0xb1, 0xb3, 0xa8, 0xab, 0xab, 0xa4, 0xb1, 0xb8}, // Artillery
  {0xa7, 0xa4, 0xab, 0xa8, 0xa2, 0xae, 0xaf, 0xb3, 0xa4, 0xb1}, // Transport
  {0xa2, 0xae, 0xaf, 0xb3, 0xa4, 0xb1}, // Copter
  {0xa1, 0xae, 0xac, 0xa1, 0xa4, 0xb1}, // Bomber
  {0xa5, 0xa8, 0xa6, 0xa7, 0xb3, 0xa4, 0xb1}, // Fighter
  {0xab, 0xa0, 0xad, 0xa3, 0xa4, 0xb1}, // Lander
  {0xb2, 0xb4, 0xa1, 0xac, 0xa0, 0xb1, 0xa8, 0xad, 0xa4}, // Submarine
  {0xa2, 0xb1, 0xb4, 0xa8, 0xb2, 0xa4, 0xb1}, // Cruiser
  {0xa1, 0xa0, 0xb3, 0xb3, 0xab, 0xa4, 0xb2, 0xa7, 0xa8, 0xaf}}; // Battleship

unsigned char unitNameLengths[] = {3,5,4,8,8,11,10,8,7,9,10,6,6,7,6,9,7,10};

unsigned char globalSize;
void drawText(unsigned char *string, unsigned char size, unsigned char x, unsigned char y, unsigned char layer) {
  unsigned char i = 0;
  POKE(0x9F20,x << 1);
  POKE(0x9F21,(layer ? 0x40 : 0x00) + y);
  POKE(0x9F22,0x10);
  while (i < size) {
    POKE(0x9F23,string[i]);
    POKE(0x9F23,0x80);
    ++i;
  }
}

#define OPTION_NULL 0
#define OPTION_END 1
#define OPTION_CONCEDE 2
#define OPTION_QUIT 3 
#define OPTION_DROP 4
#define OPTION_WAIT 5 
#define OPTION_LOAD 6
#define OPTION_CAPTURE 7
#define OPTION_ATTACK 8
#define OPTION_MOUSETOGGLE 9
#define OPTION_JOIN 10
#define OPTION_MENU 11

unsigned char optionStrings[][8] = {
  {0xad, 0xb4, 0xab, 0xab, 0x00},
	{0xa4, 0xad, 0xa3, 0x00},
	{0xa2, 0xae, 0xad, 0xa2, 0xa4, 0xa3, 0xa4, 0x00},
	{0xb0, 0xb4, 0xa8, 0xb3, 0x00},
	{0xa3, 0xb1, 0xae, 0xaf, 0x00},
	{0xb6, 0xa0, 0xa8, 0xb3, 0x00},
	{0xab, 0xae, 0xa0, 0xa3, 0x00},
	{0xa2, 0xa0, 0xaf, 0xb3, 0xb4, 0xb1, 0xa4, 0x00},
	{0xa0, 0xb3, 0xb3, 0xa0, 0xa2, 0xaa, 0x00},
	{0xac, 0xae, 0xb4, 0xb2, 0xa4, 0x00},
	{0xa9, 0xae, 0xa8, 0xad, 0x00},
	{0xac, 0xa4, 0xad, 0xb5, 0x00}};

unsigned char healthText[] = {0xa7, 0xa4, 0xa0, 0xab, 0xb3, 0xa7, 0x1c};

unsigned char baseTypes[][9] = {
  {0xa1, 0xa0, 0xb2, 0xa4, 0x00},
	{0xa7, 0xb0, 0x00},
  {0xa5, 0xa0, 0xa2, 0xb3, 0xae, 0xb1, 0xb8, 0x00}};	
unsigned char baseTypeStringLengths[] = {4,2,8};

unsigned char damageString[] = {0xa3, 0xac, 0xa6}; 

#define SCREENBYTE(a) ((a) >= 10 ? 150 + (a) : 186 + (a))

void drawUI() {
  struct Unit *unitPointer;
  void *test = NULL;
  unsigned char dummy, i;

  clearUI();
  
  /* Display memory footprint on screen *//*
  POKE(0x9F21,0x40+13);
  POKE(0x9F20,0);
  POKE(0x9F22,0x20);
  POKE(0x9F23,'M' - 'A' + 160); 
  POKE(0x9F23,28);
  test = malloc(1);
  i = ((unsigned short)test >> 12) % 16;
  POKE(0x9F23,SCREENBYTE(i));
  i = ((unsigned short)test >> 8) % 16;
  POKE(0x9F23,SCREENBYTE(i));
  i = ((unsigned short)test >> 4) % 16;
  POKE(0x9F23,SCREENBYTE(i));
  i = (unsigned short)test % 16;
  POKE(0x9F23,SCREENBYTE(i));
  free(test);
  */
  
  POKE(0x9F20,16*2);
  POKE(0x9F21,0x41);
  POKE(0x9F22,0x10);
	
  POKE(0x9F23,179 /* 'T' - 'A' + 160 */);
  POKE(0x9F23,0x80);
  POKE(0x9F23,0x03+(m.whoseTurn << 5)); // Infantry with color of current player's team 
  POKE(0x9F23,m.whoseTurn << 4);
  
  unitPointer = m.board[(c.y+m.top_view)*m.boardWidth+c.x+m.left_view].occupying;
  if (unitPointer == NULL) {
    unitPointer = c.selected;
  }
  if (pA != NULL) {
    unitPointer = attackCursor.selected;
  }
  
  if (menuOptions.length != 0) {
    //display menu options
    POKE(0x9F21,0x4B);
    POKE(0x9F22,0x20);
    for (dummy = 0; dummy < menuOptions.length; dummy++) {
      POKE(0x9F20,2);
      POKE(0x9F23,menuOptions.selected == dummy ? 196 : 28);
      for (i = 0; optionStrings[menuOptions.options[dummy]][i] != 0; i++) {
		POKE(0x9F23,optionStrings[menuOptions.options[dummy]][i]);
      }
      __asm__ ("inc $9F21");
    }
  } else if (m.board[c.x + m.left_view + m.boardWidth*(c.y+m.top_view)].base != NULL && (unitPointer == NULL || c.selected != NULL)) {
	struct Captureable *capt = m.board[c.x + m.left_view + m.boardWidth*(c.y+m.top_view)].base;
	drawText(baseTypes[capt->type],baseTypeStringLengths[capt->type],1,11,1);
	POKE(0x9F20,(baseTypeStringLengths[capt->type] + 2) << 1);
	POKE(0x9F22,0x10);
	POKE(0x9F23,'T' - 'A' + 0xA0);
	POKE(0x9F23,0x80);
	if (capt->team != 4) {
	  POKE(0x9F23,0x03);
	  POKE(0x9F23,capt->team << 4);
	} else {
	  POKE(0x9F23,'N' - 'A' + 0xA0);	
	  POKE(0x9F23,0x80);
	}
	POKE(0x9F21,0x40 + 12);
	POKE(0x9F20,2);
	POKE(0x9F22,0x20);
	POKE(0x9F23,'H' - 'A' + 160);
	POKE(0x9F23,'P' - 'A' + 160);
	POKE(0x9F23,28);
	POKE(0x9F23,186 + (capt->health / 10));
  POKE(0x9F23,186 + (capt->health % 10));
	
  } else if (unitPointer != NULL) {
    dummy = unitIndexes[unitPointer->index];
    drawText(unitNames[dummy],unitNameLengths[dummy],1,11,1);
    POKE(0x9F20,2);
    POKE(0x9F21,0x40+12);
    POKE(0x9F22,0x20);
    POKE(0x9F23,183); // X
	if (unitPointer->x >= 16) {
		POKE(0x9F23,SCREENBYTE(unitPointer->x >> 4));
	}
    POKE(0x9F23,SCREENBYTE(unitPointer->x & 15));
	
    POKE(0x9F23,28);
    POKE(0x9F23,184); // Y
	if (unitPointer->y >= 16) {
		POKE(0x9F23,SCREENBYTE(unitPointer->y >> 4));
	}
    POKE(0x9F23,SCREENBYTE(unitPointer->y & 15));
	
    if (unitPointer->health <= 99) {
      POKE(0x9F23,28);
      POKE(0x9F23,167);
      POKE(0x9F23,186 + (unitPointer->health / 10));
      POKE(0x9F23,186 + (unitPointer->health % 10));
    }
		if (unitPointer->ammo < 10) {
			POKE(0x9F23,28);
      POKE(0x9F23,160 + 'A' - 'A');
			POKE(0x9F23,160 + 'M' - 'A');
			POKE(0x9F23,160 + 'M' - 'A');
			POKE(0x9F23,160 + 'O' - 'A');
      POKE(0x9F23,186 + unitPointer->ammo);
		}
	if (pA != NULL && menuOptions.options[0] == OPTION_ATTACK && attackCursor.selected != NULL) {
	  static unsigned char oldDamagePreviewNum = 0;
	  static unsigned char damageHundredsDigit;
		static unsigned char damageModTen;
	  static unsigned char damageDivTen;
	  
	  unsigned char damagePreviewNum = calcPower(c.selected, attackCursor.selected); 	 
	  
	  if (damagePreviewNum != oldDamagePreviewNum) {
			damageHundredsDigit = 186 + (damagePreviewNum / 100);
			damageDivTen = 186 + ((damagePreviewNum / 10) % 10);
			damageModTen = 186 + (damagePreviewNum % 10);
	  }
	  
	  drawText(damageString, sizeof(damageString), 1, 13, 1);
	  POKE(0x9F20,2*5);
	  POKE(0x9F21,0x40 + 13);
	  POKE(0x9F22, 0x20);
		if (damagePreviewNum >= 100) {
			POKE(0x9F23, damageHundredsDigit);
		}
	  POKE(0x9F23, damageDivTen);
	  POKE(0x9F23, damageModTen);
	  
	  oldDamagePreviewNum = damagePreviewNum;
	}
  } 
}


void clearUI() {
  POKE(0x9F20,0x30);
  POKE(0x9F21,0x40);
  POKE(0x9F22,0x10);
  while (PEEK(0x9F21) < 0x44) {
	__asm__ ("lda #28");
	__asm__ ("ldx #$80");
	
	__asm__ ("sta $9F23");
	__asm__ ("stx $9F23");
    __asm__ ("sta $9F23");
	__asm__ ("stx $9F23");
    __asm__ ("sta $9F23");
	__asm__ ("stx $9F23");
    __asm__ ("sta $9F23");
	__asm__ ("stx $9F23");
	__asm__ ("sta $9F23");
	__asm__ ("stx $9F23");
    
    POKE(0x9F20,30);
    __asm__ ("inc $9F21");
  }

  POKE(0x9F21,0x4A);	
  while (PEEK(0x9F21) < 0x54) {
    POKE(0x9F20,0);
	__asm__ ("lda #28");
	__asm__ ("ldx #$80");
    __asm__ ("ldy #20");
	
  clearUILoop:
	__asm__ ("sta $9F23"); // 20 times (width of screen)
	__asm__ ("stx $9F23");
	__asm__ ("dey");
	__asm__ ("bne %g",clearUILoop);
	
	__asm__ ("inc $9F21");
  }
}

extern unsigned char unitLastX;
extern unsigned char unitLastY;
extern unsigned char unitsdeadthisturn;

unsigned char dropOffsetsX[] = {0, 1, 2, 1};
unsigned char dropOffsetsY[] = {1, 0, 1, 2};

void keyPressed() {  
	if (keyCode == 0x91) { keyCode = 0x41 + 'w' - 'a'; }
	if (keyCode == 0x9D) { keyCode = 0x41; }
	if (keyCode == 0x11) { keyCode = 0x41 + 's' - 'a'; }
	if (keyCode == 0x1D) { keyCode = 0x41 + 'd' - 'a'; }
	
  if (menuOptions.length != 0) {
    if (keyCode == 0x57) /* W */ {
      if (menuOptions.selected != 0) {menuOptions.selected--;}
    } else if (keyCode == 0x53)	/* S */ {
      if (menuOptions.selected < menuOptions.length - 1) {menuOptions.selected++;}
    } else if (keyCode == 0x55) /* U */ {
      menuOptions.length = 0;
	  if (c.selected != NULL) {
		undoMove(c.selected);  
	  }
	  if (pA != NULL) { free(pA); }
    } else if (keyCode == 0x49) /* I */ {
      switch (menuOptions.options[menuOptions.selected]) {
          case OPTION_END:
		menuOptions.length = 0;
		nextTurn();
		break;
	  case OPTION_CONCEDE:
		menuOptions.length = 0;
		win(m.whoseTurn == player1team ? player2team : player1team);
		break;
	  case OPTION_QUIT:
		menuOptions.length = 0;
		POKE(0x9F25,0x80);
		__asm__ ("jmp ($FFFC)");
	  case OPTION_WAIT:
		unitLastX = 255;
		unitLastY = 255;
		c.selected->takenAction = 1;
		c.selected = NULL;
		menuOptions.length = 0;
		break;
	  case OPTION_CAPTURE:
		capture(c.selected,m.board[c.selected->x + m.boardWidth*c.selected->y].base);
		unitLastX = 255;
		unitLastY = 255;
		c.selected->takenAction = 1;
		c.selected = NULL;
		menuOptions.length = 0;
		break;
	  case OPTION_DROP:
		// Code for dropping off units 
		menuOptions.length = 0;
		pA = malloc(sizeof(struct possibleAttacks));
		getPossibleDrops(pA,c.selected);
		actionNo = 1; 
		selIndex = 1;
		while (pA->attacks[selIndex] == NULL) {  
		  ++selIndex;
		  POKE(0x9fb6,0);
		  if (selIndex >= pA->length) {selIndex = 0;}
		}
		attackCursor.x = c.selected->x + dropOffsetsX[selIndex] - 1;
		attackCursor.y = c.selected->y + dropOffsetsY[selIndex] - 1;
		
		break;
	  case OPTION_LOAD:
		attackCursor.selected->carrying = c.selected;
		m.board[c.selected->x + m.boardWidth*c.selected->y].occupying = attackCursor.selected;
		attackCursor.selected = NULL;
		unitLastX = 255;
		unitLastY = 255;
		c.selected = NULL;
		menuOptions.length = 0;
	  case OPTION_ATTACK:
		/* Atttacking */
		actionNo = 0; 
		selIndex = 0;
		attackCursor.selected = pA->attacks[0]->occupying;
		attackCursor.x = attackCursor.selected->x;
		attackCursor.y = attackCursor.selected->y;
		
		menuOptions.length = 0;
		break;
	  case OPTION_JOIN:
		unitLastX = 255;
		unitLastY = 255;
		
		c.selected->health = MIN(100, c.selected->health + attackCursor.selected->health);
		c.selected->takenAction = 1;
		c.selected = NULL;
		free(attackCursor.selected);
		attackCursor.selected = NULL;
		
		menuOptions.length = 0;
	  case OPTION_MOUSETOGGLE:
		if (mouseEnabled) {
			__asm__ ("lda #0");
			__asm__ ("tax");
			__asm__ ("tay");
			__asm__ ("jsr $FF68"); // mouse_config
			
		} else {
			__asm__ ("sec");
			__asm__ ("jsr $FF5F"); // get screen mode
			__asm__ ("lda #1"); // enable mouse 
			__asm__ ("jsr $FF68"); // mouse_config
		}
		mouseEnabled = !mouseEnabled;
		break;
      default:
		break;
      }
    }
  } else {
    if (pA != NULL) {
      //Switch between attack targets.
      if (keyCode == 0x41) /* A */ {
		do {  
		  selIndex = (selIndex == 0) ? pA->length - 1 : selIndex - 1;
		} while (pA->attacks[selIndex] == NULL);
		if (actionNo == 0) {
		  attackCursor.selected = pA->attacks[selIndex]->occupying;
		  attackCursor.x = attackCursor.selected->x;
		  attackCursor.y = attackCursor.selected->y;
		} else if (actionNo == 1) {
		  attackCursor.x = c.selected->x + dropOffsetsX[selIndex] - 1;
		  attackCursor.y = c.selected->y + dropOffsetsY[selIndex] - 1;	
		}
      } else if (keyCode == 0x44) /* D */ {
		do {  
		  ++selIndex;
		  if (selIndex >= pA->length) {selIndex = 0;}
		} while (pA->attacks[selIndex] == NULL);
		if (actionNo == 0) {
		  attackCursor.selected = pA->attacks[selIndex]->occupying;
		  attackCursor.x = attackCursor.selected->x;
		  attackCursor.y = attackCursor.selected->y;
		} else if (actionNo == 1) {
		  attackCursor.x = c.selected->x + dropOffsetsX[selIndex] - 1;
		  attackCursor.y = c.selected->y + dropOffsetsY[selIndex] - 1;	
		}
      } else if (keyCode == 0x55) /* U */ {
		undoMove(c.selected);
		c.selected->takenAction = 0;
		free(pA);
		pA = NULL;
      } else if (keyCode = 0x49) /* I */ {
		switch (actionNo) {
		case 0:
			attack(c.selected,attackCursor.selected);
			unitLastX = 255;
			unitLastY = 255;
			c.selected->takenAction = 1;
			c.selected = NULL;
			free(pA);
			pA = NULL;
			break;
		case 1:
			pA->attacks[selIndex]->occupying = c.selected->carrying;
			c.selected->carrying->x = attackCursor.x;
			c.selected->carrying->y = attackCursor.y;
			c.selected->carrying->takenAction = 1;
			c.selected->carrying = NULL;
			++unitsdeadthisturn;
		
			unitLastX = 255;
			unitLastY = 255;
			c.selected->takenAction = 1;
			c.selected = NULL;
			free(pA);
			pA = NULL;
			break;
		}
      }
    } else {
	  //Move cursor around
	  if (mouseEnabled) {
		__asm__ ("jsr $FF71"); 
		__asm__ ("jsr $FF6B"); 
		__asm__ ("sta %v", mouseButtons);
		__asm__ ("lda $00,x");
		__asm__ ("sta %v",mouseX);
		__asm__ ("lda $01,x");
		__asm__ ("sta %v+1",mouseX);
		__asm__ ("lda $02,x");
		__asm__ ("sta %v",mouseY);
		__asm__ ("lda $03,x");
		__asm__ ("sta %v+1",mouseY);
		
		if (mouseButtons % 2 != 0) {
		  c.x = m.left_view + mouseX >> 5;
		  c.y = m.top_view + mouseY >> 5;		  
		  mouseButtons ^= 1;
		}
	  }
      if (keyCode == 0x57) /* W */ {
		if (c.y == 0) {
			if (m.top_view != 0) { --m.top_view; }
		  } else {
			--c.y;
		  }
		} /* A */ else if (keyCode == 0x41) {
		  if (c.x == 0) {
			if (m.left_view != 0) { --m.left_view; }				
		  } else {
			--c.x;
		  }
		} /* S */ else if (keyCode == 0x53) {
		  if (c.y >= 9) {
			if (m.boardHeight - m.top_view >= 11 /*10 + 1*/) { ++m.top_view; }
		  } else {
			++c.y;
		  }
		} /* D */ else if (keyCode == 0x44) {
		  if (c.x >= 14) {
			if (m.boardWidth - m.left_view >= 16 /*15 + 1*/) { ++m.left_view; }
		  } else {
			++c.x;
		  }
		} /* U */ else if (keyCode == 0x55) {
		  if (c.selected != 0) {
			if (unitLastX == 255) {
			  c.selected = 0;
			  c.x = c.storex;
			  c.storex = -1;
			  c.y = c.storey;
			  c.storey = -1;
			} else {
			  undoMove(c.selected);
			}
		  }
		} /* I */ else if (keyCode == 0x49) {
		  if (c.selected == NULL) {
			c.selected = m.board[c.x+m.left_view+m.boardWidth*(c.y+m.top_view)].occupying;
			if (c.selected == NULL) {
			  menuOptions.length = 3;
			  menuOptions.selected = 0;
			  menuOptions.options[0] = OPTION_END;
			  menuOptions.options[1] = OPTION_QUIT;
				menuOptions.options[2] = OPTION_CONCEDE;
			  //menuOptions.options[3] = OPTION_MOUSETOGGLE;
			  //++menuOptions.length;
			} else {
			  c.storex = c.x;
			  c.storey = c.y;
			}
		  } else {
			if (unitLastX == 255) {
			  if (c.selected->team == m.whoseTurn && !c.selected->takenAction && move(c.selected,c.x+m.left_view,c.y+m.top_view)) {
				if (attackCursor.selected != NULL) {
					menuOptions.length = 1;
					menuOptions.selected = 0;
					if (attackCursor.selected->index == c.selected->index) {
						menuOptions.options[0] = OPTION_JOIN;
					} else {
						menuOptions.options[0] = OPTION_LOAD;
					}
				} else {
				  pA = malloc(sizeof(struct possibleAttacks));
				  getPossibleAttacks(pA,c.x+m.left_view,c.y+m.top_view);
				  menuOptions.selected = 0;  
				  menuOptions.length = 0;
				  if (pA->length != 0) {
					menuOptions.options[menuOptions.length] = OPTION_ATTACK;
					++menuOptions.length;  
				  } else {
					free(pA);
				    pA = NULL;  
				  }
				  if (c.selected->carrying != NULL && sizeofGetPossibleDrops(c.selected) != 0) {
 					menuOptions.options[menuOptions.length] = OPTION_DROP;
					++menuOptions.length;
				  } else if ((c.selected->index == 2 || c.selected->index == 3) && m.board[c.selected->x + m.boardWidth*c.selected->y].base != NULL && m.board[c.selected->x + m.boardWidth*c.selected->y].base->team != c.selected->team) {
					menuOptions.options[menuOptions.length] = OPTION_CAPTURE;
					++menuOptions.length;
				  }
				  menuOptions.options[menuOptions.length] = OPTION_WAIT;	
 				  ++menuOptions.length;
				}
			  }
			} else {
			  // unitLastX != 255
			  // Do attack action
			}
		  } 
        }
      }
    }
  return;
}

void loadPalette() {
  unsigned char i;

  POKE(0x9F20,00);
  POKE(0x9F21,0xFA);
  POKE(0x9F22,0x11);
  i = 0;
  do {
    POKE(0x9F23,customPalette[i]);
    ++i;
  } while (i != 0);
  do {
    POKE(0x9F23,customPalette[256+i]);
    ++i;
  } while (i != 0);
  return;
}

void clearScreen() {
  unsigned char i = 0;

  POKE(0x9F20,0);
  POKE(0x9F21,0);
  POKE(0x9F22,0x10);
  while (PEEK(0x9F21) < 15) {
    if (PEEK(0x9F20) >= 30 || PEEK(0x9F21) >= 10) {
      if (PEEK(0x9F20) >= 40) {
				POKE(0x9F20,0);
				__asm__ ("inc $9F21");
      } else {
				POKE(0x9F23,0x88);
				POKE(0x9F23,0x80);
      }
    } else {
      POKE(0x9F23,28);
      POKE(0x9F23,0);
    }
  }
  POKE(0x9F20,0);
  POKE(0x9F21,0x40);
  while (PEEK(0x9F21) < 0x4F) {
    POKE(0x9F23,28);
    POKE(0x9F23,0);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20,0);
      __asm__ ("inc $9F21");
    }
  }

  return;
}
