CC = cl65
EMU_LOCATION = /cygdrive/c/Users/cjnel/Desktop/x16emu/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c structs.h
	$(CC) -o wars.prg -t cx16 -Oi --codesize 200 map.c main.c unitgraphics.c palette.c

copy:
	cp wars.prg $(EMU_LOCATION)WARS.PRG;
	cp tilegraphics.chr $(EMU_LOCATION)TILE.CHR;
	cp lettergraphics.chr $(EMU_LOCATION)LETTER.CHR;
	cp spritegraphics.chr $(EMU_LOCATION)SPRITES.CHR; 

clean:
	rm *.d
	rm *~
	rm *.o
