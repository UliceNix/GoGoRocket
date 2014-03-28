#pragma once

#include	<stdio.h>
#include	<curses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#define ROWS    5
#define NUMS    7
#define	TUNIT   20000		/* timeunits in microseconds */

struct	saucer {
        char *str;	
        int row;	
        int col;
int hit;
        int delay;  
        int dir;

};

struct baseline {
        int col;
};

struct rocket {
        int speed;
        int row;
        int col;
};


/* mutex for modifying window*/
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

/* mutex for accessing a saucer's position and hit value */
pthread_mutex_t rk = PTHREAD_MUTEX_INITIALIZER;

/* mutex for modifying escape variable */
pthread_mutex_t es = PTHREAD_MUTEX_INITIALIZER;

/* mutex for decreasing/ increasing the number of rockets*/
pthread_mutex_t dc = PTHREAD_MUTEX_INITIALIZER;

/* mutex for increasing score*/
pthread_mutex_t sc = PTHREAD_MUTEX_INITIALIZER;


pthread_t thrds[5];

struct saucer saucer[5];

struct baseline base;

int escape = 0;

int level = 1;

int rockets = 10;

int score = 0;

int limit = 10;

int reward = 1;

int requiredScore = 5;

int delay = 15;

int rocketdelay = 8;

int done = 0;

int getLimit(int level);

int getReward(int level);

int levelUpLimit(int level);

void printInstruction();

void printUserMenu();

void moveRight();

void moveLeft();

void updateStatus();

int setup(struct saucer saucer[]);

int levelup(struct saucer saucer[]);
