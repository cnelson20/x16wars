.importzp ptr1, ptr2
.importzp sreg
.importzp sp

;
; void __fastcall__ clear_sprite_table();
;
.export _clear_sprite_table
_clear_sprite_table:
	lda #$06
	sta $9F20
	lda #$FC
	sta $9F21
	
	lda #$41
	sta $9F22
	:
	stz $9F23
	
	iny
	bne :-
	
	rts 
	
	