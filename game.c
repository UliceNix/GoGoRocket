/*
 * tanimate.c: animate several strings using threads, curses, usleep()
 *
 *	bigidea one thread for each animated string
 *		one thread for keyboard control
 *		shared variables for communication
 *	compile	cc tanimate.c -lcurses -lpthread -o tanimate
 *	
 *      to do   [functionalities]
 *              needs locks for shared variables
 *	        to increase escapes variable
 *              develop levels 
 *              [Format and Documents]
 *              User documentation
 *              Format, and divide this file into some .h files.
 *              
 *              
 */

#include	<stdio.h>
#include	<curses.h>
#include	<pthread.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include         "game.h"


int getLimit(int level)
{
        switch (level){
        case 2:
                return 15;
        case 3:
                return 25;
        case 4:
                return 35;
        case 5:
                return 45;
        default:
                return -1;
        }
}

int getReward(int level)
{
        switch (level){
        case 2:
                return 2;
        case 3:
                return 3;
        case 4:
                return 5;
        case 5:
                return 10;
        default:
                return 8;
        }
}

int levelUpLimit(int level)
{
        switch (level){
        case 2:
                return 160;
        case 3:
                return 280;
        case 4:
                return 340;
        default:
                return 1000;
        }
}

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

void updateSetting(int level)
{
        limit = getLimit(level);
        reward = getReward(level);
        requiredScore = levelUpLimit(level);
        delay = getDelay(level);
        rocketdelay = getRocketDelay(level);
}


void printInstruction()
{
        printf("This is a shooting game\n");
        return;
}

void printUserMenu()
{
        printf("Welcome to kill-that-saucer game\n");
        printf("================================\n");
        printf("|    Start Game (S)            |\n");
        printf("+------------------------------+\n");
        printf("|        Quit   (Q)            |\n");
        printf("================================\n");
        return;
}

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

void moveLeft()
{
        if(base.col -1  < 0)
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

void updateStatus()
{
        pthread_mutex_lock(&dc);
        pthread_mutex_lock(&sc);
        pthread_mutex_lock(&es);
        pthread_mutex_lock(&mx);
        mvprintw(LINES-1,
                 0,
                 "Quit(Q), Left(,), Right(.), Fire(SPACE)." 
                 " Rockets: %d Score:%d Escape:%d",
                 rockets, score, escape);
        refresh();
        pthread_mutex_unlock(&mx);
        pthread_mutex_unlock(&es);
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&dc);
        return;
}

