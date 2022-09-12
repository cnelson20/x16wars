#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>

#define SABS(a,b) ((a > b) ? (a - b) : (b - a))
#define SHRTCIRCUIT_AND(a,b) ((a) ? (b) : 0)
#define SHRTCIRCUIT_OR(a,b) ((a) ? 1 : (b))

#include "structs.h"
#include "map.h"
#include "waitforjiffy.h"
#include "main.h"

// Variables
extern struct Map m;
extern struct Cursor c;
extern struct Cursor attackCursor;
extern struct possibleAttacks *pA;

unsigned char returnToMenu;
unsigned char player1team;
unsigned char player2team;
unsigned short turncounter;
unsigned short daycount;
unsigned char unitsdeadthisturn = 0;
unsigned char currentbases = 0;
unsigned char oldbases;
unsigned char currentunitsprites = 0;
unsigned char oldunitsprites;
unsigned char remove_old = 0;

// Map methods
void initMap() {
  m.whoseTurn = player1team;
  turncounter = 0;
	daycount = 0;

  m.top_view = 0;
  m.oldtop_view = 0;
  m.left_view = 0;
  m.oldleft_view = 0;
  m.whoseTurn = player1team;
  m.boardWidth = 3;
  m.boardHeight = 3;
  m.boardArea = m.boardWidth * m.boardHeight;
  m.board = malloc(m.boardWidth * m.boardHeight * sizeof(struct Tile));
}
void initMapData(char data[]) {
  unsigned short i, mapI, temp;
	
  initMap();
  free(m.board);
  m.boardWidth = data[0];
  m.boardHeight = data[1];
  m.boardArea = m.boardWidth * m.boardHeight;
  m.board = malloc(m.boardArea * sizeof(struct Tile));

  for (i = 2; data[i] != 0xFF; i += 3) {}
  i++;
  for (; data[i] != 0xFF; i += 3) {}
  i++;
  for (mapI = 0; mapI < m.boardArea; ++mapI) {
    initTile(&(m.board[mapI]),data[i]);
    ++i;
  }
  for (i = 2; data[i] != 0xFF; i += 3) {
		
    temp = data[i]+m.boardWidth * data[i+1];
    m.board[temp].occupying = malloc(sizeof (struct Unit));
		
		initUnit(m.board[temp].occupying, data[i], data[i+1], data[i+2], player1team);
  }
  i++;
  for (; data[i] != 0xFF; i += 3) {
    temp = data[i]+m.boardWidth*data[i+1];
    m.board[temp].occupying = malloc(sizeof (struct Unit));
    initUnit(m.board[temp].occupying, data[i], data[i+1], data[i+2], player2team);
  }
  i++;
}

extern struct Menu menuOptions;
unsigned char captureablePaletteOffsets[] = {0xd, 0xd, 0xe, 0xe, 0x8};
unsigned char captureableSpriteOffsets[][] = 
   {{18, 26, 18, 26, 26},
	{34, 42, 34, 42, 42}};

