#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>
#include <string.h>

#include "zsound/pcmplayer.h"
#include "zsound/zsmplayer.h"

#include "structs.h"
#include "main.h"
#include "map.h"
#include "waitforjiffy.h"
#include "fastroutines.h"

/* global variables */
extern unsigned char player1team;
extern unsigned char player2team;
extern unsigned char returnToMenu;
extern struct Terrain terrainArray[];

unsigned char screen_width = 20;
unsigned char screen_height = 15;

unsigned char game_width = 15;
unsigned char game_height = 10;

unsigned char gui_vera_offset = 0x4A;

unsigned char build_mode;

unsigned char vera_display_mode;

unsigned char joystick_num;
unsigned char joystickInput[2];
unsigned char keyCode;
struct Map m;
struct Cursor c;
struct Cursor attackCursor;
unsigned char actionNo;
unsigned char selIndex;
struct possibleAttacks useaspossibleAttacks;
struct possibleAttacks *pA = NULL;
struct Menu menuOptions;

extern unsigned char cbm_k_chrin();

void main() {
  setup();
	POKE(0x00, MAP_HIRAM_BANK);
	pcm_init();
	zsm_init();

  while (1) {
    menu();
    game_start();
    returnToMenu = 0;
    while (!returnToMenu) {
      waitforjiffy();
			pcm_play();
			zsm_play();
			POKE(0x00, MAP_HIRAM_BANK);
			
      __asm__("jsr $FFE4");
      __asm__("sta %v", keyCode);
			handle_joystick();
			
      if (keyCode != 0) {
        keyPressed();
      }
      draw();
    }
		/* stop any sound effects */
		zsm_stopmusic();
    pcm_stop();
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

	POKE(0x00, FILENAMES_BANK);
	menu_files_array = (char **)0xB000;
  while (1) {
    char * temp;
    if (cbm_k_readst()) {
      break;
    }
    ++menu_files_length;

    while (cbm_k_chrin() != '"') { // Discard up to first quote
      if (cbm_k_readst()) {
        break;
      }
    }
    if (cbm_k_readst()) {
      break;
    }

		temp = (char *)(0xA000 + menu_files_length * 32);
    menu_files_array[menu_files_length - 1] = temp;
    while (1) {
      char c = cbm_k_chrin();
      if (c == 0) {
        break;
      }
      if (c == '"') {
        break;
      }
      *temp = c;
      ++temp;
    }
    *temp = '\0';

    while (cbm_k_chrin() != 0) {} // Read rest of entry until the end
    cbm_k_chrin();
    cbm_k_chrin();
  }
  cbm_k_clrch();
  cbm_k_close(12);

  map_files_array = (char **)0xB800;
  map_files_length = 0;
  for (i = 0; i < menu_files_length; ++i) {
    char *temp = menu_files_array[i];
    if (strstr(temp, ".map") != 0) {
      map_files_array[map_files_length] = temp;
      ++map_files_length;
    }
  }
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
  while ( *string) {
    char c = *string;

    if (c >= 0x30 && c <= 0x39) {
      c = c + (186 - 0x30);
    } else if (c >= 'a' && c <= 'z') {
      c = c + (0xA0 - 'a');
    } else if (c == '.') {
      if (break_on_period) {
        return;
      }
      c = 28;
    } else if (c == ' ' || c == '_') {
      c = 28;
    }
    POKE(0x9F23, c);
    ++string;
  }
}

void print_converted_str(unsigned char *string) {
	POKE(0x9F22, 0x10);
	while (*string) {
		POKE(0x9F23, *string);
		POKE(0x9F23, 0x80);
		++string;
	}
}


char choose_co_string[] = "choose a co";

extern char change_filename[];

#define DEVICE_NUM 8

char *map_space = (char *)0xB800;

void menu() {
  static unsigned char already_loaded_dir = 0;
  unsigned short i;
  unsigned char j;
	
  setup_menu();
  change_directory("maps");
  if (!already_loaded_dir) {
    load_dir_menu();
    already_loaded_dir = 1;
  } else {
		POKE(0x00, FILENAMES_BANK);
	}

  player1team = 0;
  player2team = 2;

  POKE(0x9F20, 4 * 2);
  POKE(0x9F21, 0x41);
  POKE(0x9F22, 0x10);

  POKE(0x9F23, 3);
  POKE(0x9F23, 0x00);
  POKE(0x9F22, 0x20);
  print_ascii_str(" x16 wars ", 0);
  POKE(0x9F22, 0x10);
  POKE(0x9F23, 3 + 32 * 2 /* blue unit */ );
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
    print_ascii_str(menu_files_array[i], 1); // "CD:31438"   
  }

  do {
    __asm__("jsr $FFE4");
    __asm__("sta %v", keyCode);
  }
  while (keyCode != 0);

  while (1) {
		waitforjiffy();
    __asm__("jsr $FFE4");
    __asm__("sta %v", keyCode);
		handle_joystick();

    if (keyCode != 0) {
      if (keyCode == 0x57 /* W */ && c.x > 0) {
        --c.x;
      } else if (keyCode == 0x53 /* S */ && c.x < menu_files_length - 1) {
        ++c.x;
      } else if (keyCode == 0x0d /* Enter */ || keyCode == 0x49 /* I */ ) {
        break;
			} else if (keyCode == 'q') {
				reset_quit();
      } else if (keyCode >= 0x31 && keyCode <= 0x38 /* Between 1 & 8 */ ) {
        unsigned char tempteam = (keyCode - 0x31) % 4;
        // change player 2 team
        if (keyCode >= 0x35) {
          if (player1team != tempteam) {
            player2team = tempteam;
          }
          // change player 1 team
        } else {
          if (player2team != tempteam) {
            player1team = tempteam;
          }
        }
      }
      /* redraw cursor showing selected map */
      POKE(0x9F20, 2);
      POKE(0x9F22, 0);
      for (i = 6; i < screen_height; ++i) {
        POKE(0x9F21, 0x40 + i);
        POKE(0x9F23, 0x88);
      }
      POKE(0x9F20, 2);
      POKE(0x9F21, c.x + 0x46);
      POKE(0x9F23, 196);

      POKE(0x9F22, 0x10);
      POKE(0x9F20, 4 * 2);
      POKE(0x9F21, 0x41);
      POKE(0x9F23, 3 + player1team * 32);
      POKE(0x9F23, player1team << 4);
      POKE(0x9F20, 15 * 2);
      POKE(0x9F23, 3 + player2team * 32);
      POKE(0x9F23, (player2team << 4) | 0x4);
    }
  }
	POKE(0x00, MAP_HIRAM_BANK);
	memset((char *)HIRAM_START, 0, 0x2000);
	
	i = 0;
	j = 1;
	while (j != 0) {
		POKE(0x00, FILENAMES_BANK);
		j = menu_files_array[c.x][i];
		POKE(0x00, MAP_HIRAM_BANK);
		map_space[i] = j;
		++i;
	}
  cbm_k_setnam(map_space);
  cbm_k_setlfs(0xFF, DEVICE_NUM, 0);
	POKE(0x00, MAP_HIRAM_BANK);
  i = cbm_k_load(0, (unsigned short) map_space);
  initMapData(map_space);
  change_directory("..");
	
	j = 0;
	choose_co_stuff:
	clear_layer1();
	POKE(0x9F20, 4);
	POKE(0x9F21, 0x43);
	POKE(0x9F22, 0x20);
	
	POKE(0x9F23, ('p' - 'a' + 0xA0));
	POKE(0x9F23, 187 + j);
	POKE(0x9F23, 28);
	print_ascii_str(choose_co_string, 0);
	
	for (i = 0; i < CO_NAMES_ARRAY_LEN; ++i) {
		POKE(0x9F21, i + 0x45);
		POKE(0x9F20, 6);
		print_ascii_str(co_names_array[i], 0);
	}
	POKE(0x9F20, 4);
	POKE(0x9F21, 0x45);
	POKE(0x9F23, 196);
	
	c.x = 0;
	POKE(0x9F20, 4);
	POKE(0x9F22, 0);
	POKE(0x9F21, 0x45);
	while (1) {
		waitforjiffy();
    __asm__("jsr $FFE4");
    __asm__("sta %v", keyCode);
		handle_joystick();
		
    if (keyCode != 0) {
			if (keyCode == 0x57 /* W */ && c.x > 0) {
        --c.x;
				POKE(0x9F23, 28);
				__asm__ ("dec $9F21");
				POKE(0x9F23, 196);
      } else if (keyCode == 0x53 /* S */ && c.x < CO_NAMES_ARRAY_LEN - 1) {
        ++c.x;
				POKE(0x9F23, 28);
				__asm__ ("inc $9F21");
				POKE(0x9F23, 196);
      } else if (keyCode == 0x0d /* Enter */ || keyCode == 0x49 /* I */ ) {
        break;
      }
		}
	}
	
	if (j == 0) {
		j = 1;
		player1co = c.x;
		goto choose_co_stuff;
	} else {
		player2co = c.x;
	}
	load_co_music();	
  return;
}

