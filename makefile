CC = cl65.exe
EMU_LOCATION = /mnt/d/x16emu-r41/

all: wars.prg

wars.prg: main.c main.h map.c map.h palette.c structs.h waitforjiffy.s waitforjiffy.h fastroutines.s fastroutines.h render_horse.s
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c palette.c waitforjiffy.s fastroutines.s render_horse.s zsound/zsound.lib

copy:
	cp WARS.PRG $(EMU_LOCATION)WARS.PRG;
	cp TILEGRAPHICS.CHR $(EMU_LOCATION)TILE.CHR;
	cp LETTERGRAPHICS.CHR $(EMU_LOCATION)LETTER.CHR;
	cp SPRITEGRAPHICS.CHR $(EMU_LOCATION)SPRITES.CHR; 

	cp REDGRAPHICS.CHR $(EMU_LOCATION)RED.CHR;
	cp GREENGRAPHICS.CHR $(EMU_LOCATION)GREEN.CHR;
	cp BLUEGRAPHICS.CHR $(EMU_LOCATION)BLUE.CHR;
	cp YELLOWGRAPHICS.CHR $(EMU_LOCATION)YELLOW.CHR;
	cp EXPLOSION.CHR $(EMU_LOCATION)EXPL.CHR;


clean:
	-rm *.d
	-rm *~
	-rm *.o