void renderMap() {
  unsigned char x,y;
  unsigned short i,temp;

  checkOldUnits();
  
	POKE(0x9F20, 0);
	POKE(0x9F21, 0);
	POKE(0x9F22, 0x10);
	x = 0;
	y = 0;
	i = m.left_view + m.top_view * m.boardWidth; 
	while (y < 10) {
		POKE(0x9F23, m.board[i].t->tileIndex);
		POKE(0x9F23, m.board[i].t->paletteOffset);
		if (m.board[i].occupying != NULL) {
			renderUnit(m.board[i].occupying);
			
			POKE(0x9F20, (x+1) * 2);
			POKE(0x9F21, y);
		}
		
		++x;
		if (x >= 15) {
			i += m.boardWidth - 15;
			
			++y;
			x = 0;
			__asm__ ("inc $9F21");
			POKE(0x9F20, 0);
		}
		++i;
	}
	
	oldunitsprites = currentunitsprites;
	currentunitsprites = 0;
	POKE(0x9F20,40);
  POKE(0x9F21,0xFC);
  POKE(0x9F22,0x11);
  for (i = m.top_view * m.boardWidth + m.left_view; i < m.boardArea; ++i) {
		struct Unit *tu = m.board[i].occupying;
		if (tu == NULL) { continue ; }
		if (tu->x < m.left_view || tu->x >= m.left_view + 15 || tu->y < m.top_view || tu->y >= m.top_view + 10) { continue;	}
		if (tu->carrying != NULL) {
			++currentunitsprites;
			POKE(0x9F23, 16);
			POKE(0x9F23, 8);
			temp = (tu->x - m.left_view) << 4;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			temp = ((tu->y - m.top_view) << 4) + 8;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			POKE(0x9F23, 0xC);
			POKE(0x9F23, (tu->takenAction ? 9 : 0) + tu->team);
		} else if (m.board[i].base != NULL && m.board[i].base->health < 20) {
			++currentunitsprites;
			POKE(0x9F23, 17);
			POKE(0x9F23, 8);
			temp = (tu->x - m.left_view) << 4;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			temp = ((tu->y - m.top_view) << 4) + 8;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			POKE(0x9F23, 0xC);
			POKE(0x9F23, (tu->takenAction ? 9 : 0) + tu->team);
		}
		if (tu->health <= 90) {
			++currentunitsprites;
			POKE(0x9F23, 6 + ((tu->health + 9) / 10));
			POKE(0x9F23, 8);
			temp = ((tu->x - m.left_view) << 4) + 8;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			temp = ((tu->y - m.top_view) << 4) + 8;
			POKE(0x9F23, temp);
			POKE(0x9F23, temp>>8);
			POKE(0x9F23, 0xC);
			POKE(0x9F23, 0x08);
		}
	}
	while (oldunitsprites > currentunitsprites) {
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		--oldunitsprites;
	}
	
	oldbases = currentbases;
	currentbases = 0;
	
	POKE(0x9F20, 0xFF);
	POKE(0x9F21, 0xFF);
	POKE(0x9F22, 0x19);
	x = 0;
	y = 0;
	i = m.left_view + m.top_view * m.boardWidth; 
	while (y < 10) {
		struct Captureable *base = m.board[i].base;
		if (base != NULL) {
			++currentbases;
			if (y == 0) {
				POKE(0x9F23,0x50 + captureablePaletteOffsets[base->team]); /* 32 x 16 sprite */
				POKE(0x9F23,0x08); // Z-depth (b/w layers 0 & 1)
				POKE(0x9F23,y >> 4);
				POKE(0x9F23,y << 4);
				POKE(0x9F23,x >> 4);
				POKE(0x9F23,x << 4);
				POKE(0x9F23,8);
				POKE(0x9F23,captureableSpriteOffsets[base->type][base->team] + 4);	
			} else {
				POKE(0x9F23,0x90 + captureablePaletteOffsets[base->team]); /* 32 x 32 sprite */
				POKE(0x9F23,0x08); // Z-depth (b/w layers 0 & 1)
				POKE(0x9F23,(y-1) >> 4);
				POKE(0x9F23,(y-1) << 4);
				POKE(0x9F23,x >> 4);
				POKE(0x9F23,x << 4);
				POKE(0x9F23,8);
				POKE(0x9F23,captureableSpriteOffsets[base->type][base->team]);
			}				
		}
		
		++x;
		if (x >= 15) {
			i += m.boardWidth - 15;
			++y;
			x = 0;
		}
		++i;
	}
	while (oldbases > currentbases) {
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		__asm__ ("stz $9F23");
		--oldbases;
	}
	
  m.oldtop_view = m.top_view;
  m.oldleft_view = m.left_view;	
	
	renderCursor(1);
	if (menuOptions.length != 0) {
    POKE(0x9F20,0x06);
    POKE(0x9F21,0xFC);
    POKE(0x9F22,0x41);
    POKE(0x9F23,0);
    POKE(0x9F23,0);
    POKE(0x9F23,0);
    POKE(0x9F23,0);
  }
  
  m.oldtop_view = m.top_view;
  m.oldleft_view = m.left_view;
}

void checkOldUnits() {
  unsigned short i;
  unsigned char units_exist[4];
  units_exist[player1team] = 0; 
  units_exist[player2team] = 0;
  
  remove_old = 1;
  for (i = 0; i < m.boardArea; ++i) {
    if (m.board[i].occupying != NULL) {
      renderUnit(m.board[i].occupying);
	  units_exist[m.board[i].occupying->team] = 1;
    }
  }
	remove_old = 0;
  
  if (units_exist[player2team] == 0) {
	//player 1 wins
	win(player1team);
  } else if (units_exist[player1team] == 0) {
	//player 2 wins 
	win(player2team);
  }	
}

unsigned char colorstrings[4][7] = 
  {{0xb1, 0xa4, 0xa3, 0, 0, 0, 0}, /* red */
   {0xa6, 0xb1, 0xa4, 0xa4, 0xad, 0, 0}, /* green */
   {0xa1, 0xab, 0xb4, 0xa4, 0, 0, 0}, /* blue */
   {0xb8, 0xa4, 0xab, 0xab, 0xae, 0xb6, 0}}; /* yellow */
