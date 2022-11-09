.setcpu "65c02"

.importzp tmp1, tmp2

;
; unsigned char vera_scroll_array[][2];
;
_vera_scroll_array:
    .byte 0, 64
    .byte 14, 85
    .byte 23, 128

VERA_SCROLL_ARRAY_LEN = 3

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

.import _m

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