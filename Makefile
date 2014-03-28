all: saucer

saucer:game.c game.h
	cc game.c -lcurses -lpthread -o saucer
