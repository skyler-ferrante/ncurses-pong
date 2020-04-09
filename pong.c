#include <ncurses.h>
#include<stdlib.h>
#include <unistd.h>

typedef struct _win_border_struct {
	chtype 	ls, rs, ts, bs, 
	 	tl, tr, bl, br;
}WIN_BORDER;

typedef struct _WIN_struct {

	int startx, starty;
	int height, width;
	int colorp;
	WIN_BORDER border;
}WIN;

void init_stick(WIN *p_win);
void init_ball(WIN *p_win);
void init_middleline(WIN *p_win);
void print_win_params(WIN *p_win);
void create_box(WIN *win, bool flag);
void bounce_ball(WIN *win);
void end_message(int lscore,int rscore,int bounces);

const static int STICK_HEIGHT = 12;
const static int STICK_WIDTH = 1;

int main(int argc, char *argv[])
{
	WIN lstick; //Declaring sprites
	WIN rstick;
	WIN ball;
	WIN line;

	initscr(); 			
	start_color();	
	cbreak();		
	keypad(stdscr, TRUE);		
	noecho();		
	curs_set(0);	

	init_pair(1, COLOR_CYAN, COLOR_BLACK);	 //Color for text
	init_pair(2, COLOR_BLUE, COLOR_BLACK);	 //Color for sticks
	init_pair(3, COLOR_RED, COLOR_BLACK); 	 //Color for ball

	attron(COLOR_PAIR(1));
	printw("Press any key to play\n");
	printw("By Skyler Ferrante");
	attroff(COLOR_PAIR(1));

	attron(COLOR_PAIR(2)); //Spells out pong, http://patorjk.com/software/taag/#p=display&f=Univers&t=PONG
	mvprintw(LINES/2,COLS/2-27.5,"88888888ba     ,ad8888ba,    888b      88    ,ad8888ba,\n");   
	mvprintw(LINES/2+1,COLS/2-27.5,"88      \"8b   d8\"'    `\"8b   8888b     88   d8\"'    `\"8b\n");  
	mvprintw(LINES/2+2,COLS/2-27.5,"88      ,8P  d8'        `8b  88 `8b    88  d8'          \n");  
	mvprintw(LINES/2+3,COLS/2-27.5,"88aaaaaa8P'  88          88  88  `8b   88  88            \n"); 
	mvprintw(LINES/2+4,COLS/2-27.5,"88\"\"\"\"\"\"\"    88          88  88   `8b  88  88      88888 \n"); 
	mvprintw(LINES/2+5,COLS/2-27.5,"88           Y8,        ,8P  88    `8b 88  Y8,        88 \n"); 
	mvprintw(LINES/2+6,COLS/2-27.5,"88            Y8a.    .a8P   88     `8888   Y8a.    .a88 \n"); 
	mvprintw(LINES/2+7,COLS/2-27.5,"88             `\"Y8888Y\"'    88      `888    `\"Y88888P\" \n "); 
	attroff(COLOR_PAIR(1));

	getch();
	clear();
	nodelay(stdscr,TRUE);	

	init_stick(&lstick); //Initialize left stick
	lstick.startx = lstick.startx*.125; //Put left stick on the left side

	init_stick(&rstick); 
	rstick.startx = rstick.startx*.875;	

	init_ball(&ball);
	
	init_middleline(&line);

	attron(COLOR_PAIR(1)); 
	printw("Press F1 to exit"); 
	refresh();
	attroff(COLOR_PAIR(1));

	int ball_velocity_x = 1;
	int ball_velocity_y = 1;
	
	int rscore = 0;
	int lscore = 0;

	int ch; //For user input in while loop
	int bounces = 0; //Amount of bounces
	while((ch = getch()) != KEY_F(1))
	{	
		create_box(&line,TRUE); //Adds middle line to the screen
		
		create_box(&lstick,FALSE); //Clears sticks from the screen
		create_box(&rstick,FALSE);
		
		switch(ch){ //Updates sticks position
			case KEY_F(1):
				printf("User pressed F1 \n");
				endwin();
				return 0;
			case 119: //W
				lstick.starty-=3; //Moves left stick up
				break;
			case 115: //S
				lstick.starty+=3;
				break;
			case KEY_UP: //Arrow key up
				rstick.starty-=3; //Moves right stick up
				break;
			case KEY_DOWN:
				rstick.starty+=3;
				break;
		}
		create_box(&lstick,TRUE); //Creates sticks with new position	
		create_box(&rstick,TRUE);

		create_box(&ball,FALSE); //Clears box
		ball.startx -= ball_velocity_x;//Calculates balls new position
		ball.starty -= ball_velocity_y;
		create_box(&ball,TRUE); //Creates box in new position

		//Stops sticks from going off screen
		if(lstick.starty > LINES-STICK_HEIGHT){
			lstick.starty = LINES-STICK_HEIGHT;
		}
		if(lstick.starty < 0){
			lstick.starty = 0;
		}
		
		if(rstick.starty > LINES-STICK_HEIGHT){
			rstick.starty = LINES-STICK_HEIGHT;
		}
		if(rstick.starty < 0){
			rstick.starty = 0;
		}

		//Stops ball from going off screen (Vertical)
		if(ball.starty > LINES-3){
			ball.starty = LINES-3;
			ball_velocity_y = -ball_velocity_y;
		}
		if(ball.starty < 0){
			ball.starty = 0;
			ball_velocity_y = -ball_velocity_y;
		}
		
		//Bounce ball off of sticks
		if((abs((lstick.startx+STICK_WIDTH-1)-ball.startx)<3 //Check if in the same x plane
		&& lstick.starty<ball.starty && (lstick.starty+STICK_HEIGHT)>ball.starty) //Check if in the same y plane
 		|| (abs((rstick.startx-1)-ball.startx )<3
		&& rstick.starty<ball.starty && (rstick.starty+STICK_HEIGHT)>ball.starty))
		{
			//See which half we are on, just to make sure we don't collide with the ball twice
			ball_velocity_x = (ball.startx>COLS/2) ? 1 : -1;
			bounces++;
		}		

		//Check if ball went off screen (Horizontal)
		if(ball.startx<1){
			attron(COLOR_PAIR(1));
			lscore++;
			mvprintw(1,COLS-1,"%i",lscore);
			attroff(COLOR_PAIR(2));
			create_box(&ball,FALSE);
			ball.startx = COLS/2;
			ball.starty = LINES/2;
			if(lscore>=5){
				end_message(lscore,rscore,bounces);
				return 0;
			}		
		}
		else if(ball.startx>COLS){
			attron(COLOR_PAIR(1));
			rscore++;
			mvprintw(1,1,"%i",rscore);
			attroff(COLOR_PAIR(2));
			ball.startx = COLS/2;
			ball.starty = LINES/2;
			if(rscore>=5){
				end_message(lscore,rscore,bounces);
				return 0;
			}		
		}
		usleep(30000);
	}
	clear();
	printw("F1 was pressed");
	usleep(10000);
	end_message(lscore,rscore,bounces);
	return 0;
}