char change_filename[10] = "cd:";
void change_directory(char *s) {
  strcpy(change_filename + 3, s);
  cbm_k_setnam(change_filename);
  cbm_k_setlfs(15, DEVICE_NUM, 15);
  cbm_k_open();
  cbm_k_clrch();
  cbm_k_close(15);
}

char co_load_filename[12];
void load_co_music() {
	static unsigned char i;
	
	change_directory("sound");
	
	POKE(0x9F20, 4);
	POKE(0x9F21, 0x43);
	POKE(0x9F22, 0x20);
	print_ascii_str("loading ", 0);
	/* Generate file to load */
	for (i = 0; i < 2; ++i) {
		strcpy(co_load_filename, co_names_array[i ? player2co : player1co]);
		strcat(co_load_filename, ".zsm");
		POKE(0x9F20, 20); 
		print_ascii_str(co_load_filename, 0);
		clearRestOfLine();
		
		cbm_k_setnam(co_load_filename);
		cbm_k_setlfs(0, DEVICE_NUM, 2);
		POKE(0x00, i ? CO2_MUSIC_BANK : CO_MUSIC_BANK);
		cbm_k_load(0, HIRAM_START);
	}
	POKE(0x00, MAP_HIRAM_BANK);
	change_directory("..");
}

void clear_layer1() {
	POKE(0x9F20, 0);
  POKE(0x9F21, 0x40);
	POKE(0x9F22, 0x10);
  while (PEEK(0x9F21) < 0x5F) {
    POKE(0x9F23, 28);
    POKE(0x9F23, 0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20, 0);
      __asm__("inc $9F21");
    }
  }
}

void setup_menu() {
  initCursor();
	clear_sprite_table(0);
	
  POKE(0x9F25, 0);
  POKE(0x9F29, 0x30 | vera_display_mode);
	POKE(0x9F2A, 64); // hscale
	POKE(0x9F2B, 64); // vscale 
	

  POKE(0x9F20, 0);
  POKE(0x9F21, 0);
  POKE(0x9F22, 0x10);
  while (PEEK(0x9F21) < 15) {
    POKE(0x9F23, 0x88);
    POKE(0x9F23, 0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20, 0);
      __asm__("inc $9F21");
    }
  }
  clear_layer1();
}

extern unsigned short turncounter;
extern unsigned char unitsdeadthisturn;
extern unsigned char currentbases;
extern unsigned char oldbases;
extern unsigned char currentunitsprites;
extern unsigned char oldunitsprites;

extern unsigned char terrainIsSet[LEN_TERRAIN_ARRAY];

extern unsigned char path_array_x[12];
extern unsigned char path_array_y[12];

extern void mapData_setScreenRegisters();

void game_start() {
  /* Reset some game variables */
  mapData_setScreenRegisters();
	
  unitLastX = 255;
  unitLastX = 255;
  unitLastFuel = 255;
  baseLastHP = 255;
	
	build_mode = 0;
	
  POKE(0x9F25, 0);
  POKE(0x9F29, 0x70 | vera_display_mode);
  clearScreen();
  // initMapData(testMap);
  clearUI();
  initCursor();
	
	zsm_startmusic(CO_MUSIC_BANK, HIRAM_START);
	zsm_forceloop(0);
}

#define UNIT_CHR_FILELEN 4096
#define TILE_CHR_FILELEN 1922 
#define SPRITE_CHR_FILELEN 3138
#define LETTER_CHR_FILELEN 4738
#define EXPL_CHR_FILELEN 4608

#define ARROW_CHR_FILELEN 2048

