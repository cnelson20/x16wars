CC = cl65
EMU_LOCATION = /cygdrive/c/Users/cjnel/Desktop/x16emu/
REPO_LOCATION = /cygdrive/c/Users/cjnel/Desktop/x16wars/

all: wars.prg

wars.prg: map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
	$(CC) -o wars.prg -t cx16 map.c main.c unitgraphics.c spritegraphics.c palette.c

copy:
	cp wars.prg $(EMU_LOCATION)wars.prg;
	cp tilegraphics.chr $(EMU_LOCATION)tile.chr;
	cp lettergraphics.chr $(EMU_LOCATION)letter.chr;
	cp tilegraphics.chr $(EMU_LOCATION)tile.chr; 

clean:
	rm *.d
	rm *~