void end_message(int lscore,int rscore,int bounces){
	endwin();
	if(lscore==rscore){
		printf("Tie game\n");
	}else if(lscore>rscore){
		printf("Left player wins\n");
	}else{
		printf("Right player wins\n");
	}
	printf("Score was %i-%i\n",lscore,rscore);
	printf("%i bounces\n",bounces);
}

void init_stick(WIN *p_win)
{
	p_win->height = STICK_HEIGHT;
	p_win->width = STICK_WIDTH;
	p_win->starty = (LINES - p_win->height)/2;	
	p_win->startx = (COLS - p_win->width);
	p_win->colorp = 2;

	p_win->border.ls = '|';
	p_win->border.rs = '|';
	p_win->border.ts = '-';
	p_win->border.bs = '-';
	p_win->border.tl = '+';
	p_win->border.tr = '+';
	p_win->border.bl = '+';
	p_win->border.br = '+';
}

void init_ball(WIN *p_win)
{
	p_win->height = 2;
	p_win->width = 2;
	p_win->starty = (LINES - p_win->height)/2;	
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 3;

	p_win->border.ls = '|';
	p_win->border.rs = '|';
	p_win->border.ts = '-';
	p_win->border.bs = '-';
	p_win->border.tl = '+';
	p_win->border.tr = '+';
	p_win->border.bl = '+';
	p_win->border.br = '+';
}

void init_middleline(WIN *p_win)
{
	p_win->height = LINES;
	p_win->width = 2;
	p_win->starty = (LINES - p_win->height)/2;	
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 3;

	p_win->border.ls = '|';
	p_win->border.rs = '|';
	p_win->border.ts = ' ';
	p_win->border.bs = ' ';
	p_win->border.tl = ' ';
	p_win->border.tr = ' ';
	p_win->border.bl = ' ';
	p_win->border.br = ' ';
}
void create_box(WIN *p_win, bool flag)
{	int i, j;
	int x, y, w, h;

	x = p_win->startx;
	y = p_win->starty;
	w = p_win->width;
	h = p_win->height;

	//If flag is true, print p_win, else clear p_win
	if(flag == TRUE)
	{	
		attron(COLOR_PAIR(p_win->colorp));
		mvaddch(y, x, p_win->border.tl);
		mvaddch(y, x + w, p_win->border.tr);
		mvaddch(y + h, x, p_win->border.bl);
		mvaddch(y + h, x + w, p_win->border.br);
		mvhline(y, x + 1, p_win->border.ts, w - 1);
		mvhline(y + h, x + 1, p_win->border.bs, w - 1);
		mvvline(y + 1, x, p_win->border.ls, h - 1);
		mvvline(y + 1, x + w, p_win->border.rs, h - 1);
		attron(COLOR_PAIR(p_win->colorp));
	}else{
		for(j = y; j <= y + h; ++j)
			for(i = x; i <= x + w; ++i)
				mvaddch(j, i, ' ');			
	}
	//Getch acts as a refresh, so we do not refresh here to prevent flickering
}
