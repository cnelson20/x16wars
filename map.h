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
void win(unsigned char team);
void nextTurn();

unsigned char move(struct Unit *u, unsigned char x, unsigned char y);
void undoMove(struct Unit *u);
void attack(struct Unit *attacker, struct Unit *defender);
void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy);