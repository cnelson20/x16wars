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
