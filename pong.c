#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define left_right_border_ch	'|'
#define top_bottom_border_ch	'-'
#define corners_ch		'+'

typedef struct{
	double startx, starty;
	unsigned short height, width;
	unsigned short colorp;
}WIN;

typedef struct{
	float ball_velocity_x, ball_velocity_y;
	unsigned short rscore;
	unsigned short lscore;
	unsigned short bounces;
	bool is_done;
	WIN lstick;
	WIN rstick;
	WIN ball;
	WIN middle_line;
	WIN outer_line;
}PONG_GAME;

void init_ncurses();
void get_arguments();
void print_intro();
void init_game(PONG_GAME *game);
void create_box(WIN *p_win,bool FLAG);
void draw_screen(PONG_GAME *game);
void update_sticks(PONG_GAME *game,int ch);
void update_ball(PONG_GAME *game);
void end_message(int lscore,int rscore,int bounces);

int STICK_HEIGHT = 12;
int STICK_WIDTH = 1;
int BALL_WIDTH = 3;
int BALL_HEIGHT = 1;

int timer; //Used to make game go faster over time

const int MAX_BOUNCE_ANGLE = 75;
const double BALL_START_SPEED_X = .5;
const double BALL_START_SPEED_Y = .15;
bool PRACTICE_MODE = false;

int main(int argc, char **argv)
{
	init_ncurses();
	get_arguments(argc,argv);
	print_intro();

	PONG_GAME game;
	init_game(&game);

	int ch;
	timer = 0;
	while((ch = getch()) != 27 && !game.is_done)
	{
		if(ch != (int)'p'){
			create_box(&game.ball,FALSE); //Ball is the only thing cleared and drawn every frame
			update_ball(&game);
			update_sticks(&game,ch);
			draw_screen(&game); //Draw Ball, Middle Line, and Text
			usleep(8000-timer); //Sleeps for less time until score
			if(timer<5000){
				timer+=1;
			}
		}else{ //Pause screen
			while((ch = getch()) != (int)'p' && !game.is_done){
				if(ch == 27){
					game.is_done = true;
				}
				usleep(10000);
			}
		}
	}
	end_message(game.lscore,game.rscore,game.bounces);
	return 0;
}

void fill_area(int x, int y, int w, int h){
	for(int j = y; j <= y + h; ++j)
		for(int i = x; i <= x + w; ++i)
			mvaddch(j, i, ' ');
}

void fill_win(WIN *p_win)
{
	attron(COLOR_PAIR(p_win->colorp));
	fill_area(p_win->startx+1,p_win->starty+1,p_win->width-2,p_win->height-2);
	attroff(COLOR_PAIR(p_win->colorp));
}

void clear_area(int x,int y, int w, int h)
{
	attron(COLOR_PAIR(5));
	fill_area(x,y,w,h);
	attroff(COLOR_PAIR(5));
}