unsigned char colorstringlengths[4] = {3,5,4,6};
void win(unsigned char team) {
  unsigned char i;
	
  POKE(0x9F20,(7 -  (colorstringlengths[team] >> 1)) << 1);
  POKE(0x9F21,0x44);
  POKE(0x9F22,0x10);
  for (i = 0; i < colorstringlengths[team]; ++i) {
  	POKE(0x9F23,colorstrings[team][i]);
  	POKE(0x9F23,0x80);
  }
  POKE(0x9F20,10);
  POKE(0x9F21,0x45);

  POKE(0x9F23,0xb6); // w
  POKE(0x9F23,0x80);
  POKE(0x9F23,0xa8); // i
  POKE(0x9F23,0x80);
  POKE(0x9F23,0xad); // n 
  POKE(0x9F23,0x80);
  POKE(0x9F23,0xb2); // s
  POKE(0x9F23,0x80);
	
	i = 300;
  while(i != 0) {
		waitforjiffy();
		--i;
  }
	returnToMenu = 1;
}

void nextTurn() {
  unsigned short i = 0;

  m.whoseTurn = (m.whoseTurn == player1team) ? player2team : player1team;
  ++turncounter;
	if (m.whoseTurn == player1team) {
		++daycount;
	}
  for (; i < m.boardArea; ++i) {
    if ((m.board[i].occupying) != 0) {newTurnUnit(m.board[i].occupying,i);}
  }
}

#define TILE_REEF 0
#define TILE_WATER 1
#define TILE_ROAD_VERTICAL 2
#define TILE_ROAD_HORIZONTAL 3
#define TILE_CITY 4
#define TILE_PLAINS 5
#define TILE_FOREST 6
#define TILE_MOUNTAIN 7
#define TILE_MENU_FILLED 8
#define TILE_SHOAL 9
#define TILE_RIVER 10

//Tile methods
void initTile(struct Tile *t, unsigned char index) {
  //t->t = malloc(sizeof(struct Terrain));
  initTerrain(&(t->t), index >= 0x40 ? TILE_CITY : index);
  t->base = NULL;
  t->occupying = NULL;
  
  if (index >> 4 == 0x04) { // 0x40 <= index <= 0x4F
	unsigned char team = (index % 4 == 0 ? player1team : (index % 4 == 1 ? player2team : 4));
  
	t->base = malloc(sizeof(struct Captureable));
	initCaptureable(t->base,team,(index & 0x0F) >> 2);
	t->t->tileIndex = 0x85;
	t->t->paletteOffset = 0x60;
  }
}

unsigned char terrainDefenseArray[] = {
	0x00, 0x00, 0x00, 0x00, 
	0x03, 0x01, 0x02, 0x04, 
	0xFF, 0x00, 0x00,
};
unsigned char terrainPaletteOffsetArray[] = {
	0x40, 0x40, 0x50, 0x50, 
	0x50, 0x60, 0x60, 0x70,
	0xFF, 0x40, 0x40,
};

unsigned char terrainMvmtCostsArray[][6] = {
	{0, 0, 0, 2, 0, 2}, // Reef 
	{0, 0, 0, 1, 0, 1}, // Water
	{1, 1, 1, 0, 1, 0}, // Horizontal Road
	{1, 1, 1, 0, 1, 0}, // Vertical Road
	{1, 1, 1, 0, 1, 0}, // Cities
	{1, 1, 2, 0, 1, 0}, // Plains
	{1, 2, 3, 0, 1, 0}, // Forest
	{2, 0, 0, 0, 1, 0}, // Mountain
	{0, 0, 0, 0, 0, 0}, // Filled tile for menu
	{1, 1, 1, 0, 1, 1}, // Shoal
	{2, 0, 0, 0, 1, 0}, // River
};

//Terrain Method
// mvmtCosts[0] = infantry
// mvmtCosts[1] = treads
// mvmtCosts[2] = tires
// mvmtCosts[3] = boat
// mvmtCosts[4] = mech
// mvmtCosts[5] = lander

struct Terrain *terrainArray[16];

void initTerrain(struct Terrain **t_pointer, unsigned char index) {
	unsigned char i;
	
	if (terrainArray[index] != NULL) {
		*t_pointer = terrainArray[index];
	} else {
		struct Terrain *t = malloc(sizeof(struct Terrain));
		terrainArray[index] = t;
		*t_pointer = t;
		
		t->tileIndex = index + 0x80;
		t->paletteOffset = terrainPaletteOffsetArray[index];
		t->defense = terrainDefenseArray[index];
		
		for (i = 0; i < 6; ++i) {
			t->mvmtCosts[i] = terrainMvmtCostsArray[index][i];
		}
	}
}

