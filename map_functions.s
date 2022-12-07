.setcpu "65c02"

.importzp tmp1, tmp2, tmp3, tmp4
.importzp ptr1, ptr2, ptr3, ptr4

.include "structs.inc"

;
; Struct SIZEOF defines
;
SIZEOF_UNIT = .sizeof (Unit)
SIZEOF_CAPTUREABLE = .sizeof (Captureable)
SIZEOF_TERRAIN = .sizeof (Terrain)
SIZEOF_TILE = .sizeof (Tile)
SIZEOF_MAP = .sizeof (Map)
SIZEOF_CURSOR = .sizeof (Cursor)
SIZEOF_POSSIBLEATTACKS = .sizeof (possibleAttacks)
SIZEOF_MENU = .sizeof (Menu)

GAME_MAX_UNITS = 100
GAME_MAX_BASES = 64
LEN_TERRAIN_ARRAY = 16

;
; Global variable declaration in golden RAM
;
.export _unitArrayUses
_unitArrayUses := $400

.export _useaspossibleAttacks
_useaspossibleAttacks := _unitArrayUses + GAME_MAX_UNITS

.export _pA
_pA := _useaspossibleAttacks + SIZEOF_POSSIBLEATTACKS

.export _menuOptions
_menuOptions := _pA + 2

.export _m
_m := _menuOptions + SIZEOF_MENU

.export _c
_c := _m + SIZEOF_MAP

.export _attackCursor
_attackCursor := _c + SIZEOF_CURSOR

.export _path_array_x
_path_array_x := _attackCursor + SIZEOF_CURSOR

.export _path_array_y
_path_array_y := _path_array_x + 12

.export _terrainIsSet
_terrainIsSet := _path_array_y + 12

.export _terrainArray
_terrainArray := _terrainIsSet + LEN_TERRAIN_ARRAY

.export _captureableArray
_captureableArray := _terrainArray + (SIZEOF_TERRAIN * LEN_TERRAIN_ARRAY)

.export _captureableArrayUses
_captureableArrayUses := _captureableArray + (SIZEOF_CAPTUREABLE * GAME_MAX_BASES)


.assert (_captureableArrayUses + GAME_MAX_BASES) <= $07ff, error, "Overflow of golden RAM"

;
; unsigned char vera_scroll_array[][2];
;
_vera_scroll_array:
    .byte 0, 64
    .byte 14, 85
    .byte 23, 128

VERA_SCROLL_ARRAY_LEN = 3

;.import _m

.import _screen_width
.import _screen_height
.import _game_width
.import _game_height
.import _gui_vera_offset

;
; void mapData_setScreenRegisters();
;
.export _mapData_setScreenRegisters
_mapData_setScreenRegisters:
    lda #15
    sta tmp1
    lda #10
    sta tmp2

    ldy #1
@array_loop:
    tya
    asl A
    tax
    lda _vera_scroll_array, X
    cmp _m + Map::boardWidth
    bcs @end_array_loop
    cmp _m + Map::boardHeight
    bcs @end_array_loop

    sta tmp2
    sta tmp1
    asl A
    clc
    adc tmp1
    lsr A
    sta tmp1

    iny
    cpy #VERA_SCROLL_ARRAY_LEN
    bcc @array_loop
@end_array_loop:
    dey

    tya
    asl
    inc A
    tax
    lda _vera_scroll_array, X
    sta $9F2A
    sta $9F2B

    lda tmp1
    clc
    adc #6
    sta _screen_width
    lda tmp2
    clc
    adc #6
    sta _screen_height

    lda tmp1
    cmp _m + Map::boardWidth
    bcc :+
    lda _m + Map::boardWidth
    :
    sta _game_width
    lda tmp2
    cmp _m + Map::boardHeight
    bcc :+
    lda _m + Map::boardHeight
    :
    sta _game_height
    ora #$40
    sta _gui_vera_offset

    rts

.import popax
.import mulax6

