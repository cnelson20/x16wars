CC = cl65.exe
EMU_LOCATION = /mnt/d/x16emu-r41/

all: wars.prg

wars.prg: main.c main.h map.c map.h structs.h waitforjiffy.s waitforjiffy.h fastroutines.s fastroutines.h render_horse.s
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c waitforjiffy.s fastroutines.s render_horse.s zsound/zsound.lib

copy:
	cp WARS.PRG $(EMU_LOCATION)WARS.PRG;
	cp char_data/TILEGRAPHICS.CHR $(EMU_LOCATION)TILE.CHR;
	cp char_data/LETTERGRAPHICS.CHR $(EMU_LOCATION)LETTER.CHR;
	cp char_data/SPRITEGRAPHICS.CHR $(EMU_LOCATION)SPRITES.CHR; 

	cp char_data/REDGRAPHICS.CHR $(EMU_LOCATION)RED.CHR;
	cp char_data/GREENGRAPHICS.CHR $(EMU_LOCATION)GREEN.CHR;
	cp char_data/BLUEGRAPHICS.CHR $(EMU_LOCATION)BLUE.CHR;
	cp char_data/YELLOWGRAPHICS.CHR $(EMU_LOCATION)YELLOW.CHR;
	cp char_data/EXPLOSION.CHR $(EMU_LOCATION)EXPL.CHR;

	cp char_data/ARROW.CHR $(EMU_LOCATION)ARROW.CHR;


clean:
	-rm *.d
	-rm *~
	-rm *.o
