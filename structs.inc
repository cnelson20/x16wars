.struct Map
	top_view .byte
	left_view .byte
	oldtop_view .byte
	oldleft_view .byte
	store_top_view .byte
	store_left_view .byte

	whoseTurn .byte
	boardWidth .byte
	boardHeight .byte
	boardArea .word
	board .word
.endstruct

.struct Tile
	occupying .word
	t .word
	base .word
.endstruct

.struct Terrain
	tileIndex .byte
	paletteOffset .byte
	defense .byte
	mvmtCosts .word
.endstruct

.struct Unit
	index .byte
	health .byte
	ammo .byte
	fuel .byte
	xx .byte
	yy .byte

	mvmtRange .byte
	mvmtType .byte
	attackRangeMin .byte
	attackRangeMax .byte

	team .byte
	canAttackAndMove .byte
	takenAction .byte
	airborne .byte
	isVehicle .byte
	navalOnly .byte

	carrying .word
.endstruct

.struct Captureable
	team .byte
	type .byte
	health .byte
	critical .byte
.endstruct

CAPTUREABLE_CITY = 0
CAPTUREABLE_HQ = 1
CAPTUREABLE_FACTORY = 2
CAPTUREABLE_AIRPORT = 3
CAPTUREABLE_PORT = 4