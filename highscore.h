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

/*
 * highscore.h
 * printStruct()     : print a highscore struct in three lines 
 * populate()        : populate a highscore struct with information 
 *                     from score.txt file
 * printHighscore()  : print the high score board
 * lowestHighscore() : return the lowest highscore in current score 
 *                     board
 * writeHighscore()  : add new highscore record to high score board
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct highscore{
        char *s1;
        char *p1;
        char *s2;
        char *p2;
        char *s3;
        char *p3;
        int count;
};

void printStruct(struct highscore highscore);

struct highscore populate();

void printHighscore();

int lowestHighscore();

void writeNewHighscore(int score, char* name);
