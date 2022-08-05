CC = cl65.exe
EMU_LOCATION = /mnt/d/x16emu-r40/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c structs.h
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c unitgraphics.c palette.c

copy:
	cp wars.prg $(EMU_LOCATION)WARS.PRG;
	cp tilegraphics.chr $(EMU_LOCATION)TILE.CHR;
	cp lettergraphics.chr $(EMU_LOCATION)LETTER.CHR;
	cp spritegraphics.chr $(EMU_LOCATION)SPRITES.CHR; 

clean:
	-rm *.d
	-rm *~
	-rm *.o