void get_arguments(int argc,char *argv[]){
	//Color pair 1 is text
	//Color pair 2 is ball
	//Color pair 3 sticks
	//Color pair 4 middle and outer lines
	//Color pair 5 is for clearing screen 
	//-p makes right stick take the entire screen and redirects inputs from key up/down to left player
	int c;
	bool colors_set = false;
	while((c = getopt(argc,argv,"p123456")) != -1){
		switch(c){
			case 'p':
				PRACTICE_MODE = true;
				break;
			case '2':
				init_pair(1, COLOR_CYAN, COLOR_BLACK);
				init_pair(2, COLOR_CYAN, COLOR_BLACK);
				init_pair(3, COLOR_BLUE, COLOR_BLACK);
				init_pair(4, COLOR_RED, COLOR_BLACK);
				init_pair(5, COLOR_RED, COLOR_BLACK);
				BALL_HEIGHT = 2;
				colors_set = true;
				break;
			case '3':
				init_pair(1, COLOR_WHITE, COLOR_GREEN);
				init_pair(2, COLOR_WHITE, COLOR_WHITE);
				init_pair(3, COLOR_WHITE, COLOR_WHITE);
				init_pair(4, COLOR_WHITE, COLOR_WHITE);
				init_pair(5, COLOR_WHITE, COLOR_GREEN);
				colors_set = true;
				break;
			case '4':
				init_pair(1, COLOR_WHITE, COLOR_BLACK);	
				init_pair(2, COLOR_WHITE, COLOR_WHITE);
				init_pair(3, COLOR_WHITE, COLOR_WHITE);
				init_pair(4, COLOR_WHITE, COLOR_WHITE);
				init_pair(5, COLOR_WHITE, COLOR_BLACK);
				colors_set = true;
				break;
			case '5':
				init_pair(1, COLOR_BLACK, COLOR_WHITE);	
				init_pair(2, COLOR_RED, COLOR_RED);
				init_pair(3, COLOR_RED, COLOR_RED);
				init_pair(4, COLOR_RED, COLOR_RED);
				init_pair(5, COLOR_WHITE, COLOR_WHITE); 
				BALL_WIDTH = 2;
				BALL_HEIGHT = 1;
				colors_set = true;
				break;
			case '6': //More for testing edge cases than anything
				init_pair(1, COLOR_BLACK, COLOR_YELLOW);
				init_pair(2, COLOR_BLACK, COLOR_RED);
				init_pair(3, COLOR_BLACK, COLOR_YELLOW);
				init_pair(4, COLOR_BLACK, COLOR_MAGENTA);
				init_pair(5, COLOR_BLACK, COLOR_BLACK); 
				BALL_WIDTH = 4;
				BALL_HEIGHT = 4;
				colors_set = true;
				break;
		}
	}
	if(!colors_set){
		init_pair(1, COLOR_WHITE, COLOR_BLUE);
		init_pair(2, COLOR_WHITE, COLOR_WHITE);
		init_pair(3, COLOR_WHITE, COLOR_WHITE);
		init_pair(4, COLOR_WHITE, COLOR_WHITE);
		init_pair(5, COLOR_WHITE, COLOR_BLUE);
	}
}

void init_ncurses(){
	initscr();
	start_color();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
}

void print_intro(){
	clear_area(0,0,COLS,LINES);
	attron(COLOR_PAIR(1));

	(!PRACTICE_MODE) ? mvprintw(0,0,"(W/S for left player paddle, key up/down for right player paddle, p for pause)")
	: mvprintw(0,0,"(W/S or key up/down to control paddle, p for pause)");
	
	mvprintw(1,0,"Press any key to start");
	mvprintw(2,0,"By Skyler Ferrante");

	mvprintw(LINES/2,COLS/2-27.5,"88888888ba     ,ad8888ba,    888b      88    ,ad8888ba,");
	mvprintw(LINES/2+1,COLS/2-27.5,"88      \"8b   d8\"'    `\"8b   8888b     88   d8\"'    `\"8b");
	mvprintw(LINES/2+2,COLS/2-27.5,"88      ,8P  d8'        `8b  88 `8b    88  d8'          ");
	mvprintw(LINES/2+3,COLS/2-27.5,"88aaaaaa8P'  88          88  88  `8b   88  88            ");
	mvprintw(LINES/2+4,COLS/2-27.5,"88\"\"\"\"\"\"\"    88          88  88   `8b  88  88      88888");
	mvprintw(LINES/2+5,COLS/2-27.5,"88           Y8,        ,8P  88    `8b 88  Y8,        88 ");
	mvprintw(LINES/2+6,COLS/2-27.5,"88            Y8a.    .a8P   88     `8888   Y8a.    .a88 ");
	mvprintw(LINES/2+7,COLS/2-27.5,"88             `\"Y8888Y\"'    88      `888    `\"Y88888P\"");
	attroff(COLOR_PAIR(1));

	getch();
	clear();
	nodelay(stdscr,TRUE); //I wanted to put this in init_ncurses but then we could not pause at the above getch
}

void end_message(int lscore,int rscore,int bounces){
	endwin();
	if(PRACTICE_MODE){
		printf("Practice mode\n");
	}
	else if(lscore==rscore){
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
	p_win->width =  STICK_WIDTH;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width);
	p_win->colorp = 3;
}

void init_ball(WIN *p_win)
{
	p_win->height = BALL_HEIGHT;
	p_win->width = BALL_WIDTH;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 2;
}

void init_middle_line(WIN *p_win)
{
	p_win->height = LINES+2;
	p_win->width = 2;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 4;
}

void init_outer_line(WIN *p_win)
{
	p_win->height = LINES-1;
	p_win->width = COLS-1;
	p_win->starty = (LINES - p_win->height)/2;
	p_win->startx = (COLS - p_win->width)/2;
	p_win->colorp = 4;
}

