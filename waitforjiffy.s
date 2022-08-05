RDTIM = $FFDE

.importzp ptr1

.export _waitforjiffy
_waitforjiffy:

jsr RDTIM
sta @byte
:
jsr RDTIM
cpy @byte
beq :-

rts

@byte:
	.byte 0