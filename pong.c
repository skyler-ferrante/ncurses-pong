#include <ncurses.h>
#include<stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

typedef struct _win_border_struct {
	chtype 	ls, rs, ts, bs,
	 	tl, tr, bl, br;
}WIN_BORDER;

typedef struct _WIN_struct {

	double startx, starty;
	int height, width;
	int colorp;
	WIN_BORDER border;
}WIN;

typedef struct pong_game_struct {
	float ball_velocity_x, ball_velocity_y;
	int rscore;
	int lscore;
	int bounces;
	bool is_done;
	WIN lstick;
	WIN rstick;
	WIN ball;
	WIN middle_line;
	WIN outer_line;
}PONG_GAME;

void init_ncurses(int argc, char **argv);
void print_intro();
void init_game(PONG_GAME *game);
void draw_screen(PONG_GAME *game);
void clear_shapes(PONG_GAME *game);
void update_sticks(PONG_GAME *game,int ch);
void update_ball(PONG_GAME *game);

void end_message(int lscore,int rscore,int bounces);

const int STICK_HEIGHT = 12;
int STICK_WIDTH = 1;
const double BALL_START_SPEED_X = .5;
const double BALL_START_SPEED_Y = .15;

int main(int argc, char **argv)
{
	init_ncurses(argc,argv);
	print_intro();

	PONG_GAME game;
	init_game(&game);

	int ch; //For user input in while loop
	while((ch = getch()) != 27 && !game.is_done)
	{
		if(ch != (int)'p'){
			clear_shapes(&game);
			update_sticks(&game,ch);
			update_ball(&game);
			draw_screen(&game);
			usleep(6000);
		}else{
			while((ch = getch()) != (int)'p'){}
		}
	}
	clear();
	printf("ESC was pressed\n");
	end_message(game.lscore,game.rscore,game.bounces);
	return 0;
}

void init_ncurses(int argc,char *argv[]){
	initscr();
	start_color();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	int c;
	while((c = getopt(argc,argv,"123")) != -1){
		switch(c){
			case '2':
				init_pair(1, COLOR_WHITE, COLOR_GREEN);	 //Color for text
				init_pair(2, COLOR_WHITE, COLOR_WHITE);	 //Color for ball
				init_pair(3, COLOR_WHITE, COLOR_WHITE);	 //Color for sticks
				init_pair(4, COLOR_WHITE, COLOR_WHITE);  //Color for ball
				init_pair(5, COLOR_WHITE, COLOR_GREEN);  //Color for background
				return;
			case '3':
				init_pair(1, COLOR_WHITE, COLOR_BLUE);	 //Color for text
				init_pair(2, COLOR_WHITE, COLOR_WHITE);	 //Color for ball
				init_pair(3, COLOR_WHITE, COLOR_WHITE);	 //Color for sticks
				init_pair(4, COLOR_WHITE, COLOR_WHITE);  //Color for ball
				init_pair(5, COLOR_WHITE, COLOR_BLUE);  //Color for background
				return;
		}
	}
	init_pair(1, COLOR_CYAN, COLOR_BLACK);	 //Color for text
	init_pair(2, COLOR_CYAN, COLOR_BLACK);	 //Color for text
	init_pair(3, COLOR_BLUE, COLOR_BLACK);	 //Color for sticks
	init_pair(4, COLOR_RED, COLOR_BLACK); 	 //Color for ball
}

void print_intro(){
	attron(COLOR_PAIR(1));
	printw("(W/S for left player, key up/down for right player, p for pause)\n");
	printw("Press any key to start\n");
	printw("By Skyler Ferrante");

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
	nodelay(stdscr,TRUE); //I wanted to put this in init_ncurses but then we could not pause during this function	
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


void init_border(WIN *p_win){
	p_win->border.ls = '|';
	p_win->border.rs = '|';
	p_win->border.ts = '-';
	p_win->border.bs = '-';
	p_win->border.tl = '+';
	p_win->border.tr = '+';
	p_win->border.bl = '+';
	p_win->border.br = '+';
}

void init_stick(WIN *p_win)
{
	p_win->height = STICK_HEIGHT;
	p_win->width = STICK_WIDTH;
	p_win->starty = (LINES - p_win->height)/2;	
	p_win->startx = (COLS - p_win->width);
	p_win->colorp = 3;
	init_border(p_win);
}

void init_ball(WIN *p_win)
{
	p_win->height = 2;
	p_win->width = 2;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 2;
	init_border(p_win);
}

void init_middle_line(WIN *p_win)
{
	p_win->height = LINES+2;
	p_win->width = 2;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 4;
	init_border(p_win);
}

void init_outer_line(WIN *p_win)
{
	p_win->height = LINES-1;
	p_win->width = COLS-1;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 4;
	init_border(p_win);
}

void init_game(PONG_GAME *game){
	game->lscore = 0;
	game->rscore = 0;
	game->bounces = 0;
	game->is_done = false;

	init_stick(&game->lstick);
	game->lstick.startx = game->lstick.startx*.125;

	init_stick(&game->rstick);
	game->rstick.startx = game->rstick.startx*.875;

	init_ball(&game->ball);

	init_middle_line(&game->middle_line);

	init_outer_line(&game->outer_line);

	game->ball_velocity_x = BALL_START_SPEED_X;
	game->ball_velocity_y = BALL_START_SPEED_Y;
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
		attroff(COLOR_PAIR(p_win->colorp));
	}else{
		attron(COLOR_PAIR(5));
		for(j = y; j <= y + h; ++j)
			for(i = x; i <= x + w; ++i)
				mvaddch(j, i, ' ');
		attroff(COLOR_PAIR(5));
	}
	//Getch acts as a refresh, so we do not refresh here to prevent flickering
}