/* load_address = 0xA000, start of hiram */
#define LOAD_ADDRESS 0xA000

void setup() {
	
	__asm__ ("lda $9F29");
	__asm__ ("and #$0F");
	__asm__ ("sta %v", vera_display_mode);
  POKE(0x9F29, 0x70 | vera_display_mode);
  
	POKE(0x9F2A, 64); // hscale
	POKE(0x9F2B, 64); // vscale 
  
  __asm__("stz $9F25");

  __asm__("stz $9F30");
  __asm__("stz $9F31");
  __asm__("stz $9F32");
  __asm__("stz $9F33");
  __asm__("stz $9F37");
  __asm__("stz $9F38");
  __asm__("stz $9F39");
  __asm__("stz $9F3A"); // reset scroll

  POKE(0x9F2D, 0x62); // default map height & width, 4 bpp tiles
  POKE(0x9F2E, 0x00); // map based at vram addr 0
  POKE(0x9F2F, 0x43); // tile base addr 0x8000

  POKE(0x9F34, 0x62); // default map height & width, 4 bpp tiles
  POKE(0x9F35, 0x20); // map based at vram addr 0x4000 (0x20 * 512)
  POKE(0x9F36, 0x43); // tile base addr 0x8000

  POKE(0x9F30, 0); // Scroll
  POKE(0x9F37, 0);
  POKE(0x9F31, 0);
  POKE(0x9F38, 0);
  POKE(0x9F33, 0);
  POKE(0x9F3A, 0);
	
	POKE(0x00, MAP_HIRAM_BANK);
  cbm_k_setnam("red.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0x80);
  POKE(0x9F22, 0x10);
  POKEW(0x2, LOAD_ADDRESS);
  POKEW(0x4, 0x9F23);
  POKEW(0x6, UNIT_CHR_FILELEN);
  __asm__("jsr $FEE7");

  cbm_k_setnam("green.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0x90);
  __asm__("jsr $FEE7");

  cbm_k_setnam("blue.chr");
  cbm_k_setlfs(0xFF, DEVICE_NUM, 0x00);
  cbm_k_load(0, LOAD_ADDRESS);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0xA0);
  __asm__("jsr $FEE7");

  cbm_k_setnam("yellow.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0xB0);
  __asm__("jsr $FEE7");

  cbm_k_setnam("tile.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0xC0);
  //POKEW(0x2, (unsigned short)load_address);
  //POKEW(0x4, 0x9F23);
  POKEW(0x6, TILE_CHR_FILELEN);
  __asm__("jsr $FEE7");

  cbm_k_setnam("letter.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0xD0);
  POKEW(0x6, LETTER_CHR_FILELEN);
  __asm__("jsr $FEE7");

  cbm_k_setnam("sprites.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0x00);
  POKE(0x9F22, 0x11);
  POKEW(0x6, SPRITE_CHR_FILELEN);
  __asm__("jsr $FEE7");

  cbm_k_setnam("expl.chr");
  load_file(0);
  POKE(0x9F20, 0x00);
  POKE(0x9F21, 0x10);
  //POKE(0x9F22, 0x11);
  POKEW(0x6, EXPL_CHR_FILELEN);
  __asm__("jsr $FEE7");

	cbm_k_setnam("arrow.chr");
	load_file(2);
	POKE(0x9F20, 0x00);
  POKE(0x9F21, 0x28);
  //POKE(0x9F22, 0x11);
	POKEW(0x6, ARROW_CHR_FILELEN);
	__asm__("jsr $FEE7");
	
	cbm_k_setnam("palette.bin");
	load_file(2);
	POKE(0x9F20, 0x00);
  POKE(0x9F21, 0xFA);
	POKE(0x9F22, 0x11);
	POKEW(0x6, 512);
	__asm__("jsr $FEE7");
	
	change_directory("sound");
	
	/* Load sound effects */
	cbm_k_setnam("expling.zcm");
	POKE(0x00, UNIT_EXPLODING_BANK);
	load_file(2); // Header-less loading 
	
	cbm_k_setnam("mvcursor1.zcm");
	POKE(0x00, MAP_CURSOR_MOVE_BANK);
	load_file(2);
	
	cbm_k_setnam("mvcursor2.zcm");
	POKE(0x00, MENU_CURSOR_MOVE_BANK);
	load_file(2);
	
	cbm_k_setnam("selnounit.zcm");
	POKE(0x00, SELECT_NO_UNIT_BANK);
	load_file(2);
	
	cbm_k_setnam("selunit.zcm");
	POKE(0x00, SELECT_UNIT_BANK);
	load_file(2);
	
	cbm_k_setnam("unselect.zcm");
	POKE(0x00, UNSELECT_BANK);
	load_file(2);
	
	cbm_k_setnam("inaction.zcm");
	POKE(0x00, INVALID_ACTION_BANK);
	load_file(2);
	
	cbm_k_setnam("missuccess.zcm");
	POKE(0x00, MISSION_SUCCESS_MUSIC_BANK);
	load_file(2);
	
	/* 
	Leaving this here for now:
	https://devster.proboards.com/thread/1218/tip-convert-midi-music-xgc
	*/
	
	change_directory("..");
	setup_joystick();
}

void __fastcall__ load_file(unsigned char sa) {
	cbm_k_setlfs(0, DEVICE_NUM, sa);
	cbm_k_load(0, LOAD_ADDRESS);
}


void draw() {
  renderMap();
  drawUI();
}

/*
Unit index listing:
$0 : APC
$1 : Recon
$2 : Mech
$3 : Infantry
$9 : Anti Air
$A : Medium Tank
$B : Small Tank
$C : Missiles
$D : Rockets
$E : Artillery
$10 : Transport
$11 : Copter
$12 : Bomber
$13 : Fighter
$18 : Lander
$19 : Submarine
$1A : Cruiser
$1B : Battleship

*/
unsigned char unitIndexes[] = {
	0, 1, 2, 3,
	255, 255, 255, 255,
	255, 4, 5, 6,
	7, 8, 9, 255,
	10, 11, 12, 13,
	255, 255, 255, 255,
	14, 15, 16, 17};

unsigned char unitNames[][11] = {
	/* 00 | 00 */ {0xa0, 0xaf, 0xa2},												 // APC
	/* 01 | 01 */ {0xb1, 0xa4, 0xa2, 0xae, 0xad},									 // Recon
	/* 02 | 02 */ {0xac, 0xa4, 0xa2, 0xa7},											 // Mech
	/* 03 | 03 */ {0xa8, 0xad, 0xa5, 0xa0, 0xad, 0xb3, 0xb1, 0xb8},					 // Infantry
	/* 04 | 09 */ {0xa0, 0xad, 0xb3, 0xa8, 0x1C, 0xa0, 0xa8, 0xb1},					 // Anti Air
	/* 05 | 10 */ {0xac, 0xa4, 0xa3, 0xa8, 0xb4, 0xac, 0x1C, 0xb3, 0xa0, 0xad, 0xaa}, // Medium Tank
	/* 06 | 11 */ {0xb2, 0xac, 0xa0, 0xab, 0xab, 0x1c, 0xb3, 0xa0, 0xad, 0xaa},		 // Small Tank
	/* 07 | 12 */ {0xac, 0xa8, 0xb2, 0xb2, 0xa8, 0xab, 0xa4, 0xb2},					 // Missiles
	/* 08 | 13 */ {0xb1, 0xae, 0xa2, 0xaa, 0xa4, 0xb3, 0xb2},						 // Rockets
	/* 09 | 14 */ {0xa0, 0xb1, 0xb3, 0xa8, 0xab, 0xab, 0xa4, 0xb1, 0xb8},			 // Artillery
	/* 10 | 16 */ {0xb3, 0xb1, 0xa0, 0xad, 0xb2, 0xaf, 0xae, 0xb1, 0xb3},		 // Transport
	/* 11 | 17 */ {0xa1, 28,   0xa2, 0xae, 0xaf, 0xb3, 0xa4, 0xb1},								 // Copter
	/* 12 | 18 */ {0xa1, 0xae, 0xac, 0xa1, 0xa4, 0xb1},								 // Bomber
	/* 13 | 19 */ {0xa5, 0xa8, 0xa6, 0xa7, 0xb3, 0xa4, 0xb1},						 // Fighter
	/* 14 | 24 */ {0xab, 0xa0, 0xad, 0xa3, 0xa4, 0xb1},								 // Lander
	/* 15 | 25 */ {0xb2, 0xb4, 0xa1, 0xac, 0xa0, 0xb1, 0xa8, 0xad, 0xa4},			 // Submarine
	/* 16 | 26 */ {0xa2, 0xb1, 0xb4, 0xa8, 0xb2, 0xa4, 0xb1},						 // Cruiser
	/* 17 | 27 */ {0xa1, 0xa0, 0xb3, 0xb3, 0xab, 0xa4, 0xb2, 0xa7, 0xa8, 0xaf}};		 // Battleship

unsigned char unitNameLengths[] = {3, 5, 4, 8, 8, 11, 10, 8, 7, 9, 9, 8, 6, 7, 6, 9, 7, 10};

unsigned char globalSize;
void drawText(unsigned char * string, unsigned char size, unsigned char x, unsigned char y, unsigned char layer) {
  unsigned char i = 0;
  POKE(0x9F20, x << 1);
  POKE(0x9F21, (layer ? 0x40 : 0x00) + y);
  POKE(0x9F22, 0x10);
  while (i < size) {
    POKE(0x9F23, string[i]);
    POKE(0x9F23, 0x80);
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
#define OPTION_SUPPLY 12

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
	{0xac, 0xa4, 0xad, 0xb5, 0x00},
	{0xb2, 0xb4, 0xaf, 0xaf, 0xab, 0xb8}};


unsigned char healthText[] = {0xa7, 0xa4, 0xa0, 0xab, 0xb3, 0xa7, 0x1c};

unsigned char baseTypes[][8] = {
  {0xa1, 0xa0, 0xb2, 0xa4}, // CITY
  {0xa7, 0xb0}, // HQ 
  {0xa1, 0xa0, 0xb2, 0xa4}, // BASE 
	{0xa0, 0xa8, 0xb1, 0xaf, 0xae, 0xb1, 0xb3}, // AIRPORT
	{0xaf, 0xae, 0xb1, 0xb3}, // PORT
};
unsigned char baseTypeStringLengths[] = {4, 2, 4, 7, 4};

unsigned char damageString[] = {0xa3, 0xac, 0xa6};

void clearRestOfLine() {
  POKE(0x9F22, 0x20);
	POKE(0x9F20, PEEK(0x9F20) & 0xFE);
  while (PEEK(0x9F20) < 40) {
    POKE(0x9F23, 28);
  }
}

void clearOtherLines() {
  __asm__("inc $9F21");
  while (PEEK(0x9F21) < screen_height + 0x40) {
    __asm__("stz $9F20");
    clearRestOfLine();
    __asm__("inc $9F21");
  }
}

extern unsigned char unitsCanBuild[][10];
extern unsigned char sizeOfUnitsBuildListMinus1[];
extern unsigned char unitsCost[];
extern unsigned char playerFunds[];
extern unsigned char playerUnitCounts[];
extern unsigned char moneyMatters;

#define SCREENBYTE(a)((a) >= 10 ? 150 + (a) : 186 + (a))

void drawUI() {
	static unsigned char old_player_funds = 0xFF;
	static unsigned char funds_tens_digit;
	static unsigned char funds_ones_digit;
  struct Unit *unitPointer;
  void *test = NULL;
  unsigned char dummy;

  POKE(0x9F20, (screen_width - 6) * 2);
  POKE(0x9F21, 0x41);
  POKE(0x9F22, 0x10);

  POKE(0x9F23, 179 /* 'T' - 'A' + 160 */ );
  POKE(0x9F23, 0x80);
  POKE(0x9F23, 0x03 + (m.whoseTurn << 5)); // Infantry with color of current player's team
  POKE(0x9F23, m.whoseTurn << 4);
	if (moneyMatters) {
		__asm__ ("inc $9F21");
		__asm__ ("inc $9F21");
		POKE(0x9F20, (screen_width - 6) * 2);
		POKE(0x9F23, 0xA0 + 'g' - 'a');
	
		__asm__ ("inc $9F21");
		POKE(0x9F20, (screen_width - 7) * 2);
		POKE(0x9F22, 0x10);
		if (playerFunds[m.whoseTurn] != old_player_funds) {
			funds_tens_digit = 186 + playerFunds[m.whoseTurn] / 10;
			funds_ones_digit = 186 + playerFunds[m.whoseTurn] % 10;
			
			old_player_funds = playerFunds[m.whoseTurn];
		}
		if (funds_tens_digit != 186) {
			POKE(0x9F23, funds_tens_digit);
			POKE(0x9F23, 0x80);
		}
		POKE(0x9F23, funds_ones_digit);
		POKE(0x9F23, 0x80);
		POKE(0x9F23, 186);
		POKE(0x9F23, 0x80);
		POKE(0x9F23, 186);
		POKE(0x9F23, 0x80);
		POKE(0x9F23, 186);
		POKE(0x9F23, 0x80);
		POKE(0x9F23, 28);
	}
	
  unitPointer = m.board[(c.y + m.top_view) * m.boardWidth + c.x + m.left_view].occupying;
  if (unitPointer == NULL) {
    unitPointer = c.selected;
  }
  if (pA != NULL && menuOptions.length == 0) {
    unitPointer = attackCursor.selected;
  }
	
	if (build_mode != 0) {
		static unsigned char lastindex = 0xFF;
		static unsigned char lasttens;
		static unsigned char lastones;
		unsigned char scroll = attackCursor.x;
		unsigned char index;
		
		if (attackCursor.x == 0) {
			scroll = 0;
		} else if (attackCursor.x == sizeOfUnitsBuildListMinus1[build_mode - 2]) {
			scroll = attackCursor.x - 2;
		} else {
			scroll = attackCursor.x - 1;	
		}
		
		for (dummy = scroll; dummy < scroll + 3 && dummy <= sizeOfUnitsBuildListMinus1[build_mode - 2]; ++dummy) {
			index = unitIndexes[ unitsCanBuild[build_mode - 2][dummy] ];
			drawText(unitNames[index], unitNameLengths[index], 2, gui_vera_offset - 0x40 + 2 + dummy - scroll, 1);
			clearRestOfLine();
			
			POKE(0x9F22, 0x20);
			POKE(0x9F20, 28);
			
			if (index != lastindex) {
				lastindex = index;
				
				lasttens = 186 + unitsCost[index] / 10;
				lastones = 186 + unitsCost[index] % 10;
			}
			if (lasttens != 186) { POKE(0x9F23, lasttens); }
			POKE(0x9F23, lastones);
			
			POKE(0x9F23, 186); // 000 
			POKE(0x9F23, 186);
			POKE(0x9F23, 186);
			clearRestOfLine();
			
		}
		clearOtherLines();
		POKE(0x9F21, gui_vera_offset + 2);
		for (dummy = scroll; dummy < scroll + 3; ++dummy) {
			POKE(0x9F20, 2);
			POKE(0x9F23, attackCursor.x == dummy ? 196 : 28);
			__asm__ ("inc $9F21");
		}
		
	} else if (menuOptions.length != 0) {
    // display menu options
    POKE(0x9F21, gui_vera_offset + 1);
    POKE(0x9F22, 0x20);
    for (dummy = 0; dummy < menuOptions.length; dummy++) {
      POKE(0x9F20, 2);
      POKE(0x9F23, menuOptions.selected == dummy ? 196 : 28);
			print_converted_str(optionStrings[menuOptions.options[dummy]]);
      clearRestOfLine();
      __asm__("inc $9F21");
    }
    __asm__("dec $9F21");
    clearOtherLines();
  } else {
    if (m.board[c.x + m.left_view + m.boardWidth * (c.y + m.top_view)].base != NULL && (unitPointer == NULL || c.selected != NULL)) {
      struct Captureable *capt = m.board[c.x + m.left_view + m.boardWidth * (c.y + m.top_view)].base;
      drawText(baseTypes[capt->type], baseTypeStringLengths[capt->type], 1, gui_vera_offset - 0x40 + 1, 1);
      POKE(0x9F20, (baseTypeStringLengths[capt->type] + 1) << 1);
      POKE(0x9F22, 0x10);

      POKE(0x9F23, 28);
      POKE(0x9F23, 0x80);
      POKE(0x9F23, 'T' - 'A' + 0xA0);
      POKE(0x9F23, 0x80);
      if (capt->team != 4) {
        POKE(0x9F23, 0x03);
        POKE(0x9F23, capt->team << 4);
      } else {
        POKE(0x9F23, 'N' - 'A' + 0xA0);
        POKE(0x9F23, 0x80);
      }
      clearRestOfLine();

      POKE(0x9F21, gui_vera_offset + 2);
      POKE(0x9F20, 2);
      POKE(0x9F22, 0x20);
      POKE(0x9F23, 'H' - 'A' + 160);
      POKE(0x9F23, 'P' - 'A' + 160);
      POKE(0x9F23, 28);
      POKE(0x9F23, 186 + (capt->health / 10));
      POKE(0x9F23, 186 + (capt->health % 10));
      clearRestOfLine();
      clearOtherLines();
    } else if (unitPointer != NULL) {
      dummy = unitIndexes[unitPointer->index];
      drawText(unitNames[dummy], unitNameLengths[dummy], 1, gui_vera_offset - 0x40 + 1, 1);
      clearRestOfLine();

      POKE(0x9F20, 2);
      POKE(0x9F21, gui_vera_offset + 2);
      POKE(0x9F22, 0x20);
      POKE(0x9F23, 183); // X
      if (unitPointer->x >= 10) {
        POKE(0x9F23, 186 + unitPointer->x / 10);
      }
      POKE(0x9F23, 186 + (unitPointer->x % 10));

      POKE(0x9F23, 28);
      POKE(0x9F23, 184); // Y
      if (unitPointer->y >= 10) {
        POKE(0x9F23, 186 + unitPointer->y / 10);
      }
      POKE(0x9F23, 186 + (unitPointer->y % 10));

      /* Show mem address */
      /*POKE(0x9F23, 28);
      POKE(0x9F23, SCREENBYTE(((unsigned short)unitPointer >> 12) & 0xF));
      POKE(0x9F23, SCREENBYTE(((unsigned short)unitPointer >> 8) & 0xF));
      POKE(0x9F23, SCREENBYTE(((unsigned short)unitPointer >> 4) & 0xF));
      POKE(0x9F23, SCREENBYTE((unsigned short)unitPointer & 0xF));
      */
      if (unitPointer->health <= 99) {
        POKE(0x9F23, 28);
        POKE(0x9F23, 167);
        POKE(0x9F23, 186 + (unitPointer->health / 10));
        POKE(0x9F23, 186 + (unitPointer->health % 10));
      }
      if (unitPointer->ammo < 10) {
        POKE(0x9F23, 28);
        POKE(0x9F23, 160 + 'A' - 'A');
        POKE(0x9F23, 186 + unitPointer->ammo);
      }
      if (unitPointer->fuel < maxFuel[unitPointer->index]) {
        POKE(0x9F23, 28);
        POKE(0x9F23, 0xa5); // F
        POKE(0x9F23, 186 + (unitPointer->fuel / 10));
        POKE(0x9F23, 186 + (unitPointer->fuel % 10));
      }
      clearRestOfLine();

      if (pA != NULL && menuOptions.options[0] == OPTION_ATTACK && attackCursor.selected != NULL) {
        static unsigned char damageHundredsDigit;
        static unsigned char damageModTen;
        static unsigned char damageDivTen;
        static struct Unit *cs = NULL;
        static struct Unit *acs = NULL;

        if (c.selected != cs || attackCursor.selected != acs) {
          unsigned char damagePreviewNum;

          cs = c.selected;
          acs = attackCursor.selected;

          damagePreviewNum = damagePreview(c.selected, attackCursor.selected);
          damageHundredsDigit = 186 + (damagePreviewNum / 100);
          damageDivTen = 186 + ((damagePreviewNum / 10) % 10);
          damageModTen = 186 + (damagePreviewNum % 10);
        }
        drawText(damageString, sizeof(damageString), 1, 13, 1);

        POKE(0x9F20, 2 * 5);
        POKE(0x9F21, gui_vera_offset + 3);
        POKE(0x9F22, 0x20);
        if (damageHundredsDigit != 186 /* A zero */ ) {
          POKE(0x9F23, damageHundredsDigit);
        }
        POKE(0x9F23, damageDivTen);
        POKE(0x9F23, damageModTen);
        clearRestOfLine();
      }
      clearOtherLines();
    } else {
      POKE(0x9F22, 0x20);
      POKE(0x9F21, gui_vera_offset);
      clearOtherLines();
    }
  }
}

void clearUI() {
  POKE(0x9F20, 0x30);
  POKE(0x9F21, 0x40);
  POKE(0x9F22, 0x10);
  while (PEEK(0x9F21) < 0x46) {
    __asm__("lda #28");
    __asm__("ldx #$80");

    __asm__("sta $9F23");
    __asm__("stx $9F23");
    __asm__("sta $9F23");
    __asm__("stx $9F23");
    __asm__("sta $9F23");
    __asm__("stx $9F23");
    __asm__("sta $9F23");
    __asm__("stx $9F23");
    __asm__("sta $9F23");
    __asm__("stx $9F23");

    POKE(0x9F20, game_width << 1);
    __asm__("inc $9F21");
  }

  POKE(0x9F21, 0x40 + game_height);
  while (PEEK(0x9F21) < 0x40 + screen_height) {
    POKE(0x9F20, 0);
    __asm__("lda #28");
    __asm__("ldx #$80");
    __asm__("ldy #40");

    clearUILoop:
      __asm__("sta $9F23"); // 20 times (width of screen)
    __asm__("stx $9F23");
    __asm__("dey");
    __asm__("bne %g", clearUILoop);

    __asm__("inc $9F21");
  }
}

extern unsigned char maxSteps;
extern struct Unit * checkU;
extern struct Tile tempT;

extern unsigned char actually_move;
extern unsigned char mvmtNegFactor;
extern unsigned char recurs_depth;

unsigned char dropOffsetsX[] = {
  0, 1, 2, 1
};
unsigned char dropOffsetsY[] = {
  1, 0, 1, 2
};



void keyPressed() {
	
	if (build_mode != 0) {
		if (keyCode == 0x57 /* W */) {
			if (attackCursor.x != 0) {
				--attackCursor.x;
			}
			
		} else if (keyCode == 0x53 /* S */) {
			if (attackCursor.x < sizeOfUnitsBuildListMinus1[build_mode - 2]) {
				++attackCursor.x;
			}
			
		} else if (keyCode == 0x55) /* U */ {
			build_mode = 0;
			
		} else if (keyCode == 0x49) /* I */ {
			unsigned char index = unitsCanBuild[build_mode - 2][attackCursor.x];
			unsigned char cost = unitsCost[unitIndexes[index]];
			if (playerFunds[m.whoseTurn] >= cost && playerUnitCounts[m.whoseTurn] < 50) {
				struct Unit *new_unit = malloc_unit();
				
				playerFunds[m.whoseTurn] -= cost;
				// build 
				initUnit(new_unit, c.x + m.left_view, c.y + m.top_view, index, m.whoseTurn);
				new_unit->takenAction = 1;
				build_mode = 0;
			} else {
				pcm_trigger_digi(INVALID_ACTION_BANK, HIRAM_START);
			}
		}
		
	} else if (menuOptions.length != 0) {
    if (keyCode == 0x57) /* W */ {
      if (menuOptions.selected != 0) {
        menuOptions.selected--;
				pcm_trigger_digi(MENU_CURSOR_MOVE_BANK, HIRAM_START);
      }
    } else if (keyCode == 0x53) /* S */ {
      if (menuOptions.selected < menuOptions.length - 1) {
        menuOptions.selected++;
				pcm_trigger_digi(MENU_CURSOR_MOVE_BANK, HIRAM_START);
      }
    } else if (keyCode == 0x55) /* U */ {
      static unsigned char typemenu;
			typemenu = 1;
			menuOptions.length = 0;
      if (c.selected != NULL) {
				pcm_trigger_digi(UNSELECT_BANK, HIRAM_START);
        undoMove(c.selected);
				calculate_unit_path();
				typemenu = 0;
      }
      if (pA != NULL) {
        pA = NULL;
				typemenu = 0;
      }
			if (typemenu) { pcm_trigger_digi(UNSELECT_BANK, HIRAM_START); }
    } else if (keyCode == 0x49) /* I */ {
      switch (menuOptions.options[menuOptions.selected]) {
      case OPTION_END:
        menuOptions.length = 0;
				pcm_trigger_digi(SELECT_NO_UNIT_BANK, HIRAM_START);
        nextTurn();
        break;
      case OPTION_CONCEDE:
        menuOptions.length = 0;
        win(m.whoseTurn == player1team ? player2team : player1team);
        break;
      case OPTION_QUIT:
        menuOptions.length = 0;
				zsm_stopmusic();
				pcm_stop();
        reset_quit();
      case OPTION_WAIT:
        if (pA != NULL) {
          pA = NULL;
          attackCursor.selected = NULL;
        }
				wait_code_plus_sound:
				pcm_trigger_digi(SELECT_NO_UNIT_BANK, HIRAM_START);
        c.selected->takenAction = 1;
        wait_code:
        unitLastX = 255;
        unitLastY = 255;
        unitLastFuel = 255;
        c.x = c.selected->x - m.left_view;
        c.y = c.selected->y - m.top_view;
        c.selected = NULL;
        menuOptions.length = 0;
        break;
      case OPTION_CAPTURE:
        capture(c.selected, m.board[c.selected->x + m.boardWidth * c.selected->y].base);
				
        goto wait_code_plus_sound;
        /*unitLastX = 255;
        unitLastY = 255;
        unitLastFuel = 255;
        c.selected = NULL;
        menuOptions.length = 0;*/
        break;
      case OPTION_SUPPLY:
        supplyUnits(c.selected);

        c.selected->takenAction = 1;
				/* Cash sound effect maybe at some point */
        goto wait_code;
        break;
      case OPTION_DROP:
        // Code for dropping off units
        menuOptions.length = 0;
        pA = &useaspossibleAttacks;
        getPossibleDrops(pA, c.selected);
        actionNo = 1;
        selIndex = 1;
        while (pA->attacks[selIndex] == NULL) {
          if (++selIndex >= 4) {
            selIndex = 0;
          }
        }
        attackCursor.x = c.selected->x + dropOffsetsX[selIndex] - 1;
        attackCursor.y = c.selected->y + dropOffsetsY[selIndex] - 1;

        break;
      case OPTION_LOAD:
        attackCursor.selected->carrying = c.selected;
        m.board[c.selected->x + m.boardWidth * c.selected->y].occupying = attackCursor.selected;
        attackCursor.selected = NULL;

        goto wait_code_plus_sound;
        /*unitLastX = 255;
        unitLastY = 255;
        unitLastFuel = 255;
        c.selected = NULL;
        menuOptions.length = 0;*/
        //break;
      case OPTION_ATTACK:
        /* Atttacking */
        actionNo = 0;
        selIndex = 0;
        attackCursor.selected = pA->attacks[0]->occupying;
        attackCursor.x = attackCursor.selected->x;
        attackCursor.y = attackCursor.selected->y;

        menuOptions.store_length = menuOptions.length;
        menuOptions.length = 0;
        break;
      case OPTION_JOIN:
        c.selected->health = MIN(100, c.selected->health + attackCursor.selected->health);
        c.selected->ammo = MIN(10, c.selected->ammo + attackCursor.selected->ammo);
        c.selected->takenAction = 1;

        free(attackCursor.selected);
        attackCursor.selected = NULL;

        goto wait_code;
        /*unitLastX = 255;
        unitLastY = 255;
        unitLastFuel = 255;
        c.selected = NULL;
        menuOptions.length = 0;*/
        //break;
      default:
        break;
      }
    }
  } else /* menuOptions.length == 0 */ {
    if (pA != NULL) {
      // Switch between attack targets.
      if (keyCode == 0x41) /* A */ {
        do {
          selIndex = (selIndex == 0) ? 3 : selIndex - 1;
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
          if (selIndex >= 4) {
            selIndex = 0;
          }
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
        menuOptions.length = menuOptions.store_length;
      } else if (keyCode = 0x49) /* I */ {
        switch (actionNo) {
        case 0:
          attack(c.selected, attackCursor.selected);
          unitLastX = 255;
          unitLastY = 255;
          unitLastFuel = 255;
          c.selected->takenAction = 1;
          c.selected = NULL;
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
          unitLastFuel = 255;
          c.selected->takenAction = 1;
          c.selected = NULL;
          pA = NULL;
          break;
        }
      }
    } else { /* pA == NULL */
      // Move cursor around
			static unsigned char cursorHasMoved;
			cursorHasMoved = 0;
			
      if (keyCode == 0x57) /* W */ {
        if (c.y <= 2) {
          if (m.top_view != 0) {
            --m.top_view;
						cursorHasMoved = 1;
          } else if (c.y != 0) {
						--c.y;
						cursorHasMoved = 1;
					}
        } else {
          --c.y;
					cursorHasMoved = 1;
        }
      } /* A */
      else if (keyCode == 0x41) {
        if (c.x <= 2) {
          if (m.left_view != 0) {
            --m.left_view;
						cursorHasMoved = 1;
          } else if (c.x != 0) {
						--c.x;
						cursorHasMoved = 1;
					}
        } else {
          --c.x;
					cursorHasMoved = 1;
        }
      } /* S */
      else if (keyCode == 0x53) {
        if (c.y >= game_height - 3 /* eg. 10 - 3 */) {
          if (m.boardHeight - m.top_view >= game_height + 1 /* eg. 10 + 1 */ ) {
            ++m.top_view;
						cursorHasMoved = 1;
          } else if (c.y < game_height - 1) {
						++c.y;
						cursorHasMoved = 1;
					}
        } else {
          ++c.y;
					cursorHasMoved = 1;
        }
      } /* D */
      else if (keyCode == 0x44) {
        if (c.x >= game_width - 3 /* eg. 15 - 3 */) {
          if (m.boardWidth - m.left_view >= game_width + 1 /* eg. 15 + 1 */ ) {
            ++m.left_view;
						cursorHasMoved = 1;
          } else if (c.x < game_width - 1) {
						++c.x;
						cursorHasMoved = 1;
					}
        } else {
          ++c.x;
					cursorHasMoved = 1;
        }
      } /* U */
      else if (keyCode == 0x55) {
        if (c.selected != NULL) {
					unsigned char i;
          
					if (unitLastX == 255) {
						pcm_trigger_digi(UNSELECT_BANK, HIRAM_START);
            c.selected = NULL;
            c.x = c.storex;
            m.left_view = m.store_left_view;
            c.storex = -1;
            c.y = c.storey;
            m.top_view = m.store_top_view;
            c.storey = -1;
						
						POKEW(0x9F20, 0xFC28);
						POKE(0x9F22, 0x11);
						for (i = 120; i > 0; --i) {
							__asm__ ("stz $9F23");
						}
          } else {
            // This shouldn't happen i think 
						undoMove(c.selected);
						//calculate_unit_path();
          }
        }
      } /* I */
      else if (keyCode == 0x49) {
        if (c.selected == NULL) {
					struct Tile *tempT = &(m.board[c.x + m.left_view + m.boardWidth * (c.y + m.top_view)]);
          c.selected = tempT->occupying;
					if (c.selected->takenAction) { c.selected = NULL; goto cannot_select_base; }
          if (c.selected == NULL) {
						if (tempT->base != NULL && tempT->base->team == m.whoseTurn && tempT->base->type >= CAPTUREABLE_FACTORY) {
							build_mode = tempT->base->type;
							attackCursor.x = 0;
						} else {
							cannot_select_base:
							pcm_trigger_digi(SELECT_NO_UNIT_BANK, HIRAM_START);
							menuOptions.length = 3;
							menuOptions.selected = 0;
							menuOptions.options[0] = OPTION_END;
							menuOptions.options[1] = OPTION_QUIT;
							menuOptions.options[2] = OPTION_CONCEDE;
						}
          } else {
						pcm_trigger_digi(SELECT_UNIT_BANK, HIRAM_START);
						c.storex = c.x;
            c.storey = c.y;
            m.store_left_view = m.left_view;
            m.store_top_view = m.top_view;
						
						path_array_x[0] = 0xFF;
						//path_array_y[1] = 0xFF;
          }
        } else {
          if (unitLastX == 255) {
						actually_move = 1;
            if (c.selected->team == m.whoseTurn && !c.selected->takenAction && move(c.selected, c.x + m.left_view, c.y + m.top_view)) {
							unsigned char i;
							POKEW(0x9F20, 0xFC28);
							POKE(0x9F22, 0x11);
							for (i = 120; i > 0; --i) {
								__asm__ ("stz $9F23");
							}
							
              if (attackCursor.selected != NULL) {
                menuOptions.length = 1;
                menuOptions.selected = 0;
                if (attackCursor.selected->index == c.selected->index) {
                  menuOptions.options[0] = OPTION_JOIN;
                } else {
                  menuOptions.options[0] = OPTION_LOAD;
                }
              } else {
                pA = &useaspossibleAttacks;
                getPossibleAttacks(pA, c.x + m.left_view, c.y + m.top_view, c.selected->attackRangeMax);
                menuOptions.selected = 0;
                menuOptions.length = 0;
                if (pA->length != 0) {
                  menuOptions.options[menuOptions.length] = OPTION_ATTACK;
                  ++menuOptions.length;
                } else {
                  pA = NULL;
                }
                if (c.selected->carrying != NULL && sizeofGetPossibleDrops(c.selected) != 0) {
                  menuOptions.options[menuOptions.length] = OPTION_DROP;
                  ++menuOptions.length;
                } else if ((c.selected->index == UNIT_MECH || c.selected->index == UNIT_INFANTRY) && m.board[c.selected->x + m.boardWidth * c.selected->y].base != NULL && m.board[c.selected->x + m.boardWidth * c.selected->y].base->team != c.selected->team) {
                  menuOptions.options[menuOptions.length] = OPTION_CAPTURE;
                  ++menuOptions.length;
                }
                menuOptions.options[menuOptions.length] = OPTION_WAIT;
                ++menuOptions.length;
                if (canSupply(c.selected)) {
                  menuOptions.options[menuOptions.length] = OPTION_SUPPLY;
                  ++menuOptions.length;
                }
              }
            } else {
							pcm_trigger_digi(INVALID_ACTION_BANK, HIRAM_START);
						}
          } else {
            // unitLastX != 255
            // Do attack action
          }
        }
      }
			if (cursorHasMoved) {
				pcm_trigger_digi(MAP_CURSOR_MOVE_BANK, HIRAM_START);
				if (c.selected != NULL) {
					calculate_unit_path();
				}
			}				
    }
  }
  return;
}

void calculate_unit_path() {
	unsigned char i;
					
	mvmtNegFactor = 0;
	if (c.x + m.left_view != c.selected->x || c.y + m.top_view != c.selected->y) {
		actually_move = 0;
		for (i = 0; 1; ++i) {
			if (0 == move(c.selected, c.x + m.left_view, c.y + m.top_view)) { break; }
			++mvmtNegFactor;
		}		
		if (mvmtNegFactor != 0) {
			--mvmtNegFactor;
			
			POKEW(0x9F20, 0xFC28);
			POKE(0x9F22, 0x11);
			for (i = 120; i > 0; --i) {
				__asm__ ("stz $9F23");
			}
			memset(path_array_x, 0xFF, 16);
			memset(path_array_y, 0xFF, 16);
				
			move(c.selected, c.x + m.left_view, c.y + m.top_view);
			mvmtNegFactor = 0;
			//for (i = 0; path_array_x[i] != 0xFF; ++i) {}
			//drawMvmtArrow(i);
		}
		for (i = 0; path_array_x[i] != 0xFF; ++i) {}
		drawMvmtArrow(i);
		actually_move = 1;
	} else {
		POKEW(0x9F20, 0xFC28);
		POKE(0x9F22, 0x11);
		for (i = 120; i > 0; --i) {
			__asm__ ("stz $9F23");
		}
	}
}

void clearScreen() {
  unsigned char i = 0;

  POKE(0x9F20, 0);
  POKE(0x9F21, 0);
  POKE(0x9F22, 0x10);
  while (PEEK(0x9F21) < game_height) {
		if (PEEK(0x9F20) >= (screen_width << 1)) {
			__asm__ ("stz $9F20");
			__asm__ ("inc $9F21");
			continue;
		} else if (PEEK(0x9F20) < (game_width << 1)) {
			POKE(0x9F23, 28);
		} else {
			POKE(0x9F23, 0x88);
		}
		POKE(0x9F23, 0x80);
	}
	while (PEEK(0x9F21) < screen_height) {
    POKE(0x9F23, 0x88);
    POKE(0x9F23, 0x80);
  }
	
  POKE(0x9F20, 0);
  POKE(0x9F21, 0x40);
  while (PEEK(0x9F21) < 0x40 + screen_height) {
    POKE(0x9F23, 28);
    POKE(0x9F23, 0);
  }

  return;
}