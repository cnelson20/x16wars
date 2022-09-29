.setcpu "65c02"

.import _m
.import _player1team
.import _player2team

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

.import pushax, mulax6, tosmulax, tosaddax
.importzp ptr1, ptr2, ptr3
.importzp tmp1, tmp2

;
; void render_tiles();
;
.export _render_tiles
_render_tiles:
@x = $10
@y = $11
@i = $12

stz $9F20
stz $9F21
stz @x
stz @y

lda #$10
sta $9F22

ldx #0
lda _m + Map::left_view
jsr pushax

ldx #0
lda _m + Map::top_view
jsr pushax

ldx #0
lda _m + Map::boardWidth
jsr tosmulax ; multiply top_view * board_width
jsr tosaddax

sta @i
stx @i + 1

@draw_loop:
lda @y
cmp #10
bne :+
jmp @end
:

lda @i
ldx @i + 1
jsr mulax6
clc 
adc _m + Map::board
sta ptr1
txa 
adc _m + Map::board + 1
sta ptr1 + 1

ldy #Tile::t
lda (ptr1), Y
sta ptr2
iny
lda (ptr1), Y
sta ptr2 + 1 ; load terrain pointer into ptr2

ldy #Terrain::tileIndex
lda (ptr2), Y
sta $9F23
ldy #Terrain::paletteOffset
lda (ptr2), Y
sta $9F23

ldy #Tile::occupying + 1
lda (ptr1), Y
tax
ldy #Tile::occupying
lda (ptr1), Y

bne @render_unit ; if low byte != 0 render unit 
cpx #0 
beq @past_render_unit ; if high byte == 0, pointer == NULL --> dont render
@render_unit: 
jsr _renderUnit

lda @x
inc A
asl A
sta $9F20
lda @y
sta $9F21
@past_render_unit:

lda @x
inc A 
sta @x
cmp #15
bcc @dont_inc_y

clc ; Add map boardWidth to i, then subtract 15
lda @i
adc _m + Map::boardWidth
sta @i
lda @i + 1
adc #0
sta @i + 1
sec 
lda @i
sbc #15
sta @i 
lda @i + 1
sbc #0
sta @i + 1

inc @y
inc $9F21
stz @x
stz $9F20

@dont_inc_y:

inc @i 
bne :+
inc @i + 1
:

jmp @draw_loop
@end:
rts 

;
; void __fastcall__ removeRenderUnit(struct Unit *u);
;
.export _removeRenderUnit
_removeRenderUnit:
sta ptr1 
stx ptr1 + 1 ; store unit pointer to ptr1

lda #1
jsr set_vera_index
lda #28
sta $9F23
@dont_remove:
rts

; unit pointer in ptr1
; A = 1 --> oldleft_view / oldtop_view
; A = 0 --> left_view / top_view
set_vera_index:
cmp #0
bne :+
; use left_view and top_view A== 0
lda _m + Map::left_view
sta tmp1
lda _m + Map::top_view
sta tmp2
bra :++
: ; use oldleft_view and oldtop_view A == 1
lda _m + Map::oldleft_view
sta tmp1
lda _m + Map::oldtop_view
sta tmp2
:
lda #$10
sta $9F22

ldy #Unit::xx 
lda (ptr1), Y
sec
sbc tmp1 ; either oldleft or left view 
asl 
sta $9F20
ldy #Unit::yy
lda (ptr1), Y
sec
sbc tmp2 ; either oldtop or top view 
adc #$3F ; carry is set, can add AMNT - 1 to save cycle 
sta $9F21
rts 

;
; void __fastcall__ renderUnit(struct Unit *u);
;
.export _renderUnit
_renderUnit:
sta ptr1 
stx ptr1 + 1

lda #0
jsr set_vera_index

ldy #Unit::team
lda (ptr1), Y
sta tmp2
asl
asl
asl
asl
sta tmp1 ; team << 4
asl
ldy #Unit::index
adc (ptr1), Y ; carry is clear from shifts
sta $9F23 ; tile index 

ldy #Unit::takenAction
lda (ptr1), Y
beq :+
lda #$90
:
clc
adc tmp1 ; team << 4
sta tmp1 ; (takenAction ? $90 : $0) + team << 4

lda tmp2 ; team
sec 
sbc _player1team 
beq :+
lda #4 ; reverse sprite horizontally
:
ora tmp1
sta $9F23 ; palette offset | reverse 
rts 
	
; unit pointer in ptr2 
; x offset in tmp1
; y offset in tmp2
store_x_y_sprite_offsets:
@x = $10
@y = $11
@temp = $14

