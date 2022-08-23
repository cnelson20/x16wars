CC = cl65.exe
EMU_LOCATION = /mnt/d/x16emu-r41/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c structs.h waitforjiffy.s waitforjiffy.h fastroutines.s fastroutines.h
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c unitgraphics.c palette.c waitforjiffy.s fastroutines.s

copy:
	cp WARS.PRG $(EMU_LOCATION)WARS.PRG;
	cp TILEGRAPHICS.CHR $(EMU_LOCATION)TILE.CHR;
	cp LETTERGRAPHICS.CHR $(EMU_LOCATION)LETTER.CHR;
	cp SPRITEGRAPHICS.CHR $(EMU_LOCATION)SPRITES.CHR; 

clean:
	-rm *.d
	-rm *~
	-rm *.o
