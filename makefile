CC = ~/Desktop/cc65/bin/cl65
EMU_LOCATION = ~/Desktop/x16-emulator/

ALL: wars.prg
	

wars.prg: map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
	$(CC) -o wars.prg -t cx16 map.c main.c unitgraphics.c spritegraphics.c palette.c
	cp wars.prg $(EMU_LOCATION)wars.prg
	cp tilegraphics.chr $(EMU_LOCATION)tile.chr
	cp lettergraphics.chr $(EMU_LOCATION)letter.chr

run:
	$(EMU_LOCATION)x16emu -prg $(EMU_LOCATION)wars.prg -run

copy:
	cp tilegraphics.chr ../x16emu/tile.chr; 
	cp lettergraphics.chr ../x16emu/letter.chr; 
	cp spritegraphics.chr ../x16emu/sprites.chr; 
	cp wars.prg ../x16emu/;

clean:
	rm *.d
	rm *~