// Cursor methods
void initCursor() {
  c.x = 0;
  c.y = 0;
  c.storex = 0;
  c.storey = 0;
  c.frame = 0;
}

unsigned char cursoroffsets[] = {0, 1, 2, 1};
extern struct Menu menuOptions;

void renderCursor(unsigned char incFrame) {
  unsigned short tempx, tempy;
  unsigned char coff = cursoroffsets[c.frame >> 3];
  if (pA == NULL || menuOptions.length != 0) {

    POKE(0x9F20,0x08);
    POKE(0x9F21,0xFC);
    POKE(0x9F22,0x11);

    // Part 0
    POKE(0x9F23,0);
    POKE(0x9F23,8);
    tempx = (c.x << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = (c.y << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 1
    POKE(0x9F23,1);
    POKE(0x9F23,8);
    tempx = ((c.x+1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = (c.y << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 2
    POKE(0x9F23,2);
    POKE(0x9F23,8);
    tempx = (c.x << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((c.y+1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 3
    POKE(0x9F23,3);
    POKE(0x9F23,8);
    tempx = ((c.x+1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((c.y+1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x58);
  } else {

    POKE(0x9F20,0x08);
    POKE(0x9F21,0xFC);
    POKE(0x9F22,0x11);

    // Part 0
    POKE(0x9F23,0);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x - m.left_view) << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y - m.top_view) << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 1
    POKE(0x9F23,1);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x - m.left_view + 1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y - m.top_view) << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 2
    POKE(0x9F23,2);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x - m.left_view) << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y - m.top_view + 1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 3
    POKE(0x9F23,3);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x - m.left_view + 1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y - m.top_view + 1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x58);
  }
  if (incFrame) {c.frame = (c.frame+1)&0x1F;}
}

// Captureable Methods 
void initCaptureable(struct Captureable *c, unsigned char init_team, unsigned char init_type) {
  c->team = init_team;
  c->type = init_type;
  c->critical = init_type == 1;
  c->health = 20;
  /* 
	type 0 = base 
	type 1 = hq
	type 2 = factory 
  */
}

void capture(struct Unit *u, struct Captureable *c) {
  unsigned char i;
  
  if (u->team != c->team) {
	i = (u->health + 9) / 10;
	if (i >= c->health) {
	  c->team = u->team;
	  c->health = 20;
	  ++unitsdeadthisturn;
	  if (c->critical) {
		win(m.whoseTurn);  
	  }
	} else {
	  c->health -= i;	
	}
  }
}

//Unit methods
unsigned char mvmtRanges[] = {
  6,8,2,3, /* APC  Recon  Mech  Infantry */
  0,0,0,0,
  0,6,4,5, /* blank  Anti-Air  medium tank  small tank */
  4,4,4,0, /* Missiles  Rockets  Artillery  blank */
  6,6,7,9, /* Transport  Copter  Bomber  Fighter */ 
  0,0,0,0, 
  5,4,6,5}; /* Lander  Submarine  Cruiser  Battleship */
unsigned char mvmtTypes[] = {
  1,2,4,0,
  9,9,9,9,
  9,1,1,1,
  2,2,1,9,
  1,1,1,1, /* airborne units arent affected by this */
  9,9,9,9,
  5,3,3,3
};
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team) {
  u->x = init_x;
  u->y = init_y;
  m.board[init_x+m.boardWidth*init_y].occupying = u;
  u->index = index;
  u->team = team;
	u->health = 100;
	u->ammo = 10;
  u->mvmtType = mvmtTypes[index];
  u->takenAction = 0;
  u->mvmtRange = mvmtRanges[index];
  u->attackRangeMin = (index >= 12 && index <= 14) ? (index == 14 ? 2 : 3) : 0;
  u->attackRangeMax = (index >= 12 && index <= 14) ? (index == 14 ? 3 : 5) : 1;
  u->airborne = index >= 16 && index <= 19;
  
	u->canAttackAndMove = !(index == UNIT_MISSILES || index == UNIT_ROCKETS);
	
  u->carrying = NULL;
}

void newTurnUnit(struct Unit *u, unsigned short i) {
  u->takenAction = 0;
  if (m.whoseTurn == u->team && m.board[i].base != NULL && m.board[i].base->team == u->team) {
    u->health += 20;
		u->ammo = 10;
    if (u->health > 100) {u->health = 100;}
  } 
	if (m.whoseTurn == u->team) {
		struct Unit *up;
		/* If a APC unit is surrounding a unit, refill ammo */
		if (u->x > 0 && m.board[i-1].occupying != NULL) {
			up = m.board[i - 1].occupying;
			if (up->team == u->team && up->index == 0) { u->ammo = 10; goto apc_nearby_exit; }
		}
		if (u->x < m.boardWidth - 1 && m.board[i+1].occupying != NULL) {
			up = m.board[i + 1].occupying;
			if (up->team == u->team && up->index == 0) { u->ammo = 10; goto apc_nearby_exit; }
		}
		if (u->y > 0 && m.board[i - m.boardWidth].occupying != NULL) {
			up = m.board[i - m.boardWidth].occupying;
			if (up->team == u->team && up->index == 0) { u->ammo = 10; goto apc_nearby_exit; }
		}
		if (u->y < m.boardHeight - 1 && m.board[i + m.boardWidth].occupying != NULL) {
			up = m.board[i + m.boardWidth].occupying;
			if (up->team == u->team && up->index == 0) { u->ammo = 10; }
		}
		apc_nearby_exit:
		; /* semicolon so compiler says a-ok */
		
	}
}

void renderUnit(struct Unit *u) {
  POKE(0x9F22,0x10);
  if (remove_old) {
	if (m.oldleft_view != m.left_view || m.oldtop_view != m.top_view) {
	POKE(0x9F20,(u->x - m.oldleft_view) << 1);
	POKE(0x9F21,u->y - m.oldtop_view + 0x40);
	POKE(0x9F23,28);
	}
  } else {
	POKE(0x9F20,(u->x - m.left_view) << 1);
	POKE(0x9F21,u->y - m.top_view +0x40);
	POKE(0x9F23,(u->team << 5)+u->index);
	POKE(0x9F23,(u->takenAction ? 0x90 : 0x00) + (u->team << 4)+ ((u->team != player1team) << 2));
  }
}
unsigned char maxSteps;
struct Unit *checkU;
struct Tile tempT;

unsigned char canCarryUnit(unsigned char carrier_index, unsigned char carried_index) {
	if ((carrier_index == UNIT_APC || carrier_index == UNIT_TRANSPORT) && carried_index >= UNIT_MECH && carried_index <= UNIT_INFANTRY) { return 1; }
	else if (carrier_index == UNIT_LANDER && carried_index <= UNIT_ARTILLERY) { return 1; }
	else if (carrier_index == UNIT_CRUISER && (carried_index == UNIT_COPTER || carried_index == UNIT_TRANSPORT)) { return 1; } 
	return 0;
}

unsigned char checkSpaceInMvmtRange(unsigned char tx, unsigned char ty, unsigned char steps) {
  if (tx >= m.boardWidth || ty >= m.boardHeight) {return 0;}
  if (tx == checkU->x && ty == checkU->y) {return 1;}
  tempT = m.board[ty*m.boardWidth+tx];
  if (checkU->airborne) {
		++steps;
  } else {
    if (!tempT.t->mvmtCosts[checkU->mvmtType]) {return 0;}
    steps += tempT.t->mvmtCosts[checkU->mvmtType];
		/*if (tempT.t->mvmtCosts[checkU->mvmtType] >= 2 && steps > maxSteps) {
			--steps;
		}*/
  }
  if (steps > maxSteps) {return 0;}
  
  /* recursive calls */
  if (SHRTCIRCUIT_AND(tx != 0,checkSpaceInMvmtRange(tx-1,ty,steps))) {return 1;}
  if (SHRTCIRCUIT_AND(ty != 0,checkSpaceInMvmtRange(tx,ty-1,steps))) {return 1;}
  return checkSpaceInMvmtRange(tx+1,ty,steps) || checkSpaceInMvmtRange(tx,ty+1,steps);
}

unsigned char unitLastX = 255;
unsigned char unitLastY = 255;
unsigned char baseLastHP = 255;

unsigned char move(struct Unit *u, unsigned char x, unsigned char y) {
  if (!u->takenAction && x < m.boardWidth && y < m.boardHeight) {
		if (m.board[y*m.boardWidth+x].occupying != NULL && m.board[y*m.boardWidth+x].occupying != u) {
			if (m.board[y*m.boardWidth+x].occupying->team != u->team) {
				return 0;
			/* Check if unit can be loaded on another, if yes, continue to main routine */
			} else if (!canCarryUnit(m.board[y*m.boardWidth+x].occupying->index, u->index) || m.board[y*m.boardWidth+x].occupying->carrying != NULL) {
				/* Check if units can be joined together. if yes continue to main part */
				if (m.board[y*m.boardWidth+x].occupying->index != u->index || m.board[y*m.boardWidth+x].occupying->health == 100 || u->health == 100) {
					return 0;
				}
			}
		}
		maxSteps = u->mvmtRange;
		checkU = u;
		if ((u->airborne) ? (SABS(u->x, x) + SABS(u->y, y) <= u->mvmtRange) : checkSpaceInMvmtRange(x,y,0)) {
			checkU = NULL;
			maxSteps = 0;
			unitLastX = u->x;
			unitLastY = u->y;
			if (m.board[unitLastY*m.boardWidth+unitLastX].base != NULL && (unitLastX != x || unitLastY != y)) {
				baseLastHP = m.board[unitLastY*m.boardWidth+unitLastX].base->health;  
				m.board[unitLastY*m.boardWidth+unitLastX].base->health = 20;
			}
			if (u->x >= m.left_view && u->x < m.left_view + 15 && u->y >= m.top_view && u->y < m.top_view + 10) {
				POKE(0x9F20,(unitLastX - m.left_view) << 1);
				POKE(0x9F21,0x40+unitLastY-m.top_view);
				POKE(0x9F22,0x00);
				POKE(0x9F23,28);
			}
			m.board[unitLastY*m.boardWidth+unitLastX].occupying = NULL; 
			u->x = x;
			u->y = y;
			attackCursor.selected = m.board[y*m.boardWidth+x].occupying;
			m.board[y*m.boardWidth+x].occupying = u;
			return 1;
		}
  }
  return 0;
}
void undoMove(struct Unit *u) {
  m.board[u->y*m.boardWidth+u->x].occupying = NULL;
  POKE(0x9F20,(u->x-m.left_view)*2);
  POKE(0x9F21,0x40+u->y-m.top_view);
  POKE(0x9F22,0x00);
  POKE(0x9F23,28);
  if (attackCursor.selected != NULL && attackCursor.selected->team == u->team) {
	m.board[u->y*m.boardWidth+u->x].occupying = attackCursor.selected;
	attackCursor.selected = NULL;
  }
  u->x = unitLastX;
  u->y = unitLastY;
  u->takenAction = 0;
  m.board[unitLastY*m.boardWidth+unitLastX].occupying = u;
  if (m.board[unitLastY*m.boardWidth+unitLastX].base != NULL) {
	m.board[unitLastY*m.boardWidth+unitLastX].base->health = baseLastHP;  
	baseLastHP = 255;
  }
  unitLastX = 255;
  unitLastY = 255;
}

extern unsigned char unitIndexes[];
unsigned char damageChart[] = {
				/* 0 ,   1,   2,     3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17 */
/* 0 */ 0x00, 0x2d, 0x4b, 0x0e, 0x32, 0x69, 0x4b, 0x00, 0x50, 0x46, 0x00, 0x41, 0x00, 0x69, 0x00, 0x00, 0x00, 0x50,
/* 1 */ 0x00, 0x23, 0x55, 0x0c, 0x3c, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x37, 0x00, 0x69, 0x00, 0x00, 0x00, 0x5a,
/* 2 */ 0x00, 0x41, 0x37, 0x2d, 0x69, 0x5f, 0x46, 0x00, 0x5a, 0x55, 0x00, 0x4b, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x5a,
/* 3 */ 0x00, 0x46, 0x41, 0x37, 0x69, 0x69, 0x4b, 0x00, 0x5f, 0x5a, 0x00, 0x4b, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x5f,
/* 4 */ 0x00, 0x04, 0x41, 0x05, 0x2d, 0x69, 0x41, 0x00, 0x55, 0x4b, 0x00, 0x19, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x55,
/* 5 */ 0x00, 0x01, 0x0f, 0x01, 0x0a, 0x37, 0x0f, 0x00, 0x37, 0x2d, 0x00, 0x19, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x37,
/* 6 */ 0x00, 0x06, 0x37, 0x05, 0x19, 0x55, 0x37, 0x00, 0x55, 0x46, 0x00, 0x37, 0x00, 0x69, 0x00, 0x00, 0x00, 0x55,
/* 7 */ 0x00, 0x1c, 0x55, 0x19, 0x37, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x41, 0x00, 0x69, 0x00, 0x00, 0x00, 0x5a,
/* 8 */ 0x00, 0x37, 0x55, 0x19, 0x2d, 0x69, 0x55, 0x00, 0x55, 0x50, 0x00, 0x41, 0x00, 0x69, 0x00, 0x00, 0x00, 0x55,
/* 9 */ 0x00, 0x2d, 0x46, 0x0f, 0x32, 0x69, 0x46, 0x00, 0x50, 0x4b, 0x00, 0x41, 0x00, 0x69, 0x00, 0x00, 0x00, 0x50,
/*10 */ 0x00, 0x23, 0x23, 0x1e, 0x78, 0x2d, 0x28, 0x78, 0x00, 0x00, 0x00, 0x5f, 0x64, 0x00, 0x00, 0x00, 0x73, 0x00,
/*11 */ 0x00, 0x0a, 0x09, 0x07, 0x78, 0x0c, 0x0a, 0x78, 0x00, 0x00, 0x00, 0x41, 0x64, 0x00, 0x00, 0x00, 0x73, 0x00,
/*12 */ 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x41, 0x00,
/*13 */ 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x37, 0x00,
/*14 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x0a, 0x00, 0x3c, 0x37, 0x00, 0x19, 0x00, 0x5f, 0x00, 0x5f, 0x00, 0x5f,
/*15 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x55, 0x3c, 0x00, 0x19, 0x00, 0x5f, 0x00, 0x37, 0x5a, 0x5f,
/*16 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x05, 0x00, 0x55, 0x41, 0x00, 0x37, 0x00, 0x55, 0x00, 0x19, 0x00, 0x5f,
/*17 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x37, 0x28, 0x00, 0x19, 0x00, 0x4b, 0x00, 0x37, 0x00, 0x32};

unsigned char canAttack(struct Unit *a, struct Unit *b) {
  if (a->team != b->team && a->ammo > 0 && SABS(b->x,a->x) + SABS(b->y,a->y) >= a->attackRangeMin && SABS(b->x,a->x) + SABS(b->y,a->y) <= a->attackRangeMax) {
    return damageChart[unitIndexes[b->index] * 18 + unitIndexes[a->index]];
  } else {
		return 0;
	}
}

unsigned char calcPower(struct Unit *a, struct Unit *b) {
  return canAttack(a,b) * a->health / 100;
}

void attack(struct Unit *attacker, struct Unit *defender) {
  defender->health -= calcPower(attacker,defender);
	if (attacker->ammo > 0) { --attacker->ammo; }
  if (defender->health >= 128) {defender->health = 0;}
  if (defender->health > 0) {
    attacker->health -= calcPower(defender,attacker);
		if (defender->ammo > 0) { --defender->ammo; }
    if (attacker->health >= 128) {attacker->health = 0;}
    if (attacker->health == 0) {
      m.board[m.boardWidth*attacker->y+attacker->x].occupying = NULL;
      POKE(0x9F20,(attacker->x - m.left_view) * 2);
      POKE(0x9F21, attacker->y - m.top_view + 0x40);
      __asm__ ("lda #0");
      __asm__ ("sta $9F22");
      __asm__ ("lda #28");
      __asm__ ("sta $9F23");
	  if (attacker->carrying != NULL) {free(attacker->carrying);}
	  else if (m.board[attacker->y*m.boardWidth + attacker->x].base != NULL) {
	    m.board[attacker->y*m.boardWidth + attacker->x].base->health = 20;
	  }
      free(attacker);
      ++unitsdeadthisturn;
    }
  } else {
    m.board[m.boardWidth*defender->y+defender->x].occupying = NULL;
    POKE(0x9F20,(defender->x - m.left_view) * 2);
    POKE(0x9F21,defender->y - m.top_view + 0x40);
    __asm__ ("lda #0");
    __asm__ ("sta $9F22");
    __asm__ ("lda #28");
    __asm__ ("sta $9F23");
    ++unitsdeadthisturn;
	if (defender->carrying != NULL) {free(defender->carrying);}
	else if (m.board[defender->y*m.boardWidth + defender->x].base != NULL) {
	  m.board[defender->y*m.boardWidth + defender->x].base->health = 20;
	}
    free(defender);
  }
}

void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy, unsigned char attackRangeMax) {
	if (!c.selected->canAttackAndMove && (c.x != unitLastX || c.y != unitLastY)) { pA->length = 0; return; }
	
	if (attackRangeMax == 1) {
		/* Handle direct attack units */
		unsigned char i = 0;
		struct Tile *north = NULL;
		struct Tile *east = NULL;
		struct Tile *south = NULL;
		struct Tile *west = NULL;
		if (cy != 0) { north = &(m.board[(cy-1) * m.boardWidth+cx]); }
		if (cx != 0) { west = &(m.board[cy * m.boardWidth+cx-1]); }
		if (cy < m.boardHeight - 1) { south = &(m.board[(cy+1) * m.boardWidth+cx]); }
		if (cx < m.boardWidth - 1) { east = &(m.board[cy * m.boardWidth+cx+1]); }
		
		if (north->occupying != NULL && north->occupying->team != m.whoseTurn && canAttack(c.selected, north->occupying)) {
			pA->attacks[i] = north; i++;
		}
		if (east->occupying != NULL && east->occupying->team != m.whoseTurn && canAttack(c.selected, east->occupying)) {
			pA->attacks[i] = east; i++;
		}
		if (south->occupying != NULL && south->occupying->team != m.whoseTurn && canAttack(c.selected, south->occupying)) {
			pA->attacks[i] = south; i++;
		}
		if (west->occupying != NULL && west->occupying->team != m.whoseTurn && canAttack(c.selected, west->occupying)) {
			pA->attacks[i] = west; i++;
		}
		pA->length = i;
	} else {
		/* Handle units with ranged attacks */
		unsigned char xmin, ymin, xmax, ymax;
		unsigned char x;
		struct Tile *temp;
		
		change_directory("range");
		
		xmin = (c.x >= attackRangeMax) ? c.x - attackRangeMax : 0;
		ymin = (c.y >= attackRangeMax) ? c.y - attackRangeMax : 0;
		xmax = (c.x + attackRangeMax < 15) ? c.x + attackRangeMax : 14;
		ymax = (c.y + attackRangeMax < 10) ? c.y + attackRangeMax : 9;
		
		pA->length = 0;
		for (; ymin <= ymax; ++ymin) {
			for (x = xmin; x <= xmax; ++x) {
				//if (SABS(x, c.x) + SABS(ymin, c.y) <= attackRangeMin) { continue; }
				temp = &(m.board[m.boardWidth * ymin + x]);
				if (temp->occupying != NULL && temp->occupying->team != m.whoseTurn != NULL && canAttack(c.selected, temp->occupying)) {
					if (pA->length < 8) { // Cap number of possible attacks to save RAM (shouldn't really come up, limited to 8)
						pA->attacks[pA->length] = temp;
						++pA->length;
					}
				}
			}
		}
	}
}

void getPossibleDrops(struct possibleAttacks *pA, struct Unit *u) {
  struct Unit *carry = u->carrying;
  struct Tile *tile;
	
  pA->attacks[0] = NULL; 
  pA->attacks[1] = NULL;	
  pA->attacks[2] = NULL; 
  pA->attacks[3] = NULL;	
	pA->length = 0;
  if (u->x != 0) {
	tile = &(m.board[u->x - 1 + m.boardWidth*u->y]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {pA->attacks[0] = tile; ++pA->length;}  
  }
  if (u->y != 0) {
	tile = &(m.board[u->x + m.boardWidth*(u->y - 1)]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {pA->attacks[1] = tile; ++pA->length;}  
  }
  if (u->x < m.boardWidth - 1) {
	tile = &(m.board[u->x + 1 + m.boardWidth*u->y]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {pA->attacks[2] = tile; ++pA->length;}  
  }
  if (u->y < m.boardHeight - 1) {
	tile = &(m.board[u->x + m.boardWidth*(u->y + 1)]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {pA->attacks[3] = tile; ++pA->length;}  
  }
}

unsigned char sizeofGetPossibleDrops(struct Unit *u) {
	unsigned char size;
	struct possibleAttacks *pA = malloc(sizeof(struct possibleAttacks));
	
	getPossibleDrops(pA,u);
	size = pA->length;
	free(pA);
	return size;
}

void getPossibleJoins(struct possibleAttacks *pA, struct Unit *u) {
	unsigned char i = 0;
	struct Tile *tile;
	
	if (u->x != 0) {
		tile = &(m.board[u->x - 1 + m.boardWidth*u->y]);
		if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
			pA->attacks[i] = tile; i++;
		}
	}
	if (u->y != 0) {
		tile = &(m.board[u->x + m.boardWidth*(u->y - 1)]);
		if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
			pA->attacks[i] = tile; i++;
		}
	}
	if (u->x < m.boardWidth - 1) {
		tile = &(m.board[u->x + 1 + m.boardWidth*u->y]);
		if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
			pA->attacks[i] = tile; i++;
		}
	}
	if (u->y < m.boardHeight - 1) {
		tile = &(m.board[u->x + m.boardWidth*(u->y + 1)]);
		if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
			pA->attacks[i] = tile; i++;
		}
	}
	
	pA->length = i;
	return;
}

unsigned char sizeofGetPossibleJoins(struct Unit *u) {
	unsigned char size;
	struct possibleAttacks *pA = malloc(sizeof(struct possibleAttacks));
	
	getPossibleJoins(pA,u);
	size = pA->length;
	free(pA);
	return size;
}

