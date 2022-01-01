#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>

#define SABS(a,b) ((a > b) ? (a - b) : (b - a))
#define SHRTCIRCUIT_AND(a,b) ((a) ? (b) : 0)
#define SHRTCIRCUIT_OR(a,b) ((a) ? 1 : (b))

#include "structs.h"
#include "map.h"

// Variables
extern struct Map m;
extern struct Cursor c;
extern struct Cursor attackCursor;
extern struct possibleAttacks *pA;
unsigned char player1team;
unsigned char player2team;
unsigned short turncounter;
unsigned char unitsdeadthisturn = 0;
unsigned char remove_old = 0;

// Map methods
void initMap() {
  player1team = 0;
  player2team = 2;
  m.whoseTurn = player1team;
  turncounter = 0;

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
    temp = data[i]+m.boardWidth*data[i+1];
    m.board[data[i]+m.boardWidth*data[i+1]].occupying = malloc(sizeof (struct Unit));
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
unsigned char captureablePaletteOffsets[] = {0,1,2,3,8};
unsigned char captureableSpriteOffsets[] = {18};

void renderMap() {
  unsigned char x,y;
  unsigned short i, temp;
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
  
  if (units_exist[player2team] == 0) {
	//player 1 wins
	win(player1team);
  } else if (units_exist[player1team] == 0) {
	//player 2 wins 
	win(player2team);
  }
  
  x = 0;
  y = 0;
  POKE(0x9F20,0x00);
  POKE(0x9F21,0x00);
  POKE(0x9F22,0x10);
  remove_old = 0;
  for (i = m.top_view * m.boardWidth + m.left_view; y < 10; ++i) {
    POKE(0x9F23,m.board[i].t->tileIndex);
    POKE(0x9F23,m.board[i].t->paletteOffset);
    if (m.board[i].occupying != NULL) {
      renderUnit(m.board[i].occupying);
      POKE(0x9F20,(x+1)*2);
      POKE(0x9F21,y);
      POKE(0x9F22,0x10);
    }
    ++x;
    if (x >= 15) {
	  i += m.boardWidth - 15;	
		
      ++y;
      __asm__ ("inc $9F21");
	  x = 0;
	  POKE(0x9F20,0);
    }
  }
  
  POKE(0x9F20,38 /* 4x8+6 */);
  POKE(0x9F21,0xFC);
  POKE(0x9F22,0x01);
  
  while (PEEK(0x9F23) & 0x0C /* != 0 */) {
	POKE(0x9F23,0);
	POKEW(0x9F20,PEEK(0x9F20)+8);
  }
  POKE(0x9F23,0);
  
  POKE(0x9F20,32);
  POKE(0x9F21,0xFC);
  POKE(0x9F22,0x11);
  for (i = 0; i < m.boardArea; ++i) {
    if (m.board[i].occupying != NULL && m.board[i].occupying->x >= m.left_view && m.board[i].occupying->x < m.left_view + 15 && m.board[i].occupying->y >= m.top_view && m.board[i].occupying->y < m.top_view + 10) {
	  if (m.board[i].occupying->carrying != NULL) {
		POKE(0x9F23,16);
		POKE(0x9F23,8);
		temp = (m.board[i].occupying->x - m.left_view) << 4;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp>>8);
		temp = ((m.board[i].occupying->y - m.top_view) << 4) + 8;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp >> 8);
		POKE(0x9F23,0x0C);
		POKE(0x9F23,(m.board[i].occupying->takenAction ? 9 : 0) + m.board[i].occupying->team);
	  }	else if (m.board[i].base != NULL && m.board[i].base->health < 20) {
		POKE(0x9F23,17);
		POKE(0x9F23,8);
		temp = (m.board[i].occupying->x - m.left_view) << 4;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp>>8);
		temp = ((m.board[i].occupying->y - m.top_view) << 4) + 8;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp >> 8);
		POKE(0x9F23,0x0C);
		POKE(0x9F23,(m.board[i].occupying->takenAction ? 9 : 0) + m.board[i].occupying->team);
	  }		  
	  if (m.board[i].occupying->health <= 90) {
		POKE(0x9F23,6 + ((m.board[i].occupying->health + 9) / 10));
		POKE(0x9F23,8);
		temp = ((m.board[i].occupying->x - m.left_view) << 4) + 8;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp>>8);
		temp = ((m.board[i].occupying->y - m.top_view) << 4) + 8;
		POKE(0x9F23,temp);
		POKE(0x9F23,temp >> 8);
		POKE(0x9F23,0x0C);
		POKE(0x9F23,0x08);
	  }
    }
  }
  // Clear sprites for dead units 
  POKEW(0x9F20,PEEKW(0x9F20)+6);
  POKE(0x9F23,0);
  
  POKE(0x9F20,0xFF);
  POKE(0x9F21,0xFC);
  POKE(0x9F22,0x19);
  x = 0;
  y = 0;
  for (i = m.top_view * m.boardWidth + m.left_view; y < 10; ++i) {
    if (m.board[i].base != NULL) {
	  POKE(0x9F23,0x50 + captureablePaletteOffsets[m.board[i].base->team]); 
	  POKE(0x9F23,0x08); // Z-depth (b/w layers 0 & 1)
	  POKE(0x9F23,y >> 4);
	  POKE(0x9F23,y << 4);
	  POKE(0x9F23,x >> 4);
	  POKE(0x9F23,x << 4);
	  POKE(0x9F23,8);
	  POKE(0x9F23,captureableSpriteOffsets[m.board[i].base->type]);
	}
	
	++x;
    if (x >= 15) {
	  i += m.boardWidth - 15;	
      ++y;
	  x = 0;
    }
  }
  
  
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
  while(1) {
	waitvsync();
  }
}