;
; void initTile(struct Tile *t, unsigned char index);
;

.import _terrainPaletteOffsetArray
.import _terrainDefenseArray
.import _terrainMvmtCostsArray
.import _mvmtCostArrayIndex
;
; void initTerrain(struct Terrain **t_pointer, unsigned char index);
;
.export _initTerrain
_initTerrain:
    tax
    sta tmp1
    lda _terrainIsSet, X
    sta tmp2

    jsr popax

    sta ptr1
    stx ptr1 + 1

    lda tmp1
    asl
    asl
    clc
    adc tmp1
    clc
    adc #<_terrainArray
    ldy #0
    sta (ptr1), Y
    sta ptr2
    lda #>_terrainArray
    adc #0
    iny
    sta (ptr1), Y
    sta ptr2 + 1

    ldx tmp2
    beq @terrain_not_set
    rts

@terrain_not_set:
    lda tmp1
    tax

    ora #$80
    ldy #Terrain::tileIndex
    sta (ptr2), Y ; t->tileIndex = index + 0x80;

    lda #1
    sta _terrainIsSet, X ; terrainIsSet[index] = 1;

    lda _terrainPaletteOffsetArray, X
    ldy #Terrain::paletteOffset
    sta (ptr2), Y ; t->paletteOffset = terrainPaletteOffsetArray[index];

    lda _terrainDefenseArray, X
    ldy #Terrain::defense
    sta (ptr2), Y ; t->defense = terrainDefenseArray[index];

    lda _mvmtCostArrayIndex, X
    asl
    sta tmp1
    asl
    clc
    adc tmp1
    clc
    adc #<_terrainMvmtCostsArray
    ldy #Terrain::mvmtCosts
    sta (ptr2), Y
    lda #>_terrainMvmtCostsArray
    adc #0
    iny
    sta (ptr2), Y

    rts

.import _playerFactoryCounts
.import _win
;
; void capture(struct Unit *u, struct Captureable *c);
;
.export _capture
_capture:
    sta ptr1
    stx ptr1 + 1 ; captureable is in ptr1

    jsr popax
    sta ptr2
    stx ptr2 + 1 ; unit is in ptr2

    ldy #Unit::team
    lda (ptr2), Y
    ldy #Captureable::team
    cmp (ptr1), Y
    bne @different_teams
    rts ; if teams are same, can't capture

@different_teams:
    ldy #Unit::health
    lda (ptr2), Y
    ; this calculates (unit health + 9) / 10
    clc
    adc #9
    sec
    ldx #0
    :
    inx
    sbc #10
    bcs :-
    dex
    txa ; move result into a
    ldy #Captureable::health
    cmp (ptr1), Y ; compare unit capture power to building health
    bcs @capture_building ; if enough to capture, branch
    sta tmp1 ; stash
    lda (ptr1), Y ; load building health
    sec
    sbc tmp1 ; subtract unit capture power
    sta (ptr1), Y ; store new health
    rts ; return
@capture_building:
    ldy #Captureable::type
    lda (ptr1), Y
    cmp #CAPTUREABLE_FACTORY ; can building produce units ?
    bcc @not_factory ; if not, branch

    ldx _m + Map::whoseTurn
    inc _playerFactoryCounts, X ; capturer's factory count increases by 1
    ldy #Captureable::team
    lda (ptr1), Y
    tax
    dec _playerFactoryCounts, X ; former owner's factory count decreases by 1

@not_factory:
    ldy #Captureable::critical
    lda (ptr1), Y ; is base the hq ?
    beq @not_hq ; if not, branch

    lda _m + Map::whoseTurn ; load winning player
    jmp _win ; jump to win routine, let it return to our caller
@not_hq:

    lda _m + Map::whoseTurn ; player whose turn it is always is controlling these units
    ldy #Captureable::team
    sta (ptr1), Y ; update team

    ldy #Captureable::health
    lda #20
    sta (ptr1), Y ; set health to 20

    rts