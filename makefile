CC = cl65

ALL: map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
	$(CC) -o wars.prg -t cx16 map.c main.c unitgraphics.c spritegraphics.c palette.c

copy:
	cp tilegraphics.chr ../x16emu/tile.chr; 
	cp lettergraphics.chr ../x16emu/letter.chr; 
	cp spritegraphics.chr ../x16emu/sprites.chr; 
	cp wars.prg ../x16emu/;

clean:
	rm *.d
	rm *~
