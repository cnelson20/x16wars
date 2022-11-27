.setcpu "65c02"

.import _m
.import _player1team
.import _player2team
.import _game_width
.import _game_height

.include "structs.inc"

.import pushax, mulax6, tosmulax, tosaddax, popa
.importzp ptr1, ptr2, ptr3, ptr4
.importzp tmp1, tmp2, tmp3, tmp4

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
jsr mulax6

clc
adc _m + Map::board
sta ptr4
txa 
adc _m + Map::board + 1
sta ptr4 + 1

@draw_loop:
lda @y
cmp _game_height
bne :+
jmp @end
:

ldy #Tile::t
lda (ptr4), Y
sta ptr2
iny
lda (ptr4), Y
sta ptr2 + 1 ; load terrain pointer into ptr2

ldy #Terrain::tileIndex
lda (ptr2), Y
sta $9F23
ldy #Terrain::paletteOffset
lda (ptr2), Y
sta $9F23

ldy #Tile::occupying + 1
lda (ptr4), Y
tax
ldy #Tile::occupying
lda (ptr4), Y

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
cmp _game_width
bcc @dont_inc_y

lda _m + Map::boardWidth
sec 
sbc _game_width
inc A ; Increment A here to add the extra 6 we would anyway for the next iteration

asl A ; Multiply by 6
sta tmp1 
asl A 
clc 
adc tmp1

adc ptr4 
sta ptr4 
lda ptr4 + 1
adc #0
sta ptr4 + 1

inc @y
inc $9F21
stz @x
stz $9F20

jmp @draw_loop
@dont_inc_y:
lda ptr4 
clc 
adc #6
sta ptr4 
lda ptr4 + 1
adc #0
sta ptr4 + 1

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
sta $9F22 ; set vera to beginning of unit & captureable sprite table 
	
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
cmp _game_height
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
asl
asl ; * 8
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
cpy _game_height
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
cmp _game_width
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
sbc _game_width
sta @i 
lda @i + 1
sbc #0
sta @i + 1

lda @y
inc A
sta @y
dec A ; sub 1
cmp _game_height
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
.import _playerUnitCounts
.import _playerFactoryCounts
;
; void __fastcall__ checkOldUnits();
;
.export _checkOldUnits
_checkOldUnits:
@i = $10 ; word size 
	
lda #0
ldx _m + Map::left_view
cpx _m + Map::oldleft_view
bne :+
ldx _m + Map::top_view
cpx _m + Map::oldtop_view
bne :+
jmp @end_loop
:

lda _m + Map::board
sta ptr1
lda _m + Map::board + 1
sta ptr1 + 1

stz @i
stz @i + 1	
	
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
lda ptr1
sta ptr3
lda ptr1 + 1
sta ptr3 + 1

lda ptr2
ldx ptr2 + 1
jsr _removeRenderUnit

lda ptr3 + 1
sta ptr1 + 1
lda ptr3
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
lda _playerUnitCounts, X
bne :+
lda _playerFactoryCounts, X
bne :+
lda _player2team ; If player 1 has no units, player 2 wins 
;jmp _win ; when win() returns, it will return to caller of this function
:
ldx _player2team
lda _playerUnitCounts, X
bne :+
lda _playerFactoryCounts, X
bne :+
lda _player1team ; vice versa from above 
;jmp _win
:
rts

.import _checkU
.import _maxSteps
.import _tempT

.import _actually_move
.import _mvmtNegFactor
.import _recurs_depth

.import _path_array_x
.import _path_array_y

.import _asm_tx
.import _asm_ty
.import _asm_steps

_sabs_use_ax:
	stx _sabs_temp
	jmp _sabs_entry

.export _test_check_space
_test_check_space:
	lda _asm_tx
	cmp _m + Map::boardWidth
	bcs check_return_0
	lda _asm_ty
	cmp _m + Map::boardHeight
	bcs check_return_0
	
	lda _checkU
	sta ptr1 
	lda _checkU + 1
	sta ptr1 + 1
	
	ldy #Unit::xx
	lda (ptr1), Y
	sta tmp1
	cmp _asm_tx 
	bne :+
	ldy #Unit::yy
	lda (ptr1), Y
	cmp _asm_ty
	beq check_return_1
	:
	
	ldy #Unit::xx
	lda (ptr1), Y
	ldx _asm_tx
	jsr _sabs_use_ax
	sta tmp2 
	
	ldy #Unit::yy
	lda (ptr1), Y
	ldx _asm_ty
	jsr _sabs_use_ax
	clc 
	adc tmp2
	adc _asm_steps
	cmp _maxSteps 
	beq :+
	bcs check_return_0
	:
	
	
	lda #$FF
	rts
