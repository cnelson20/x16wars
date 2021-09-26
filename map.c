#include <stdlib.h>
#include <peekpoke.h>
#include "structs.h"
#define SABS(a,b) ((a > b) ? (a - b) : (b - a))

// Method list //
void initMap();
void initMapData(char data[]);
void renderMap();
void initCursor();
void renderCursor(unsigned char incFrame);
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team);
void newTurnUnit(struct Unit *u, unsigned short i);
void renderUnit(struct Unit *u);
void initTile(struct Tile *t, unsigned char index);
void initTerrain(struct Terrain *t, unsigned char index);
void nextTurn();


// Variables
extern struct Map m;
extern struct Cursor c;
extern struct Cursor attackCursor;
extern struct possibleAttacks *pA;
unsigned char player1team;
unsigned char player2team;
unsigned short turncounter;


// Map methods
void initMap() {
  player1team = 0;
  player2team = 2;
  m.whoseTurn = player1team;
  turncounter = 0;

  //m = malloc(sizeof(struct Map));
  m.top_view = 0;
  m.left_view = 0;
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
void renderMap() {
  unsigned char j = m.left_view + m.top_view * m.boardWidth;
  unsigned char maxX = m.boardWidth + m.left_view;
  unsigned char maxY = m.boardHeight + m.top_view;
  unsigned short i;
  unsigned char x,y;
  unsigned short temp;

  POKE(0x9F30,m.left_view << 4);
  POKE(0x9F37,PEEK(0x9F30));
  POKE(0x9F31,m.left_view >> 4);
  POKE(0x9F38,PEEK(0x9F31));
  POKE(0x9F30,m.top_view << 4);
  POKE(0x9F37,PEEK(0x9F32));
  POKE(0x9F33,m.top_view >> 4);
  POKE(0x9F3A,PEEK(0x9F33));
  x = m.left_view;
  y = m.top_view;
  __asm__ ("lda #0");
  __asm__ ("sta $9F20");
  __asm__ ("sta $9F21");
  __asm__ ("lda #$10");
  __asm__ ("sta $9F22");
  for (i = j--; y < maxY; ++i) {
    POKE(0x9F23,m.board[i].t->tileIndex);
    POKE(0x9F23,m.board[i].t->paletteOffset);
    if (m.board[i].occupying != NULL) {
      renderUnit(m.board[i].occupying);
      POKE(0x9F20,(x+1)*2);
      POKE(0x9F21,y);
      POKE(0x9F22,0x10);
    }
    ++x;
    if (x >= maxX || x >= 15) {
      y++;
      x = m.left_view;
      j += m.boardWidth;
      i = j;
      __asm__ ("inc $9F21");
      POKE(0x9F20,0);
    }
  }
  POKE(0x9F20,0x04*8);
  POKE(0x9F21,0xFC);
  POKE(0x9F22,0x11);
  for (i = 0; i < m.boardArea; ++i) {
    if (m.board[i].occupying != NULL && m.board[i].occupying->health <= 90) {
      POKE(0x9F23,6 + ((m.board[i].occupying->health + 9) / 10));
      POKE(0x9F23,8);
      temp = ((m.board[i].occupying->x) << 4) + 8;
      POKE(0x9F23,temp);
      POKE(0x9F23,temp>>8);
      temp = ((m.board[i].occupying->y) << 4) + 8;
      POKE(0x9F23,temp);
      POKE(0x9F23,temp >> 8);
      POKE(0x9F23,0x0C);
      POKE(0x9F23,0x08);
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
  t->index = index;
  t->base = 0;
  t->occupying = 0;
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
  //c = malloc(sizeof (struct Cursor));
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
    tempx = (attackCursor.x << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = (attackCursor.y << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 1
    POKE(0x9F23,1);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x+1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = (attackCursor.y << 4) - 4 + coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 2
    POKE(0x9F23,2);
    POKE(0x9F23,8);
    tempx = (attackCursor.x << 4) - 3 + coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y+1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x08);
    // Part 3
    POKE(0x9F23,3);
    POKE(0x9F23,8);
    tempx = ((attackCursor.x+1) << 4) - 3 - coff;
    POKE(0x9F23,tempx);
    POKE(0x9F23,tempx >> 8);
    tempy = ((attackCursor.y+1) << 4) - 4 - coff;
    POKE(0x9F23,tempy);
    POKE(0x9F23,tempy >> 8);
    POKE(0x9F23,12);
    POKE(0x9F23,0x58);
  }
  if (incFrame) {c.frame = (c.frame+1)%32;}
}

//Unit methods
unsigned char mvmtRanges[] = {6,8,2,3,0,0,0,0,0,6,4,5,4,4,4,0,6,6,7,9,0,0,0,0,5,4,6,5};
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
}

void newTurnUnit(struct Unit *u, unsigned short i) {
  u->takenAction = 0;
  if (m.whoseTurn == u->team && m.board[i].base != 0 && m.board[i].base->team == u->team) {
    u->health += 20;
    if (u->health > 100) {u->health = 100;}
  }
}

void renderUnit(struct Unit *u) {
  POKE(0x9F20,u->x*2);
  POKE(0x9F21,u->y+0x40);
  POKE(0x9F22,0x10);
  POKE(0x9F23,(u->team << 5)+u->index);
  POKE(0x9F23,(u->takenAction ? 0x90 : 0x00) + (u->team << 4)+ ((u->team != player1team) << 2));
}
unsigned char maxSteps;
struct Unit *checkU;
struct Tile tempT;
unsigned char checkSpaceInMvmtRange(unsigned char tx, unsigned char ty, unsigned char steps) {
  if (tx > m.boardWidth || ty > m.boardHeight) {return false;}
  if (tx == checkU->x && ty == checkU->y) {return true;}
  tempT = m.board[ty*m.boardWidth+tx];
  if (!checkU->airborne) {
    if (tempT.occupying != NULL && tempT.t->mvmtCosts[checkU->mvmtType] == 0) {return false;}
    steps += tempT.t->mvmtCosts[checkU->mvmtType];
    if (tempT.t->mvmtCosts[checkU->mvmtType] > 1 && steps >= maxSteps) {steps--;}
  } else {
    steps++;
  }
  if (steps > maxSteps) {return false;}
  /* recursive calls */
  return checkSpaceInMvmtRange(tx+1,ty,steps) || checkSpaceInMvmtRange(tx-1,ty,steps) || checkSpaceInMvmtRange(tx,ty-1,steps) || checkSpaceInMvmtRange(tx,ty+1,steps);
}

unsigned char unitLastX = 255;
unsigned char unitLastY = 255;
unsigned char move(struct Unit *u, unsigned char x, unsigned char y) {
  if ((u->x != x || u->y != y) && !u->takenAction && m.board[y*m.boardWidth+x].occupying == NULL && x < m.boardWidth && y < m.boardHeight) {
    maxSteps = u->mvmtRange;
    checkU = u;
    if (checkSpaceInMvmtRange(x,y,0)) {
      checkU = NULL;
      maxSteps = 0;
      unitLastX = u->x;
      unitLastY = u->y;
      POKE(0x9F20,unitLastX*2);
      POKE(0x9F21,0x40+unitLastY);
      POKE(0x9F22,0x00);
      POKE(0x9F23,28);
      m.board[unitLastY*m.boardWidth+unitLastX].occupying = NULL; // this line does not work
      u->x = x;
      u->y = y;
      m.board[y*m.boardWidth+x].occupying = u;
      return 1;
    }
  }
  return 0;
}
void undoMove(struct Unit *u) {
  m.board[u->y*m.boardWidth+u->x].occupying = NULL;
  POKE(0x9F20,u->x*2);
  POKE(0x9F21,0x40+u->y);
  POKE(0x9F22,0x00);
  POKE(0x9F23,28);
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

unsigned char canAttack(struct Unit *a, struct Unit *b) {
  if (SABS(b->x,a->x) + SABS(b->y,a->y) >= a->attackRangeMin && SABS(b->x,a->x) + SABS(b->y,a->y) <= a->attackRangeMax) {
    return damageChart[unitIndexes[b->index] * 18 + unitIndexes[a->index]];
  }
  return 0;
}
unsigned char calcPower(struct Unit *a, struct Unit *b) {
  unsigned char t = canAttack(a,b) * a->health / 100;
  if (t == 0) {return 0;}
  if (b->airborne) {return t;}
  return t ;//- (m.board[c.x+m.boardWidth*c.y].t->defense * t / 10);
}
void attack(struct Unit *attacker, struct Unit *defender) {
  defender->health -= calcPower(attacker,defender);
  if (defender->health >= 128) {defender->health = 0;}
  if (defender->health > 0) {
    attacker->health -= calcPower(defender,attacker);
    if (attacker->health >= 128) {attacker->health = 0;}
    if (attacker->health == 0) {
      m.board[m.boardWidth*attacker->y+attacker->x].occupying = NULL;
	  POKE(0x9F20,(attacker->x /*- m.left_view*/) * 2);
	  POKE(0x9F21,attacker->y /*- m.top_view*/ + 0x40);
	  __asm__ ("lda #0");
	  __asm__ ("sta $9F22");
	  __asm__ ("lda #28");
	  __asm__ ("sta $9F23");
      free(attacker);
    }
  } else {
    m.board[m.boardWidth*defender->y+defender->x].occupying = NULL;
	POKE(0x9F20,(defender->x /*- m.left_view*/) * 2);
	POKE(0x9F21,defender->y /*- m.top_view*/ + 0x40);
	__asm__ ("lda #0");
	__asm__ ("sta $9F22");
	__asm__ ("lda #28");
	__asm__ ("sta $9F23");
    free(defender);
  }
}
void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy) {
  unsigned char i = 0;
  struct Unit *north = NULL;
  struct Unit *east = NULL;
  struct Unit *south = NULL;
  struct Unit *west = NULL;

  if (cy > 0) {north = m.board[(cy-1)*m.boardWidth+cx].occupying;}
  if (cx > 0) {west = m.board[cy*m.boardWidth+cx-1].occupying;}
  if (cy < m.boardHeight - 1) {south = m.board[(cy+1)*m.boardWidth+cx].occupying;}
  if (cx < m.boardWidth - 1) {east = m.board[cy*m.boardWidth+cx+1].occupying;}
  if (north != NULL) {pA->attacks[i] = north; i++;}
  if (east != NULL) {pA->attacks[i] = east; i++;}
  if (south != NULL) {pA->attacks[i] = south; i++;}
  if (west != NULL) {pA->attacks[i] = west; i++;}
  pA->length = i;
} // not tested
