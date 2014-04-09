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

/* game.c
 *   This is the main game file, which contains threads and managers
 *   threads for rockets, saucers.
 *
 *   Main game:
 *     getLimit()      : get the limit of escaped rockets for each level
 *     getReward()     : get the reward value for each correct shot in each 
 *                       level
 *     levelUpLimit()  : get the limit of scores for user to level up
 *     getDelay()      : return the delay value for a saucer thread
 *     getRocketDelay(): return the delay value for a rocket thread
 *     updateSetting() : update all information like limit, score, rocket in 
 *                       level up process
 *     gameOn()        : the game function that runs in each level
 *     setup()         : setup function initializes necessary variables for game
 *                       to start
 *     moveLeft()      : move launcher to left
 *     moveRight()     : move launcher to right
 *     printUserMenu() : print welcome menu
 *     printInstruction: print user instruction
 *     levelup()       : modify necessary variables and level up
 *     updateStatus()  : update the status line at the bottom of the screen
 *     enterShop()     : prompt user to buy rockets by trading scores
 *     recordHighscore : record a user's score if the score is higher than the
 *                       lowest score in highscore board
 *     unlockEverything: unlock all locks for game to resume
 *     lockEverything  : lock every lock to pause the game
 * 
 *   Rocket:
 *     moveRocket()   : move the rocket up.
 *     disposeRocket(): dispose a rocket by eliminating it from screen
 *     hitReward()    : when hit a saucer, this function will add proper scores
 *                      and rockets to user's current scores and rockets
 *     *fire()        : a rocket thread that moves up and detects if it hits a 
 *                      saucer
 *   Saucer:
 *     moveSaucer()   : move the saucer from left to right 
 *     vanish()       : let a saucer escape the screen character by character
 *     *attack()      : a saucer thread that moves saucer and tries to escape
 *     spawn()        : after got hit or escaped, a saucer thread is recycled 
 *                      and put to spawn process.
 */

#include	<stdio.h>
#include	<curses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include         "game.h"
#include         "highscore.h"

/* depending on the level number, the limit of escaped saucers varies.
 * Thus getLimit returns the corresponding limit when given a level number 
 */
int getLimit(int level)
{
        switch (level){
        case 2:
                return 10;
        case 3:
                return 18;
        case 4:
                return 30;
        case 5:
                return 45;
        default:
                return -1;
       }
}


/* depending on the level number, the reward scheme varies.
 * Thus getReward returns the corresponding reward when given a level number 
 */
int getReward(int level)
{
        switch (level){
        case 2:
                return 1;
        case 3:
                return 1;
        case 4:
                return 2;
        case 5:
                return 2;
        default:
                return 4;
        }
}


/* depending on the level number, the scores that are needed to level up  varies
 * Thus levelUpLimit returns the corresponding scores when given a level 
 * number 
 */
int levelUpLimit(int level)
{
        switch (level){
        case 2:
                return 100;
        case 3:
                return 260;
        case 4:
                return 500;
        default:
                return 5000;
        }
}

/* depending on the level number, the delay for a saucer varies.
 * Thus getDelay returns the corresponding delay value  when given a level 
 * number 
 */
int getDelay(int level)
{
        switch (level){
        case 2:
                return 10;
        case 3:
                return 7;
        case 4:
                return 5;
        default:
                return 2;
        }

}

/* depending on the level number, the delay for a rocket varies.
 * Thus getRocketDelay returns the corresponding delay value when given a level 
 * number 
 */
int getRocketDelay(int level)
{
        switch (level){
        case 2:
                return 7;
        case 3:
                return 6;
        case 4:
                return 5;
        default:
                return 4;
        }
}

/* update all "status" variables when leveling up */
void updateSetting(int level)
{
        limit = getLimit(level);
        reward = getReward(level);
        requiredScore = levelUpLimit(level);
        delay = getDelay(level);
        rocketdelay = getRocketDelay(level);
}

