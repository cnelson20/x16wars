.importzp ptr1, ptr2
.importzp sreg
.importzp sp

.export _load_data_vera_from
_load_data_vera_from:
	.word 0
	
; void __fastcall__ load_data_vera(unsigned short bytes);
.export _load_data_vera
_load_data_vera:

tay
iny 
inx

lda _load_data_vera_from
sta ptr1 
lda _load_data_vera_from + 1
sta ptr1 + 1

@loop:
lda (ptr1)
sta $9F23

inc ptr1
bne :+
inc ptr1 + 1
:

dey
bne @loop
dex
bne @loop

rts

;
; void __fastcall__ clear_sprite_table(unsigned char from_index);
;
.export _clear_sprite_table
_clear_sprite_table:
	stz $9F21
	
	tay
	asl A 
	rol $9F21
	asl A
	rol $9F21
	asl A 
	rol $9F21
	clc ; i dont think this is necessary, carry should be clear
	adc #6
	sta $9F20
	lda $9F21
	adc #$FC
	sta $9F21
	
	lda #$41
	sta $9F22
	:
	stz $9F23
	
	iny
	bne :-
	
	rts 
	
	