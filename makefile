CC = cl65
DIR = /cygdrive/c/Users/cjnel/Desktop/x16emu-r40/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c structs.h
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c unitgraphics.c palette.c

copy:
	cp wars.prg $(DIR)WARS.PRG;
	cp tilegraphics.chr $(DIR)TILE.CHR;
	cp lettergraphics.chr $(DIR)LETTER.CHR;
	cp spritegraphics.chr $(DIR)SPRITES.CHR; 

clean:
	-rm *.d
	-rm *~
	-rm *.o
