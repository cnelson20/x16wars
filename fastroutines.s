.importzp ptr1, ptr2, ptr3, ptr4
.importzp tmp1, tmp2, tmp3, tmp4
.importzp sreg
.importzp sp

.import popa
.import popax

_cbm_k_chrin = $FFCF
.export _cbm_k_chrin

;
; void __fastcall__ clear_sprite_table(unsigned char from_index);
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

;
; _sabs_use_ax - to be called by other asm routines
;
.export _sabs_use_ax
_sabs_use_ax:
	stx _sabs_temp
	jmp _sabs_entry

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
	stp
	jmp ($FFFC)
	

	