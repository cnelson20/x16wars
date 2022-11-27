#include <stdlib.h>
#include <cbm.h>
#include <peekpoke.h>
#include <string.h>

#define SABS(a, b)((a >= b) ? (a - b) : (b - a))
#define SHRTCIRCUIT_AND(a, b)((a) ? (b) : 0)
#define SHRTCIRCUIT_OR(a, b)((a) ? 1 : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#include "zsound/pcmplayer.h"
#include "zsound/zsmplayer.h"

#include "structs.h"
#include "map.h"
#include "waitforjiffy.h"
#include "main.h"

// Variables
extern struct Map m;
extern struct Cursor c;
extern struct Cursor attackCursor;
extern struct possibleAttacks useaspossibleAttacks;
extern struct possibleAttacks *pA;
extern struct Menu menuOptions;
extern unsigned char vera_display_mode;

unsigned char returnToMenu;
unsigned char player1team = 0;
unsigned char player1co = 0;
unsigned char player2team = 2;
unsigned char player2co = 0;
unsigned char turncounter;
unsigned char unitsdeadthisturn = 0;
unsigned char currentbases = 0;
unsigned char oldbases;
unsigned char currentunitsprites = 0;
unsigned char oldunitsprites;

unsigned char playerFunds[4];
unsigned char playerUnitCounts[4];
unsigned char playerFactoryCounts[4];
unsigned char moneyMatters;

#define GAME_MAX_UNITS 100
#define GAME_MAX_BASES 64

// Malloc but not broken
struct Unit unitArray[GAME_MAX_UNITS];
unsigned char unitArrayUses[GAME_MAX_UNITS];

struct Captureable captureableArray[GAME_MAX_BASES];
unsigned char captureableArrayUses[GAME_MAX_BASES];

void setup_mem() {
    memset(unitArrayUses, 0, GAME_MAX_UNITS);
    memset(captureableArrayUses, 0, sizeof(captureableArrayUses));

    memset( &c, 0, sizeof(c));
    memset( &attackCursor, 0, sizeof(attackCursor));
    memset( &menuOptions, 0, sizeof(menuOptions));
}

struct Unit *malloc_unit() {
    static unsigned char i;
    for (i = 0; i < GAME_MAX_UNITS; ++i) {
        if (!unitArrayUses[i]) {
            unitArrayUses[i] = 1;
            return &(unitArray[i]);
        }
    }
    return NULL;
}
void free_unit(struct Unit *u) {
    --playerUnitCounts[u->team];
    unitArrayUses[(u - unitArray) / sizeof(struct Unit)] = 0;
}

struct Captureable *malloc_captureable() {
    static unsigned char i;
    for (i = 0; i < GAME_MAX_BASES; ++i) {
        if (!captureableArrayUses[i]) {
            captureableArrayUses[i] = 1;
            return &(captureableArray[i]);
        }
    }
}

extern unsigned char screen_width;
extern unsigned char screen_height;

extern unsigned char game_width;
extern unsigned char game_height;
extern unsigned char gui_vera_offset;

// Map methods
void initMap() {
    setup_mem();
    memset( &terrainIsSet, 0, LEN_TERRAIN_ARRAY);

    m.whoseTurn = player1team;
    turncounter = 0;

    m.top_view = 0;
    m.oldtop_view = 0;
    m.left_view = 0;
    m.oldleft_view = 0;

    screen_width = 20;
    screen_height = 15;
    game_width = 15;
    game_height = 10;
    gui_vera_offset = 0x4A;

    moneyMatters = 0;

    playerUnitCounts[player1team] = 0;
    playerUnitCounts[player2team] = 0;
    playerFunds[player1team] = 5;
    playerFunds[player2team] = 5;
}
void initMapData(char data[]) {
    unsigned short i, mapI, temp;

    initMap();
    m.boardWidth = data[0];
    m.boardHeight = data[1];
    m.boardArea = m.boardWidth * m.boardHeight;
    m.board = (void *)0xA000;
    POKE(0x00, MAP_HIRAM_BANK);

    for (i = 2; data[i] != 0xFF; i += 3) {}
    i++;
    for (; data[i] != 0xFF; i += 3) {}
    for (; data[i] == 0xFF; ++i) {} // Allow excess $FF values to make map easier to edit in hxd
    for (mapI = 0; mapI < m.boardArea; ++mapI) {
        initTile( & (m.board[mapI]), data[i]);
        ++i;
    }
    for (i = 2; data[i] != 0xFF; i += 3) {
        temp = data[i] + m.boardWidth * data[i + 1];
        m.board[temp].occupying = malloc_unit();

        initUnit(m.board[temp].occupying, data[i], data[i + 1], data[i + 2], player1team);
        ++playerUnitCounts[player1team];
    }
    ++i;
    for (; data[i] != 0xFF; i += 3) {
        temp = data[i] + m.boardWidth * data[i + 1];
        m.board[temp].occupying = malloc_unit();
        initUnit(m.board[temp].occupying, data[i], data[i + 1], data[i + 2], player2team);
        ++playerUnitCounts[player2team];
    }
    ++i;
}

extern struct Menu menuOptions;
unsigned char captureablePaletteOffsets[] = {0xd, 0xd, 0xe, 0xe, 0x8};
unsigned char captureableSpriteOffsets[] = {
        18, 34, 50, 66, 82, 0, 0, 0, // Red
        26, 42, 58, 74, 90, 0, 0, 0, // Green
        18, 34, 50, 66, 82, 0, 0, 0, // Blue
        26, 42, 58, 74, 90, 0, 0, 0, // Yellow
        26, 42, 58, 74, 90, 0, 0, 0, // Neutral
};

extern void render_tiles();
extern void render_unit_sprites();
extern void checkOldUnits();
extern void __fastcall__ renderUnit(struct Unit *u);
extern void __fastcall__ removeRenderUnit(struct Unit *u);

void renderMap() {

    checkOldUnits();
    render_tiles();

    render_unit_sprites();
    while (oldunitsprites > currentunitsprites) {
        __asm__("stz $9F23");
        __asm__("stz $9F23");
        __asm__("stz $9F23");
        __asm__("stz $9F23");

        __asm__("stz $9F23");
        __asm__("stz $9F23");
        __asm__("stz $9F23");
        __asm__("stz $9F23");
        --oldunitsprites;
    }
    m.oldtop_view = m.top_view;
    m.oldleft_view = m.left_view;

    renderCursor(1);
    if (menuOptions.length != 0) {
        POKE(0x9F20, 0x06);
        POKE(0x9F21, 0xFC);
        POKE(0x9F22, 0x41);
        POKE(0x9F23, 0);
        POKE(0x9F23, 0);
        POKE(0x9F23, 0);
        POKE(0x9F23, 0);
    }

    m.oldtop_view = m.top_view;
    m.oldleft_view = m.left_view;
}