/* print game instruction to users */
void printInstruction()
{
        printf("The scenario is you are the guard of planet Alderaan.\n" 
               "Now your home planet is underattack.\n"
               "A large number of saucers are dispatching from Death Star"
               " to Alderaan.\n"
               "Take action and protect your home planet!\n");
        printf("=============================================================\n"
               "|Quit(Q), Pause(P), Resume(R), Left(,), Right(.),Fire(SPACE)|\n"
               "+-----------------------------------------------------------+\n"
               "| A rockest is deconstructed as soon as it hits a saucer or |\n"
               "| more rockets in the same attacking range. Thus it cannot  |\n"
               "| proceed to hit other saucers.                             |\n"
               "+-----------------------------------------------------------+\n"
               "| This game has multiple levels for you to discover!        |\n"
               "| Each level, the score and rewards will vary as well as the|\n"
               "| speed of a saucer and the interval between saucers.       |\n"
               "+-----------------------------------------------------------+\n"
               "| As soon as you level up, you could choose to buy extra    |\n"
               "| rockets by trading some scores in the game store.         |\n"
               "+-----------------------------------------------------------+\n"
               "| * For best performance, please use 80*20 screen           |\n"
               "+-----------------------------------------------------------+\n"
                );
        return;
}

/* print welcome menu*/
void printUserMenu()
{
        printf("Welcome to kill-that-saucer game\n");
        printf("================================\n");
        printf("|       Start Game (S)         |\n");
        printf("+------------------------------+\n");
        printf("|      Instruction (I)         |\n");
        printf("+------------------------------+\n");
        printf("|       High Score (H)         |\n");
        printf("+------------------------------+\n");
        printf("|        Quit   (Q)            |\n");
        printf("================================\n");
        return;
}

/* move the launcher to right */
void moveRight()
{
        if(base.col + 1 >= COLS - 1)
                return;
        
        pthread_mutex_lock(&mx);
        move(LINES-2, base.col);
        addch(' ');
        addch('|');
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&mx);

        base.col++;
        return;
}

/* move the launcher to left*/
void moveLeft()
{
        if(base.col - 5  < 1)
                return;
        
        pthread_mutex_lock(&mx);
        move(LINES-2, base.col);
        addch(' ');
        addch(' ');
        move(LINES-2, base.col - 1);
        addch('|');
        move(LINES-1, COLS-1);
        refresh();
        pthread_mutex_unlock(&mx);

        base.col--;
        return;
}

/* update the status in the bottom line */
void updateStatus()
{
        pthread_mutex_lock(&dc);
        pthread_mutex_lock(&sc);
        pthread_mutex_lock(&es);
        pthread_mutex_lock(&mx);
        mvprintw(LINES-1,
                 0,
                 "Pause(P) Resume(R)"
                 " Rockets: %5d Score:%5d Escape:%4d/%4d Lv:%d",
                 rockets, score, escape, limit, level);
        refresh();
        pthread_mutex_unlock(&mx);
        pthread_mutex_unlock(&es);
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&dc);
        return;
}

/* setup screens, status variables for game to start*/
int setup(struct saucer saucer[])
{
        int i;
        char c;
	/* assign rows and velocities to each string */
	srand(getpid());
	for(i = 0 ; i < NUMS; i++){
            saucer[i].str = "<--->";
            saucer[i].row = rand()%ROWS;		
            saucer[i].col = 0;
            saucer[i].hit = 0;
            saucer[i].delay = 15 + (rand()%RANGE);	
            saucer[i].live = 1;
	}
    
        srand(getpid());
               
	/* set up curses */
	initscr();
	crmode();
	noecho();
	clear();
        move(LINES-2, ((COLS+1)/2));
        addch(' ');
        addch('|');
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();

        /* print initial status */
        mvprintw(LINES-1,
                 0,
                 "Pause(P) Resume(R)"
                 " Rockets: %5d Score:%5d Escape:%4d/%4d Lv:%d",
                 rockets, score, escape, limit, level);

        /* set up launcher */
        base.col = (COLS+1)/2;
        done = 1;

	return NUMS;
}

/*
 * enter the gameshop in which users could trade scores to buy
 * some extra rockets. 
 */