void create_box(WIN *p_win, bool flag)
{
	double x,y,w,h;
	x = p_win->startx;
	y = p_win->starty;
	w = p_win->width;
	h = p_win->height;

	//If flag is true, print p_win, else clear p_win
	if(flag == TRUE)
	{
		attron(COLOR_PAIR(p_win->colorp));
		mvaddch(y, x, corners_ch);
		mvaddch(y, x + w, corners_ch);
		mvaddch(y + h, x, corners_ch);
		mvaddch(y + h, x + w, corners_ch);
		mvhline(y, x + 1, top_bottom_border_ch, w - 1);
		mvhline(y + h, x + 1, top_bottom_border_ch, w - 1);
		mvvline(y + 1, x, left_right_border_ch, h - 1);
		mvvline(y + 1, x + w, left_right_border_ch, h - 1);
		attroff(COLOR_PAIR(p_win->colorp));
	}else{
		clear_area(x,y,w,h);
	}
	//Getch acts as a refresh, so we do not refresh here to prevent flickering
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

	//Draws the full background once, instead of every frame 
	create_box(&game->outer_line,FALSE);
	create_box(&game->lstick,TRUE);
	create_box(&game->rstick,TRUE);
	//Draws the outerlines
	create_box(&game->outer_line,TRUE);
}

void draw_screen(PONG_GAME *game){
	//Draw Text
	attron(COLOR_PAIR(1));
	mvprintw(1,2,"Press ESC to exit");
	mvprintw(2,2,"%i",game->lscore);
	mvprintw(2,COLS-2,"%i",game->rscore);
	attroff(COLOR_PAIR(1));

	if(abs(game->ball.startx-game->middle_line.startx) < 5){ //If ball near middle line
		create_box(&game->middle_line,TRUE); //Draws middle line
	}
	create_box(&game->ball,TRUE); //Draws ball
}

void set_stick_pos(WIN *stick,int new_height){
	stick->starty = new_height;
	create_box(stick,TRUE);
}

void stop_sticks_from_going_off_screen(PONG_GAME *game){
	//LEFT
	if(game->lstick.starty > LINES-STICK_HEIGHT-3){
		set_stick_pos(&game->lstick,LINES-STICK_HEIGHT-3);
	}
	else if(game->lstick.starty < 2){
		set_stick_pos(&game->lstick,2);
	}

	//RIGHT
	if(game->rstick.starty > LINES-STICK_HEIGHT-3){
		set_stick_pos(&game->rstick,LINES-STICK_HEIGHT-3);
	}
	else if(game->rstick.starty < 2){
		set_stick_pos(&game->rstick,2);
	}
}

void update_sticks_position(PONG_GAME *game,int ch){
	int change_amount_lstick = 0;
	int change_amount_rstick = 0;

	switch(ch){ //Updates sticks position
		case (int)'w': //W		
			change_amount_lstick-=3; //Moves left stick up	
			break;
		case (int)'s': //S
			change_amount_lstick+=3;
			break;
		case KEY_UP: //Arrow key up
			if(!PRACTICE_MODE){
				change_amount_rstick-=3; //Moves right stick up
			}else{
				change_amount_lstick-=3; //Moves left stick up	
			}
			break;
		case KEY_DOWN: //Arrow key down
			if(!PRACTICE_MODE){
				change_amount_rstick+=3; //Moves right stick up
			}else{
				change_amount_lstick+=3; //Moves left stick up	
			}
			break;
	}
	
	if(PRACTICE_MODE){
		if(game->ball.starty-STICK_WIDTH/2-BALL_WIDTH/2 > game->rstick.starty){
			change_amount_rstick += BALL_WIDTH/2;
		}
		else if(game->ball.starty-STICK_WIDTH/2+BALL_WIDTH/2 < game->rstick.starty){
			change_amount_rstick -= BALL_WIDTH/2;
		}
	}
	
	if(change_amount_lstick != 0){
		create_box(&game->lstick,FALSE);
		game->lstick.starty += change_amount_lstick;
		stop_sticks_from_going_off_screen(game);
		create_box(&game->lstick,TRUE);
	}
	
	if(change_amount_rstick != 0){
		create_box(&game->rstick,FALSE);
		game->rstick.starty += change_amount_rstick;
		stop_sticks_from_going_off_screen(game);
		create_box(&game->rstick,TRUE);
	}
}

void update_sticks(PONG_GAME *game,int ch){
	update_sticks_position(game,ch);
}

