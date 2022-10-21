CC = cl65

MOUNT_LOCATION = mnt/

SOURCES = main.c map.c waitforjiffy.s fastroutines.s render_horse.s
HEADERS = main.h map.h structs.h fastroutines.h

FLAGS = -m x16wars.map -o wars.prg -tcx16 -Ois --codesize 200

all: wars.prg

wars.prg: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) $(SOURCES) zsound/zsound.lib

build: wars.prg blank copy
	
blank:
	cp blank_sd.img wars_sd.img

copy: wars.prg
	./scripts/mount_sd.sh wars_sd.img
	sudo cp WARS.PRG $(MOUNT_LOCATION)WARS.PRG;
	sudo cp char_data/PALETTE.BIN $(MOUNT_LOCATION)PALETTE.BIN

	sudo cp char_data/TILEGRAPHICS.CHR $(MOUNT_LOCATION)TILE.CHR;
	sudo cp char_data/LETTERGRAPHICS.CHR $(MOUNT_LOCATION)LETTER.CHR;
	sudo cp char_data/SPRITEGRAPHICS.CHR $(MOUNT_LOCATION)SPRITES.CHR; 

	sudo cp char_data/REDGRAPHICS.CHR $(MOUNT_LOCATION)RED.CHR;
	sudo cp char_data/GREENGRAPHICS.CHR $(MOUNT_LOCATION)GREEN.CHR;
	sudo cp char_data/BLUEGRAPHICS.CHR $(MOUNT_LOCATION)BLUE.CHR;
	sudo cp char_data/YELLOWGRAPHICS.CHR $(MOUNT_LOCATION)YELLOW.CHR;
	sudo cp char_data/EXPLOSION.CHR $(MOUNT_LOCATION)EXPL.CHR;
	sudo cp char_data/ARROW.CHR $(MOUNT_LOCATION)ARROW.CHR;

	sudo cp MAPS/ $(MOUNT_LOCATION) -r
	sudo cp SOUND/ $(MOUNT_LOCATION) -r

	./scripts/close_sd.sh


clean:
	-rm *.d
	-rm *~
	-rm *.o
