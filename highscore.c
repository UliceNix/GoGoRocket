#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "highscore.h"


void printStruct(struct highscore highscore){

        if(highscore.count ==  0)
                return;

        if(highscore.count > 0){
            printf("%40s", "***       HIGH SCORE      ***\n");
            printf("%20s | %20s\n", "User Name", "Score");
            printf("%20s | %20s\n", highscore.p1, highscore.s1);
        }

        if(highscore.count > 1)
                printf("%20s | %20s\n", highscore.p2, highscore.s2);

        if(highscore.count > 2)
                printf("%20s | %20s\n", highscore.p3, highscore.s3);

        return;
}

struct highscore populate()
{
        FILE *fp; 
        char c;
        int count = 0;
        struct highscore highscore;
        
        fp =  fopen("score.txt", "a+");

        if(fp == NULL)
                return highscore;

        fseek(fp, 0, 0);

        highscore.s1 = malloc(20);
        highscore.s2 = malloc(20);
        highscore.s3 = malloc(20);
        highscore.p1 = malloc(20);
        highscore.p2 = malloc(20);
        highscore.p3 = malloc(20);

        fscanf(fp, "%s", highscore.p1);
        fscanf(fp, "%s", highscore.s1);

        count = (strlen(highscore.s1) == 0) ? count : ++count;

        fscanf(fp, "%s", highscore.p2);
        fscanf(fp, "%s", highscore.s2);

        count = (strlen(highscore.s2) == 0) ? count : ++count;

        fscanf(fp, "%s", highscore.p3);
        fscanf(fp, "%s", highscore.s3); 

        count = (strlen(highscore.s3) == 0) ? count : ++count;
      
        highscore.count = count;

        fclose(fp);

        return highscore;
}

void printHighscore(){
     
        struct highscore high = populate();
        printStruct(high);
        return;

}

int lowestHighscore(){
        
        struct highscore highscore = populate();
        if (highscore.count == 3)
                return (int) strtol(highscore.s3, (char **)NULL, 10);
        else
                return 0;
}

void writeNewHighscore(int intScore, char* name){

        FILE *fp; 
        int count = 0;
        char score[20];

        struct highscore highscore = populate();
        
        int array[3] = {(int) strtol(highscore.s1, (char **)NULL, 10),
                        (int) strtol(highscore.s2, (char **)NULL, 10),
                        (int) strtol(highscore.s3, (char **)NULL, 10)
        };
        
        fp =  fopen("score.txt", "w+");

        sprintf(score, "%d", intScore);

        if(intScore >= array[0]){
            highscore.p3 = highscore.p2;
            highscore.p2 = highscore.p1;
            highscore.p1 = name;

            highscore.s3 = highscore.s2;
            highscore.s2 = highscore.s1;
            highscore.s1 = score;
        }else if(intScore >= array[1]){
            highscore.p3 = highscore.p2;
            highscore.p2 = name;
            
            highscore.s3 = highscore.s2;
            highscore.s2 = score;
        }else if(intScore >= array[2]){
            highscore.p3 = name;
            highscore.s3 = score;
        }

        if(highscore.count >= 0){
            fprintf(fp, "%20s\n", highscore.p1);
            fprintf(fp, "%20s\n", highscore.s1);
        }

        if(highscore.count >= 1){
            fprintf(fp, "%20s\n", highscore.p2);
            fprintf(fp, "%20s\n", highscore.s2);
        }

        if(highscore.count >= 2){
            fprintf(fp, "%20s\n", highscore.p3);
            fprintf(fp, "%20s\n", highscore.s3);
        }

        fclose(fp);

}
