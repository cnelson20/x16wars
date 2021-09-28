ALL: map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c
	cl65 -o wars.prg -t cx16 map.c main.c unitgraphics.c tilegraphics.c spritegraphics.c palette.c