void enterShop(){

        int choice_available = score / 10;
        int i;
        char c;
        int min = (choice_available > 3) ? 3 : choice_available;

        /* print at most 3 choices for users to choose */
        for(i = 0; i < min; i++){
            mvprintw(i, 0, " %d) Use %d score to buy %d rockets", 
                     i+1, (score - i*10)/10 * 10, (score - i*10)/5);
        }

        /* if there's no deal that satisfies the user, he could use Q to quit*/
        mvprintw(4, 0, "Or Enter 'Q' to Quit\n");

        /* get the user's input */
        while( (c = getch()) != EOF){
            if(c == '1'){
                rockets +=  score/5;
                score -=  (score)/10 * 10;
                break;
            }else if(c == '2' && min > 1){
                rockets +=  (score - 10)/5;
                score -=  (score - 10)/10 * 10;
                break;
            }else if(c == '3' && min > 2){
                rockets +=  (score - 20)/5;
                score -=  (score - 20)/10 * 10;
                break;
            }else if(c == 'Q'){
                return;
            }
            
        }
}

/* the levelup function will perform a level up action by updating all
 * status values and print some ascii work and let the user to shop
 */
int levelup(struct saucer saucer[])
{
        int i;
        char c;
        srand(getpid());
        
        /* Since when doing the levelup process, all the threads that 
         * executes gameOn() are all terminated. There will not be any
         * processes modifying the level variable.
         */
        updateSetting(level);
        
        for(i = 0 ; i < NUMS; i++){
            saucer[i].str = "<--->";
            saucer[i].row = rand()%ROWS;		
            saucer[i].col = 0;
            saucer[i].hit = 0;
            saucer[i].delay = delay + (rand()%RANGE);	
            saucer[i].live = 1;
	}

        pthread_mutex_lock(&mx);
        
        /* print levelup message*/
        erase();
        move(0, 0);
        refresh();
        mvprintw(1,0," #       ####### #     # ####### #          #     "
                 "# ######");  
        mvprintw(2,0," #       #       #     # #       #          #     #"
                 " #     #");
        mvprintw(3,0,      " #       #       #     # #       #          #"
                 "     # #     #");
        mvprintw(4,0," #       #####   #     # #####   #          #     #"
                 " ######");
        mvprintw(5,0," #       #        #   #  #       #          #     # #"); 
        mvprintw(6,0," #       #         # #   #       #          #     # # ");
        mvprintw(7,0," ####### #######    #    ####### #######     #####  #");
        mvprintw(8,0,"LEVEL %d. MAX ESCAPE: %d. THE SCORE TO THE NEXT LEVEL: %d"
                 , level, limit, requiredScore);

        /* there are only 6 levels, and level 6 is a free play level. Thus
         * there will be no more levels after level 6
         */
        if(level == 6){
            mvprintw(9, 0, "WELCOME TO LEVEL 6. LOOKS LIKE NO ONE HAS EVER"
                     " MADE IT HERE! CONGRATS! ");
            mvprintw(10, 0, "THERE WILL BE NO LEVELS ANY MORE"
                     ". LEVEL 6 IS A FREE PLAY MODE. THE GAME WILL ONLY"
                     " TERMINATE AS SOON AS YOU RUN OF OUT ROCKETS.");
        }
        
        /* ask the user if he wants to buy some rockets*/
        mvprintw(11, 0,
                 "Would you like to buy some extra rockets from the store?"
                 "(Y/N)\n"
                );

        refresh();

        /* handle user input */
        while((c = getch()) != EOF){
            if(c == 'N') 
                    break;
            else if(c == 'Y'){
                 erase();
                 move(0,0);
                 refresh();
                 enterShop();
                 break;
            }
        }

        erase();
        move(0,0);
        refresh();

        /* re-position the launcher */
        move(LINES-2, ((COLS+1)/2));
        addch(' ');
        addch('|');
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();

        /* print the status line */
        mvprintw(LINES-1,
                 0,
                 "Pause(P) Resume(R)"
                 " Rockets: %5d Score:%5d Escape:%4d/%4d Lv:%d",
                 rockets, score, escape, limit, level);
        base.col = (COLS+1)/2;
        done = 1;
        pthread_mutex_unlock(&mx);
        return NUMS;
}

