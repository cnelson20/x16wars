#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>
#include "structs.h"

// functions //
void main();
void setup();
void loadGraphics();
void draw();
void drawUI();
void clearUI();
void keyPressed();
void initMapData(char data[]);
void initCursor();
void renderCursor();
void renderMap();
unsigned char move(struct Unit *u, unsigned char x, unsigned char y);
void undoMove(struct Unit *u);
void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy);
void attack(struct Unit *attacker, struct Unit *defender);
void clearScreen();
void loadPalette();
void nextTurn();

// global variables //
char testMap[] = {19,6,0,0,2,255,0,3,3,1,2,3,255,
		  7,7,1,1,1,1,6,5,6,1,6,5,6,1,1,1,1,7,7,
		  7,5,6,1,1,1,5,5,5,1,5,5,5,1,1,1,6,5,7,
		  7,5,6,1,1,1,6,5,7,1,7,5,6,1,1,1,6,5,7,
		  4,5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,5,5,4,
		  5,4,7,1,1,1,1,1,1,1,1,1,1,1,1,1,7,4,5,
		  5,5,6,5,5,5,5,6,5,5,5,6,5,5,5,5,6,5,5,
};
/*char testMap[] = {5,5,0,1,2,255,0,3,3,1,2,3,255,
  3,3,3,3,3,
  2,4,4,5,2,
  2,5,5,6,2,
  2,5,6,6,2,
  3,3,3,3,3,
  };*/

extern unsigned char redgraphics[];
extern unsigned char greengraphics[];
extern unsigned char bluegraphics[];
extern unsigned char yellowgraphics[];
extern unsigned char spritegraphics[];
extern unsigned char customPalette[];
unsigned char keyCode;
struct Map m;
struct Cursor c;
struct Cursor attackCursor;
unsigned char actionNo;
unsigned char selIndex;
struct possibleAttacks *pA = NULL;
struct Menu menuOptions;

void main() {
  setup();
  while (1) {
    __asm__("jsr $FFE4");

    __asm__("cmp #0");
    __asm__("beq %g", afterKeyPressed);
    __asm__("sta %v", keyCode);
    keyPressed();
  afterKeyPressed:
    draw();
    waitvsync(); 
  }
}

void setup() {
  unsigned short i;
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

  POKE(0x9F20,0x00);
  POKE(0x9F21,0x80);
  POKE(0x9F22,0x10);
  for (i = 0; i < 4096; ++i) {
    POKE(0x9F23,redgraphics[i]);
  }			
	
  POKE(0x9F20,0x00);
  POKE(0x9F21,0x90); // 0x80 + 0x20/2
  for (i = 0; i < 4096; ++i) {
    POKE(0x9F23,redgraphics[i]);
  }		

  POKE(0x9F20,0x00);
  POKE(0x9F21,0xA0);
  for (i = 0; i < 4096; ++i) {
    POKE(0x9F23,redgraphics[i]);
  }
	
  POKE(0x9F20,0x00);
  POKE(0x9F21,0xB0);
  for (i = 0; i < 4096; ++i) {
    POKE(0x9F23,redgraphics[i]);
  }
	
  load_address = malloc(4864); // 128 more than 4,736 (size of letter.c, biggest one)
  cbm_k_setnam("tile.chr");
  cbm_k_setlfs(0xFF,0x08,0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00);
  POKE(0x9F21,0xC0);
  for (i = 0; i < 1200; ++i) {
    POKE(0x9F23,load_address[i]);
  }
  
  cbm_k_setnam("letter.chr");
  cbm_k_setlfs(0xFF,0x08,0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00); // x16 chops off first two bytes (takes it as load address), so two padding bytes in file
  POKE(0x9F21,0xD0);
  for (i = 0; i < 4864; ++i) {
    POKE(0x9F23,load_address[i]);
  }
  
  cbm_k_setnam("sprites.chr");
  cbm_k_setlfs(0xFF,0x08,0x00);
  cbm_k_load(0,(unsigned short)load_address);
  POKE(0x9F20,0x00);
  POKE(0x9F21,0x00);
  POKE(0x9F22,0x11);
  for (i = 0; i < 514; ++i) {
    POKE(0x9F23,load_address[i]);
  }
  
  free(load_address);
  menuOptions.length = 0;

  loadPalette();
  clearScreen();
  initMapData(testMap);
  initCursor();
}

