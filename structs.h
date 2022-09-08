struct Map;
struct Tile;
struct Terrain;
struct Unit;
struct Cursor;
struct Captureable;

typedef struct Unit {
  unsigned char index;
  unsigned char health;
  unsigned char ammo;
	unsigned char x,y;

  unsigned char mvmtRange;
  unsigned char mvmtType;
  unsigned char attackRangeMin:4;
  unsigned char attackRangeMax:4;

  unsigned char team:2;
  unsigned char canAttack:1;
  unsigned char canAttackAndMove:1;
  unsigned char takenAction:1;
  unsigned char airborne:1;
  unsigned char isVehicle:1;
  unsigned char navalOnly:1;
  
  struct Unit *carrying;
};

typedef struct Terrain {
  unsigned char tileIndex;
  unsigned char paletteOffset;
  unsigned char defense;
  unsigned char mvmtCosts[5];
};

typedef struct Tile {
  unsigned char dummy;

  struct Unit *occupying;
  struct Terrain *t;
  struct Captureable *base;
};

typedef struct Captureable {
  unsigned char team:6;
  unsigned char type:2;
  unsigned char health;
  unsigned char critical;
};

typedef struct Cursor {
  struct Unit *selected;
  unsigned char x,y,storex,storey;
  unsigned char frame;
};

typedef struct Map {
  unsigned char top_view, left_view, oldtop_view, oldleft_view;
  unsigned char whoseTurn;
  unsigned char boardWidth, boardHeight;
  unsigned short boardArea;
  unsigned char totalTiles;
  struct Tile *board;
};

typedef struct possibleAttacks {
  unsigned char length;
  struct Tile *attacks[4];
};

typedef struct Menu {
  unsigned char length;
  unsigned char selected;
  unsigned char options[3];
};
