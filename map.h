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

#define LEN_TERRAIN_ARRAY 16

#define CAPTUREABLE_CITY 0
#define CAPTUREABLE_HQ 1
#define CAPTUREABLE_FACTORY 2
#define CAPTUREABLE_AIRPORT 3
#define CAPTUREABLE_PORT 4


void setup_mem();

void initMap();
void initMapData(char data[]);
void renderMap();

void initCursor();
void renderCursor(unsigned char incFrame);

void renderUnitExplosion(unsigned char x, unsigned char y, unsigned char move_camera);

//void checkOldUnits();
struct Unit *malloc_unit();
void free_unit(struct Unit *u);

void initUnit(struct Unit *u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team);
void newTurnUnit(struct Unit *u, unsigned short i);

void clearUnitFromScreen(unsigned char x, unsigned char y);

/* Unit vars */
extern unsigned char unitLastX;
extern unsigned char unitLastY;
extern unsigned char unitLastFuel;
extern unsigned char baseLastHP;
extern unsigned char unitsdeadthisturn;

extern unsigned char terrainIsSet[];
extern struct Terrain terrainArray[];

extern unsigned char maxFuel[];

void initTile(struct Tile *t, unsigned char index);
void initTerrain(struct Terrain **t_pointer, unsigned char index);

void initCaptureable(struct Captureable *c, unsigned char init_team, unsigned char init_type);
void capture(struct Unit *u, struct Captureable *c);

void win(unsigned char team);
void nextTurn();

void drawMvmtArrow(unsigned char arr_len);

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

unsigned char canSupply(struct Unit *u);
void supplyUnits(struct Unit *u);

extern unsigned char player1co;
extern unsigned char player2co;

extern char co_names_array[][8];
#define CO_NAMES_ARRAY_LEN 6

#define CO_ANDY 0
#define CO_SAMI 1
#define CO_NELL 2
#define CO_GRIT 3
#define CO_DRAKE 4
#define CO_EAGLE 5