/* move the rocket upward */
void moveRocket(struct rocket *rocket)
{
        pthread_mutex_lock(&mx);
        move( rocket->row, rocket->col);
        addch(' ');
        refresh();
        pthread_mutex_unlock(&mx);
        return;
}

/* dispose the rocket by eliminating it from screen */
void disposeRocket(struct rocket *rocket)
{
        move( rocket->row, rocket->col);
        addch(' ');
        refresh();
        return;
}

/* reward the user with a number of rockets and scores depending on how
 * many saucers a rocket has hit
 */
void hitReward(int countHits)
{
        
        pthread_mutex_lock(&sc);
        score += countHits * level;
        pthread_mutex_unlock(&sc);

        pthread_mutex_lock(&dc);
        rockets += countHits * reward;
        pthread_mutex_unlock(&dc);

        updateStatus();
        return;
}

/*
 * The function that runs in each rocket thread
 */
void *fire(void *arg)
{
        struct rocket *rocket = arg;
        int dispose = 0;
        int countHits = 0;

        /* keep the rocket moving and while the rocket has not escaped */
        while (rocket->row >= 0 && !gameover){

            /* rocket movement */
            usleep(rocket->speed*TUNIT);
            moveRocket(rocket);
            
            /* when rocket is in saucer area, detect if the rocket is hitting
             * any saucers
             */
            if(rocket->row < ROWS){
                int i;
                countHits = 0;
                pthread_mutex_lock(&rk);
                for(i = 0; i < NUMS; i++){
                    if(saucer[i].live > 0
                       && saucer[i].row == rocket->row
                       && rocket->col >= saucer[i].col - 1
                       && rocket->col <= 
                       saucer[i].col + strlen(saucer[i].str) - 1){
                        saucer[i].hit = 1;
                        saucer[i].str = " ";
                        refresh();
                        dispose = 1;
                        ++countHits;
                    }
                }
                pthread_mutex_unlock(&rk); 
            }

            /* if the rocket hits a saucer, then reward the user */
            if(countHits > 0)
                    hitReward(countHits);
            
            /* if need to dispose the rocket, then dispose it*/
            pthread_mutex_lock(&mx);
            if(dispose || gameover){
                disposeRocket(rocket);
                pthread_mutex_unlock(&mx);
                break;
                
            }else{
                if(rocket->row - 1 >= 0){
                    move(rocket->row - 1, rocket->col);
                    addch('^');
                    addch(' ');
                }
            }
            move(LINES-1,COLS-1);
            refresh();		
            pthread_mutex_unlock(&mx);
            rocket->row--;
        }        
        pthread_exit(NULL);
}

/* the function that spawns a new saucer but based on an old thread */
void spawn(struct saucer *info)
{
        /* change the live variable*/
        pthread_mutex_lock(&rk);
        info->live = 0;

        /* restore the saucer's value after it's hit*/
        if(info->hit){
            info->hit = 0;
            pthread_mutex_lock(&mx);
            info->str = "       ";
            move( info->row, info->col );
            addch(' ');
            addstr(info->str);
            addch(' ');
            move(LINES-1, COLS-1);
            refresh();
            pthread_mutex_unlock(&mx);
        }else{
            pthread_mutex_lock(&es);
            escape++;

            /* if escape limit has been reached, then game is over */
            if(limit > 0 && escape >= limit)
                    gameover = 1;
            pthread_mutex_unlock(&es);
            updateStatus();
        }
        
        /* position the saucer back to the left of the screen */
        info->col = 0;
        info->row = rand()%ROWS;
        pthread_mutex_unlock(&rk);
        
        /* let the saucer sleep for a while before it's ready to appear again*/
        usleep(info->delay * (rand()%RANGE) * TUNIT);
    
        /* spawn process is done*/
        pthread_mutex_lock(&rk);
        info->live = 1;
        pthread_mutex_unlock(&rk);
        info->delay = delay + (rand()%RANGE);
        info->str = "<--->";

}