void stop_ball_vertical(PONG_GAME *game){
	//Stops ball from going off screen (Vertical)
	if(game->ball.starty > LINES-BALL_HEIGHT-1){ //Bottom
		game->ball.starty = LINES-BALL_HEIGHT-2;
		game->ball_velocity_y = -game->ball_velocity_y;
	}
	if(game->ball.starty < 1){ //Top
		game->ball.starty = 2;
		game->ball_velocity_y = -game->ball_velocity_y;
	}
}

void hit_horizontal_wall(PONG_GAME *game){
	create_box(&game->ball,FALSE); //undraw ball before moving
	game->ball.startx = COLS/2 - BALL_WIDTH/2;
	srand(time(0));
	game->ball.starty = LINES/2 + LINES*.2*pow(-1,rand());
	game->ball_velocity_y = BALL_START_SPEED_Y * pow(-1,rand());
	draw_screen(game);
	refresh(); //We don't want to see the ball while sleeping
	sleep(1);
}

void stop_ball_horizontal(PONG_GAME *game){
	if(game->ball.startx < 2){
		game->rscore++;
		hit_horizontal_wall(game);
	}
	else if(game->ball.startx > COLS-BALL_WIDTH-2){
		game->lscore++;
		hit_horizontal_wall(game);
	}else{
		return;
	}
	if(game->rscore>=5 || game->lscore>=5){
		game->is_done = true;
		return;
	}
	timer = 0;
}

bool checkIfBallTouchingLeftStick(PONG_GAME *game){
	return fabs(game->lstick.startx - game->ball.startx) < BALL_WIDTH + STICK_WIDTH - 1 //Check if in the same x plane with ball
	&&     fabs( (game->lstick.starty+(STICK_HEIGHT/2)) - game->ball.starty)-BALL_HEIGHT < STICK_HEIGHT/2;
}

bool checkIfBallTouchingRightStick(PONG_GAME *game){
	return fabs(game->rstick.startx - game->ball.startx) < BALL_WIDTH + STICK_WIDTH + 2 //Check if in the same x plane with ball
	&&     fabs( (game->rstick.starty+(STICK_HEIGHT/2)) - game->ball.starty)-BALL_HEIGHT < STICK_HEIGHT/2;
}

void bounce_helper(PONG_GAME *game,bool onright){
		int temp = (onright) ? -1 : 1;
		create_box(&game->ball,FALSE); //Make sure we don't get artifacts off the ball
		
		game->ball.startx += temp;
		game->ball_velocity_x = -BALL_START_SPEED_X*temp;

		(onright) ? create_box(&game->rstick,TRUE) : create_box(&game->lstick,TRUE);
		game->bounces++;
}

double find_multiplier(double stick_starty,double ball_starty,int stick_height){
		double relativeIntersectY = (stick_starty+(stick_height/2)) - ball_starty;
		double normalized = relativeIntersectY/(stick_height/2);
		return normalized;
}	

void bounce_ball_off_stick(PONG_GAME *game){
	if(checkIfBallTouchingLeftStick(game)){
		game->ball_velocity_y = find_multiplier(game->lstick.starty,game->ball.starty,STICK_HEIGHT)*.2;
		bounce_helper(game,false);
	}else if(checkIfBallTouchingRightStick(game)){
		game->ball_velocity_y = find_multiplier(game->rstick.starty,game->ball.starty,STICK_HEIGHT)*.2;
		bounce_helper(game,true);
	}
}

void bounce_ball_off_stickDiff(PONG_GAME *game){
	if(checkIfBallTouchingLeftStick(game)	||
	   checkIfBallTouchingRightStick(game))
	{
		if( fabs(game->ball_velocity_y) > BALL_START_SPEED_Y*3 ){ //Make sure we Don't go faster than 3 * Start speed in the y direction
 			//Set gameball_velocity_y to BALL_START_SPEED_Y, but the same direction/sign
			game->ball_velocity_y = ( game->ball_velocity_y < 0 ) ? -BALL_START_SPEED_Y : BALL_START_SPEED_Y;
		}
		if(game->ball.startx < game->lstick.startx){ 
		}else if(game->ball.startx > game->rstick.startx){
		}
		//See which half we are on, just to make sure we don't go the wrong way
		bool on_right = (game->ball.startx>COLS/2);
		int temp = (on_right) ? 1 : -1;
		game->ball_velocity_x = temp * fabs(game->ball_velocity_x);
		game->ball_velocity_y *= 1.1;
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
