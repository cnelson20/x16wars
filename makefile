CC ?= cl65.exe

MOUNT_LOCATION = mnt/

all: wars.prg

wars.prg: main.c main.h map.c map.h structs.h waitforjiffy.s waitforjiffy.h fastroutines.s fastroutines.h render_horse.s
	$(CC) -o wars.prg -t cx16 -Ois --codesize 200 map.c main.c waitforjiffy.s fastroutines.s render_horse.s zsound/zsound.lib

copy:
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
