#define NULL 0

#define UNIT_APC 0
#define UNIT_RECON 1
#define UNIT_MECH 2
#define UNIT_INFANTRY 3
#define UNIT_ANTI_AIR 9
#define UNIT_MEDIUM_TANK 0xa
#define UNIT_SMALL_TANK 0xb
#define UNIT_MISSILES 0xc
#define UNIT_ROCKETS 0xd
#define UNIT_ARTILLERY 0xe
#define UNIT_TRANSPORT 0x10
#define UNIT_COPTER 0x11
#define UNIT_BOMBER 0x12
#define UNIT_FIGHTER 0x13
#define UNIT_LANDER 0x18
#define UNIT_SUBMARINE 0x19
#define UNIT_CRUISER 0x1A
#define UNIT_BATTLESHIP 0x1B

void initMap();
void initMapData(char data[]);
void renderMap();

void initCursor();
void renderCursor(unsigned char incFrame);

void checkOldUnits();
void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team);
void newTurnUnit(struct Unit *u, unsigned short i);
void renderUnit(struct Unit *u);
void removeRenderUnit(struct Unit *u);

/* Unit vars */
extern unsigned char unitLastX;
extern unsigned char unitLastY;
extern unsigned char unitLastFuel;
extern unsigned char baseLastHP;
extern unsigned char unitsdeadthisturn;

void initTile(struct Tile *t, unsigned char index);
void initTerrain(struct Terrain **t_pointer, unsigned char index);

void initCaptureable(struct Captureable *c, unsigned char init_team, unsigned char init_type);
void capture(struct Unit *u, struct Captureable *c);

void win(unsigned char team);
void nextTurn();

unsigned char move(struct Unit *u, unsigned char x, unsigned char y);
void undoMove(struct Unit *u);

unsigned char canAttack(struct Unit *a, struct Unit *b);
unsigned char damagePreview(struct Unit *a, struct Unit *b);
unsigned char calcPower(struct Unit *a, struct Unit *b);
void attack(struct Unit *attacker, struct Unit *defender);

void getPossibleAttacks(struct possibleAttacks *pA, unsigned char cx, unsigned char cy, unsigned char attackRangeMax);
void getPossibleDrops(struct possibleAttacks *pA, struct Unit *u);
unsigned char sizeofGetPossibleDrops(struct Unit *u);

void getPossibleJoins(struct possibleAttacks *pA, struct Unit *u);
unsigned char sizeofGetPossibleJoins(struct Unit *u);