/* vanish function makes the saucer escape the screen character by character*/
int vanish(struct saucer *info, int count)
{
        if (count == 0)
                info->str = "<---";
        else if (count == 1)
                info->str = "<--";
        else if (count == 2)
                info->str = "<-";
        else if (count == 3)
                info->str = "<";
        else if (count == 4)
                info->str = ""; 

        /* move the saucer */
        pthread_mutex_lock(&mx);
        move( info->row, info->col );
        addch(' ');
        addstr( info->str );	
        addch(' ');
        move(LINES-1,COLS-1);
        refresh();		
        pthread_mutex_unlock(&mx);
        
        return ++count;
}

/* move the saucer from left to right */
void moveSaucer(struct saucer *info)
{
        pthread_mutex_lock(&mx);	
        move( info->row, info->col );
        addch(' ');			
        addstr( info->str );	
        addch(' ');			
        move(LINES-1,COLS-1);
        refresh();		
        pthread_mutex_unlock(&mx);
        return;
}


/* the code that runs in each saucer thread */
void *attack(void *arg)
{
	struct saucer *info = arg;    
	int len = strlen(info->str)+2;
        int  vanished = 0;
        
	while( 1 )
	{
            usleep(info->delay*TUNIT);
            
            
            pthread_mutex_lock(&rk);
            if(info->hit)
                    vanished = 5;
            pthread_mutex_unlock(&rk);
            
            if ( info->col + len >= COLS && vanished < 5) 
                    vanished = vanish(info, vanished);
            
            if(vanished >= 5){
                spawn(info);
                vanished = 0;
            }
            
            moveSaucer(info);

            /* move item to next column and check for bouncing	*/  
            pthread_mutex_lock(&rk);
            info->col++;
            pthread_mutex_unlock(&rk);
            
	}
}

/* lock everything to pause the game */
void lockEverything()
{
        pthread_mutex_lock(&mx);
        pthread_mutex_lock(&rk);
        pthread_mutex_lock(&es);
        pthread_mutex_lock(&dc);
        pthread_mutex_lock(&sc);
        pthread_mutex_lock(&lv);
        gamepause = 1;
}

/* unlock every mutex in order to resume the game */
void unlockEverything()
{
        pthread_mutex_unlock(&mx);
        pthread_mutex_unlock(&rk);
        pthread_mutex_unlock(&es);
        pthread_mutex_unlock(&dc);
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&lv);
        gamepause = 0;

}

/* the input thread function that handles user input*/
void *inputHandler(){

        int c;
        int i = 0;

	while(1) {
            
            c = getch();
            
            if ( c == 'Q' ){
                for (i=0; i<NUMS; i++ )
                        pthread_cancel(thrds[i]);
                gameover = 1;
                pthread_exit(NULL);
            }
            
            else if (c == 'P' && !gamepause){
                lockEverything();
                continue;
            }
            else if (c == 'R' && gamepause){
                unlockEverything();
                continue;
            }
            if(gamepause)
                    continue;

           if ( c == '.')
                    moveRight();
            
            else if ( c == ',')
                    moveLeft();

            else if (c == 32){
                struct rocket *rocket = malloc(sizeof(rocket));
                pthread_t *rocket_thread = malloc(sizeof(pthread_t));

                rocket->speed = rocketdelay;
                rocket->row = LINES - 3;
                rocket->col = base.col;

                if ( pthread_create(rocket_thread, NULL, fire, rocket)){
                    fprintf(stderr,"error creating thread");
                    endwin();
                    exit(0);
                }
                
                pthread_mutex_lock(&dc);
                rockets--;
                if(rockets == 0){
                    gameover = 1;
                    pthread_exit(NULL);
                }
                pthread_mutex_unlock(&dc);
                updateStatus();
            }
        }
}

/* function that monitors current game status, to determine whether
 * to stop a game or not
 */