lda @x
stz @temp
asl A
rol @temp
asl A
rol @temp
asl A
rol @temp
asl A
rol @temp
clc 
adc tmp1
sta $9F23
lda @temp
adc #0
sta $9F23

lda @y
stz @temp
asl A
rol @temp
asl A
rol @temp
asl A
rol @temp
asl A
rol @temp
clc 
adc tmp2
sta $9F23
lda @temp
adc #0
sta $9F23
rts 
	
	


	
.import _oldunitsprites
.import _currentunitsprites

.import _captureablePaletteOffsets
.import _captureableSpriteOffsets

; 
; void __fastcall__ render_unit_sprites();
;
.export _render_unit_sprites
_render_unit_sprites:
@x = $10
@y = $11
@i = $12
lda _currentunitsprites
sta _oldunitsprites
stz _currentunitsprites
	
stz @x
stz @y	
	
lda #$A0
sta $9F20 
lda #$FC 
sta $9F21
lda #$11
sta $9F22 ; set vera to beginning of sprite table 
	
ldx #0
lda _m + Map::left_view
jsr pushax

ldx #0
lda _m + Map::top_view
jsr pushax

ldx #0
lda _m + Map::boardWidth
jsr tosmulax ; multiply top_view * board_width
jsr tosaddax

sta @i
stx @i + 1 ; init I with (top_view * board_width) + left_view
	
;lda @i
;ldx @i + 1
jsr mulax6
clc 
adc _m + Map::board
sta ptr1
txa 
adc _m + Map::board + 1
sta ptr1 + 1	
	
@check_loop:
lda @i + 1 ; check high byte
cmp _m + Map::boardArea + 1
bcc @draw_loop
beq :+
jmp @end ; if hi byte >, then goto end of routine
:
; check low byte 
lda @i
cmp _m + Map::boardArea
bcc @draw_loop ; if < go into loop
jmp @end ; else exit to end of routine 
@draw_loop:
; load unit pointer into ptr2 , if null continue ;
ldy #Tile::occupying
lda (ptr1), Y
sta ptr2
iny 
lda (ptr1), Y
sta ptr2 + 1
bne @unit_pointer_not_null
lda ptr2
bne @unit_pointer_not_null
@check_offscreen_jmp_inc_vars:
jmp @no_unit_present
@unit_pointer_not_null:

@check_offscreen:
; if unit off bottom of screen continue ;
;ldy #Unit::yy
;lda (ptr2), Y
lda @y
sec 
sbc _m + Map::top_view
cmp #10
bcs @check_offscreen_jmp_inc_vars

ldy #Unit::carrying
lda (ptr2), Y
bne @display_load_sprite
iny 
lda (ptr2), Y
bne @display_load_sprite
jmp @end_display_load_sprite
@display_load_sprite:
inc _currentunitsprites
lda #16
sta $9F23
lda #8
sta $9F23

stz tmp1
lda #8
sta tmp2
jsr store_x_y_sprite_offsets

lda #$0C 
sta $9F23

ldy #Unit::takenAction
lda (ptr2), Y
beq :+
lda #9
:
ldy #Unit::team
clc 
adc (ptr2), Y
sta $9F23

@end_display_load_sprite:
ldy #Tile::base
lda (ptr1), Y
sta ptr3
tax 
iny 
lda (ptr1), Y
sta ptr3 + 1
bne @display_capture_sprite
cpx #0
bne @display_capture_sprite
jmp @end_display_capture_sprite
@display_capture_sprite:
ldy #Captureable::health
lda (ptr3), Y
cmp #20
bcs @end_display_capture_sprite

inc _currentunitsprites
lda #17
sta $9F23
lda #8
sta $9F23
stz tmp1
lda #8
sta tmp2
jsr store_x_y_sprite_offsets

lda #$0C 
sta $9F23

ldy #Unit::takenAction
lda (ptr2), Y
beq :+
lda #9
:
ldy #Unit::team
clc 
adc (ptr2), Y
sta $9F23

@end_display_capture_sprite:

ldy #Unit::health
lda (ptr2), Y
cmp #91
bcs @end_display_health_sprites
; display health sprites ;
inc _currentunitsprites

; add 9, then divide unit health by 10 ; (89 -> 9, 80 -> 8, etc.)
clc 
adc #9
ldy #0
:
sec 
sbc #10
bcc :+
iny
bra :-
:
;clc 
; add 6 to get correct sprite offset
tya
adc #6
sta $9F23 