void draw() {
  m.top_view = 0;
  m.left_view = 0;
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

unsigned char unitNames[][11] =
  {{0xa0, 0xaf, 0xa2}, // APC
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

unsigned char optionStrings[][8] =
  {{0xad, 0xb4, 0xab, 0xab, 0x00},
   {0xa4, 0xad, 0xa3, 0x00}};

unsigned char healthText[] = {0xa7, 0xa4, 0xa0, 0xab, 0xb3, 0xa7, 0x1c};

#define SCREENBYTE(a) (a >= 10 ? 150 + a : 186 + a)

void drawUI() {
  struct Unit *unitPointer;
  void *test = NULL;
  unsigned char dummy, i;

  clearUI();
  
  POKE(0x9F21,0x40+13);
  POKE(0x9F20,0);
  POKE(0x9F22,0x20);
  POKE(0x9F23,'m' - 'a' + 160);
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
  
  unitPointer = m.board[c.y*m.boardWidth+c.x].occupying;
  if (unitPointer == NULL) {
    unitPointer = c.selected;
  }
  if (pA != NULL) {
    unitPointer = attackCursor.selected;
  }
  if (menuOptions.length != 0) {
    //display menu options
    clearUI();
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
  } else if (unitPointer != NULL) {
    dummy = unitIndexes[unitPointer->index];
    drawText(unitNames[dummy],unitNameLengths[dummy],1,11,1);
    POKE(0x9F20,2);
    POKE(0x9F21,0x40+12);
    POKE(0x9F22,0x20);
    POKE(0x9F23,183);
    POKE(0x9F23,186 + unitPointer->x);;
    POKE(0x9F23,28);
    POKE(0x9F23,184);
    POKE(0x9F23,186 + unitPointer->y);
    if (unitPointer->health <= 99) {
      POKE(0x9F23,28);
      POKE(0x9F23,167);
      POKE(0x9F23,186 + (unitPointer->health / 10));
      POKE(0x9F23,186 + (unitPointer->health % 10));
    }
  }
}


void clearUI() {
  POKE(0x9F20,0x30);
  POKE(0x9F21,0x40);
  POKE(0x9F22,0x10);
  while (PEEK(0x9F21) < 0x4A) {
    POKE(0x9F23,28);
    POKE(0x9F23,0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20,30);
      __asm__ ("inc $9F21");
    }
  }

  POKE(0x9F20,0x00);
  while (PEEK(0x9F21) < 0x54) {
    POKE(0x9F23,28);
    POKE(0x9F23,0x80);
    if (PEEK(0x9F20) >= 40) {
      POKE(0x9F20,0);
      __asm__ ("inc $9F21");
    }
  }
}

extern unsigned char unitLastX;
extern unsigned char unitLastY;

void keyPressed() {
  if (menuOptions.length != 0) {
    if (keyCode == 0x57) /* W */ {
      if (menuOptions.selected != 0) {menuOptions.selected--;}
    } else if (keyCode == 0x53)	/* S */ {
      if (menuOptions.selected < menuOptions.length - 1) {menuOptions.selected++;}
    } else if (keyCode == 0x55) /* U */ {
      menuOptions.length = 0;
    } else if (keyCode == 0x49) /* I */ {
      switch (menuOptions.options[menuOptions.selected]) {
      case OPTION_END:
		menuOptions.length = 0;
		nextTurn();
		break;
      default:
		break;
      }
    }
  } else {
    if (pA != NULL) {
      //Switch between attack targets.
      if (keyCode == 0x41) /* A */ {
		selIndex = (selIndex == 0) ? pA->length - 1 : selIndex - 1;
		attackCursor.selected = pA->attacks[selIndex];
		attackCursor.x = attackCursor.selected->x;
		attackCursor.y = attackCursor.selected->y;
      } else if (keyCode == 0x44) /* D */ {
		++selIndex;
		if (selIndex >= pA->length) {selIndex = 0;}
		attackCursor.selected = pA->attacks[selIndex];
		attackCursor.x = attackCursor.selected->x;
		attackCursor.y = attackCursor.selected->y;
      } else if (keyCode == 0x55) /* U */ {
		undoMove(c.selected);
		c.selected->takenAction = 0;
		free(pA);
		pA = NULL;
      } else if (keyCode = 0x49) /* I */ {
		switch (actionNo) {
		case 0:
			attack(c.selected,attackCursor.selected); // x > 1 attack breaks before here
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
      if (keyCode == 0x57) /* W */ {
		if (c.y == 0) {
		  if (m.top_view > 0 && m.top_view == c.y) {--m.top_view;}
		  } else {
			--c.y;
		  }
		} /* A */ else if (keyCode == 0x41) {
		  if (c.x == 0) {
			if (m.left_view > 0 && m.left_view == c.x) {--m.left_view;}
		  } else {
			--c.x;
		  }
		} /* S */ else if (keyCode == 0x53) {
		  if (c.y >= 9 && c.y || c.y >= m.boardHeight - 1) {
			if (m.top_view < m.boardHeight - 10) {if(c.y >= m.top_view + 9){++m.top_view;}}
		  } else {
			++c.y;
		  }
		} /* D */ else if (keyCode == 0x44) {
		  if (c.x >= 14 || c.x >= m.boardWidth) {
			if (m.left_view < m.boardWidth - 15) {if (c.x >= m.left_view + 14){++m.left_view;}}
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
		  if (c.selected == 0) {
			c.selected = m.board[c.x+m.boardWidth*c.y].occupying;
			if (c.selected == 0) {
			  menuOptions.length = 1;
			  menuOptions.options[0] = OPTION_END;
			} else {
			  c.storex = c.x;
			  c.storey = c.y;
			}
		  } else { // crash in this section
			if (unitLastX == 255) {
			  if (c.selected->team == m.whoseTurn && !c.selected->takenAction && /* i think move is broken */ move(c.selected,c.x,c.y)) {
				/* crash is before here */
				POKE(0x9FB6,0xFF); // makes emu give invalid addr warning
				// test if code reaches here 
				
				pA = malloc(sizeof(struct possibleAttacks));
				getPossibleAttacks(pA,c.x,c.y);
				if (pA->length == 0) {
				  unitLastX = 255;
				  unitLastY = 255;
				  c.selected->takenAction = 1;
				  c.selected = NULL;
				  free(pA);
				  pA = NULL;
				} else {
				  actionNo = 0; // ATTACK
				  selIndex = 0;
				  attackCursor.selected = pA->attacks[selIndex];
				  attackCursor.x = attackCursor.selected->x;
				  attackCursor.y = attackCursor.selected->y;
				}
			  }
			} else {
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
  while (PEEK(0x9F21) < 0x0F) {
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