void nextTurn() {
  unsigned short i = 0;

  m.whoseTurn = (m.whoseTurn == player1team) ? player2team : player1team;
  ++turncounter;
  for (; i < m.boardArea; ++i) {
    if ((m.board[i].occupying) != 0) {newTurnUnit(m.board[i].occupying,i);}
  }
}


//Tile methods
void initTile(struct Tile *t, unsigned char index) {
  t->t = malloc(sizeof(struct Terrain));
  initTerrain(t->t,index);
  t->base = NULL;
  t->occupying = NULL;
  
  if (index >> 4 == 0x04) { // 0x40 <= index <= 0x4F
	t->base = malloc(sizeof(struct Captureable));
	initCaptureable(t->base,(index & 0x0F)%5,(index & 0x0F)/5);
	t->t->tileIndex = 0x85;
	t->t->paletteOffset = 0x60;
  }
}

//Terrain Method
// mvmtCosts[0] = infantry
// mvmtCosts[1] = treads
// mvmtCosts[2] = tires
// mvmtCosts[3] = boat
// mvmtCosts[4] = mech
void initTerrain(struct Terrain *t, unsigned char index) {
  t->tileIndex = index + 0x80;
  switch (index) {
    case 0: // Reef
      t->paletteOffset = 0x40;
      t->defense = 0;
      t->mvmtCosts[0] = 0;
      t->mvmtCosts[1] = 0;
      t->mvmtCosts[2] = 0;
      t->mvmtCosts[3] = 2;
      t->mvmtCosts[4] = 0;
      break;
    case 1: // Water
      t->paletteOffset = 0x40;
      t->defense = 0;
      t->mvmtCosts[0] = 0;
      t->mvmtCosts[1] = 0;
      t->mvmtCosts[2] = 0;
      t->mvmtCosts[3] = 1;
      t->mvmtCosts[4] = 0;
      break;
    case 2: // Vertical Road
    case 3: // Horizontal Road
      t->paletteOffset = 0x50;
      t->defense = 0;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 1;
      t->mvmtCosts[3] = 0;
      t->mvmtCosts[4] = 1;
      break;
	case 0x40: /* captureable indices */
	case 0x41:	
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4A:
	case 0x4B:
	case 0x4C:
	case 0x4D:
	case 0x4E:
    case 4: // Gray City
      t->paletteOffset = 0x50;
      t->defense = 3;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 1;
      t->mvmtCosts[3] = 0;
      t->mvmtCosts[4] = 1;
      break;
    case 5: //Plains
      t->paletteOffset = 0x60;
      t->defense = 1;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 2;
      t->mvmtCosts[3] = 0;
      t->mvmtCosts[4] = 1;
    case 6: // Forest
      t->paletteOffset = 0x60;
      t->defense = 2;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 2;
      t->mvmtCosts[2] = 3;
      t->mvmtCosts[3] = 0;
      t->mvmtCosts[4] = 1;
      break;
    case 7: // Mountain
      t->paletteOffset = 0x70;
      t->defense = 4;
      t->mvmtCosts[0] = 2;
      t->mvmtCosts[1] = 0;
      t->mvmtCosts[2] = 0;
      t->mvmtCosts[3] = 0;
      t->mvmtCosts[4] = 1;
      break;
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

void renderCursor(unsigned char incFrame) {
  unsigned short tempx, tempy;
  unsigned char coff = cursoroffsets[c.frame >> 3];
  if (pA == NULL) {

    POKE(0x9F20,0x00);
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

    POKE(0x9F20,0x00);
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
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team) {
  u->x = init_x;
  u->y = init_y;
  m.board[init_x+m.boardWidth*init_y].occupying = u;
  u->index = index;
  u->team = team;
  u->health = 100;
  u->mvmtType = (index >= 24) ? 3 : ((index == 2 || index == 3) ? (index == 3 ? 0 : 4) : (index == 1 || index == 12 || index == 13) ? 2 : 1);
  u->takenAction = 0;
  u->mvmtRange = mvmtRanges[index];
  u->attackRangeMin = (index >= 12 && index <= 14) ? (index == 14 ? 2 : 3) : 0;
  u->attackRangeMax = (index >= 12 && index <= 14) ? (index == 14 ? 3 : 5) : 1;
  u->airborne = index >= 16 && index <= 19;
  
  u->carrying = NULL;
}

void newTurnUnit(struct Unit *u, unsigned short i) {
  u->takenAction = 0;
  if (m.whoseTurn == u->team && m.board[i].base != NULL && m.board[i].base->team == u->team) {
    u->health += 20;
    if (u->health > 100) {u->health = 100;}
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

unsigned char checkSpaceInMvmtRange(unsigned char tx, unsigned char ty, unsigned char steps) {
  if (tx >= m.boardWidth || ty >= m.boardHeight) {return false;}
  if (tx == checkU->x && ty == checkU->y) {return true;}
  tempT = m.board[ty*m.boardWidth+tx];
  if (checkU->airborne) {
	++steps;
  } else {
    if (!tempT.t->mvmtCosts[checkU->mvmtType]) {return false;}
    steps += tempT.t->mvmtCosts[checkU->mvmtType];
	if (tempT.t->mvmtCosts[checkU->mvmtType] >= 2 && steps > maxSteps) {
		--steps;
	}
  }
  if (steps > maxSteps) {return false;}
  
  /* recursive calls */
  if (SHRTCIRCUIT_AND(tx != 0,checkSpaceInMvmtRange(tx-1,ty,steps))) {return true;}
  if (SHRTCIRCUIT_AND(ty != 0,checkSpaceInMvmtRange(tx,ty-1,steps))) {return true;}
  return checkSpaceInMvmtRange(tx+1,ty,steps) || checkSpaceInMvmtRange(tx,ty+1,steps);
}

unsigned char unitLastX = 255;
unsigned char unitLastY = 255;

unsigned char move(struct Unit *u, unsigned char x, unsigned char y) {
  if ((u->x != x || u->y != y) && !u->takenAction && x < m.boardWidth && y < m.boardHeight) {
	if (m.board[y*m.boardWidth+x].occupying != NULL) {
		if (m.board[y*m.boardWidth+x].occupying->team != u->team || (m.board[y*m.boardWidth+x].occupying->index != 16 && m.board[y*m.boardWidth+x].occupying->index != 0) || (u->index < 2 || u->index > 3) || m.board[y*m.boardWidth+x].occupying->carrying != NULL) {
			return 0;
		}
	}
	maxSteps = u->mvmtRange;
    checkU = u;
    if (checkSpaceInMvmtRange(x,y,0)) {
      checkU = NULL;
      maxSteps = 0;
      unitLastX = u->x;
      unitLastY = u->y;
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
  unitLastX = 255;
  unitLastY = 255;
}

extern unsigned char unitIndexes[];
unsigned char damageChart[] = {
0x00, 0x2d, 0x4b, 0x0e, 0x32, 0x69, 0x4b, 0x00, 0x50, 0x46, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x50,
0x00, 0x23, 0x55, 0x0c, 0x3c, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x37, 0x69, 0x00, 0x00, 0x00, 0x00, 0x5a,
0x00, 0x41, 0x37, 0x2d, 0x69, 0x5f, 0x46, 0x00, 0x5a, 0x55, 0x00, 0x4b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x5a,
0x00, 0x46, 0x41, 0x37, 0x69, 0x69, 0x4b, 0x00, 0x5f, 0x5a, 0x00, 0x4b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x5f,
0x00, 0x04, 0x41, 0x05, 0x2d, 0x69, 0x41, 0x00, 0x55, 0x4b, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x55,
0x00, 0x01, 0x0f, 0x01, 0x0a, 0x37, 0x0f, 0x00, 0x37, 0x2d, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x37,
0x00, 0x06, 0x37, 0x05, 0x19, 0x55, 0x37, 0x00, 0x55, 0x46, 0x00, 0x37, 0x69, 0x00, 0x00, 0x00, 0x00, 0x55,
0x00, 0x1c, 0x55, 0x19, 0x37, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x5a,
0x00, 0x37, 0x55, 0x19, 0x2d, 0x69, 0x55, 0x00, 0x55, 0x50, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x55,
0x00, 0x2d, 0x46, 0x0f, 0x32, 0x69, 0x46, 0x00, 0x50, 0x4b, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x50,
0x00, 0x23, 0x23, 0x1e, 0x78, 0x2d, 0x28, 0x78, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x64, 0x00, 0x00, 0x73, 0x00,
0x00, 0x0a, 0x09, 0x07, 0x78, 0x0c, 0x0a, 0x78, 0x00, 0x00, 0x00, 0x41, 0x00, 0x64, 0x00, 0x00, 0x73, 0x00,
0x00, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x41, 0x00,
0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x37, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x0a, 0x00, 0x3c, 0x37, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x5f, 0x00, 0x5f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x55, 0x3c, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x37, 0x5a, 0x5f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x05, 0x00, 0x55, 0x41, 0x00, 0x37, 0x55, 0x00, 0x00, 0x19, 0x00, 0x5f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x37, 0x28, 0x00, 0x19, 0x4b, 0x00, 0x00, 0x37, 0x00, 0x32};

/*unsigned char canAttack(struct Unit *a, struct Unit *b) {
  if (SABS(b->x,a->x) + SABS(b->y,a->y) >= a->attackRangeMin && SABS(b->x,a->x) + SABS(b->y,a->y) <= a->attackRangeMax) {
    return damageChart[unitIndexes[b->index] * 18 + unitIndexes[a->index]];
  }
  return 0;
}*/
#define CANATTACK(a,b) ((a->team != b->team && SABS(b->x,a->x) + SABS(b->y,a->y) >= a->attackRangeMin && SABS(b->x,a->x) + SABS(b->y,a->y) <= a->attackRangeMax) ? (damageChart[unitIndexes[b->index] * 18 + unitIndexes[a->index]]) : 0)

unsigned char calcPower(struct Unit *a, struct Unit *b) {
  return CANATTACK(a,b) * a->health / 100;
}

void attack(struct Unit *attacker, struct Unit *defender) {
  defender->health -= calcPower(attacker,defender);
  if (defender->health >= 128) {defender->health = 0;}
  if (defender->health > 0) {
    attacker->health -= calcPower(defender,attacker);
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
    free(defender);
  }
}

void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy) {
  unsigned char i = 0;
  struct Tile *north = NULL;
  struct Tile *east = NULL;
  struct Tile *south = NULL;
  struct Tile *west = NULL;

  if (cy != 0) { north = &(m.board[(cy-1)*m.boardWidth+cx]); }
  if (cx != 0) { west = &(m.board[cy*m.boardWidth+cx-1]); }
  if (cy < m.boardHeight - 1) { south = &(m.board[(cy+1)*m.boardWidth+cx]); }
  if (cx < m.boardWidth - 1) { east = &(m.board[cy*m.boardWidth+cx+1]); }
  
  if (north->occupying != NULL && north->occupying->team != m.whoseTurn) {pA->attacks[i] = north; i++;}
  if (east->occupying != NULL && east->occupying->team != m.whoseTurn) {pA->attacks[i] = east; i++;}
  if (south->occupying != NULL && south->occupying->team != m.whoseTurn) {pA->attacks[i] = south; i++;}
  if (west->occupying != NULL && west->occupying->team != m.whoseTurn) {pA->attacks[i] = west; i++;}
  pA->length = i;
} // not tested

void getPossibleDrops(struct possibleAttacks *pA, struct Unit *u) {
  struct Unit *carry = u->carrying;
  struct Tile *tile;
  unsigned int i = 0; 
	
  pA->actives = 0;
	
  if (u->x != 0) {
	tile = &(m.board[u->x - 1 + m.boardWidth*u->y]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0) {pA->attacks[i] = tile; i++; pA->actives += 1;}  
  }
  if (u->y != 0) {
	tile = &(m.board[u->x + m.boardWidth*(u->y - 1)]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0) {pA->attacks[i] = tile; i++; pA->actives += 2;}  
  }
  if (u->x < m.boardWidth - 1) {
	tile = &(m.board[u->x + 1 + m.boardWidth*u->y]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0) {pA->attacks[i] = tile; i++; pA->actives += 4;}  
  }
  if (u->y < m.boardHeight - 1) {
	tile = &(m.board[u->x + m.boardWidth*(u->y + 1)]);
	if (tile->t->mvmtCosts[carry->mvmtType] != 0) {pA->attacks[i] = tile; i++; pA->actives += 8;}  
  }
  
  pA->length = i;
}

/* x_or_y = 0 -> x, != 0 -> y*/
unsigned char getXYFromActiveFilters[4] = {0xE, 0xD, 0xB, 0x7};
unsigned char getXYFromActive(struct Unit *u, unsigned char active, unsigned char index, unsigned char x_or_y) {
	if (index == 3) {return x_or_y ? u->y + 1: u->x;}
	if (index == 0) {
	  if (active & 1) {return x_or_y ? u->y : u->x + 1;}
	  if (active & 2) {return x_or_y ? u->y - 1: u->x;}
	  if (active & 4) {return x_or_y ? u->y : u->x - 1;}
	  return x_or_y ? u->y - 1: u->x;
	}
	if (active & 1) {return getXYFromActive(u,getXYFromActiveFilters[0], index - 1, x_or_y);}
	if (active & 2) {return getXYFromActive(u,getXYFromActiveFilters[1], index - 1, x_or_y);}
	if (active & 4) {return getXYFromActive(u,getXYFromActiveFilters[2], index - 1, x_or_y);}
	return getXYFromActive(u,getXYFromActiveFilters[3], index - 1, x_or_y);

}

unsigned char sizeofGetPossibleDrops(struct Unit *u) {
  struct Unit *carry = u->carrying;
  unsigned char size = 0;
	
  if (u == NULL) {return 0;}
	
  if (u->x != 0) {
	if (m.board[u->x - 1 + m.boardWidth*u->y].t->mvmtCosts[carry->mvmtType] != 0) {size += 1;}  
  }
  if (u->y != 0) {
	if (m.board[u->x + m.boardWidth*(u->y - 1)].t->mvmtCosts[carry->mvmtType] != 0) {size += 2;}  
  }
  if (u->x < m.boardWidth - 1) {
	if (m.board[u->x + 1 + m.boardWidth*u->y].t->mvmtCosts[carry->mvmtType] != 0) {size += 4;}  
  }
  if (u->y < m.boardHeight - 1) {
	if (m.board[u->x + m.boardWidth*(u->y + 1)].t->mvmtCosts[carry->mvmtType] != 0) {size += 8;}  
  }	
	
  return size;
}