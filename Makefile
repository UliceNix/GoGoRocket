all: saucer score.o

saucer:game.c game.h score.o
	cc game.c score.o -lcurses -lpthread -o saucer

score.o: highscore.c highscore.o
	cc -c highscore.c -o score.o
