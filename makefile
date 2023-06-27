CC = cl65
EMU = /mnt/d/box16-r41-2/box16.exe
FOLDER ?= game/

SOURCES = main.c map.c waitforjiffy.s fastroutines.s render_horse.s map_functions.s
HEADERS = main.h map.h structs.h fastroutines.h map_functions.h

FLAGS = -m x16wars.map -o wars.prg -tcx16 -Ois --codesize 200

all: wars.prg

wars.prg: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) $(SOURCES) zsound/zsound.lib

build: wars.prg blank copy
	
blank:
	cp blank_sd.img wars_sd.img

sd:
	./scripts/mount_sd.sh wars_sd.img
	sudo make copy FOLDER=mnt/
	./scripts/close_sd.sh

copy: wars.prg
	cp WARS.PRG $(FOLDER)WARS.PRG;
	cp char_data/PALETTE.BIN $(FOLDER)PALETTE.BIN

	cp char_data/TILEGRAPHICS.CHR $(FOLDER)TILE.CHR;
	cp char_data/LETTERGRAPHICS.CHR $(FOLDER)LETTER.CHR;
	cp char_data/SPRITEGRAPHICS.CHR $(FOLDER)SPRITES.CHR; 

	cp char_data/REDGRAPHICS.CHR $(FOLDER)RED.CHR;
	cp char_data/GREENGRAPHICS.CHR $(FOLDER)GREEN.CHR;
	cp char_data/BLUEGRAPHICS.CHR $(FOLDER)BLUE.CHR;
	cp char_data/YELLOWGRAPHICS.CHR $(FOLDER)YELLOW.CHR;
	cp char_data/EXPLOSION.CHR $(FOLDER)EXPL.CHR;
	cp char_data/ARROW.CHR $(FOLDER)ARROW.CHR;

	cp MAPS/ $(FOLDER) -r
	cp SOUND/ $(FOLDER) -r

run: copy
	$(EMU) -sdcard wars_sd.img

run2m: copy
	$(EMU) -sdcard wars_sd.img -ram 2048

clean:
	-rm *.d
	-rm *~
	-rm *.o
