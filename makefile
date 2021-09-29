CC = ~/Desktop/cc65/bin/cl65

ALL: map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
	$(CC) -o wars.prg -t cx16 map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
