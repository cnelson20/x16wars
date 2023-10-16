CC = cl65

DIR ?= game
MOUNT_LOCATION = mnt

SOURCES = main.c map.c waitforjiffy.s fastroutines.s render_horse.s map_functions.s
HEADERS = main.h map.h structs.h fastroutines.h map_functions.h

FLAGS = -m x16wars.map -o wars.prg -tcx16 -Ois --codesize 200

all: wars.prg

wars.prg: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) $(SOURCES) zsound/zsound.lib

build: wars.prg blank copy
	
blank:
	cp blank_sd.img wars_sd.img

sd: wars.prg
	./scripts/mount_sd.sh wars_sd.img
	sudo make copy DIR=$(MOUNT_LOCATION)
	./scripts/close_sd.sh


copy: wars.prg
	cp WARS.PRG $(DIR)/WARS.PRG;
	cp char_data/PALETTE.BIN $(DIR)/PALETTE.BIN

	cp char_data/TILEGRAPHICS.CHR $(DIR)/TILE.CHR;
	cp char_data/LETTERGRAPHICS.CHR $(DIR)/LETTER.CHR;
	cp char_data/SPRITEGRAPHICS.CHR $(DIR)/SPRITES.CHR; 

	cp char_data/REDGRAPHICS.CHR $(DIR)/RED.CHR;
	cp char_data/GREENGRAPHICS.CHR $(DIR)/GREEN.CHR;
	cp char_data/BLUEGRAPHICS.CHR $(DIR)/BLUE.CHR;
	cp char_data/YELLOWGRAPHICS.CHR $(DIR)/YELLOW.CHR;
	cp char_data/EXPLOSION.CHR $(DIR)/EXPL.CHR;
	cp char_data/ARROW.CHR $(DIR)/ARROW.CHR;

	cp MAPS/ $(DIR) -r
	cp SOUND/ $(DIR) -r

run: copy
	$(EMU) -sdcard wars_sd.img

run2m: copy
	$(EMU) -sdcard wars_sd.img -ram 2048

clean:
	-rm *.d
	-rm *~
	-rm *.o
