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
  unsigned char attackRangeMin;
  unsigned char attackRangeMax;

  unsigned char team;
  unsigned char canAttackAndMove;
  unsigned char takenAction;
  unsigned char airborne;
  unsigned char isVehicle;
  unsigned char navalOnly;
  
  struct Unit *carrying;
};

typedef struct Terrain {
  unsigned char tileIndex;
  unsigned char paletteOffset;
  unsigned char defense;
  unsigned char mvmtCosts[6];
};

typedef struct Tile {
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
  struct Tile *attacks[8];
};

typedef struct Menu {
  unsigned char length;
  unsigned char selected;
  unsigned char options[3];
};