int setup(struct saucer saucer[])
{
	/* assign rows and velocities to each string */
	srand(getpid());
	for(int i=0 ; i<NUMS; i++){
            saucer[i].str = "<--->";
            saucer[i].row = rand()%ROWS;		
            saucer[i].col = 0;
            saucer[i].hit = 0;
            saucer[i].delay = 15 + (rand()%RANGE);	
            saucer[i].dir = 1;
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
	mvprintw(LINES-1,
                 0,
                 "Quit(Q), Left(,), Right(.), Fire(SPACE)."
                 " Rockets: 10 Score:0 Escape:0");
        base.col = (COLS+1)/2; 
        done = 1;

	return NUMS;
}


int levelup(struct saucer saucer[])
{
        srand(getpid());
        
        /* Since when doing the levelup process, all the threads that 
         * executes gameOn() are all terminated. There will not be any
         * processes modifying the level variable.
         */
        updateSetting(level);
        
        for(int i=0 ; i<NUMS; i++){
            saucer[i].str = "<--->";
            saucer[i].row = rand()%ROWS;		
            saucer[i].col = 0;
            saucer[i].hit = 0;
            saucer[i].delay = delay + (rand()%RANGE);	
            saucer[i].dir = 1;
	}

        pthread_mutex_lock(&mx);
        erase();
        move(0, 0);
        refresh();
        mvprintw(1,0," #       ####### #     # ####### #          #     # ######");  
        mvprintw(2,0," #       #       #     # #       #          #     # #     #");
        mvprintw(3,0,      " #       #       #     # #       #          #     # #     #");
        mvprintw(4,0," #       #####   #     # #####   #          #     # ######");
        mvprintw(5,0," #       #        #   #  #       #          #     # #"); 
        mvprintw(6,0," #       #         # #   #       #          #     # # ");  
        mvprintw(7,0," ####### #######    #    ####### #######     #####  #");
        mvprintw(8,0,"LEVEL %d. MAX ESCAPE: %d. THE SCORE TO THE NEXT LEVEL: %d"
                 , level, limit, requiredScore);

        if(level == 6){
            mvprintw(9, 0, "WELCOME TO LEVEL 6. LOOKS LIKE NO ONE HAS EVER"
                     " MADE IT HERE! CONGRATS! ");
            mvprintw(10, 0, "THERE WILL BE NO LEVELS ANY MORE"
                     ". LEVEL 6 IS A FREE PLAY MODE. THE GAME WILL ONLY"
                     " TERMINATE AS SOON AS YOU RUN OF OUT ROCKETS.");
        }

        refresh();
        sleep(4);

        erase();
        refresh();

        move(LINES-2, ((COLS+1)/2));
        addch(' ');
        addch('|');
        addch(' ');
        move(LINES-1, COLS-1);
        refresh();
	mvprintw(LINES-1,
                 0,
                 "Quit(Q), Left(,), Right(.), Fire(SPACE)."
                 " Rockets: %d Score:%d Escape:%d", rockets, score, escape);
        base.col = (COLS+1)/2; 
        done = 1;
        pthread_mutex_unlock(&mx);
        return NUMS;
        }

void moveRocket(struct rocket *rocket)
{
        pthread_mutex_lock(&mx);
        move( rocket->row, rocket->col);
        addch(' ');
        //addch(' ');
        /*
        if(rocket->row - 1 >= 0){
            move(rocket->row - 1, rocket->col);
            addch('^');
            addch(' ');
            }*/
        refresh();
        pthread_mutex_unlock(&mx);
        return;
}

void disposeRocket(struct rocket *rocket)
{
        move( rocket->row, rocket->col);
        addch(' ');
        
        move( rocket->row - 1, rocket->col);
        addch(' ');
        //rocket->row = -1;
        return;
}

void hitReward(int countHits){
        pthread_mutex_lock(&sc);
        score += countHits * countHits;
        pthread_mutex_unlock(&sc);
        pthread_mutex_lock(&dc);
        rockets += reward * countHits;
        pthread_mutex_unlock(&dc);
        updateStatus();
        return;
}

void *fire(void *arg)
{
        struct rocket *rocket = arg;
        int dispose = 0;
        int countHits = 0;

        while (rocket->row >= 0){
            usleep(rocket->speed*TUNIT);
            moveRocket(rocket);
            
            if(rocket->row < ROWS){
                pthread_mutex_lock(&rk);
                for(int i = 0; i < NUMS; i++){
                    if(saucer[i].row == rocket->row 
                       && rocket->col >= saucer[i].col - 1 
                       && rocket->col <= 
                       saucer[i].col + strlen(saucer[i].str) - 1){
                        saucer[i].hit = 1;
                        dispose = 1;
                        countHits++;
                    }
                }
                pthread_mutex_unlock(&rk); 
            }

            if(countHits)
                    hitReward(countHits);

            pthread_mutex_lock(&mx);
            if(dispose)
                    disposeRocket(rocket);
            else{
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

void spawn(struct saucer *info)
{
        pthread_mutex_lock(&rk);

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
            pthread_mutex_unlock(&es);
            updateStatus();
        }
        
        info->col = 0;
        info->row = rand()%ROWS;
        pthread_mutex_unlock(&rk);
        
        usleep(info->delay * (rand()%RANGE) * TUNIT);
        
        info->str = "<--->";
        info->delay = delay + (rand()%RANGE);

}

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


/* the code that runs in each thread */
void *attack(void *arg)
{
	struct saucer *info = arg;    
	int len = strlen(info->str)+2;
        int  vanished = 0;

	while( 1 )
	{
            usleep(info->delay*TUNIT);

            pthread_mutex_lock(&rk);
            if(info->hit > 0)
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

void unlockEverything(){
        pthread_mutex_unlock(&mx);
        pthread_mutex_unlock(&rk);
        pthread_mutex_unlock(&es);
        pthread_mutex_unlock(&dc);
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&lv);
        gamepause = 0;

}

int gameOn(){
	int c;
        int over = 0;
	/* process user input */
	while(1) {
            
            c = getch();
            
            if ( c == 'Q' ) break;
            
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
            

            pthread_mutex_lock(&dc);
            pthread_mutex_lock(&es);
            if(rockets == 0 || (limit > 0 && escape >= limit)){
                over = 1;
                break;
            }

            pthread_mutex_unlock(&es);
            pthread_mutex_unlock(&dc);

            
            pthread_mutex_lock(&sc);
            pthread_mutex_lock(&lv);
            if(level < 6 && score >= requiredScore){
                level++;
                done = 0;
                break;
            }
            pthread_mutex_unlock(&sc);
            pthread_mutex_unlock(&lv);

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
                pthread_mutex_unlock(&dc);
                updateStatus();
            }

	}

        /* When the code breaks from while loop, these three
         * mutexes may be still locked. So doing the unlock
         * steps just to be safe.
         */
        pthread_mutex_unlock(&es);
        pthread_mutex_unlock(&dc);
        pthread_mutex_unlock(&sc);
        pthread_mutex_unlock(&lv);

        return over;
}

int main()
{
	int c;		
	int num_msg ;
	int i;
        int over;

        printUserMenu();
        printInstruction();

        while((c = getchar()) != EOF){
            if(c == 'Q') 
                    return 0;
            else if(c == 'S') 
                    break;
            else{
                    printf("Please try again!\n");
            }
        }
        
        /* SINGLE LEVEL HERE:
        num_msg = setup(saucer);
        for(i=0 ; i<num_msg; i++)
                if ( pthread_create(&thrds[i], NULL, attack, &saucer[i])){
                    fprintf(stderr,"error creating thread");
                    endwin();
                        exit(0);
                }
        over = gameOn();
        pthread_mutex_lock(&mx);
        for (i=0; i<num_msg; i++ )
                pthread_cancel(thrds[i]);
        */
        
        while(1){

            if(level == 1 && !done){
                num_msg = setup(saucer);

                
            }else if(level > 1 && level < 6 && !done){
                num_msg = levelup(saucer);
            }

            for(i=0 ; i<num_msg; i++)
                    if ( pthread_create(&thrds[i], NULL, attack, &saucer[i])){
                        fprintf(stderr,"error creating thread");
                            endwin();
                            exit(0);
                    }
            over = gameOn();

            for (i=0; i<num_msg; i++ )
                    pthread_cancel(thrds[i]);
            
            erase();
            refresh();

            if(over)
                    break;
        }
         
        erase();
        refresh();
	endwin();
        printf( "  ####    ##   #    # ######     ####  #    # ###### ##### \n"    
                " #    #  #  #  ##  ## #         #    # #    # #      #    #  \n"  
                " #      #    # # ## # #####     #    # #    # #####  #    #  \n"  
                " #  ### ###### #    # #         #    # #    # #      #####   \n"  
                " #    # #    # #    # #         #    #  #  #  #      #   #   \n"  
                "  ####  #    # #    # ######     ####    ##   ###### #    #  \n");
	return 0;
}
