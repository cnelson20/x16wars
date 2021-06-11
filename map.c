#include <stdlib.h>
#include <peekpoke.h>
#include "structs.h"

// Method list
void initMap(struct Map *m);
void initMapData(struct Map *m, char data[]);
void renderMap(struct Map *m);
void initCursor(struct Cursor *c);
void renderCursor(struct Cursor *c, unsigned char incFrame);
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team);
void renderUnit(struct Unit *u);
void initTile(struct Tile *t, unsigned char index);
void initTerrain(struct Terrain *t, unsigned char index);
void newTurn(struct Unit *u, unsigned char restore);
// Variables;
extern struct Map *m;

// Map methods
void initMap(struct Map *m) {
  m = malloc(sizeof(struct Map));
  m->top_view = 0;
  m->left_view = 0;
  m->c = malloc(sizeof(struct Cursor));
  initCursor(m->c);
  m->turns = 0;
  m->player1Team = 0;
  m->player2Team = 2;
  m->whoseTurn = m->player1Team;
  m->gameOver = 0;
  m->boardWidth = 3;
  m->boardHeight = 3;
  m->board = malloc(m->boardWidth * m->boardHeight * sizeof(struct Tile));
}
void initMapData(struct Map *m, char data[]) {
  unsigned short i, mapI, boardArea;

  initMap(m);
  free(m->board);
  m->boardWidth = data[0];
  m->boardHeight = data[1];
  boardArea = m->boardWidth * m->boardHeight;
  m->board = malloc(boardArea * sizeof(struct Tile));

  for (i = 2; data[i] != 0xFF; i += 3) {}
  i++;
  for (; data[i] != 0xFF; i += 3) {}
  i++;
  for (mapI = 0; mapI < boardArea; mapI++) {
    initTile(&(m->board[mapI]),data[i]);
    i++;
  }
  for (i = 2; data[i] != 0xFF; i += 3) {
    m->board[data[i]+m->boardWidth*data[i+1]].occupying = malloc(sizeof (struct Unit));
    initUnit(m->board[data[i] + m->boardWidth*data[i+1]].occupying, data[i], data[i+1], data[i+2], m->player1Team);
  }
  i++;
  for (; data[i] != 0xFF; i += 3) {
    m->board[data[i]+m->boardWidth*data[i+1]].occupying = malloc(sizeof (struct Unit));
    initUnit(m->board[data[i] + m->boardWidth*data[i+1]].occupying, data[i], data[i+1], data[i+2], m->player2Team);
  }
  i++;
}

void renderMap(struct Map *m) {
  unsigned short i;
  unsigned char j,x,y;
  j = m->boardWidth;

  POKE(0x9F30,m->left_view << 4);
  POKE(0x9F37,PEEK(0x9F30));
  POKE(0x9F31,m->left_view >> 4);
  POKE(0x9F38,PEEK(0x9F31));
  POKE(0x9F30,m->top_view << 4);
  POKE(0x9F37,PEEK(0x9F32));
  POKE(0x9F33,m->top_view >> 4);
  POKE(0x9F3A,PEEK(0x9F33));
  x = 0; y = 0;
  __asm__ ("lda #0");
  __asm__ ("sta $9F20");
  __asm__ ("sta $9F21");
  __asm__ ("lda #$10");
  __asm__ ("sta $9F22");
  for (i = 0; y < m->boardHeight; i++) {
    POKE(0x9F23,m->board[i].t->tileIndex);
    POKE(0x9F23,m->board[i].t->paletteOffset);
    if (m->board[i].occupying != 0) {
      renderUnit(m->board[i].occupying);
      POKE(0x9F20,(x+1)*2);
      POKE(0x9F21,y);
      POKE(0x9F22,0x10);
    }
    if (++x >= m->boardWidth) {
      y++;
      //x = 0;
      __asm__ ("inc $9F21");
      __asm__ ("lda #0");
      __asm__ ("sta %o",x);
      __asm__ ("sta $9F20");
    }
  }
  renderCursor(m->c,1);
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
      break;
    case 1: // Water
      t->paletteOffset = 0x40;
      t->defense = 0;
      t->mvmtCosts[0] = 0;
      t->mvmtCosts[1] = 0;
      t->mvmtCosts[2] = 0;
      t->mvmtCosts[3] = 1;
      break;
    case 2: // Vertical Road
    case 3: // Horizontal Road
      t->paletteOffset = 0x50;
      t->defense = 0;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 1;
      t->mvmtCosts[3] = 0;
      break;
    case 4: // Gray City
      t->paletteOffset = 0x50;
      t->defense = 3;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 1;
      t->mvmtCosts[3] = 0;
      break;
    case 5: //Plains
      t->paletteOffset = 0x60;
      t->defense = 1;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 1;
      t->mvmtCosts[2] = 2;
      t->mvmtCosts[3] = 0;
    case 6: // Forest
      t->paletteOffset = 0x60;
      t->defense = 2;
      t->mvmtCosts[0] = 1;
      t->mvmtCosts[1] = 2;
      t->mvmtCosts[2] = 3;
      t->mvmtCosts[3] = 0;
      break;
    case 7: // Mountain
      t->paletteOffset = 0x70;
      t->defense = 4;
      t->mvmtCosts[0] = 2;
      t->mvmtCosts[1] = 0;
      t->mvmtCosts[2] = 0;
      t->mvmtCosts[3] = 0;
      break;
  }
}


// Cursor methods
void initCursor(struct Cursor *c) {
  c->x = 0;
  c->y = 0;
  c->storex = 0;
  c->storey = 0;
  c->frame = 0;
}

void renderCursor(struct Cursor *c, unsigned char incFrame) {
  POKEW(0x9F20,0xFC00);
  POKE(0x9F22,0x11);
  POKE(0x9F23,c->frame * 8 % 256);
  POKE(0x9F23,(c->frame * 8 / 256)&8);
  POKE(0x9F23,((c->x*16)-8)%256);
  POKE(0x9F23,((c->x*16)-8)/256);
  POKE(0x9F23,((c->y*16)-8)%256);
  POKE(0x9F23,((c->y*16)-8)/256);
  POKE(0x9F23,0x0F);
  POKE(0x9F23,0xA0);

  if (incFrame) {c->frame++;}
}

//Unit methods
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team) {
  u->x = init_x;
  u->y = init_y;
  m->board[init_x+m->boardWidth*init_y].occupying = u;
  u->index = index;
  u->team = team;
  u->health = 100;
}

void newTurn(struct Unit *u, unsigned char restore) {
  u->takenAction = 0;
  if (restore && 0) {
    u->health += 20;
    if (u->health > 100) {u->health = 100;}
  }
}

void renderUnit(struct Unit *u) {
  POKE(0x9F20,u->x*2);
  POKE(0x9F21,u->y+0x40);
  POKE(0x9F22,0x10);
  POKE(0x9F23,u->index);
  POKE(0x9F23,(u->team << 4)+ ((u->team != m->player1Team) << 2));
}
