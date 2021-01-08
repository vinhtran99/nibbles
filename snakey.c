#include <stdio.h>
#include <stdlib.h>	// rand(), dynamic memory
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#define YMax 24
#define XMax 80
#define ESC 0x1B
enum BGCOLORS {BLACK = 40, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

// screen procedures, only applied to Linux
/* move cursor to given location */
void gotoxy(int row, int col) {
	printf("%c[%d;%df", ESC, row, col);
	fflush(stdin);
};
/* set color to the current position */
void setBackgroundColor(int color) {
	printf("%c[1;%dm", ESC, color);
	fflush(stdout);
}

// keyboard processes
/* detect key press */
int kbhit(void) {	// copy from stackoverflow
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
};
/* */
int getch(void) {	// copy from ubuntuforums
	int ch;
	struct termios oldt;
	struct termios newt;
	tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
	newt = oldt; /* copy old settings to new settings */
	newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
	ch = getchar(); /* standard getchar call */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
	return ch; /*return received char */
};
/* coordinate type */
struct xy {
	int row, col;
};

// snake data
struct node {
	struct node* next;	// point to next segment
	int row;
	int col;
};
/* queue */
struct snake {
	struct node* Head;
	struct node* Tail;
	int direction;
	int length;
};
static struct snake Slither;
/* enqueue */
void addHead(int row, int col) {
	struct node* cell = (struct node*) malloc(sizeof(struct node));
	cell->row = row;
	cell->col = col;
	cell->next = NULL;
	Slither.Head->next = cell;
	Slither.Head = cell;
};
/* dequeue */
void cutTail(void) {
	struct node* cell = Slither.Tail;
	Slither.Tail = Slither.Tail->next;
	free(cell);
};

// screen data 24 lines 80 columns
static int Playfield[YMax][XMax];


// end game situations
int hitWall(void) {
	if ((Slither.Head->row == 1) || (Slither.Head->row == YMax) || (Slither.Head->col == 1) || (Slither.Head->col == XMax))
		return 1;
	return 0;
}
int hitBody(int row, int col) {
	if (Playfield[row-1][col-1] == 1)
		return 1;
	return 0;
};
	
int main() {
	// initialize snake as a dot at (1, 1) moving to the right
	struct node* Seed = (struct node*) malloc(sizeof(struct node));
	Seed->next = NULL;
	Seed->row = Seed->col = 2;
	Slither.Head = Slither.Tail = Seed;
	Slither.direction = 4;
	Slither.length = 1;
	Playfield[1][1] = 1;

	// draw wall
	system("clear");	// clear screen, only applied to Linux
	for (int i=2; i<YMax; ++i) {
		for (int j=2; j<XMax; ++j) {
			gotoxy(i, j);
			setBackgroundColor(GREEN);
			printf(" ");
		}
	}
	for (int i = 0; i < YMax; ++i) {
		gotoxy(i+1, 1); 
		setBackgroundColor(BLUE);
		printf(" ");
		gotoxy(i+1, XMax); 
		setBackgroundColor(BLUE);
		printf(" ");
	}
	for (int i = 0; i < XMax; ++i) {
		gotoxy(1, i+1); 
		setBackgroundColor(BLUE);
		printf(" ");
		gotoxy(YMax, i+1); 
		setBackgroundColor(BLUE);
		printf(" ");
	}
	gotoxy(2, 2);
	setBackgroundColor(YELLOW);
	printf(" ");

	struct xy Food;
	int foodRemain = 0;
	int score = 0;
	int curLen = 1;
	int c1, c2, c3;
	gotoxy(25, 1);
	printf("Score: %d", score);

	// main routine: play until quit
	while (1) {
		// check if food is present, else make it
		if (!foodRemain) {
			do {
				Food.row = rand() % (YMax - 3) + 2;
				Food.col = rand() % (XMax - 3) + 2;
			} while (hitBody(Food.row, Food.col));
				foodRemain = 1;
			gotoxy(Food.row, Food.col); 
			setBackgroundColor(RED);	// food appear as red dot
			printf(" ");
		}

		// delay 500ms, only applied to Linux
		if (Slither.direction == 1 || Slither.direction == 3)
			system("sleep 0.2");
		else 
			system("sleep 0.1");
		
		// check key pressed and plays
		if (kbhit()) {
			c1 = getch();
			if ((c1 == 27)) {
				c2 = getch();
				c3 = getch();
			}
			if (c1 == 81 || c1 == 113) break;	// 'Q' or 'q'	
			if (c1 == 27 && c2 == 91) {
				if (c3 == 65) {	// 'A' or up
					if (Slither.direction != 3) Slither.direction = 1;
				}
				else if (c3 == 68) {	// 'D' or left
					if (Slither.direction != 4) Slither.direction = 2;
				}
				else if (c3 == 66) {	// 'B' or down
					if (Slither.direction != 1) Slither.direction = 3;
				}
				else if (c3 == 67) {	// 'C' or right
					if (Slither.direction != 2) Slither.direction = 4;
				}
			}
		}

		// moves snake
		switch (Slither.direction) {
		case 1:
			addHead(Slither.Head->row - 1, Slither.Head->col);
			break;
		case 3:
			addHead(Slither.Head->row + 1, Slither.Head->col);
			break;
		case 2:
			addHead(Slither.Head->row, Slither.Head->col - 1);
			break;
		case 4:
			addHead(Slither.Head->row, Slither.Head->col + 1);
			break;
		}

		// if snake grabs food
		if ((Food.row == Slither.Head->row) && (Food.col == Slither.Head->col)) {
			foodRemain = 0;
			score += 10;
			curLen += 4;
			gotoxy(25, 1);
			printf("Score: %d", score);
		}

		// if snake hit wall or itself
		if (hitWall() || hitBody(Slither.Head->row, Slither.Head->col)) {
			gotoxy(Slither.Head->row, Slither.Head->col);
			break;
		}
		else { // else move snake
			gotoxy(Slither.Head->row, Slither.Head->col); 
			setBackgroundColor(YELLOW);
			printf(" ");
			Playfield[Slither.Head->row - 1][Slither.Head->col - 1] = 1;

			// erase tail
			if (Slither.length == curLen) {
				gotoxy(Slither.Tail->row, Slither.Tail->col); 
				setBackgroundColor(GREEN);
				printf(" ");
				Playfield[Slither.Tail ->row - 1][Slither.Tail->col - 1] = 0;
				cutTail();
			}
			else Slither.length++;
		}
		
	}
	system("clear");
	printf("You died. Press any key to exit...");
	getch();
	return 0;
}