char co_names_array[][8] = {
        "andy",  /* Andy */
        "sami",  /* Sami */
        "nell",  /* Nell */
        "grit",  /* Grit */
        "drake",  /* Drake */
        "eagle",	 /* Eagle */
};
//#define CO_NAMES_ARRAY_LEN 6
// max was too OP


unsigned char colorstrings[4][7] =
        {{0xb1, 0xa4, 0xa3, 0, 0, 0, 0}, /* red */
         {0xa6, 0xb1, 0xa4, 0xa4, 0xad, 0, 0}, /* green */
         {0xa1, 0xab, 0xb4, 0xa4, 0, 0, 0}, /* blue */
         {0xb8, 0xa4, 0xab, 0xab, 0xae, 0xb6, 0}}; /* yellow */
unsigned char colorstringlengths[4] = {3,5,4,6};

extern void __fastcall__ clear_sprite_table(unsigned char from);

void __fastcall__ win(unsigned char team) {
    unsigned short i;

    POKE(0x9F20, ((game_width >> 1) - (colorstringlengths[team] >> 1)) << 1);
    POKE(0x9F21, 0x40 + (game_height >> 1));
    POKE(0x9F22, 0x10);
    for (i = 0; i < colorstringlengths[team]; ++i) {
        POKE(0x9F23, colorstrings[team][i]);
        POKE(0x9F23, 0x80);
    }
    POKE(0x9F20, 2 * ((game_width >> 1) - 2));
    POKE(0x9F21, 0x41 + (game_height >> 1));

    POKE(0x9F23, 0xb6); // w
    POKE(0x9F23, 0x80);
    POKE(0x9F23, 0xa8); // i
    POKE(0x9F23, 0x80);
    POKE(0x9F23, 0xad); // n
    POKE(0x9F23, 0x80);
    POKE(0x9F23, 0xb2); // s
    POKE(0x9F23, 0x80);

    POKEW(0x9F20, 0xFC00);
    POKE(0x9F22, 0x11);
    i = 5;
    while (i != 0) {
        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");

        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");
        __asm__ ("stz $9F23");
        --i;
    }

    zsm_stopmusic();
    pcm_trigger_digi(MISSION_SUCCESS_MUSIC_BANK, HIRAM_START);

    i = 60 * 8;
    while (i != 0) {
        pcm_play();
        zsm_play();
        waitforjiffy();
        --i;
    }
    POKE(0x9F29, 0x30 | vera_display_mode);
    POKE(0x00, MAP_HIRAM_BANK);
    returnToMenu = 1;
}

