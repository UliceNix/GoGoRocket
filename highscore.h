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
