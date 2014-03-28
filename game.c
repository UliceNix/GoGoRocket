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
                return 50;
        default:
                return 10;
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
                return 1;
        }
}

int levelUpLimit(int level)
{
        switch (level){
        case 1:
                return 60;
        case 2:
                return 160;
        case 3:
                return 280;
        case 4:
                return 340;
        default:
                return 9999;
        }
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
            saucer[i].row = rand()%5;		
            saucer[i].col = 0;
            saucer[i].hit = 0;
            saucer[i].delay = 15 + (rand()%15);	
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
	return NUMS;
}
/*
int levelup(struct saucer saucer[])
{
        srand(getpid());
        for(int i = 0; i < NUMS; i++){
        
        }
}*/

void moveRocket(struct rocket *rocket)
{
        pthread_mutex_lock(&mx);
        move( rocket->row, rocket->col);
        addch(' ');
        addch(' ');
        if(rocket->row - 1 >= 0){
            move(rocket->row - 1, rocket->col);
            addch('^');
            addch(' ');
        }
        pthread_mutex_unlock(&mx);
        return;
}

void disposeRocket(struct rocket *rocket)
{
        move( rocket->row, rocket->col);
        addch(' ');
        move( rocket->row - 1, rocket->col);
        addch(' ');
        rocket->row = -1;
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

            if(rocket->row < 3){
                for(int i = 0; i < NUMS; i++){
                    pthread_mutex_lock(&rk);
                    if(saucer[i].row == rocket->row 
                       && rocket->col >= saucer[i].col - 1 
                       && rocket->col <= 
                       saucer[i].col + strlen(saucer[i].str) - 1){
                        saucer[i].hit = 1;
                        dispose = 1;
                        countHits++;
                    }
                    pthread_mutex_unlock(&rk); 
                }
            }

            if(countHits)
                    hitReward(countHits);

            pthread_mutex_lock(&mx);
            if(dispose)
                    disposeRocket(rocket);
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
        info->row = rand()%5;
        pthread_mutex_unlock(&rk);
        
        usleep(info->delay * (rand()%20) * TUNIT);
        
        info->str = "<--->";
        info->delay = 15 + (rand()%15);

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
            
            if(vanished == 5){
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

void gameOn(){
	int c;

	/* process user input */
	while(1) {

            pthread_mutex_lock(&dc);
            pthread_mutex_lock(&es);
            if(rockets == 0 || escape >= limit)
                    break;
            pthread_mutex_unlock(&es);
            pthread_mutex_unlock(&dc);
  
            c = getch();

            if ( c == 'Q' ) break;

            else if ( c == '.'){
                moveRight();

            }else if ( c == ','){
                moveLeft();

            }else if (c == 32){
                struct rocket *rocket = malloc(sizeof(rocket));
                pthread_t *rocket_thread = malloc(sizeof(pthread_t));

                rocket->speed = 8;
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


}
int main()
{
	int c;		
	int num_msg ;
	int i;

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

        if(level == 1){
            num_msg = setup(saucer);
            /* create all the threads */
            for(i=0 ; i<num_msg; i++)
                    if ( pthread_create(&thrds[i], NULL, attack, &saucer[i])){
                        fprintf(stderr,"error creating thread");
                        endwin();
                        exit(0);
                    }
            gameOn();
            /* cancel all the threads */
            pthread_mutex_lock(&mx);
            for (i=0; i<num_msg; i++ )
                    pthread_cancel(thrds[i]);
            
            erase();
            refresh();
        }

	endwin();
        printf( "  ####    ##   #    # ######     ####  #    # ###### ##### \n"    
                " #    #  #  #  ##  ## #         #    # #    # #      #    #  \n"  
                " #      #    # # ## # #####     #    # #    # #####  #    #  \n"  
                " #  ### ###### #    # #         #    # #    # #      #####   \n"  
                " #    # #    # #    # #         #    #  #  #  #      #   #   \n"  
                "  ####  #    # #    # ######     ####    ##   ###### #    #  \n");
	return 0;
}