check_return_0:
	lda #0
	rts
check_return_1:
	lda #1
	rts


.import _asm_tempt_occupying
.import _asm_temp_t_mvmtcosts
;
; void test_check_unit(); 
;
.export _test_check_unit
_test_check_unit:
	lda _checkU
	sta ptr1 
	lda _checkU + 1
	sta ptr1 + 1
	ldy #Unit::airborne
	lda (ptr1), Y
	bne @unit_is_airborne
	
	lda _asm_tempt_occupying
	sta ptr2 
	bne :+
	lda _asm_tempt_occupying + 1
	beq @past_overlap_check	
	:
	
	lda _asm_tempt_occupying + 1
	sta ptr2 + 1
	ldy #Unit::team 
	lda (ptr1), Y
	cmp (ptr2), Y
	beq :+
	lda _actually_move
	beq :+
	jmp check_return_0
	:
@past_overlap_check:
	lda _asm_temp_t_mvmtcosts
	sta ptr3 
	lda _asm_temp_t_mvmtcosts + 1
	sta ptr3 + 1
	
	ldy #Unit::mvmtType
	lda (ptr1), Y
	tay 
	lda (ptr3), Y
	bne :+
	jmp check_return_0
	:
	
	clc 
	adc _asm_steps
	sta _asm_steps

	bra @past_increments
@unit_is_airborne:
	inc _asm_steps
@past_increments:
	lda _asm_steps
	dec A 
	cmp _maxSteps
	bcs check_return_0
	
	jmp check_return_1	
	
;
; unsigned char __fastcall__ sabs(unsigned char a, unsigned char b);
;
.export _sabs
_sabs:
	sta _sabs_temp
	jsr popa
_sabs_entry:
	cmp _sabs_temp
	bcc :+
	sec 
	sbc _sabs_temp
	rts
	:
	ldx _sabs_temp
	sta _sabs_temp
	txa
	sec 
	sbc _sabs_temp
	rts
	
_sabs_temp:
	.byte 0

.import _joystick_num
;
; void setup_joystick();
;
.export _setup_joystick
_setup_joystick:
	lda #4
	sta tmp1
	:
	lda tmp1 
	jsr $FF56 ; joystick_get
	cpy #0
	beq :+
	ldy tmp1
	dey 
	sty tmp1
	bne :-
	:
	sty _joystick_num
	rts 

.import _keyCode

;
; void handle_joystick();
;	
.export _handle_joystick
_handle_joystick:
	lda @holder 
	beq :+
	dec @holder 
	rts 
	:
	lda #@amnt
	sta @holder

	lda _joystick_num
	jsr $FF56
	
	sta tmp1
	stx tmp2
	;sta $04
	;stx $05
	
	and #%10000000
	beq :+
	stz @b_was_pressed
	:
	txa
	and #%10000000
	beq :+
	stz @a_was_pressed
	:
	
	lda tmp1
	and #1
	bne :+
	lda #'d'
	sta _keyCode
	rts 
	:
	lda tmp1 
	and #2
	bne :+
	lda #'a'
	sta _keyCode
	rts 
	:
	lda tmp1 
	and #4
	bne :+
	lda #'s'
	sta _keyCode
	rts 
	:
	lda tmp1 
	and #8
	bne :+
	lda #'w'
	sta _keyCode
	rts 
	:
	
	lda tmp1
	and #128
	bne :+
	lda @b_was_pressed
	bne :+
	lda #'u'
	sta _keyCode
	sta @b_was_pressed
	rts
	:
	
	lda tmp2 
	and #128
	bne :+
	lda @a_was_pressed
	bne :+
	lda #'i'
	sta _keyCode
	sta @a_was_pressed
	rts 
	:
	
	rts
@holder:
	.byte 0
@amnt = 4
@a_was_pressed:
	.byte 0
@b_was_pressed:
	.byte 0

;
; void reset_quit();
;
.export _reset_quit
_reset_quit:
	lda #$80
	sta $9F25
	jmp ($FFFC)
	
;
; void cbm_k_chrin();
;
.export _cbm_k_chrin
_cbm_k_chrin := $FFE4

