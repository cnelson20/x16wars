struct Map;
struct Tile;
struct Terrain;
struct Unit;
struct Cursor;
struct Captureable;

typedef struct Unit {
  unsigned char index;
  unsigned char x,y;
  unsigned char health;

  unsigned char mvmtRange;
  unsigned char attackRangeMin:4;
  unsigned char attackRangeMax:4;

  unsigned char team:2;
  unsigned char canAttack:1;
  unsigned char canAttackAndMove:1;
  unsigned char takenAction:1;
  unsigned char airborne:1;
  unsigned char isVehicle:1;
  unsigned char navalOnly:1;
};

typedef struct Terrain {
  unsigned char tileIndex;
  unsigned char paletteOffset;
  unsigned char defense;
  unsigned char mvmtCosts[4];
};

typedef struct Tile {
  unsigned char index;

  struct Unit *occupying;
  struct Terrain *t;
  struct Captureable *base;
};

typedef struct Captureable {
  char team;
  unsigned char critical:1;
};

typedef struct Cursor {
  struct Unit *selected;
  unsigned char x,y,storex,storey;
  unsigned char frame;
};

typedef struct Map {
  unsigned char top_view, left_view;
  struct Cursor *c;
  unsigned char player1Team:4;
  unsigned char player2Team:4;
  unsigned short turns;
  unsigned char whoseTurn;
  unsigned char gameOver; //boolean
  unsigned char boardWidth, boardHeight;
  unsigned char totalTiles;
  struct Tile *board;
};
