/*  Copyright (C) 2014 Alice (Mingxun) Wu

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#pragma once

#include	<stdio.h>
#include	<curses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#define ROWS    3
#define NUMS    5
#define RANGE   30
#define	TUNIT   20000		

struct	saucer {
        char *str;    /* the string represents the appearance of a saucer*/
        int row;      /* the row indicates the row number of the saucer*/
        int col;      /* the col indicates the column number of the saucer*/
        int hit;      /* the hit is a flag meaning whether a saucer is hit */
        int delay;    /* delay determines the time between each move */
        int live;     /* If a saucer is in spawn process, live is 0 */

};

struct baseline {
        int col;
};

struct rocket {
        int speed;   /* speed determins the time between each move*/
        int row;     /* the row indicates the row number of the rocket*/
        int col;    /* the col indicates the column number of the rocket*/
};


/* mutex for modifying window*/
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

/* mutex for accessing a saucer's position and hit value */
pthread_mutex_t rk = PTHREAD_MUTEX_INITIALIZER;

/* mutex for modifying escape variable */
pthread_mutex_t es = PTHREAD_MUTEX_INITIALIZER;

/* mutex for decreasing/ increasing the number of rockets*/
pthread_mutex_t dc = PTHREAD_MUTEX_INITIALIZER;

/* mutex for modifying score*/
pthread_mutex_t sc = PTHREAD_MUTEX_INITIALIZER;

/* mutext for level variable*/
pthread_mutex_t lv = PTHREAD_MUTEX_INITIALIZER;


pthread_t thrds[5];

pthread_t inputThread;

struct saucer saucer[5];

struct baseline base;

int escape = 0;

int level = 1;

int rockets = 10;

int score = 0;

int limit = 10;

int reward = 1;

int requiredScore = 30;

int delay = 15;

int rocketdelay = 8;

int done = 0;

int gamepause = 0;

int gameover = 0;

/* functions that belong to configuration phase*/
int getLimit(int level);

int getReward(int level);

int levelUpLimit(int level);

int getDelay(int level);

int getRocketDelay(int level);

void updateSetting(int level);

void updateStatus();

int setup(struct saucer saucer[]);

int levelup(struct saucer saucer[]);


/* functions that responsible for running the game*/
void enterShop();

void recordHighscore();

int gameOn();

void unlockEverything();

void lockEverything();

void printInstruction();

void printUserMenu();

void moveRight();

void moveLeft();


/* functions that are related to a rocket thread */
void moveRocket(struct rocket *rocket);

void disposeRocket(struct rocket *rocket);

void hitReward(int countHits);

void *fire(void *arg);


/* functions that are related to a saucer thread */
void spawn(struct saucer *info);

int vanish(struct saucer *info, int count);

void moveSaucer(struct saucer *info);

void *attack(void *arg);