void update_screen_sise(PONG_GAME *game){
	game->middle_line.height = LINES;
	game->outer_line.height = LINES;
	game->outer_line.height = COLS;
}

void draw_screen(PONG_GAME *game){
	attron(COLOR_PAIR(1));
	mvprintw(1,2,"Press ESC to exit");
	mvprintw(2,2,"%i",game->lscore);
	mvprintw(2,COLS-2,"%i",game->rscore);
	attroff(COLOR_PAIR(1));

	create_box(&game->middle_line,TRUE); //Draws middle line
	create_box(&game->outer_line,TRUE);
	create_box(&game->lstick,TRUE); //Draws sticks
	create_box(&game->rstick,TRUE);
	create_box(&game->ball,TRUE); //Draws ball
}

void clear_shapes(PONG_GAME *game){
	create_box(&game->lstick,FALSE); //Clears sticks from the screen
	create_box(&game->rstick,FALSE);
	create_box(&game->ball,FALSE);
	create_box(&game->outer_line,FALSE);
}

void update_position(PONG_GAME *game,int ch){
	switch(ch){ //Updates sticks position
		case (int)'w': //W
			game->lstick.starty-=3; //Moves left stick up
			break;
		case (int)'s': //S
			game->lstick.starty+=3;
			break;
		case KEY_UP: //Arrow key up
			game->rstick.starty-=3; //Moves right stick up
			break;
		case KEY_DOWN: //Arrow key down
			game->rstick.starty+=3;
			break;
	}
}

void stop_sticks_from_going_off_screen(PONG_GAME *game){
	if(game->lstick.starty > LINES-STICK_HEIGHT-2){
		game->lstick.starty = LINES-STICK_HEIGHT-2;
	}
	else if(game->lstick.starty < 1){
		game->lstick.starty = 1;
	}

	if(game->rstick.starty > LINES-STICK_HEIGHT-2){
		game->rstick.starty = LINES-STICK_HEIGHT-2;
	}
	else if(game->rstick.starty < 1){
		game->rstick.starty = 1;
	}
}

void update_sticks(PONG_GAME *game,int ch){
	update_position(game,ch);
	stop_sticks_from_going_off_screen(game);
}

void stop_ball_vertical(PONG_GAME *game){
	//Stops ball from going off screen (Vertical)
	if(game->ball.starty > LINES-2){
		game->ball.starty = LINES-2;
		game->ball_velocity_y = -game->ball_velocity_y;
	}
	if(game->ball.starty < 1){
		game->ball.starty = 1;
		game->ball_velocity_y = -game->ball_velocity_y;
	}
}

void stop_ball_horizontal(PONG_GAME *game){
	if(game->ball.startx<-3){
		game->rscore++;
	}
	else if(game->ball.startx>COLS+3){
		game->lscore++;
	}else{
		return;
	}
	if(game->rscore>=5 || game->lscore>=5){
		game->is_done = true;
		return;
	}		
	create_box(&game->ball,FALSE); //undraw ball before moving
	game->ball.startx = COLS/2 - game->ball.width/2;
	srand(time(0));
	game->ball.starty = LINES/2 + LINES*.2*pow(-1,rand());
	game->ball_velocity_y = BALL_START_SPEED_Y * pow(-1,rand());
	draw_screen(game);
	refresh();
	sleep(1);
}

void bounce_ball_off_stick(PONG_GAME *game){
	//Bounce ball off of sticks
	if((fabs((game->lstick.startx+STICK_WIDTH-1)-game->ball.startx)<3 //Check if in the same x plane with lstick
	&& game->lstick.starty<game->ball.starty && (game->lstick.starty+STICK_HEIGHT)>game->ball.starty) //Check if in the same y plane with rstick
	|| (fabs((game->rstick.startx-1)-game->ball.startx )<3 //Same thing but as above but with rstick
	&& game->rstick.starty<game->ball.starty && (game->rstick.starty+STICK_HEIGHT)>game->ball.starty))
	{
		//See which half we are on, just to make sure we don't go the wrong way if we collide with the ball multiple times
		int temp = (game->ball.startx>COLS/2) ? 1 : -1;
		game->ball_velocity_x = temp * fabs(game->ball_velocity_x); 
		game->ball_velocity_y *= 1.1;
		if( fabs(game->ball_velocity_y) > BALL_START_SPEED_Y*3 ){ //Make sure we Don't go faster than 3 * Start speed in the y direction
 			//Set gameball_velocity_y to BALL_START_SPEED_Y, but the same direction/sign
			game->ball_velocity_y = ( (game->ball_velocity_y>0) - (game->ball_velocity_y<0) ) ? -BALL_START_SPEED_Y : BALL_START_SPEED_Y;
		}
		game->bounces++;
	}		
}

void update_ball(PONG_GAME *game){
	//Calculates balls new position
	game->ball.startx -= game->ball_velocity_x;
	game->ball.starty -= game->ball_velocity_y;

	stop_ball_vertical(game);
	bounce_ball_off_stick(game);
	stop_ball_horizontal(game);
}