lda #8
sta $9F23

lda #8
sta tmp1 
sta tmp2
jsr store_x_y_sprite_offsets

lda #$0C
sta $9F23
lda #8
sta $9F23

@end_display_health_sprites:
@no_unit_present:

ldy #Tile::base
lda (ptr1), Y
sta ptr2 
tax 
iny 
lda (ptr1), Y
sta ptr2 + 1

bne @render_base_sprite
cpx #0
bne @render_base_sprite
jmp @base_is_null

@render_base_sprite:
inc _currentunitsprites
nop

ldy #Captureable::team
lda (ptr2), Y
pha
asl 
asl ; * 4
ldy #Captureable::type
ora (ptr2), Y
tax 
lda _captureableSpriteOffsets, X

; if y == 0, add 4 to point to lower half of sprite ;
ldy @y
cpy #0
bne :+
clc 
adc #4
:

sta $9F23
lda #8
sta $9F23

stz tmp1 
stz tmp2 
lda @y
beq :+

dec @y 
jsr store_x_y_sprite_offsets
inc @y
bra :++
:
jsr store_x_y_sprite_offsets
:
lda #$8 ; z-depth
sta $9F23 

plx 
lda _captureablePaletteOffsets, X
ldy @y
beq :+
cpy #10
beq :+

ora #$90 ; if y != 0 use 32 px tall sprite 
bra :++
:
ora #$50 ; if y = 0 use 16 px tall sprite
:
sta $9F23

@base_is_null:

@inc_vars:
inc @i
bne :+
inc @i + 1
:

clc
lda ptr1
adc #6
sta ptr1 
lda ptr1 + 1
adc #0
sta ptr1 + 1

lda @x
inc A 
sta @x
cmp #15
bcc @dont_inc_y

clc ; Add map boardWidth to i, then subtract 15
lda @i
adc _m + Map::boardWidth
sta @i
lda @i + 1
adc #0
sta @i + 1
sec 
lda @i
sbc #15
sta @i 
lda @i + 1
sbc #0
sta @i + 1

lda @y
inc A
sta @y
cmp #11
bcs @end

stz @x

@recalc_pointer:
lda @i
ldx @i + 1
jsr mulax6
clc 
adc _m + Map::board
sta ptr1
txa 
adc _m + Map::board + 1
sta ptr1 + 1
@dont_inc_y:

jmp @check_loop
@end:
rts
	
.import _win
;
; void __fastcall__ checkOldUnits();
;
.export _checkOldUnits
_checkOldUnits:
@i = $10 ; word size 
@remove_render_flag = $12
ldx _player1team
stz @units_exist_array, X
ldx _player2team
stz @units_exist_array, X
	
lda #0
ldx _m + Map::left_view
cpx _m + Map::oldleft_view
beq :+
lda #1
bra :+
:
ldx _m + Map::top_view
cpx _m + Map::oldtop_view
beq :+
lda #1
:
sta @remove_render_flag

lda _m + Map::board
sta ptr1
lda _m + Map::board + 1
sta ptr1 + 1
	
@main_loop:	
lda @i + 1
cmp _m + Map::boardArea + 1
bcc @body_loop

lda @i
cmp _m + Map::boardArea
bcs @end_loop
@body_loop:

ldy #Tile::occupying
lda (ptr1), Y
sta ptr2
iny
lda (ptr1), Y
sta ptr2 + 1
bne @do_unit_stuff
lda ptr2
beq @tile_occupying_null
@do_unit_stuff:

; set unit team array ; 
ldy #Unit::team
lda (ptr2), Y
tax 
lda #1
sta @units_exist_array, X 

lda ptr1
pha
lda ptr1 + 1
pha 

lda @remove_render_flag
beq :+
lda ptr2
ldx ptr2 + 1
jsr _removeRenderUnit
:

pla 
sta ptr1 + 1
pla 
sta ptr1


@tile_occupying_null:
inc @i
bne :+
inc @i + 1
:

clc
lda ptr1
adc #6
sta ptr1 
lda ptr1 + 1
adc #0
sta ptr1 + 1

jmp @main_loop

@end_loop:

ldx _player1team
lda @units_exist_array
bne :+
lda _player2team ; If player 1 has no units, player 2 wins 
jmp _win ; when win() returns, it will return to caller of this function
:
ldx _player2team
lda @units_exist_array
bne :+
lda _player1team ; vice versa from above 
jmp _win
:
rts
	
@units_exist_array:
	.res 4, 0