CC = cl65
EMU_LOCATION = /cygdrive/c/Users/cjnel/Desktop/x16emu/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c
	$(CC) -o wars.prg -t cx16 map.c main.c unitgraphics.c palette.c

copy:
	cp wars.prg $(EMU_LOCATION)wars.prg;
	cp tilegraphics.chr $(EMU_LOCATION)tile.chr;
	cp lettergraphics.chr $(EMU_LOCATION)letter.chr;
	cp spritegraphics.chr $(EMU_LOCATION)sprites.chr; 

clean:
	rm *.d
	rm *~
