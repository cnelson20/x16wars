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

clean:
	rm *.d
	rm *~