int gameOn(){
        int over = 0;

        /* process user input*/
	while(1) {
            
            if(gameover){
                pthread_cancel(inputThread);
                return 1;
            }

            pthread_mutex_lock(&sc);
            pthread_mutex_lock(&lv);
            if(level < 6 && score >= requiredScore){
                level++;
                done = 0;
                erase();
                refresh();
                pthread_cancel(inputThread);
                break;
            }
            pthread_mutex_unlock(&sc);
            pthread_mutex_unlock(&lv);

	}

        /* when the code breaks from while loop, these three
         * mutexes may be still locked. So doing the unlock
         * steps just to be safe.
         */
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&lv);

        return over;
}

/* the function that is respsonsible for recording new highscore*/
void recordHighscore(){
        char* name = "default";
        struct highscore highscore;

        highscore = populate();

        if(highscore.count ==  0)
                mvprintw(7, 0, "No Highscores!");
        
        if(highscore.count > 0){
            mvprintw(7,0,"%40s", "***       HIGH SCORE      ***\n");
            mvprintw(8,0,"%20s | %20s\n", "User Name", "Score");
            mvprintw(9,0,"%20s | %20s\n", highscore.p1, highscore.s1);
        }

        if(highscore.count > 1)
                mvprintw(10,0,"%20s | %20s\n", highscore.p2, highscore.s2);

        if(highscore.count > 2)
                mvprintw(11,0,"%20s | %20s\n", highscore.p3, highscore.s3);

        refresh();

        /* prompting for user name*/
        if(highscore.count == 0 || score >= lowestHighscore()){
                mvprintw(12,0,"You score is %d. Please Enter Your username "
                         "(Max 20 characters):", score);
                refresh();
                name = malloc(20);
                mvscanw(13,0,"%20s", name);
                writeNewHighscore(score, name);
                endwin();
                
        }
}

int main()
{
	int c;		
	int num_msg ;
	int i;
        int over;

        /* user menu */
        printUserMenu();

        /* handle user choice for user menu*/
        while((c = getchar()) != EOF){
            if(c == 'Q') 
                    return 0;
            else if(c == 'S') 
                    break;
            else if(c == 'H')
                    printHighscore();
            else if(c == 'I')
                    printInstruction();
        }
        
        /* game phase*/
        while(1){

            /* on level 1, do setup(), otherwise do levelup()*/
            if(level == 1 && !done){
                num_msg = setup(saucer);
             
            }else if(level > 1 && level < 6 && !done){
                num_msg = levelup(saucer);
            }
            
            /* create saucer thread */
            for(i=0 ; i<num_msg; i++)
                    if ( pthread_create(&thrds[i], NULL, attack, &saucer[i])){
                        fprintf(stderr,"error creating thread");
                            endwin();
                            exit(0);
                    }

            /* create input thread */
            pthread_create(&inputThread, NULL, inputHandler, NULL);
            
            /* gameOn is a function that monitors if the game could levelup or 
             * the game should stop
             */
            over = gameOn();

            /* cancel all saucer threads*/
            for (i=0; i<num_msg; i++ )
                    pthread_cancel(thrds[i]);
            
            erase();
            refresh();

            pthread_mutex_unlock(&mx);
            if(over)
                    break;
        }
    
        erase();
        refresh();
        endwin();

        pthread_mutex_lock(&mx);
        initscr();
	crmode();
	noecho();
	clear();
        refresh();
        mvprintw(0, 0,"  ####    ##   #    # ######     ####  #    # ######"
                " ##### \n"    
                " #    #  #  #  ##  ## #         #    # #    # #      "
                "#    #  \n"  
                " #      #    # # ## # #####     #    # #    # #####  "
                "#    #  \n"  
                " #  ### ###### #    # #         #    # #    # #      "
                "#####   \n"  
                " #    # #    # #    # #         #    #  #  #  #      "
                "#   #   \n"  
                "  ####  #    # #    # ######     ####    ##   ###### "
                "#    #  \n");
        refresh();
        recordHighscore();
        pthread_mutex_unlock(&mx);
}
