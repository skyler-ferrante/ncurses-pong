#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
void print_win_params(WIN *p_win);
void create_box(WIN *win, bool flag);
void bounce_ball(WIN *win);

const static int STICK_HEIGHT = 12;
const static int STICK_WIDTH = 1;

int main(int argc, char *argv[])
{	
	WIN lstick;
	WIN rstick;
	WIN ball;
	
	initscr(); 	//Starts ncurses		
	start_color();	//Starts color
	cbreak();	//So we don't need to press enter	

	keypad(stdscr, TRUE);	//So we can get F1	
	noecho();		//Stops things that the user types from showing up
	nodelay(stdscr,TRUE);	//Stops ncurses from pausing (for when we use getch())
	curs_set(0);		//Makes the cursor blank

	init_pair(1, COLOR_CYAN, COLOR_BLACK);	 //Color for text
	init_pair(2, COLOR_BLUE, COLOR_BLACK);	 //Color for sticks
	init_pair(3, COLOR_RED, COLOR_BLACK); 	 //Color for ball
	init_pair(4, COLOR_YELLOW, COLOR_BLACK); //Color for ball touching sticks

	init_stick(&lstick); //Initialize left stick
	lstick.startx = lstick.startx/8; //Put left stick on the left side

	init_stick(&rstick);  //Initialize right stick
	rstick.startx = rstick.startx*.875; //Put right stick on the left side

	init_ball(&ball); //Initialize ball

	attron(COLOR_PAIR(1)); //Sets color to cyan
	printw("Press F1 to exit"); 
	refresh();
	attroff(COLOR_PAIR(1));

	create_box(&lstick, TRUE); //Puts sprites on the screen
	create_box(&rstick, TRUE);
	create_box(&ball,   TRUE);

	int ball_velocity_x = 1;
	int ball_velocity_y = 1;
	
	int ch; //For user input in while loop
	int bounces = 0; //Amount of bounces
	while((ch = getch()) != KEY_F(1)) //Runs until user wants to quit (F1)
	{	
		usleep(30000); //Pause (in microsecs)
		
		create_box(&lstick,FALSE); //Clears sticks from the screen
		create_box(&rstick,FALSE);
		
		switch(ch){
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

		//Stops sticks from going off screen/
		////////////////////////////////////////////
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
		////////////////////////////////////////////

		//Stops ball from going off screen (Vertical)
		//////////////////////////////////////////
		if(ball.starty > LINES-3){
			ball.starty = LINES-3;
			ball_velocity_y = -ball_velocity_y;
		}
		if(ball.starty < 0){
			ball.starty = 0;
			ball_velocity_y = -ball_velocity_y;
		}
		//////////////////////////////////////////

		//Bounce ball off of sticks
		////////////////////////////////////////////////////////////////////////////	
		if((abs((lstick.startx+STICK_WIDTH-1)-ball.startx)<3 //Check if in the same x plane
		&& lstick.starty<ball.starty && (lstick.starty+STICK_HEIGHT)>ball.starty) //Check if in the same y plane
 		|| (abs((rstick.startx-1)-ball.startx )<3
		&& rstick.starty<ball.starty && (rstick.starty+STICK_HEIGHT)>ball.starty))
		{
			
			ball.colorp = 4;
			create_box(&ball,TRUE);
			refresh();
			usleep(100000);
			ball.colorp = 3;
			
			if(ball.startx>COLS/2){ //See which half we are on, just to make sure if collide with the ball twice
				ball_velocity_x = 1;
			}else{
				ball_velocity_x = -1;
			}
			bounces++;
		}		
		////////////////////////////////////////////////////////////////////////////	

		//Check if ball went off screen (Horizontal)
		//////////////////////////////////////////
		if(ball.startx<0){
			endwin();
			printf("Right Player won\n");
			printf("%i bounces \n",bounces);
			return 0;
		}
		else if(ball.startx>COLS){
			endwin();
			printf("Left Player won\n");
			printf("%i bounces \n",bounces);
			return 0;
		}
		//////////////////////////////////////////
	}
	endwin();	
	return 0;
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

void create_box(WIN *p_win, bool flag)
{	int i, j;
	int x, y, w, h;

	x = p_win->startx;
	y = p_win->starty;
	w = p_win->width;
	h = p_win->height;


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
	
	//Getch also refresh's so we get flickering from calling refresh multiple times a frame
	//refresh();
}