void nextTurn() {
    unsigned short i = 0;

    zsm_stopmusic();
    if (m.whoseTurn == player2team) {
        m.whoseTurn = player1team;
        zsm_startmusic(CO_MUSIC_BANK, HIRAM_START);
        ++turncounter;
    } else {
        m.whoseTurn = player2team;
        zsm_startmusic(CO2_MUSIC_BANK, HIRAM_START);
    }
    zsm_forceloop(0);


    if (turncounter > 0) {
        for (i = 0; i < m.boardArea; ++i) {
            if (m.board[i].occupying != NULL) {
                newTurnUnit(m.board[i].occupying, i);
            }
            if (m.board[i].base != NULL && m.board[i].base->team == m.whoseTurn) {
                playerFunds[m.whoseTurn] += 1;
            }
        }
        if (playerFunds[m.whoseTurn] > 99) {
            playerFunds[m.whoseTurn] = 99;
        }
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

#define TILE_ROAD_SE 11
#define TILE_ROAD_SW 12
#define TILE_ROAD_NE 13
#define TILE_ROAD_NW 14

#define MAX_TILE_INDEX 14

//Tile methods
void initTile(struct Tile * t, unsigned char index) {
    initTerrain( & (t->t), index >= 0x40 ? TILE_CITY : index);
    t->base = NULL;
    t->occupying = NULL;

    if (index >= 0x40 && index < 0x54) {
        unsigned char team = (index % 4 == 0 ? player1team : (index % 4 == 1 ? player2team : 4));

        t->base = malloc_captureable();
        initCaptureable(t->base, team, (index & 0x1F) >> 2);
        t->t->tileIndex = 0x85;
        t->t->paletteOffset = 0x60;
    }
}

unsigned char terrainDefenseArray[] = {
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x01, 0x02, 0x04,
        0xFF, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00,
};
unsigned char terrainPaletteOffsetArray[] = {
        0x40, 0x40, 0x50, 0x50,
        0x50, 0x60, 0x60, 0x70,
        0xFF, 0x40, 0x40, 0x50,
        0x50, 0x50, 0x50,
};

//Terrain Method
// mvmtCosts[0] = infantry
// mvmtCosts[1] = treads
// mvmtCosts[2] = tires
// mvmtCosts[3] = boat
// mvmtCosts[4] = mech
// mvmtCosts[5] = lander

unsigned char terrainMvmtCostsArray[][6] = {
        {0, 0, 0, 2, 0, 2}, // Reef
        {0, 0, 0, 1, 0, 1}, // Water
        {1, 1, 1, 0, 1, 0}, // Roads / Cities
        {1, 1, 2, 0, 1, 0}, // Plains
        {1, 2, 3, 0, 1, 0}, // Forest
        {2, 0, 0, 0, 1, 0}, // Mountain / River
        {1, 1, 1, 0, 1, 1}, // Shoal
};

unsigned char mvmtCostArrayIndex[] = {
        0, 1, 2, 2, 2, 3, 4, 5, 0xFF, 6, 5, 2, 2, 2, 2,
};

unsigned char terrainIsSet[LEN_TERRAIN_ARRAY];
struct Terrain terrainArray[LEN_TERRAIN_ARRAY];

void initTerrain(struct Terrain **t_pointer, unsigned char index) {
    if (terrainIsSet[index]) {
        *t_pointer = &(terrainArray[index]);
    } else {
        struct Terrain *t = &(terrainArray[index]);
        *t_pointer = t;

        t->tileIndex = index + 0x80;
        t->paletteOffset = terrainPaletteOffsetArray[index];
        t->defense = terrainDefenseArray[index];
        t->mvmtCosts = terrainMvmtCostsArray[mvmtCostArrayIndex[index]];

        terrainIsSet[index] = 1;
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
    static unsigned short tempx, tempy;
    static unsigned char coff;

    coff = cursoroffsets[c.frame >> 3];
    if (pA == NULL || menuOptions.length != 0) {

        POKE(0x9F20, 0x08);
        POKE(0x9F21, 0xFC);
        POKE(0x9F22, 0x11);

        // Part 0
        if (menuOptions.length == 0) {
            POKE(0x9F23, 0);
            POKE(0x9F23, 8);
            tempx = (c.x << 4) - 3 + coff;
            POKE(0x9F23, tempx);
            POKE(0x9F23, tempx >> 8);
            tempy = (c.y << 4) - 4 + coff;
            POKE(0x9F23, tempy);
            POKE(0x9F23, tempy >> 8);
            POKE(0x9F23, 12);
            POKE(0x9F23, 0x08);
            // Part 1
            POKE(0x9F23, 1);
            POKE(0x9F23, 8);
            tempx = ((c.x + 1) << 4) - 3 - coff;
            POKE(0x9F23, tempx);
            POKE(0x9F23, tempx >> 8);
            tempy = (c.y << 4) - 4 + coff;
            POKE(0x9F23, tempy);
            POKE(0x9F23, tempy >> 8);
            POKE(0x9F23, 12);
            POKE(0x9F23, 0x08);
            // Part 2
            POKE(0x9F23, 2);
            POKE(0x9F23, 8);
            tempx = (c.x << 4) - 3 + coff;
            POKE(0x9F23, tempx);
            POKE(0x9F23, tempx >> 8);
            tempy = ((c.y + 1) << 4) - 4 - coff;
            POKE(0x9F23, tempy);
            POKE(0x9F23, tempy >> 8);
            POKE(0x9F23, 12);
            POKE(0x9F23, 0x08);
        }
        // Part 3
        POKE(0x9F23, 3);
        POKE(0x9F23, 8);
        tempx = ((c.x + 1) << 4) - 3 - coff;
        POKE(0x9F23, tempx);
        POKE(0x9F23, tempx >> 8);
        tempy = ((c.y + 1) << 4) - 4 - coff;
        POKE(0x9F23, tempy);
        POKE(0x9F23, tempy >> 8);
        POKE(0x9F23, 12);
        POKE(0x9F23, 0x58);
    } else {

        POKE(0x9F20, 0x08);
        POKE(0x9F21, 0xFC);
        POKE(0x9F22, 0x11);

        // Part 0
        POKE(0x9F23, 0);
        POKE(0x9F23, 8);
        tempx = ((attackCursor.x - m.left_view) << 4) - 3 + coff;
        POKE(0x9F23, tempx);
        POKE(0x9F23, tempx >> 8);
        tempy = ((attackCursor.y - m.top_view) << 4) - 4 + coff;
        POKE(0x9F23, tempy);
        POKE(0x9F23, tempy >> 8);
        POKE(0x9F23, 12);
        POKE(0x9F23, 0x08);
        // Part 1
        POKE(0x9F23, 1);
        POKE(0x9F23, 8);
        tempx = ((attackCursor.x - m.left_view + 1) << 4) - 3 - coff;
        POKE(0x9F23, tempx);
        POKE(0x9F23, tempx >> 8);
        tempy = ((attackCursor.y - m.top_view) << 4) - 4 + coff;
        POKE(0x9F23, tempy);
        POKE(0x9F23, tempy >> 8);
        POKE(0x9F23, 12);
        POKE(0x9F23, 0x08);
        // Part 2
        POKE(0x9F23, 2);
        POKE(0x9F23, 8);
        tempx = ((attackCursor.x - m.left_view) << 4) - 3 + coff;
        POKE(0x9F23, tempx);
        POKE(0x9F23, tempx >> 8);
        tempy = ((attackCursor.y - m.top_view + 1) << 4) - 4 - coff;
        POKE(0x9F23, tempy);
        POKE(0x9F23, tempy >> 8);
        POKE(0x9F23, 12);
        POKE(0x9F23, 0x08);
        // Part 3
        POKE(0x9F23, 3);
        POKE(0x9F23, 8);
        tempx = ((attackCursor.x - m.left_view + 1) << 4) - 3 - coff;
        POKE(0x9F23, tempx);
        POKE(0x9F23, tempx >> 8);
        tempy = ((attackCursor.y - m.top_view + 1) << 4) - 4 - coff;
        POKE(0x9F23, tempy);
        POKE(0x9F23, tempy >> 8);
        POKE(0x9F23, 12);
        POKE(0x9F23, 0x58);
    }
    if (incFrame) {
        c.frame = (c.frame + 1) & 0x1F;
    }
}

// Captureable Methods
void initCaptureable(struct Captureable * c, unsigned char init_team, unsigned char init_type) {
    c->team = init_team;
    c->type = init_type;
    if (init_type >= CAPTUREABLE_FACTORY) {
        ++playerFactoryCounts[init_team];
        moneyMatters = 1;
    }
    c->critical = init_type == 1;
    c->health = 20;
    /*
      type 0 = base
      type 1 = hq
      type 2 = factory
    */
}

unsigned char unitsCanBuild[][10] = {
        {0, 1, 2, 3, 9, 0xa, 0xb, 0xc, 0xd, 0xe},
        {0x10, 0x11, 0x12, 0x13},
        {0x18, 0x19, 0x1a, 0x1b},
};

unsigned char sizeOfUnitsBuildListMinus1[] = {9, 3, 3};
unsigned char unitsCost[] = {
        5,
        4,
        3,
        1,
        8,
        16,
        7,
        12,
        15,
        6,
        5,
        9,
        22,
        20,
        12,
        20,
        18,
        28
};

void capture(struct Unit * u, struct Captureable * c) {
    unsigned char i;

    if (u->team != c->team) {
        i = (u->health + 9) / 10;
        if (i >= c->health) {
            if (c->type >= CAPTUREABLE_FACTORY) {
                ++playerFactoryCounts[m.whoseTurn];
                --playerFactoryCounts[c->team];
            }
            c->team = u->team;
            c->health = 20;
            if (c->critical) {
                win(m.whoseTurn);
            }
        } else {
            c->health -= i;
        }
    }
}

unsigned char explosionSpriteOffsets[] = {
        25, 28,
        18, 19,
        14, 14,
        15, 12,
        11, 8,
        10, 6,
        10, 5,
        9, 2,
        8, 2,
};


void renderUnitExplosion(unsigned char x, unsigned char y, unsigned char move_camera) {
    static unsigned char i;
    unsigned short temp;
    unsigned char tx;
    unsigned char ty;

    if (move_camera) {
        tx = m.left_view;
        ty = m.top_view;

        if (x < m.left_view) {
            m.left_view = x;
            if (m.left_view != 0) {
                m.left_view--;
            }
        } else if (x >= m.left_view + 14) {
            m.left_view = x - 13;
        }
        if (y < m.top_view) {
            m.top_view = y;
            if (m.top_view != 0) {
                m.top_view--;
            }
        } else if (y >= m.top_view + 9) {
            m.top_view = y - 8;
        }
        c.x = x - m.left_view;
        c.y = y - m.top_view;
    }
    renderMap();
    POKE(0x9F21, 0x4A);
    clearOtherLines();

    x = (x - m.left_view) << 4;
    y = (y - m.top_view) << 4;

    POKE(0x9F20, 8 + 6);
    POKE(0x9F21, 0xFC);
    POKE(0x9F22, 0x41);
    for (i = 0; i < 4; ++i) {
        __asm__("stz $9F23");
    }

    POKE(0x9F22, 0x11);
    for (i = 0; i <= 8; ++i) {
        unsigned short addr = 16 * i + 128;

        POKEW(0x9F20, 0xFC00); // halfway through sprite table

        POKE(0x9F23, addr);
        POKE(0x9F23, 0x08 + (addr >> 8));
        temp = x - 16 + explosionSpriteOffsets[i * 2];
        POKE(0x9F23, temp);
        POKE(0x9F23, temp >> 8);
        temp = y - 32 + explosionSpriteOffsets[i * 2 + 1];
        POKE(0x9F23, temp);
        POKE(0x9F23, temp >> 8);
        POKE(0x9F23, 0xC);
        POKE(0x9F23, 0xAF);

        for (temp = 0; temp < 4; ++temp) {
            waitforjiffy();
            pcm_play();
            zsm_play();
        }
        POKE(0x00, MAP_HIRAM_BANK);
    }

    POKE(0x9F22, 0x11);
    POKEW(0x9F20, 0xFC00);
    __asm__("stz $9F23");
    __asm__("stz $9F23");
    __asm__("stz $9F23");
    __asm__("stz $9F23");

    __asm__("stz $9F23");
    __asm__("stz $9F23");
    __asm__("stz $9F23");
    __asm__("stz $9F23");

}

#define ARROW_STRAIGHT_INDEX_TD 64
#define ARROW_STRAIGHT_INDEX_LR 68

#define ARROW_HEAD_INDEX_D 72
#define ARROW_HEAD_INDEX_U 100
#define ARROW_HEAD_INDEX_R 76
#define ARROW_HEAD_INDEX_L 96

#define ARROW_CURVE_INDEX_SE 80
#define ARROW_CURVE_INDEX_NE 84
#define ARROW_CURVE_INDEX_SW 88
#define ARROW_CURVE_INDEX_NW 92

#define VFLIP 2
#define HFLIP 1

extern unsigned char path_array_x[];
extern unsigned char path_array_y[];

void drawMvmtArrow(unsigned char arr_len) {
    unsigned char i;
    unsigned char v_hflip = 0;
    unsigned short temp;

    if (arr_len == 0) { return; }

    POKEW(0x9F20, 0xFC28);
    POKE(0x9F22, 0x11);
    for (i = arr_len - 1; i != 0xFF; --i) {
        if (i == 0) {
            if (path_array_x[1] == path_array_x[0]) {
                if (path_array_y[0] > path_array_y[1]) {
                    POKE(0x9F23, ARROW_HEAD_INDEX_D);
                } else {
                    POKE(0x9F23, ARROW_HEAD_INDEX_U);
                }
            } else {
                if (path_array_x[0] > path_array_x[1]) {
                    POKE(0x9F23, ARROW_HEAD_INDEX_R);
                } else {
                    POKE(0x9F23, ARROW_HEAD_INDEX_L);
                }
            }
        } else if (i == arr_len - 1) {
            if (path_array_x[i - 1] == path_array_x[i]) {
                POKE(0x9F23, ARROW_STRAIGHT_INDEX_TD);
            } else {
                POKE(0x9F23, ARROW_STRAIGHT_INDEX_LR);
            }
        } else {
            if (path_array_x[i-1] == path_array_x[i+1] || path_array_y[i-1] == path_array_y[i+1]) {
                if (path_array_x[i] == path_array_x[i+1]) {
                    POKE(0x9F23, ARROW_STRAIGHT_INDEX_TD);
                } else {
                    POKE(0x9F23, ARROW_STRAIGHT_INDEX_LR);
                }
            } else {
                if (path_array_x[i + 1] > path_array_x[i]) {
                    POKE(0x9F23, (path_array_y[i] > path_array_y[i - 1]) ? ARROW_CURVE_INDEX_NE : ARROW_CURVE_INDEX_SE);
                } else if (path_array_x[i + 1] < path_array_x[i]) {
                    POKE(0x9F23, (path_array_y[i] > path_array_y[i - 1]) ? ARROW_CURVE_INDEX_NW : ARROW_CURVE_INDEX_SW);
                }	else {
                    if (path_array_x[i - 1] > path_array_x[i]) {
                        POKE(0x9F23, (path_array_y[i] > path_array_y[i + 1]) ? ARROW_CURVE_INDEX_NE : ARROW_CURVE_INDEX_SE);
                    } else /*if (path_array_x[i - 1] < path_array_x[i])*/ {
                        POKE(0x9F23, (path_array_y[i] > path_array_y[i + 1]) ? ARROW_CURVE_INDEX_NW : ARROW_CURVE_INDEX_SW);
                    }
                }
            }
        }

        POKE(0x9F23, 9);
        temp = path_array_x[i] - m.left_view;
        POKE(0x9F23, temp << 4);
        POKE(0x9F23, temp >> 4);
        temp = path_array_y[i] - m.top_view;
        POKE(0x9F23, temp << 4);
        POKE(0x9F23, temp >> 4);
        POKE(0x9F23, ((i == arr_len - 1) ? 0x8 : 0xC) | v_hflip);
        POKE(0x9F23, 0x50);
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
        5,4,6,5,
}; /* Lander  Submarine  Cruiser  Battleship */
unsigned char mvmtTypes[] = {
        1,2,4,0,
        9,9,9,9,
        9,1,1,1,
        2,2,1,9,
        1,1,1,1, /* airborne units arent affected by this */
        9,9,9,9,
        5,3,3,3,
};
unsigned char maxFuel[] = {
        70,80,99,99,
        0 , 0, 0, 0,
        0 ,60,50,70,
        50,50,50, 0,
        99,99,99,99,
        0 , 0, 0, 0,
        99,60,99,99,
};

void initUnit(struct Unit * u, unsigned char init_x, unsigned char init_y, unsigned char index, unsigned char team) {
    u->x = init_x;
    u->y = init_y;
    m.board[init_x + m.boardWidth * init_y].occupying = u;
    u->index = index;
    u->team = team;
    u->health = 100;
    u->ammo = 10;
    u->fuel = maxFuel[index];
    u->mvmtType = mvmtTypes[index];
    u->takenAction = 0;
    u->mvmtRange = mvmtRanges[index];
    if ((index >= UNIT_MISSILES && index <= UNIT_ARTILLERY) || index == UNIT_BATTLESHIP) {
        /* Ranges:
        artillery : [2,3]
        rockets & missiles : [3, 5]
        battleship : [2,6]
        */
        u->attackRangeMin = (index == UNIT_ARTILLERY || index == UNIT_BATTLESHIP) ? 2 : 3;
        u->attackRangeMax = index == UNIT_ARTILLERY ? 3 : (index == UNIT_BATTLESHIP ? 6 : 5);
    } else {
        u->attackRangeMin = 0;
        u->attackRangeMax = 1;
    }
    u->airborne = index >= UNIT_TRANSPORT && index <= UNIT_FIGHTER;

    u->canAttackAndMove = !(index == UNIT_MISSILES || index == UNIT_ROCKETS || index == UNIT_ARTILLERY || index == UNIT_BATTLESHIP);

    u->carrying = NULL;
}

#define FUEL_SUSTAIN_OFFSET 0x10
unsigned char fuelSustainCostsArray[] = {2, 2, 5, 5};

void newTurnUnit(struct Unit * u, unsigned short i) {
    u->takenAction = 0;
    if (m.whoseTurn == u->team && m.board[i].base != NULL && m.board[i].base->team == u->team) {
        u->health += 20;
        u->ammo = 10;
        u->fuel = maxFuel[u->index];
        if (u->health > 100) {
            u->health = 100;
        }
    }
    if (m.whoseTurn == u->team) {
        struct Unit * up;
        /* If a APC unit is surrounding a unit, refill ammo */
        if (u->x > 0 && m.board[i - 1].occupying != NULL) {
            up = m.board[i - 1].occupying;
            if (up->team == u->team && up->index == 0) {
                u->ammo = 10;
                u->fuel = maxFuel[u->index];
                goto apc_nearby_exit;
            }
        }
        if (u->x < m.boardWidth - 1 && m.board[i + 1].occupying != NULL) {
            up = m.board[i + 1].occupying;
            if (up->team == u->team && up->index == 0) {
                u->ammo = 10;
                u->fuel = maxFuel[u->index];
                goto apc_nearby_exit;
            }
        }
        if (u->y > 0 && m.board[i - m.boardWidth].occupying != NULL) {
            up = m.board[i - m.boardWidth].occupying;
            if (up->team == u->team && up->index == 0) {
                u->ammo = 10;
                u->fuel = maxFuel[u->index];
                goto apc_nearby_exit;
            }
        }
        if (u->y < m.boardHeight - 1 && m.board[i + m.boardWidth].occupying != NULL) {
            up = m.board[i + m.boardWidth].occupying;
            if (up->team == u->team && up->index == 0) {
                u->ammo = 10;
                u->fuel = maxFuel[u->index];
                goto apc_nearby_exit;
            }
        }
        apc_nearby_exit: ; /* semicolon so compiler says a-ok */
        if (u->index >= UNIT_TRANSPORT) {
            unsigned char fuel_cost = u->airborne ? fuelSustainCostsArray[u->index - FUEL_SUSTAIN_OFFSET] : 1;
            if (u->fuel > fuel_cost) {
                u->fuel -= fuel_cost;
            } else {
                // Destroy unit
                m.board[i].occupying = NULL;
                if (u->carrying) {
                    free_unit(u->carrying);
                }
                clearUnitFromScreen(u->x, u->y);
                pcm_trigger_digi(UNIT_EXPLODING_BANK, HIRAM_START);
                renderUnitExplosion(u->x, u->y, 1);
                free_unit(u);
            }
        }
    }
}

void clearUnitFromScreen(unsigned char x, unsigned char y) {
    if (x >= m.left_view && x < m.left_view + game_width && y >= m.top_view && y < m.top_view < game_height) {
        POKE(0x9F22, 0x10);
        POKE(0x9F20, (x - m.left_view) << 1);
        POKE(0x9F21, y - m.top_view + 0x40);
        POKE(0x9F23, 28);
    }
}

unsigned char canCarryUnit(unsigned char carrier_index, unsigned char carried_index) {
    if ((carrier_index == UNIT_APC || carrier_index == UNIT_TRANSPORT) && carried_index >= UNIT_MECH && carried_index <= UNIT_INFANTRY) {
        return 1;
    } else if (carrier_index == UNIT_LANDER && carried_index <= UNIT_ARTILLERY) {
        return 1;
    } else if (carrier_index == UNIT_CRUISER && (carried_index == UNIT_COPTER || carried_index == UNIT_TRANSPORT)) {
        return 1;
    }
    return 0;
}
unsigned char maxSteps;
struct Unit * checkU;
struct Tile * tempT;

unsigned char actually_move = 1;
unsigned char mvmtNegFactor;
unsigned char recurs_depth;

unsigned char asm_tx;
unsigned char asm_ty;
unsigned char asm_steps;

struct Unit *asm_tempt_occupying;
unsigned char *asm_temp_t_mvmtcosts;

unsigned char path_array_x[16];
unsigned char path_array_y[16];

extern unsigned char __fastcall__ sabs(unsigned char a, unsigned char b);
extern unsigned char __fastcall__ test_check_space();
extern unsigned char __fastcall__ test_check_unit();

unsigned char checkSpaceInMvmtRange(unsigned char tx, unsigned char ty, unsigned char steps) {
    asm_tx = tx;
    asm_ty = ty;
    asm_steps = steps;

    asm_tx = test_check_space();
    if (asm_tx == 0) { return 0; }
    if (asm_tx == 1) {
        path_array_x[recurs_depth] = tx;
        path_array_y[recurs_depth] = ty;
        return 1;
    }
    tempT = &(m.board[ty * m.boardWidth + tx]);
    asm_tempt_occupying = tempT->occupying;
    asm_temp_t_mvmtcosts = tempT->t->mvmtCosts;

    if (test_check_unit() == 0) { return 0; }
    steps = asm_steps;
    /*
  if (checkU->airborne) {
    ++steps;
  } else {
    if (tempT->occupying != NULL && tempT->occupying->team != checkU->team && actually_move) {
      return 0;
    }
    if (!tempT->t->mvmtCosts[checkU->mvmtType]) {
      return 0;
    }
    steps += tempT->t->mvmtCosts[checkU->mvmtType];
  }
  if (steps > maxSteps) {
    return 0;
  }
    */

    /* recursive calls */
    ++recurs_depth;
    if (SHRTCIRCUIT_AND(tx != 0, checkSpaceInMvmtRange(tx - 1, ty, steps))) {
        --recurs_depth;
        if (actually_move == 0) {
            path_array_x[recurs_depth] = tx;
            path_array_y[recurs_depth] = ty;
        }
        return 1;
    }
    if (SHRTCIRCUIT_AND(ty != 0, checkSpaceInMvmtRange(tx, ty - 1, steps))) {
        --recurs_depth;
        if (actually_move == 0) {
            path_array_x[recurs_depth] = tx;
            path_array_y[recurs_depth] = ty;
        }
        return 1;
    }
    if (checkSpaceInMvmtRange(tx + 1, ty, steps) || checkSpaceInMvmtRange(tx, ty + 1, steps)) {
        --recurs_depth;
        if (actually_move == 0) {
            path_array_x[recurs_depth] = tx;
            path_array_y[recurs_depth] = ty;
        }
        return 1;
    } else {
        --recurs_depth;
        return 0;
    }
}

unsigned char unitLastX = 255;
unsigned char unitLastY = 255;
unsigned char unitLastFuel = 255;
unsigned char baseLastHP = 255;

unsigned char unitChangedPosition;


unsigned char move(struct Unit * u, unsigned char x, unsigned char y) {
    if (!u->takenAction && x < m.boardWidth && y < m.boardHeight) {
        if (m.board[y * m.boardWidth + x].occupying != NULL && m.board[y * m.boardWidth + x].occupying != u) {
            if (m.board[y * m.boardWidth + x].occupying->team != u->team) {
                return 0;
                /* Check if unit can be loaded on another, if yes, continue to main routine */
            } else if (!canCarryUnit(m.board[y * m.boardWidth + x].occupying->index, u->index) || m.board[y * m.boardWidth + x].occupying->carrying != NULL) {
                /* Check if units can be joined together. if yes continue to main part */
                if (m.board[y * m.boardWidth + x].occupying->index != u->index || m.board[y * m.boardWidth + x].occupying->health == 100 || u->health == 100) {
                    // Allows arrow to overlap units on same team
                    if (actually_move) { return 0; }
                }
            }
        }
        /* If not enough fuel, can't move */
        if (u->fuel < SABS(u->x, x) + SABS(u->y, y)) {
            return 0;
        }
        maxSteps = u->mvmtRange;
        if (actually_move == 0) {
            maxSteps -= mvmtNegFactor;
        }
        checkU = u;
        recurs_depth = 0;
        if ((u->airborne) ? (SABS(u->x, x) + SABS(u->y, y) <= maxSteps) : checkSpaceInMvmtRange(x, y, 0)) {
            if (actually_move) {
                checkU = NULL;
                maxSteps = 0;
                unitLastX = u->x;
                unitLastY = u->y;
                unitLastFuel = u->fuel;
                if (m.board[unitLastY * m.boardWidth + unitLastX].base != NULL && (unitLastX != x || unitLastY != y)) {
                    baseLastHP = m.board[unitLastY * m.boardWidth + unitLastX].base->health;
                    m.board[unitLastY * m.boardWidth + unitLastX].base->health = 20;
                }
                if (u->x >= m.left_view && u->x < m.left_view + game_width && u->y >= m.top_view && u->y < m.top_view + game_height) {
                    POKE(0x9F20, (unitLastX - m.left_view) << 1);
                    POKE(0x9F21, 0x40 + unitLastY - m.top_view);
                    POKE(0x9F22, 0x00);
                    POKE(0x9F23, 28);
                }
                m.board[unitLastY * m.boardWidth + unitLastX].occupying = NULL;
                u->fuel -= SABS(u->x, x) + SABS(u->y, y);

                unitChangedPosition = (u->x != x || u->y != y);
                u->x = x;
                u->y = y;
                attackCursor.selected = m.board[y * m.boardWidth + x].occupying;
                m.board[y * m.boardWidth + x].occupying = u;
            } else if (u->airborne) {
                unsigned char i = 0;

                while (x != u->x && y != u->y) {
                    path_array_x[i] = x;
                    path_array_y[i] = y;
                    if (x > u->x) { --x; } else { ++x; }
                    ++i;
                    path_array_x[i] = x;
                    path_array_y[i] = y;
                    if (y > u->y) { --y; } else { ++y; }
                    ++i;
                }
                while (y != u->y) {
                    path_array_x[i] = x;
                    path_array_y[i] = y;
                    if (y > u->y) { --y; } else { ++y; }
                    ++i;
                }
                while (x != u->x) {
                    path_array_x[i] = x;
                    path_array_y[i] = y;
                    if (x > u->x) { --x; } else { ++x; }
                    ++i;
                }
                path_array_x[i] = x;
                path_array_y[i] = y;
            }
            return 1;
        }
    }
    return 0;
}

void undoMove(struct Unit * u) {
    m.board[u->y * m.boardWidth + u->x].occupying = NULL;
    POKE(0x9F20, (u->x - m.left_view) * 2);
    POKE(0x9F21, 0x40 + u->y - m.top_view);
    POKE(0x9F22, 0x00);
    POKE(0x9F23, 28);
    if (attackCursor.selected != NULL && attackCursor.selected->team == u->team) {
        m.board[u->y * m.boardWidth + u->x].occupying = attackCursor.selected;
        attackCursor.selected = NULL;
    }
    u->x = unitLastX;
    u->y = unitLastY;
    u->fuel = unitLastFuel;
    u->takenAction = 0;
    m.board[unitLastY * m.boardWidth + unitLastX].occupying = u;
    if (m.board[unitLastY * m.boardWidth + unitLastX].base != NULL) {
        m.board[unitLastY * m.boardWidth + unitLastX].base->health = baseLastHP;
        baseLastHP = 255;
    }
    unitLastX = 255;
    unitLastY = 255;
    unitLastFuel = 255;
}

extern unsigned char unitIndexes[];
unsigned char damageChart[] = {
/* 							0,    1,    2,    3,    9,   10,   11,   12,   13,   14    16,   17,   18,   19,   24,   25,   26,   27 */
/* 						  0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17 */
/* 0 | 00 */ 0x00, 0x2d, 0x4b, 0x0e, 0x32, 0x69, 0x4b, 0x00, 0x50, 0x46, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x50,
/* 1 | 01 */ 0x00, 0x23, 0x55, 0x0c, 0x3c, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x37, 0x69, 0x00, 0x00, 0x00, 0x00, 0x5a,
/* 2 | 02 */ 0x00, 0x41, 0x37, 0x2d, 0x69, 0x5f, 0x46, 0x00, 0x5a, 0x55, 0x00, 0x4b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x5a,
/* 3 | 03 */ 0x00, 0x46, 0x41, 0x37, 0x69, 0x69, 0x4b, 0x00, 0x5f, 0x5a, 0x00, 0x4b, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x5f,
/* 4 | 09 */ 0x00, 0x04, 0x41, 0x05, 0x2d, 0x69, 0x41, 0x00, 0x55, 0x4b, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x55,
/* 5 | 10 */ 0x00, 0x01, 0x0f, 0x01, 0x0a, 0x37, 0x0f, 0x00, 0x37, 0x2d, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x37,
/* 6 | 11 */ 0x00, 0x06, 0x37, 0x05, 0x19, 0x55, 0x37, 0x00, 0x55, 0x46, 0x00, 0x37, 0x69, 0x00, 0x00, 0x00, 0x00, 0x55,
/* 7 | 12 */ 0x00, 0x1c, 0x55, 0x19, 0x37, 0x69, 0x55, 0x00, 0x5a, 0x50, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x5a,
/* 8 | 13 */ 0x00, 0x37, 0x55, 0x19, 0x2d, 0x69, 0x55, 0x00, 0x55, 0x50, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x55,
/* 9 | 14 */ 0x00, 0x2d, 0x46, 0x0f, 0x32, 0x69, 0x46, 0x00, 0x50, 0x4b, 0x00, 0x41, 0x69, 0x00, 0x00, 0x00, 0x00, 0x50,
/*10 | 16 */ 0x00, 0x23, 0x23, 0x1e, 0x78, 0x2d, 0x28, 0x78, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x64, 0x00, 0x00, 0x73, 0x00,
/*11 | 17 */ 0x00, 0x0a, 0x09, 0x07, 0x78, 0x0c, 0x0a, 0x78, 0x00, 0x00, 0x00, 0x41, 0x00, 0x64, 0x00, 0x00, 0x73, 0x00,
/*12 | 18 */ 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x41, 0x00,
/*13 | 19 */ 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x37, 0x00,
/*14 | 24 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x0a, 0x00, 0x3c, 0x37, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x5f, 0x00, 0x5f,
/*15 | 25 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x55, 0x3c, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x37, 0x5a, 0x5f,
/*16 | 26 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x05, 0x00, 0x55, 0x41, 0x00, 0x37, 0x55, 0x00, 0x00, 0x19, 0x00, 0x5f,
/*17 | 27 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x37, 0x28, 0x00, 0x19, 0x4b, 0x00, 0x00, 0x37, 0x00, 0x32};

unsigned char canAttack(struct Unit * a, struct Unit * b) {
    if (a->team != b->team && !a->takenAction && a->ammo > 0 && SABS(b->x, a->x) + SABS(b->y, a->y) >= a->attackRangeMin && SABS(b->x, a->x) + SABS(b->y, a->y) <= a->attackRangeMax) {
        return damageChart[unitIndexes[b->index] * 18 + unitIndexes[a->index]];
    } else {
        return 0;
    }
}

unsigned char randByte() {
    static unsigned char val;
    __asm__("jsr $FECF"); /* Call entropy_get kernal routine */
    __asm__("stx %v", val);
    __asm__("eor %v", val);
    __asm__("sty %v", val);
    __asm__("eor %v", val);
    __asm__("sta %v", val);
    return val;
}

unsigned char damagePreview(struct Unit * a, struct Unit * b) {
    unsigned char temp = canAttack(a, b) * a->health / 100;
    POKE(0xF, m.board[b->y * m.boardWidth + b->x].t->defense);
    if (!b->airborne) {
        temp -= temp * (b->health * m.board[b->y * m.boardWidth + b->x].t->defense) / 1000;
    }
    return temp;

}

unsigned char calcPower(struct Unit * a, struct Unit * b) {
    unsigned char temp = canAttack(a, b) * a->health / 100 + randByte() % ((a->health + 9) / 10);
    if (!b->airborne) {
        temp -= temp * (b->health * m.board[b->y * m.boardWidth + b->x].t->defense) / 1000;
    }
    return temp;
}

void attack(struct Unit * attacker, struct Unit * defender) {
    defender->health -= calcPower(attacker, defender);
    --attacker->ammo;
    if (defender->health >= 128) {
        defender->health = 0;
    }
    if (defender->health > 0) {
        if (defender->ammo > 0) {
            static unsigned char defender_damage;
            defender_damage = calcPower(defender, attacker);
            /* If defender can't fire back, don't drain ammo */
            if (defender_damage) {
                attacker->health -= defender_damage;
                --defender->ammo;
            }
        }
        if (attacker->health >= 128) {
            attacker->health = 0;
        }
        if (attacker->health == 0) {
            m.board[m.boardWidth * attacker->y + attacker->x].occupying = NULL;
            POKE(0x9F20, (attacker->x - m.left_view) * 2);
            POKE(0x9F21, attacker->y - m.top_view + 0x40);
            __asm__("lda #0");
            __asm__("sta $9F22");
            __asm__("lda #28");
            __asm__("sta $9F23");
            if (attacker->carrying != NULL) {
                free_unit(attacker->carrying);
            } else if (m.board[attacker->y * m.boardWidth + attacker->x].base != NULL) {
                m.board[attacker->y * m.boardWidth + attacker->x].base->health = 20;
            }
            clearUnitFromScreen(attacker->x, attacker->y);
            pcm_trigger_digi(UNIT_EXPLODING_BANK, HIRAM_START);
            renderUnitExplosion(attacker->x, attacker->y, 0);
            free_unit(attacker);
            ++unitsdeadthisturn;
        }
    } else {
        m.board[m.boardWidth * defender->y + defender->x].occupying = NULL;
        POKE(0x9F20, (defender->x - m.left_view) * 2);
        POKE(0x9F21, defender->y - m.top_view + 0x40);
        __asm__("lda #0");
        __asm__("sta $9F22");
        __asm__("lda #28");
        __asm__("sta $9F23");
        ++unitsdeadthisturn;
        if (defender->carrying != NULL) {
            free_unit(defender->carrying);
        } else if (m.board[defender->y * m.boardWidth + defender->x].base != NULL) {
            m.board[defender->y * m.boardWidth + defender->x].base->health = 20;
        }

        clearUnitFromScreen(defender->x, defender->y);
        pcm_trigger_digi(UNIT_EXPLODING_BANK, HIRAM_START);
        renderUnitExplosion(defender->x, defender->y, 0);
        free_unit(defender);
    }
}

void getPossibleAttacks(struct possibleAttacks * pA, unsigned char cx, unsigned char cy, unsigned char attackRangeMax) {
    pA->length = 0;
    //POKE(0x9fbd, 0);
    if (!c.selected->canAttackAndMove && unitChangedPosition) {
        /*POKE(0x9fba, 0); */
        return;
    }

    if (attackRangeMax == 1) {
        /* Handle direct attack units */
        unsigned char i = 0;
        struct Tile * north = NULL;
        struct Tile * east = NULL;
        struct Tile * south = NULL;
        struct Tile * west = NULL;
        if (cy != 0) {
            north = & (m.board[(cy - 1) * m.boardWidth + cx]);
        }
        if (cx != 0) {
            west = & (m.board[cy * m.boardWidth + cx - 1]);
        }
        if (cy < m.boardHeight - 1) {
            south = & (m.board[(cy + 1) * m.boardWidth + cx]);
        }
        if (cx < m.boardWidth - 1) {
            east = & (m.board[cy * m.boardWidth + cx + 1]);
        }

        if (north != NULL && north->occupying != NULL && north->occupying->team != m.whoseTurn && canAttack(c.selected, north->occupying)) {
            pA->attacks[i] = north;
            i++;
        }
        if (east != NULL && east->occupying != NULL && east->occupying->team != m.whoseTurn && canAttack(c.selected, east->occupying)) {
            pA->attacks[i] = east;
            i++;
        }
        if (south != NULL && south->occupying != NULL && south->occupying->team != m.whoseTurn && canAttack(c.selected, south->occupying)) {
            pA->attacks[i] = south;
            i++;
        }
        if (west != NULL && west->occupying != NULL && west->occupying->team != m.whoseTurn && canAttack(c.selected, west->occupying)) {
            pA->attacks[i] = west;
            i++;
        }
        pA->length = i;
    } else {
        /* Handle units with ranged attacks */
        unsigned char xmin, ymin, xmax, ymax;
        unsigned char x, y;
        struct Tile * temp;
        unsigned char i;

        xmin = (c.x >= attackRangeMax) ? c.x - attackRangeMax : 0;
        ymin = (c.y >= attackRangeMax) ? c.y - attackRangeMax : 0;
        xmax = (c.x + attackRangeMax < m.boardWidth) ? c.x + attackRangeMax : m.boardWidth - 1;
        ymax = (c.y + attackRangeMax < m.boardHeight) ? c.y + attackRangeMax : m.boardHeight - 1;

        i = 0;
        for (y = ymin; y <= ymax; ++y) {
            for (x = xmin; x <= xmax; ++x) {
                POKE(0x9fbb, x);
                POKE(0x9fbc, y);
                temp = & (m.board[y * m.boardWidth + x]);
                if (temp->occupying != NULL) {
                    if (canAttack(c.selected, temp->occupying)) {

                        pA->attacks[i] = temp;
                        ++i;
                    }
                }
            }
        }
        pA->length = i;
    }
}

void getPossibleDrops(struct possibleAttacks * pA, struct Unit * u) {
    static struct Unit * carry;
    static struct Tile * tile;
    static unsigned char ux, uy;

    carry = u->carrying;

    ux = u->x;
    uy = u->y;

    pA->attacks[0] = NULL;
    pA->attacks[1] = NULL;
    pA->attacks[2] = NULL;
    pA->attacks[3] = NULL;
    pA->length = 0;
    if (ux != 0) {
        tile = & (m.board[ux - 1 + m.boardWidth * uy]);
        if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {
            pA->attacks[DROP_WEST] = tile;
            ++pA->length;
        }
    }
    if (uy != 0) {
        tile = & (m.board[u->x + m.boardWidth * (uy - 1)]);
        if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {
            pA->attacks[DROP_NORTH] = tile;
            ++pA->length;
        }
    }
    if (ux < m.boardWidth - 1) {
        tile = & (m.board[u->x + 1 + m.boardWidth * uy]);
        if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {
            pA->attacks[DROP_EAST] = tile;
            ++pA->length;
        }
    }
    if (u->y < m.boardHeight - 1) {
        tile = & (m.board[ux + m.boardWidth * (uy + 1)]);
        if (tile->t->mvmtCosts[carry->mvmtType] != 0 && tile->occupying == NULL) {
            pA->attacks[DROP_SOUTH] = tile;
            ++pA->length;
        }
    }
}

unsigned char sizeofGetPossibleDrops(struct Unit * u) {
    unsigned char size;
    struct possibleAttacks *pA = &useaspossibleAttacks;

    getPossibleDrops(pA, u);
    size = pA->length;
    return size;
}

void getPossibleJoins(struct possibleAttacks * pA, struct Unit * u) {
    unsigned char i = 0;
    struct Tile * tile;

    if (u->x != 0) {
        tile = & (m.board[u->x - 1 + m.boardWidth * u->y]);
        if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
            pA->attacks[i] = tile;
            i++;
        }
    }
    if (u->y != 0) {
        tile = & (m.board[u->x + m.boardWidth * (u->y - 1)]);
        if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
            pA->attacks[i] = tile;
            i++;
        }
    }
    if (u->x < m.boardWidth - 1) {
        tile = & (m.board[u->x + 1 + m.boardWidth * u->y]);
        if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
            pA->attacks[i] = tile;
            i++;
        }
    }
    if (u->y < m.boardHeight - 1) {
        tile = & (m.board[u->x + m.boardWidth * (u->y + 1)]);
        if (tile->occupying != NULL && tile->occupying->team == m.whoseTurn && tile->occupying->index == u->index) {
            pA->attacks[i] = tile;
            i++;
        }
    }

    pA->length = i;
    return;
}

unsigned char sizeofGetPossibleJoins(struct Unit * u) {
    unsigned char size;
    struct possibleAttacks *pA = &useaspossibleAttacks;

    getPossibleJoins(pA, u);
    size = pA->length;
    return size;
}

unsigned char canSupply(struct Unit * u) {
    struct Unit * supp;
    if (u->index != UNIT_APC) {
        return 0;
    }

    if (u->x > 0) {
        supp = m.board[u->y * m.boardWidth + u->x - 1].occupying;
        if (supp != NULL && supp->team == u->team) {
            return 1;
        }
    }
    if (u->x < m.boardWidth - 1) {
        supp = m.board[u->y * m.boardWidth + u->x + 1].occupying;
        if (supp != NULL && supp->team == u->team) {
            return 1;
        }
    }
    if (u->y > 0) {
        supp = m.board[u->y * m.boardWidth + u->x - m.boardWidth].occupying;
        if (supp != NULL && supp->team == u->team) {
            return 1;
        }
    }
    if (u->y < m.boardHeight - 1) {
        supp = m.board[u->y * m.boardWidth + u->x + m.boardWidth].occupying;
        if (supp != NULL && supp->team == u->team) {
            return 1;
        }
    }
    return 0;
}

void supplyUnits(struct Unit * u) {
    struct Unit * supp;

    if (u->x > 0) {
        supp = m.board[u->y * m.boardWidth + u->x - 1].occupying;
        if (supp != NULL && supp->team == u->team) {
            supp->ammo = 10;
            supp->fuel = maxFuel[u->index];
        }
    }
    if (u->x < m.boardWidth - 1) {
        supp = m.board[u->y * m.boardWidth + u->x + 1].occupying;
        if (supp != NULL && supp->team == u->team) {
            supp->ammo = 10;
            supp->fuel = maxFuel[u->index];
        }
    }
    if (u->y > 0) {
        supp = m.board[u->y * m.boardWidth + u->x - m.boardWidth].occupying;
        if (supp != NULL && supp->team == u->team) {
            supp->ammo = 10;
            supp->fuel = maxFuel[u->index];
        }
    }
    if (u->y < m.boardHeight - 1) {
        supp = m.board[u->y * m.boardWidth + u->x + m.boardWidth].occupying;
        if (supp != NULL && supp->team == u->team) {
            supp->ammo = 10;
            supp->fuel = maxFuel[u->index];
        }
    }
}